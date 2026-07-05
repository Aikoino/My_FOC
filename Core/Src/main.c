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

  /* NO SERIAL DEBUG - keep USART3 clean for VOFA+ JustFloat protocol */

  /* Initialize systems with step-by-step LED indication */
  HAL_GPIO_WritePin(GPIOC, LED3_Pin, GPIO_PIN_RESET);  /* LED3 ON = step start */

  BSP_ADC_Init();
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);  /* blink = step 1 OK */

  BSP_UART_VOFA_Init();
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);  /* blink = step 2 OK */

  BSP_Button_Init();
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);  /* blink = step 3 OK */

  BSP_Motor_Init();
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);  /* blink = step 4 OK */

  /* TEMPORARY: Comment out CAN to test if it's the culprit */
  /* CAN_Init(); */
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);  /* blink = step 5 OK (CAN skipped) */

  /* MiniFOC Init */
  MiniFOC_Init();
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);  /* blink = step 6 OK */

  /* Start TIM1 PWM */
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);  /* blink = step 7 OK */

  /* Start ADC for Vbus sampling */
  HAL_ADC_Start(&hadc1);
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);  /* blink = step 8 OK */

  /* Test: Send one packet to verify VOFA+ */
  float test_vbus = 12.0f;
  BSP_UART_VOFA_SendFloats(0.0f, 0.0f, 0.0f, test_vbus);
  HAL_GPIO_TogglePin(GPIOC, LED3_Pin);  /* blink = step 9 OK */

  /* LED3 OFF = initialization complete */
  HAL_GPIO_WritePin(GPIOC, LED3_Pin, GPIO_PIN_SET);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    sys_tick_ms = HAL_GetTick();

    if (sys_tick_ms - key_timer >= 10) {
        key_timer = sys_tick_ms;
        Key_Scan();
        if (key_state) {
            key_state = 0;
            HAL_GPIO_TogglePin(GPIOC, LED2_Pin);  /* LED2 controlled by button */
        }
    }

    /* MiniFOC main loop (1kHz) */
    MiniFOC_MainLoop();

    /* Software trigger ADC injected conversion (alternative to TIM1 CC4 hardware trigger) */
    BSP_ADC_SoftwareTrigger();

    /* Send data to VOFA+ (JustFloat protocol) */
    if (sys_tick_ms - last_test_ms >= TEST_INTERVAL_MS) {
        last_test_ms = sys_tick_ms;
        Vbus_Adc_Update();
        float iu = BSP_ADC_GetCurrentU();
        float iv = BSP_ADC_GetCurrentV();
        float iw = BSP_ADC_GetCurrentW();

        /* Turn ON LED3 before sending */
        HAL_GPIO_WritePin(GPIOC, LED3_Pin, GPIO_PIN_RESET);

        HAL_StatusTypeDef status = BSP_UART_VOFA_SendFloats(iu, iv, iw, vbus_voltage);

        /* Turn OFF LED3 after sending */
        HAL_GPIO_WritePin(GPIOC, LED3_Pin, GPIO_PIN_SET);

        /* If send failed, blink LED2 rapidly */
        if (status != HAL_OK) {
            HAL_GPIO_TogglePin(GPIOC, LED2_Pin);
        }
    }

    /* CAN send test - send incrementing ID every 500ms */
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

    /* CAN receive test - poll */
    uint32_t can_rx_id = 0;
    uint8_t can_rx_data[8] = {0};
    uint8_t can_rx_len = 0;
    if (CAN_Receive(&can_rx_id, can_rx_data, &can_rx_len)) {
        /* Parse commands */
        if (can_rx_id == 0x100) {
            if (can_rx_data[0] == 0x01) {
                BSP_Motor_Start();
            } else if (can_rx_data[0] == 0x00) {
                BSP_Motor_Stop();
            }
        } else if (can_rx_id == 0x101 && can_rx_len >= 2) {
            uint16_t speed = (can_rx_data[1] << 8) | can_rx_data[0];
            BSP_Motor_SetSpeed(speed);
        }
    }

    static uint32_t led_timer = 0;
    if (sys_tick_ms - led_timer >= 500) {
        led_timer = sys_tick_ms;
        HAL_GPIO_TogglePin(GPIOC, LED3_Pin);  /* LED3 as heartbeat */
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
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
