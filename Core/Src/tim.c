/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances (TIM1 for PWM + TIM4 for Hall Sensor).
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "tim.h"
#include <string.h>  /* memset */

/* USER CODE BEGIN 0 */

/* 霍尔传感器句柄 */
HALL_Handle_t HALL_Handle = {0};

/* USER CODE END 0 */

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim4;  /* TIM4 for Hall Sensor */

/* TIM1 init function */
void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED1;
  htim1.Init.Period = 7999;  /* 与 BLOC 一致：ARR=7999, PWM=10.6kHz */
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
  sConfigOC.Pulse = 4248;
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
  sBreakDeadTimeConfig.Break2AFMode = TIM_BREAK_AFMODE_INPUT;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/* TIM4 init function - Hall Sensor */
void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_HallSensor_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 50-1;           /* 50分频：160MHz / 50 = 3.2MHz */
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* 配置霍尔传感器接口模式 */
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;    /* 上升沿捕获 */
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;           /* 不分频 */
  sConfig.IC1Filter = 10;                          /* 数字滤波器10 */
  sConfig.Commutation_Delay = 5;                   /* 换相延迟5 */
  if (HAL_TIMEx_HallSensor_Init(&htim4, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC2REF;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(tim_baseHandle->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspInit 0 */

  /* USER CODE END TIM1_MspInit 0 */
    /* TIM1 clock enable */
    __HAL_RCC_TIM1_CLK_ENABLE();
  /* USER CODE BEGIN TIM1_MspInit 1 */

  /* USER CODE END TIM1_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM4)
  {
  /* USER CODE BEGIN TIM4_MspInit 0 */

  /* USER CODE END TIM4_MspInit 0 */
    /* TIM4 clock enable */
    __HAL_RCC_TIM4_CLK_ENABLE();

    /**TIM4 GPIO Configuration
    PB6     ------> TIM4_CH1
    PB7     ------> TIM4_CH2
    PB8     ------> TIM4_CH3
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* TIM4 interrupt Init */
    HAL_NVIC_SetPriority(TIM4_IRQn, 3, 0);  /* 优先级3，低于TIM1 */
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
  /* USER CODE BEGIN TIM4_MspInit 1 */

  /* USER CODE END TIM4_MspInit 1 */
  }
}
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(timHandle->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspPostInit 0 */

  /* USER CODE END TIM1_MspPostInit 0 */

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**TIM1 GPIO Configuration
    PB13     ------> TIM1_CH1N
    PB14     ------> TIM1_CH2N
    PB15     ------> TIM1_CH3N
    PA8     ------> TIM1_CH1
    PA9     ------> TIM1_CH2
    PA10     ------> TIM1_CH3
    */
    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_TIM1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_TIM1;  /* PB15 复用 AF4（与 BLOC 一致）*/
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;  /* 修复：改为高速 */
    GPIO_InitStruct.Alternate = GPIO_AF6_TIM1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM1_MspPostInit 1 */

  /* USER CODE END TIM1_MspPostInit 1 */
  }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM1)
  {
  /* USER CODE BEGIN TIM1_MspDeInit 0 */

  /* USER CODE END TIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM1_CLK_DISABLE();

    /* TIM1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM1_UP_TIM16_IRQn);
  /* USER CODE BEGIN TIM1_MspDeInit 1 */

  /* USER CODE END TIM1_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM4)
  {
  /* USER CODE BEGIN TIM4_MspDeInit 0 */

  /* USER CODE END TIM4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM4_CLK_DISABLE();

    /**TIM4 GPIO Configuration
    PB6     ------> TIM4_CH1
    PB7     ------> TIM4_CH2
    PB8     ------> TIM4_CH3
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8);

    /* TIM4 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM4_IRQn);
  /* USER CODE BEGIN TIM4_MspDeInit 1 */

  /* USER CODE END TIM4_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* 一阶RC低通滤波 */
void FirstOrderRC_LPF(float *out, float in, float alpha)
{
    *out = (1-alpha)*(*out) + alpha*in;
}

/* 获取初始霍尔电角度（完全基于 TIM4 硬件捕获，不读 GPIO）*/
void HALL_Init_Electrical_Angle(void)
{
    HALL_Handle_t *phandle = &HALL_Handle;

    /* 清零所有字段 */
    memset(phandle, 0, sizeof(HALL_Handle_t));

    /* ✅ 完全使用 TIM4 硬件捕获，不读 GPIO */
    /* 注意：TIM4 配置为霍尔传感器模式后，硬件会自动检测换相时刻 */
    /* 初始角度给一个默认值（0°），等待第一次 TIM4 中断修正 */
    phandle->HallState = 0;  /* 初始状态未知 */
    phandle->MeasuredElAngle = 0.0f;  /* 初始角度 0° */
    phandle->HallElAngle = 0.0f;
    phandle->Direction = POSITIVE;  /* 默认正转 */
    phandle->bPrevHallState = 0;
}

/* TIM4捕获回调 - 计算霍尔电角度和转速 */
void HALL_Get_Electrical_Angle(void *pHandleVoid)
{
    HALL_Handle_t *phandle = (HALL_Handle_t *)pHandleVoid;
    phandle->hHighSpeedCapture = HAL_TIM_ReadCapturedValue(&htim4, TIM_CHANNEL_1);
    phandle->bPrevHallState = phandle->HallState;
    phandle->HallState = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8);
    phandle->HallState |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) << 1;
    phandle->HallState |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) << 2;

    switch (phandle->HallState)
    {
        case STATE_5:
        {
            if (STATE_1 == phandle->bPrevHallState) {
                phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE;
                phandle->Direction = POSITIVE;
            } else if (STATE_4 == phandle->bPrevHallState) {
                phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + PI / 3.0f;
                phandle->Direction = NEGATIVE;
            }
            break;
        }
        case STATE_4:
        {
            if (STATE_5 == phandle->bPrevHallState) {
                phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + PI / 3.0f;
                phandle->Direction = POSITIVE;
            } else if (STATE_6 == phandle->bPrevHallState) {
                phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + 2 * PI / 3.0f;
                phandle->Direction = NEGATIVE;
            }
            break;
        }
        case STATE_6:
        {
            if (STATE_4 == phandle->bPrevHallState) {
                phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + 2 * PI / 3.0f;
                phandle->Direction = POSITIVE;
            } else if (STATE_2 == phandle->bPrevHallState) {
                phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + PI;
                phandle->Direction = NEGATIVE;
            }
            break;
        }
        case STATE_2:
        {
            if (STATE_6 == phandle->bPrevHallState) {
                phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + PI;
                phandle->Direction = POSITIVE;
            } else if (STATE_3 == phandle->bPrevHallState) {
                phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + 4 * PI / 3.0f;
                phandle->Direction = NEGATIVE;
            }
            break;
        }
        case STATE_3:
        {
            if (STATE_2 == phandle->bPrevHallState) {
                phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + 4 * PI / 3.0f;
                phandle->Direction = POSITIVE;
            } else if (STATE_1 == phandle->bPrevHallState) {
                phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + 5 * PI / 3.0f;
                phandle->Direction = NEGATIVE;
            }
            break;
        }
        case STATE_1:
        {
            if (STATE_3 == phandle->bPrevHallState) {
                phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + 5 * PI / 3.0f;
                phandle->Direction = POSITIVE;
            } else if (STATE_5 == phandle->bPrevHallState) {
                phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE;
                phandle->Direction = NEGATIVE;
            }
            break;
        }
        default: break;
    }

    /* 角度归一化到 [0, 2π) */
    if (phandle->MeasuredElAngle < 0.0f)
    {
        phandle->MeasuredElAngle += 2.0f * PI;
    }
    else if (phandle->MeasuredElAngle > (2.0f * PI))
    {
        phandle->MeasuredElAngle -= 2.0f * PI;
    }

    /* 计算转速（基于TIM4捕获值，精度更高）*/
    uint32_t tim4_clk = HAL_RCC_GetPCLK1Freq();  /* APB1时钟 */
    float tim4_count_freq = tim4_clk / 50.0f;    /* 实际计数频率 */
    phandle->AvrElSpeedDpp = (PI/3) / ((phandle->hHighSpeedCapture / tim4_count_freq) * 10000);

    /* 转换为 rpm/min */
    phandle->TempSpeed = (PI/3)/(phandle->hHighSpeedCapture/3200000.0f)*30/(2*PI);
    phandle->TempSpeed = phandle->TempSpeed * phandle->Direction;
    FirstOrderRC_LPF(&phandle->HallSpeed, phandle->TempSpeed, 0.2379f); /* 一阶低通 */
    phandle->AvrElSpeedDpp = phandle->AvrElSpeedDpp * phandle->Direction;

    /* 计算角度补偿量（用于ADC中断累加）*/
    phandle->DeltaAngle = (phandle->MeasuredElAngle - phandle->HallElAngle) / 10000.0f;

    /* ❌ 不要在这里更新 HallElAngle！
     * ✅ HallElAngle 由 ADC 中断累加：HallElAngle += AvrElSpeedDpp
     * ✅ 这样可以实现 20kHz 的平滑角度更新，而不是只在换相时更新
     */
}

/* USER CODE END 1 */
