/**
  ******************************************************************************
  * @file    PLL_Lock.h
  * @brief   PLL锁相环实现（参考SguanFOC v3.1.0）
  * @version 1.0.0
  * @date    2026-07-08
  *
  * 功能：PI控制器 + 积分器，平滑角度和速度
  ******************************************************************************
  */
#ifndef __PLL_LOCK_H__
#define __PLL_LOCK_H__

#include <stdint.h>
#include <stdbool.h>
#include "MiniFOC_Config.h"

/* ========== PLL锁相环句柄 ========== */
typedef struct {
    /* PI控制器输出（角速度）*/
    float X_num[2];            /* 传递函数分子 */
    float Y_num;               /* 积分项系数 */

    /* 历史数据 */
    float We_i;                /* 速度历史输入 */
    float Re_i;                /* 位置历史输入 */
    float OutWe;               /* 速度输出 */
    float OutRe;               /* 位置输出 */
    float Error;               /* 误差输入 */

    /* 模式标志 */
    uint8_t is_position_mode;  /* 0=速度模式, 1=位置模式 */

    /* PLL参数 */
    float Kp;                  /* PI比例增益 */
    float Ki;                  /* PI积分增益 */
    float T;                   /* 采样周期 */
} PLL_STRUCT;

/* ========== 函数声明 ========== */

/**
  * @brief  PLL初始化
  * @param  pll: PLL句柄
  * @retval None
  */
void PLL_Init(PLL_STRUCT *pll);

/**
  * @brief  PLL主循环
  * @param  pll: PLL句柄
  * @param  error: 角度误差 (rad)
  * @retval None
  *
  * 算法：
  * 1. PI控制器 → 输出角速度 We
  * 2. 积分器 → 输出位置 Re
  * 3. 位置归一化到 [0, 2π)
  */
void PLL_Loop(PLL_STRUCT *pll, float error);

/**
  * @brief  获取估算速度
  * @param  pll: PLL句柄
  * @retval 电角速度 (rad/s)
  */
float PLL_GetSpeed(PLL_STRUCT *pll);

/**
  * @brief  获取估算角度
  * @param  pll: PLL句柄
  * @retval 电角度 (rad)
  */
float PLL_GetAngle(PLL_STRUCT *pll);

/**
  * @brief  设置PLL增益
  * @param  pll: PLL句柄
  * @param  Kp: 比例增益
  * @param  Ki: 积分增益
  * @retval None
  */
void PLL_SetGain(PLL_STRUCT *pll, float Kp, float Ki);

#endif /* __PLL_LOCK_H__ */
