/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : bsp_hall.h
 * @brief          : 霍尔传感器驱动（对齐 SguanFOC v3.1.0）
 * @description    : 三路霍尔信号读取 + 6扇区判断 + 电角度映射
 * @dependencies   : stm32g4xx_hal.h
 * @note           : HA→PB6(TIM4_CH1), HB→PB7(TIM4_CH2), HC→PB8(TIM4_CH3)
 ******************************************************************************
 * @attention
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 ******************************************************************************
 */
/* USER CODE END Header */

#ifndef __BSP_HALL_H__
#define __BSP_HALL_H__

#include <stdint.h>
#include <stdbool.h>

/* ========== 霍尔传感器 GPIO 定义 ========== */

/* 霍尔传感器引脚定义（对齐 STM32G431 + TIM4）*/
#define HALL_HA_GPIO_Port    GPIOB
#define HALL_HA_Pin          GPIO_PIN_6   /* PB6 → TIM4_CH1 */
#define HALL_HB_GPIO_Port    GPIOB
#define HALL_HB_Pin          GPIO_PIN_7   /* PB7 → TIM4_CH2 */
#define HALL_HC_GPIO_Port    GPIOB
#define HALL_HC_Pin          GPIO_PIN_8   /* PB8 → TIM4_CH3 */

/* 霍尔极性（调试：B相反相，其他不变）*/
#define HALL_POLARITY_A      1
#define HALL_POLARITY_B      0
#define HALL_POLARITY_C      1

/* ========== 正转顺序 ========== */
/* 正转时扇区切换顺序：2→3→6→5→4→1→2...
 * 用于线性插值和速度估算 */

/* ========== 霍尔数据结构体 ========== */

/**
  * @brief  霍尔传感器数据结构体
  */
typedef struct {
    /* 输入信号 */
    uint8_t ha_raw;         /* HA 原始信号（0/1）*/
    uint8_t hb_raw;         /* HB 原始信号（0/1）*/
    uint8_t hc_raw;         /* HC 原始信号（0/1）*/

    /* 滤波后信号 */
    uint8_t ha_filtered;    /* HA 滤波后信号 */
    uint8_t hb_filtered;    /* HB 滤波后信号 */
    uint8_t hc_filtered;    /* HC 滤波后信号 */

    /* 扇区判断 */
    uint8_t sector;         /* 当前扇区（1~6）*/
    float   elec_angle;     /* 电角度（rad, 0~2π）*/

    /* 低通滤波系数 */
    float   filter_alpha;   /* 滤波系数（0~1, 越小滤波越强）*/

    /* 角度跟踪（插值用）*/
    uint32_t last_change_tick;  /* 上次扇区切换的系统时间（ms）*/
    uint8_t  last_sector;       /* 上次扇区（用于检测变化）*/
    float    last_track_angle;  /* 上次切换时的电角度（rad）*/
    float    rotor_speed;       /* 估算转速（rpm，正=顺时针，负=逆时针）*/

} BSP_Hall_t;

/* ========== 全局变量 ========== */

extern BSP_Hall_t hall_sensor;

/* ========== 函数声明 ========== */

/**
  * @brief  霍尔传感器初始化
  * @retval 无
  *
  * @init_sequence:
  *   1. 配置 GPIO（PB6/PB7/PB8 为输入）
  *   2. 初始化滤波系数
  *   3. 读取初始状态
  */
void BSP_Hall_Init(void);

/**
  * @brief  读取霍尔传感器状态
  * @note   读取 GPIO + 极性处理 + 软件滤波
  * @retval 无
  */
void BSP_Hall_Read(void);

/**
  * @brief  计算电角度（6扇区映射）
  * @note   对齐 SguanFOC Hall_Loop() 实现
  * @retval 电角度（rad, 0~2π）
  *
  * @sector_map:
  *   Sector 1 (101): 300°~0°   → 5*60° = 300°
  *   Sector 2 (001): 0°~60°    → 0*60° = 0°
  *   Sector 3 (011): 60°~120°  → 1*60° = 60°
  *   Sector 4 (010): 120°~180° → 4*60° = 240° (反转)
  *   Sector 5 (110): 180°~240° → 3*60° = 180° (反转)
  *   Sector 6 (100): 240°~300° → 2*60° = 120° (反转)
  */
void BSP_Hall_CalculateAngle(void);

/**
  * @brief  获取电角度
  * @retval 电角度（rad, 0~2π）
  */
float BSP_Hall_GetAngle(void);

/**
  * @brief  获取扇区编号
  * @retval 扇区（1~6）
  */
uint8_t BSP_Hall_GetSector(void);

/**
  * @brief  设置滤波系数
  * @param  alpha: 滤波系数（0~1）
  * @retval 无
  */
void BSP_Hall_SetFilter(float alpha);

/**
  * @brief  获取估算转速（rpm）
  * @retval 转速（rpm）
  */
float BSP_Hall_GetSpeed(void);

/**
  * @brief  重置 Hall 跟踪状态（启动电机前调用）
  * @retval 无
  */
void BSP_Hall_ResetTracking(void);

/**
  * @brief  TIM4 捕获回调函数
  * @note   在 stm32g4xx_it.c 的 TIM4_IRQHandler 中调用
  *         用于 TIM4 硬件捕获模式，精度更高
  */
void BSP_Hall_TIM4_CaptureCallback(void);

#endif /* __BSP_HALL_H__ */
