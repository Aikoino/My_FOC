/**
  ******************************************************************************
  * @file    PLL_Lock.c
  * @brief   PLL锁相环实现（参考SguanFOC v3.1.0）
  * @version 1.0.0
  * @date    2026-07-08
  ******************************************************************************
  */
#include "PLL_Lock.h"
#include "MiniFOC_math.h"
#include <math.h>

/**
  * @brief  PLL初始化
  * @param  pll: PLL句柄
  * @retval None
  *
  * 计算传递函数系数：
  * - X_num[0] = (2*Kp + Ki*T) / 2
  * - X_num[1] = (-2*Kp + Ki*T) / 2
  * - Y_num = T / 2
  */
void PLL_Init(PLL_STRUCT *pll)
{
    double temp0 = (double)(pll->T) * (double)(pll->Ki);

    pll->X_num[0] = (float)((2.0 * (double)(pll->Kp) + temp0) / 2.0);
    pll->X_num[1] = (float)((-2.0 * (double)(pll->Kp) + temp0) / 2.0);
    pll->Y_num    = (float)((double)(pll->T) / 2.0);

    /* 初始化为零 */
    pll->is_position_mode = 0;  /* 默认速度模式 */
    pll->We_i = 0.0f;
    pll->Re_i = 0.0f;
    pll->OutWe = 0.0f;
    pll->OutRe = 0.0f;
    pll->Error = 0.0f;
}

/**
  * @brief  PLL主循环
  * @param  pll: PLL句柄
  * @param  error: 角度误差 (rad)
  * @retval None
  *
  * 算法流程：
  * 1. PI控制器 → 输出角速度 We
  * 2. 积分器 → 输出位置 Re
  * 3. 位置归一化（非位置模式时）
  */
void PLL_Loop(PLL_STRUCT *pll, float error)
{
    /* 保存误差 */
    pll->Error = error;

    /* ========== 1. PI控制器 → 输出角速度 ========== */
    pll->OutWe += pll->X_num[0] * pll->Error +
                  pll->X_num[1] * pll->We_i;

    /* ========== 2. 积分器 → 输出位置 ========== */
    pll->OutRe += pll->Y_num * (pll->OutWe + pll->Re_i);

    /* 非位置模式：归一化到 [0, 2π) */
    if (!pll->is_position_mode) {
        pll->OutRe = Value_normalize(pll->OutRe);
    }

    /* ========== 3. 更新历史数据 ========== */
    pll->We_i = pll->Error;   /* 保存误差 */
    pll->Re_i = pll->OutWe;   /* 保存速度 */
}

/**
  * @brief  获取估算速度
  * @param  pll: PLL句柄
  * @retval 电角速度 (rad/s)
  */
float PLL_GetSpeed(PLL_STRUCT *pll)
{
    return pll->OutWe;
}

/**
  * @brief  获取估算角度
  * @param  pll: PLL句柄
  * @retval 电角度 (rad)
  */
float PLL_GetAngle(PLL_STRUCT *pll)
{
    return pll->OutRe;
}

/**
  * @brief  设置PLL增益
  * @param  pll: PLL句柄
  * @param  Kp: 比例增益
  * @param  Ki: 积分增益
  * @retval None
  */
void PLL_SetGain(PLL_STRUCT *pll, float Kp, float Ki)
{
    pll->Kp = Kp;
    pll->Ki = Ki;
    PLL_Init(pll);  /* 重新计算传递函数系数 */
}
