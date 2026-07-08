/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "fdcan.h"
#include "opamp.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "bsp_adc.h"
#include "bsp_uart_vofta.h"
#include "bsp_can.h"
#include "bsp_motor.h"
#include "bsp_hall.h"
#include "MiniFOC/MiniFOC.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TEST_INTERVAL_MS  500  /* 发�?�间�?? 500ms (便于观察LED闪烁) */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* 注意：所有外设句柄（hadc1, hadc2, huart3, htim1, hfdcan1, hopamp1/2/3�?
   都在对应�? peripheral 文件中定义（adc.c, usart.c, tim.c, fdcan.c, opamp.c�?
   不要在这里重复定义，否则会产�? "multiply defined" 链接错误�?
*/
static uint32_t sys_tick_ms = 0;
static uint32_t last_test_ms = 0;
static uint32_t last_can_ms = 0;
static uint8_t can_tx_id = 0;
static char print_buf[256];

/* 按键扫描相关 */
static uint32_t key_timer = 0;
static uint8_t key_state = 0;  /* 0=未按下, 1=已按下(等待释放) */

/* 母线电压 */
static float vbus_voltage = 0.0f;

/* 测试步骤 */
static uint8_t test_step = 0;

/* 控制模式选择（0=VF开环, 1=双闭环, 2=霍尔电流环, 3=霍尔速度环）
 * 按按键循环切换：0→1→2→3→0
 * - mode 0: VF压频比开环（800rpm，开环角度，适合快速测试）
 * - mode 1: 速度-电流双闭环（800rpm，VF开环角度+速度环，无传感器）
 * - mode 2: 霍尔电流环（5A，霍尔传感器角度，电流闭环）
 * - mode 3: 霍尔速度环（800rpm，霍尔传感器角度+速度环）
 * 反转可通过 CAN 指令发送负转速实现
 */
static uint8_t use_hall_mode = 0;  /* 默认使用 VF 开环 */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void Vbus_Adc_Update(void);
uint8_t CAN_Send(uint32_t id, uint8_t *data, uint8_t len);
uint8_t CAN_Receive(uint32_t *id, uint8_t *data, uint8_t *len);
static void Key_Scan(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* 注意：所有外设句柄（hadc1, hadc2, huart3, htim1, hfdcan1, hopamp1/2/3）
     都在对应的 peripheral 文件中定义（adc.c, usart.c, tim.c, fdcan.c, opamp.c）
     不要在这里重复定义，否则会产生 "multiply defined" 链接错误。
  */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART3_UART_Init();
  MX_TIM1_Init();
  MX_TIM4_Init();  /* TIM4霍尔传感器捕获 */
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_OPAMP1_Init();
  MX_OPAMP2_Init();
  MX_OPAMP3_Init();
  MX_FDCAN1_Init();
  /* USER CODE BEGIN 2 */
  HAL_GPIO_WritePin(GPIOC, LED2_Pin|LED3_Pin, GPIO_PIN_RESET);

  /*  MiniFOC初始化（必须最先调用）*/
  MiniFOC_Init();

  BSP_Button_Init();

  /* 初始化霍尔传感器（读 GPIO 获取初始角度）*/
  BSP_Hall_Init();
  BSP_Hall_ResetTracking();

  BSP_Motor_Init();
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);

  /* CAN Init */
  CAN_Init_NoIRQ();
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);

  /* ADC 最后初始化（必须在PWM启动前完成电流采样校准）*/
  BSP_ADC_Init();
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);

  /* ✅ 安全启动：PWM通道延迟启动（参考F盘）
   * 上电时只启动定时器和CH4（ADC触发用）
   * CH1/2/3（功率输出）在按键按下时才启动
   */
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);  /* 只启动CH4，用于ADC触发 */

  /* LED2 OFF = 电机停止（上电默认停止）*/
  HAL_GPIO_WritePin(GPIOC, LED2_Pin, GPIO_PIN_SET);

  /* Start TIM4 Hall Sensor */
  HAL_TIMEx_HallSensor_Start_IT(&htim4);
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);

  /* Start ADC for Vbus */
  HAL_ADC_Start(&hadc1);
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);

  /* All init complete - LEDs off */
  HAL_GPIO_WritePin(GPIOC, LED3_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOC, LED2_Pin, GPIO_PIN_SET);   /* LED2 OFF = 电机停止 */

  /* USER CODE END 2 */

  /* === Main Loop === */
  while (1)
  {
    sys_tick_ms = HAL_GetTick();

    /* Key scan (10ms) - 按键控制电机启停，LED2 指示状态 */
    if (sys_tick_ms - key_timer >= 10) {
        key_timer = sys_tick_ms;

        /* 按键消抖：连续 3 次检测都按下才认为有效 */
        static uint8_t key_cnt = 0;
        if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
            key_cnt++;
            if (key_cnt >= 3) {  /* 30ms 消抖完成 */
                if (key_state == 0) {  /* 上升沿触发 */
                    key_state = 1;
                    if (foc.motor_running) {
                        /* 停止电机 */
                        MiniFOC_MotorEnable(false);
                        HAL_GPIO_WritePin(GPIOC, LED2_Pin, GPIO_PIN_SET);   /* LED2 OFF = 电机停止 */
                    } else {
                        /* 启动电机（根据模式选择）*/
                        if (use_hall_mode == 0) {
                            /* VF 开环模式（800rpm，开环角度）*/
                            MiniFOC_SetMode(MODE_VF_OPENLOOP);
                            MiniFOC_SetTargetSpeed(800.0f);
                        } else if (use_hall_mode == 1) {
                            /* 速度-电流双闭环模式（800rpm，VF开环角度+速度环）*/
                            MiniFOC_SetMode(MODE_VelCur_DOUBLE);
                            MiniFOC_SetTargetSpeed(800.0f);
                        } else if (use_hall_mode == 2) {
                            /* 霍尔电流环模式（5A，霍尔角度，电流闭环）*/
                            MiniFOC_SetMode(MODE_Sensor_Hall_I);
                            MiniFOC_SetTargetCurrent(5.0f);
                        } else if (use_hall_mode == 3) {
                            /* 霍尔速度环模式（800rpm，霍尔角度+速度环）*/
                            MiniFOC_SetMode(MODE_Sensor_Hall_S);
                            MiniFOC_SetTargetSpeed(800.0f);
                        }
                        foc.bus_voltage = vbus_voltage;  /* 使用实际母线电压 */
                        MiniFOC_MotorEnable(true);
                        HAL_GPIO_WritePin(GPIOC, LED2_Pin, GPIO_PIN_RESET); /* LED2 ON = 电机运行 */
                    }

                    /* 切换控制模式（仅在停止状态下）*/
                    if (!foc.motor_running) {
                        use_hall_mode = (use_hall_mode + 1) % 4;  /* 0→1→2→3→0循环 */
                    }
                }
            }
        } else {
            key_cnt = 0;
            key_state = 0;
        }
    }

    /* LED3 heartbeat (1Hz, 500ms on/off) */
    {
        static uint32_t led_timer = 0;
        if (sys_tick_ms - led_timer >= 500) {
            led_timer = sys_tick_ms;
            HAL_GPIO_TogglePin(GPIOC, LED3_Pin);
        }
    }

    /* 更新母线电压（每轮循环都更新）*/
    Vbus_Adc_Update();

    /* MiniFOC main loop (1kHz) */
    MiniFOC_MainLoop();

    /* VOFA+ send (2ms, 500Hz) - 6通道: Ia, Ib, Ic, duty_a, duty_b, rotor_speed */
    if (sys_tick_ms - last_test_ms >= 2) {
        last_test_ms = sys_tick_ms;
        Vbus_Adc_Update();

        /* 三相电流 (A) */
        float ia_val = foc.phase_current_u - foc.current_offset_u;
        float ib_val = foc.phase_current_v - foc.current_offset_v;
        float ic_val = foc.phase_current_w - foc.current_offset_w;

        /* 占空比 (0-8000) */
        float duty_a_val = foc.duty_a;
        float duty_b_val = foc.duty_b;
        float duty_c_val = foc.duty_c;

        /* 串口打印（500ms一次）*/
        static uint32_t last_print_ms = 0;
        if (sys_tick_ms - last_print_ms >= 500) {
            last_print_ms = sys_tick_ms;
            extern UART_HandleTypeDef huart3;
            char buf[256];
            int len = sprintf(buf, "[VOFA] Ia=%.2f Ib=%.2f Ic=%.2f dutyA=%.0f dutyB=%.0f dutyC=%.0f\r\n",
                             ia_val, ib_val, ic_val, duty_a_val, duty_b_val, duty_c_val);
            HAL_UART_Transmit(&huart3, (uint8_t*)buf, len, 10);
        }

        /*  VOFA+发送三相电流 + 三相占空比（6个float）*/
        BSP_UART_VOFA_SendFloats(ia_val, ib_val, ic_val, duty_a_val, duty_b_val, duty_c_val);
    }

    /* CAN send test (500ms) */
    if (sys_tick_ms - last_can_ms >= 500) {
        last_can_ms = sys_tick_ms;
        uint8_t can_data[8] = {0};
        can_data[0] = can_tx_id;
        can_data[1] = 0x11;
        can_data[2] = 0x22;
        can_data[3] = 0x33;
        can_data[4] = 0x44;
        can_data[5] = 0x55;
        can_data[6] = 0x66;
        can_data[7] = 0x77;

        if (CAN_Send(0x100, can_data, 8)) {
            can_tx_id++;
        }
    }

    /* CAN receive poll */
    do {
        uint32_t can_rx_id = 0;
        uint8_t can_rx_data[8] = {0};
        uint8_t can_rx_len = 0;

        if (!CAN_Receive(&can_rx_id, can_rx_data, &can_rx_len)) break;

        /* Parse commands */
        if (can_rx_id == 0x100) {
            if (can_rx_data[0] == 0x01) {
                /* CAN启动电机 */
                MiniFOC_SetTargetCurrent(1.0f);
                MiniFOC_MotorEnable(true);
            } else if (can_rx_data[0] == 0x00) {
                /* CAN停止电机 */
                MiniFOC_MotorEnable(false);
            }
        } else if (can_rx_id == 0x101 && can_rx_len >= 2) {
            /* CAN设定转速（支持正反转）*/
            int16_t speed = (int16_t)((can_rx_data[1] << 8) | can_rx_data[0]);
            MiniFOC_SetTargetSpeed((float)speed);
            /* 调试输出：打印转速指令 */
            {
                extern UART_HandleTypeDef huart3;
                char buf[128];
                int len = sprintf(buf, "[CAN] Speed cmd: %d rpm\r\n", speed);
                HAL_UART_Transmit(&huart3, (uint8_t*)buf, len, 10);
            }
        } else if (can_rx_id == 0x102 && can_rx_len >= 2) {
            /* CAN 设置霍尔角度校准偏移 (int16 * 0.01 rad) */
            int16_t offset_raw = (int16_t)((can_rx_data[1] << 8) | can_rx_data[0]);
            MiniFOC_SetHallAngleOffset((float)offset_raw * 0.01f);
        } else if (can_rx_id == 0x103) {
            /* CAN 读取当前霍尔偏移 → 回传到 0x104 */
            uint8_t resp[4];
            int16_t cur = (int16_t)(foc.hall_angle_offset / 0.01f);
            resp[0] = (uint8_t)(cur & 0xFF);
            resp[1] = (uint8_t)((cur >> 8) & 0xFF);
            CAN_Send(0x104, resp, 4);
        }
    } while (0);
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
  * @brief  Update bus voltage
  * @retval None
  */
static void Vbus_Adc_Update(void)
{
    /* 读取母线电压 ADC (PA0 -> ADC1_IN1)
     * 计算：Vbus = ADC_Value * (3.3V / 4095) * 分压系数26
     */
    uint16_t adc_val = BSP_ADC_GetVbus();
    vbus_voltage = (float)adc_val * (3.3f / 4095.0f) * 26.0f;
}

/**
  * @brief  Test step indicator
  * @param  step: step number
  * @param  name: step name
  * @retval None
  */
static void Test_Step(uint8_t step, const char *name)
{
    test_step = step;
    /* 可以添加LED指示或其他测试�?�辑 */
}

/**
  * @brief  Button scan
  * @retval None
  */
static void Key_Scan(void)
{
    static uint8_t last_key = 1;
    uint8_t current_key = HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin);

    if (last_key == 1 && current_key == 0) {
        HAL_Delay(10);  /* 消抖 */
        if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == 0) {
            key_state = 1;
        }
    }
    last_key = current_key;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
