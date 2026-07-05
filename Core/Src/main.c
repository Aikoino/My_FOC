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
static uint8_t key_state = 0;

/* 母线电压 */
static float vbus_voltage = 0.0f;

/* 测试步骤 */
static uint8_t test_step = 0;
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
  MX_TIM1_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();
  MX_FDCAN1_Init();
  MX_OPAMP1_Init();
  MX_OPAMP2_Init();
  MX_OPAMP3_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

  /* === BSP Init === */
  /* LED ON during initialization */
  HAL_GPIO_WritePin(GPIOC, LED3_Pin, GPIO_PIN_RESET);

  BSP_ADC_Init();
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);

  BSP_UART_VOFA_Init();
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);

  BSP_Button_Init();
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);

  BSP_Motor_Init();
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);

  /* CAN Init - 先用NoIRQ版本测试 HAL_FDCAN_Start 是否正常 */
  CAN_Init_NoIRQ();
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);

  MiniFOC_Init();
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);

  /* Start TIM1 PWM */
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);

  /* Start ADC for Vbus */
  HAL_ADC_Start(&hadc1);
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);

  /* All init complete - LED off */
  HAL_GPIO_WritePin(GPIOC, LED3_Pin, GPIO_PIN_SET);

  /* USER CODE END 2 */

  /* === Main Loop === */
  while (1)
  {
    sys_tick_ms = HAL_GetTick();

    /* Key scan (10ms) */
    if (sys_tick_ms - key_timer >= 10) {
        key_timer = sys_tick_ms;
        Key_Scan();
    }

    /* LED3 heartbeat (1Hz, 500ms on/off) */
    {
        static uint32_t led_timer = 0;
        if (sys_tick_ms - led_timer >= 500) {
            led_timer = sys_tick_ms;
            HAL_GPIO_TogglePin(GPIOC, LED3_Pin);
        }
    }

    /* MiniFOC main loop (1kHz) */
    MiniFOC_MainLoop();

    /* ADC software trigger (注入通道) */
    BSP_ADC_SoftwareTrigger();

    /* VOFA+ send (500ms) */
    if (sys_tick_ms - last_test_ms >= 500) {
        last_test_ms = sys_tick_ms;
        Vbus_Adc_Update();
        float iu = BSP_ADC_GetCurrentU();
        float iv = BSP_ADC_GetCurrentV();
        float iw = BSP_ADC_GetCurrentW();

        BSP_UART_VOFA_SendFloats(iu, iv, iw, vbus_voltage);
    }

    /* KEY toggle motor */
    if (key_state) {
        key_state = 0;
        if (foc.motor_running) {
            /* 停止：关电机，PWM归零 */
            MiniFOC_MotorEnable(false);
        } else {
            /* 启动：开环电流模式，目标电流 0.5A */
            MiniFOC_SetTargetCurrent(0.5f);
            MiniFOC_MotorEnable(true);
        }
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
                /* CAN启动电机 - 开环电流模式 */
                MiniFOC_SetTargetCurrent(1.0f);
                MiniFOC_MotorEnable(true);
            } else if (can_rx_data[0] == 0x00) {
                /* CAN停止电机 */
                MiniFOC_MotorEnable(false);
            }
        } else if (can_rx_id == 0x101 && can_rx_len >= 2) {
            /* CAN设定转速 */
            uint16_t speed = (can_rx_data[1] << 8) | can_rx_data[0];
            MiniFOC_SetTargetSpeed((float)speed);
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
