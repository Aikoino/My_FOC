/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file (TIM1 PWM + TIM4 Hall Sensor)
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* 霍尔传感器状态定义（6种有效状态）*/
#define STATE_1    0x01  // 001
#define STATE_2    0x02  // 010
#define STATE_3    0x03  // 011
#define STATE_4    0x04  // 100
#define STATE_5    0x05  // 101
#define STATE_6    0x06  // 110

/* 转动方向 */
#define POSITIVE   1   // 正转
#define NEGATIVE  -1   // 反转

/* PI常量 */
#define PI         3.1415926535f
#define PHASE_SHIFT_ANGLE (float)(218.0f/360.0f*2.0f*PI)
#define MOTOR_POLE_PAIRS   7  /* 电机极对数（与 MiniFOC_Config.h 保持一致）*/

/**
  * @brief  霍尔传感器数据结构体
  */
typedef struct
{
    uint8_t  HallState;        /* 霍尔状态 */
    float    AvrElSpeedDpp;    /* 平均转速 rad/s */
    float    MeasuredElAngle;  /* 实测电角度 */
    float    HallElAngle;      /* 霍尔电角度 */
    int8_t   Direction;        /* 转动方向 */
    float    HallSpeed;        /* 霍尔转速 rpm/min */
    float    TempSpeed;        /* 临时转速 rpm/min */
    float    DeltaAngle;       /* 角度增量 */
    uint8_t  bPrevHallState;   /* 上一次霍尔状态 */
    float    hHighSpeedCapture;/* 高速捕获值 */
    float    MeasureTest;      /* 测试变量 */
} HALL_Handle_t;

/* 全局霍尔句柄 */
extern HALL_Handle_t HALL_Handle;

/* USER CODE END Includes */

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim4;  /* TIM4 for Hall Sensor */

/* USER CODE BEGIN Private defines */
void FirstOrderRC_LPF(float *out, float in, float alpha);
void HALL_Init_Electrical_Angle(void);

/* USER CODE END Private defines */

void MX_TIM1_Init(void);
void MX_TIM4_Init(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */

