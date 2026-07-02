/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main_debug.c
  * @brief          : Basic Debug Version - 带串口调试信息
  *
  * 在 Test_Step 前后打印Step信息，精确定位失败位置
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp_adc.h"
#include "bsp_uart_vofta.h"
#include "bsp_can.h"
#include "bsp_motor.h"
#include "../user/MiniFOC/MiniFOC.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TEST_INTERVAL_MS    100
#define ADC_SAMPLE_COUNT    10
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern FDCAN_HandleTypeDef hfdcan1;
extern OPAMP_HandleTypeDef hopamp1;
extern OPAMP_HandleTypeDef hopamp2;
extern OPAMP_HandleTypeDef hopamp3;
extern TIM_HandleTypeDef htim1;
extern UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
static uint8_t key_state = 0;
static uint8_t key_last = 1;
static uint32_t key_timer = 0;
static uint32_t sys_tick_ms = 0;
static uint32_t last_test_ms = 0;
static uint32_t last_can_ms = 0;
static uint8_t can_tx_id = 0;
#define PRINT_BUF_SIZE    128
static char print_buf[PRINT_BUF_SIZE];
static uint16_t vbus_raw = 0;
static float vbus_voltage = 0.0f;
static uint32_t vbus_sum = 0;
static uint8_t vbus_sample_cnt = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_OPAMP1_Init(void);
static void MX_OPAMP2_Init(void);
static void MX_OPAMP3_Init(void);
static void MX_USART3_UART_Init(void);

/* USER CODE BEGIN PFP */
static void Test_Step(uint8_t step, const char *name);
static void Key_Scan(void);
static void Vbus_Adc_Update(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void Test_Step(uint8_t step, const char *name)
{
    sprintf(print_buf, "[Step %d] %s ...\r\n", step, name);
    HAL_UART_Transmit(&huart3, (uint8_t*)print_buf, strlen(print_buf), 100);

    HAL_GPIO_WritePin(GPIOC, LED2_Pin, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(GPIOC, LED2_Pin, GPIO_PIN_SET);
    for (uint8_t i = 0; i < step; i++) {
        HAL_GPIO_WritePin(GPIOC, LED3_Pin, GPIO_PIN_RESET);
        HAL_Delay(150);
        HAL_GPIO_WritePin(GPIOC, LED3_Pin, GPIO_PIN_SET);
        HAL_Delay(150);
    }
    HAL_Delay(300);

    sprintf(print_buf, "[Step %d] %s - OK\r\n\r\n", step, name);
    HAL_UART_Transmit(&huart3, (uint8_t*)print_buf, strlen(print_buf), 100);
}

static void Key_Scan(void)
{
    uint8_t key_now = (uint8_t)HAL_GPIO_ReadPin(GPIOC, KEY_Pin);
    if (key_last == 1 && key_now == 0) {
        key_state = 1;
    } else if (key_now == 1) {
        key_state = 0;
    }
    key_last = key_now;
}

static void Vbus_Adc_Update(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    vbus_sum += HAL_ADC_GetValue(&hadc1);
    vbus_sample_cnt++;
    HAL_ADC_Stop(&hadc1);
    if (vbus_sample_cnt >= ADC_SAMPLE_COUNT) {
        vbus_raw = (uint16_t)(vbus_sum / ADC_SAMPLE_COUNT);
        vbus_voltage = (float)vbus_raw * 3.3f / 4096.0f * 26.0f;
        vbus_sum = 0;
        vbus_sample_cnt = 0;
    }
}
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n=== SYSTEM START ===\r\n", 25, 100);

  SystemClock_Config();
  Test_Step(1, "SystemClock_Config");

  MX_GPIO_Init();
  MX_USART3_UART_Init();
  Test_Step(2, "USART3_Init");

  MX_TIM1_Init();
  Test_Step(3, "TIM1_Init");

  MX_ADC1_Init();
  Test_Step(4, "ADC1_Init");

  MX_ADC2_Init();
  Test_Step(5, "ADC2_Init");

  MX_OPAMP1_Init();
  Test_Step(6, "OPAMP1_Init");

  MX_OPAMP2_Init();
  MX_OPAMP3_Init();
  MX_FDCAN1_Init();
  Test_Step(7, "FDCAN1_Init");

  HAL_UART_Transmit(&huart3, (uint8_t*)"[Step8] BSP Layer Init ...\r\n", 30, 100);
  BSP_ADC_Init();
  BSP_UART_VOFA_Init();
  BSP_Button_Init();
  BSP_Motor_Init();
  CAN_Init();  /* CAN 必须在 BSP 之后初始化 */
  Test_Step(8, "BSP Layer Init Done");

  HAL_UART_Transmit(&huart3, (uint8_t*)"[Step8.5] MiniFOC Init ...\r\n", 30, 100);
  MiniFOC_Init();
  Test_Step(8.5, "MiniFOC Init Done");

  HAL_UART_Transmit(&huart3, (uint8_t*)"[Step9] TIM1 PWM Start ...\r\n", 30, 100);
  HAL_TIM_Base_Start(&htim1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  Test_Step(9, "TIM1 PWM Start Done");

  HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n========================================\r\n", 40, 100);
  HAL_UART_Transmit(&huart3, (uint8_t*)"  Basic Project - Enter Main Loop\r\n", 30, 100);
  HAL_UART_Transmit(&huart3, (uint8_t*)"========================================\r\n\r\n", 40, 100);

  while (1)
  {
    sys_tick_ms = HAL_GetTick();

    if (sys_tick_ms - key_timer >= 10) {
        key_timer = sys_tick_ms;
        Key_Scan();
        if (key_state) {
            key_state = 0;
            HAL_GPIO_TogglePin(GPIOC, LED2_Pin);  /* 按键控制LED2 */
            sprintf(print_buf, "[KEY] Button Pressed\r\n");
            HAL_UART_Transmit(&huart3, (uint8_t*)print_buf, strlen(print_buf), 10);
        }
    }

    /* MiniFOC 主循环 (1kHz) */
    MiniFOC_MainLoop();

    if (sys_tick_ms - last_test_ms >= TEST_INTERVAL_MS) {
        last_test_ms = sys_tick_ms;
        Vbus_Adc_Update();
        float iu = BSP_ADC_GetCurrentU();
        float iv = BSP_ADC_GetCurrentV();
        float iw = BSP_ADC_GetCurrentW();
        static uint8_t print_cnt = 0;
        if (++print_cnt >= 10) {
            print_cnt = 0;
            sprintf(print_buf, "Iu=%.3fA, Iv=%.3fA, Iw=%.3fA, Vbus=%.1fV\r\n",
                    iu, iv, iw, vbus_voltage);
            HAL_UART_Transmit(&huart3, (uint8_t*)print_buf, strlen(print_buf), 100);
        }
        BSP_UART_VOFA_SendFloats(iu, iv, iw, vbus_voltage);
    }

    /* CAN 发送测试 - 每500ms发送一次递增ID */
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
            sprintf(print_buf, "[CAN TX] ID=0x100, Data=%02X\r\n", can_tx_id);
            HAL_UART_Transmit(&huart3, (uint8_t*)print_buf, strlen(print_buf), 10);
        } else {
            HAL_UART_Transmit(&huart3, (uint8_t*)"[CAN TX] Send Failed!\r\n", 25, 10);
        }
        can_tx_id++;
    }

    /* CAN 接收测试 - 轮询检查 */
    uint32_t can_rx_id = 0;
    uint8_t can_rx_data[8] = {0};
    uint8_t can_rx_len = 0;
    if (CAN_Receive(&can_rx_id, can_rx_data, &can_rx_len)) {
        sprintf(print_buf, "[CAN RX] ID=0x%03lX, Len=%d, Data=",
                can_rx_id, can_rx_len);
        HAL_UART_Transmit(&huart3, (uint8_t*)print_buf, strlen(print_buf), 10);
        for (int i = 0; i < can_rx_len; i++) {
            sprintf(print_buf, "%02X ", can_rx_data[i]);
            HAL_UART_Transmit(&huart3, (uint8_t*)print_buf, strlen(print_buf), 10);
        }
        HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n", 2, 10);

        /* 解析命令 */
        if (can_rx_id == 0x100) {
            if (can_rx_data[0] == 0x01) {
                HAL_UART_Transmit(&huart3, (uint8_t*)"[CMD] Motor Start\r\n", 19, 10);
                BSP_Motor_Start();
            } else if (can_rx_data[0] == 0x00) {
                HAL_UART_Transmit(&huart3, (uint8_t*)"[CMD] Motor Stop\r\n", 18, 10);
                BSP_Motor_Stop();
            }
        } else if (can_rx_id == 0x101 && can_rx_len >= 2) {
            uint16_t speed = (can_rx_data[1] << 8) | can_rx_data[0];
            sprintf(print_buf, "[CMD] Speed=%d\r\n", speed);
            HAL_UART_Transmit(&huart3, (uint8_t*)print_buf, strlen(print_buf), 10);
            BSP_Motor_SetSpeed(speed);
        }
    }

    static uint32_t led_timer = 0;
    if (sys_tick_ms - led_timer >= 500) {
        led_timer = sys_tick_ms;
        HAL_GPIO_TogglePin(GPIOC, LED3_Pin);  /* LED3作为心跳灯 */
    }
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
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  */
static void MX_ADC1_Init(void)
{
  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};
  ADC_InjectionConfTypeDef sConfigInjected = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_6CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigInjected.InjectedChannel = ADC_CHANNEL_3;
  sConfigInjected.InjectedRank = ADC_INJECTED_RANK_1;
  sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_6CYCLES_5;
  sConfigInjected.InjectedSingleDiff = ADC_SINGLE_ENDED;
  sConfigInjected.InjectedOffsetNumber = ADC_OFFSET_NONE;
  sConfigInjected.InjectedOffset = 0;
  sConfigInjected.InjectedNbrOfConversion = 2;
  sConfigInjected.InjectedDiscontinuousConvMode = DISABLE;
  sConfigInjected.AutoInjectedConv = DISABLE;
  sConfigInjected.QueueInjectedContext = DISABLE;
  sConfigInjected.ExternalTrigInjecConv = ADC_EXTERNALTRIGINJEC_T1_CC4;
  sConfigInjected.ExternalTrigInjecConvEdge = ADC_EXTERNALTRIGINJECCONV_EDGE_RISING;
  sConfigInjected.InjecOversamplingMode = DISABLE;
  if (HAL_ADCEx_InjectedConfigChannel(&hadc1, &sConfigInjected) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigInjected.InjectedChannel = ADC_CHANNEL_12;
  sConfigInjected.InjectedRank = ADC_INJECTED_RANK_2;
  if (HAL_ADCEx_InjectedConfigChannel(&hadc1, &sConfigInjected) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC2 Initialization Function
  */
static void MX_ADC2_Init(void)
{
  ADC_InjectionConfTypeDef sConfigInjected = {0};

  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.GainCompensation = 0;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc2.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  sConfigInjected.InjectedChannel = ADC_CHANNEL_3;
  sConfigInjected.InjectedRank = ADC_INJECTED_RANK_1;
  sConfigInjected.InjectedSamplingTime = ADC_SAMPLETIME_6CYCLES_5;
  sConfigInjected.InjectedSingleDiff = ADC_SINGLE_ENDED;
  sConfigInjected.InjectedOffsetNumber = ADC_OFFSET_NONE;
  sConfigInjected.InjectedOffset = 0;
  sConfigInjected.InjectedNbrOfConversion = 1;
  sConfigInjected.InjectedDiscontinuousConvMode = DISABLE;
  sConfigInjected.AutoInjectedConv = DISABLE;
  sConfigInjected.QueueInjectedContext = DISABLE;
  sConfigInjected.ExternalTrigInjecConv = ADC_EXTERNALTRIGINJEC_T1_CC4;
  sConfigInjected.ExternalTrigInjecConvEdge = ADC_EXTERNALTRIGINJECCONV_EDGE_RISING;
  sConfigInjected.InjecOversamplingMode = DISABLE;
  if (HAL_ADCEx_InjectedConfigChannel(&hadc2, &sConfigInjected) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief FDCAN1 Initialization Function
  */
static void MX_FDCAN1_Init(void)
{
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 20;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 10;
  hfdcan1.Init.NominalTimeSeg2 = 5;
  hfdcan1.Init.DataPrescaler = 20;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 10;
  hfdcan1.Init.DataTimeSeg2 = 5;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 1;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief OPAMP1 Initialization Function
  */
static void MX_OPAMP1_Init(void)
{
  hopamp1.Instance = OPAMP1;
  hopamp1.Init.PowerMode = OPAMP_POWERMODE_NORMALSPEED;
  hopamp1.Init.Mode = OPAMP_STANDALONE_MODE;
  hopamp1.Init.InvertingInput = OPAMP_INVERTINGINPUT_IO0;
  hopamp1.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_IO0;
  hopamp1.Init.InternalOutput = DISABLE;
  hopamp1.Init.TimerControlledMuxmode = OPAMP_TIMERCONTROLLEDMUXMODE_DISABLE;
  hopamp1.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
  if (HAL_OPAMP_Init(&hopamp1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief OPAMP2 Initialization Function
  */
static void MX_OPAMP2_Init(void)
{
  hopamp2.Instance = OPAMP2;
  hopamp2.Init.PowerMode = OPAMP_POWERMODE_NORMALSPEED;
  hopamp2.Init.Mode = OPAMP_STANDALONE_MODE;
  hopamp2.Init.InvertingInput = OPAMP_INVERTINGINPUT_IO0;
  hopamp2.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_IO0;
  hopamp2.Init.InternalOutput = DISABLE;
  hopamp2.Init.TimerControlledMuxmode = OPAMP_TIMERCONTROLLEDMUXMODE_DISABLE;
  hopamp2.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
  if (HAL_OPAMP_Init(&hopamp2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief OPAMP3 Initialization Function
  */
static void MX_OPAMP3_Init(void)
{
  hopamp3.Instance = OPAMP3;
  hopamp3.Init.PowerMode = OPAMP_POWERMODE_NORMALSPEED;
  hopamp3.Init.Mode = OPAMP_STANDALONE_MODE;
  hopamp3.Init.InvertingInput = OPAMP_INVERTINGINPUT_IO0;
  hopamp3.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_IO0;
  hopamp3.Init.InternalOutput = DISABLE;
  hopamp3.Init.TimerControlledMuxmode = OPAMP_TIMERCONTROLLEDMUXMODE_DISABLE;
  hopamp3.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
  if (HAL_OPAMP_Init(&hopamp3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM1 Initialization Function
  */
static void MX_TIM1_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED1;
  htim1.Init.Period = 7999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC4REF;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.Pulse = 7998;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 120;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.BreakAFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK2_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim1);
}

/**
  * @brief USART3 Initialization Function
  */
static void MX_USART3_UART_Init(void)
{
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 921600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOC, LED2_Pin|LED3_Pin, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = KEY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(KEY_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LED2_Pin|LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  */
void Error_Handler(void)
{
  __disable_irq();
  HAL_UART_Transmit(&huart3, (uint8_t*)"\r\n[ERROR] Error_Handler Triggered！\r\n", 35, 100);
  while (1)
  {
    HAL_GPIO_TogglePin(GPIOC, LED3_Pin);
    HAL_Delay(200);
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
