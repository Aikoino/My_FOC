/**
  ******************************************************************************
  * @file           : MiniFOC_PID.h
  * @brief          : MiniFOC PID控制器（对齐 SguanFOC v3.1.0）
  * @description    : 位置式PID + 积分抗饱和 + 微分滤波
  * @dependencies   : MiniFOC_Config.h
  * @note           : 完全对齐 SguanFOC_Library-main v3.1.0
  ******************************************************************************
  * @attention
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  ******************************************************************************
  */
#ifndef __MINIFOC_PID_H__
#define __MINIFOC_PID_H__

#include <stdint.h>

/* ========== PID 结构体定义（对齐 SguanFOC）========== */

/**
  * @brief  PID 运行数据结构体（存储中间变量）
  */
typedef struct {
    float i[2];             /* 误差历史 e(k), e(k-1) */
    float Io;               /* 积分历史输出 */
    float Do;               /* 微分历史输出 */

    float Ref;              /* 期望值（目标值） */
    float Fbk;              /* 反馈值（测量值） */
    float Output;           /* PID 输出 */

    float I_num;            /* 积分传递函数分子系数：T*Ki/2 */
    float D_num;            /* 微分传递函数分子系数：2*Kd*Wc/(Wc*T-2) */
    float D_den;            /* 微分传递函数分母系数：(Wc*T+2)/(Wc*T-2) */

    uint8_t IntegralFrozen_flag; /* 积分冻结标志（抗饱和） */
} PID_Run_t;

/**
  * @brief  PID 参数结构体
  */
typedef struct {
    PID_Run_t run;          /* PID 运行数据 */

    float T;                /* 采样周期（s），默认 0.000025（40kHz） */
    float Wc;               /* 微分滤波截止频率（rad/s），默认 1000 */
    float Kp;               /* 比例增益 */
    float Ki;               /* 积分增益 */
    float Kd;               /* 微分增益 */

    float OutMax;           /* 输出上限 */
    float OutMin;           /* 输出下限 */
    float IntMax;           /* 积分上限 */
    float IntMin;           /* 积分下限 */
} PID_Controller_t;

/* ========== PID 函数声明 ========== */

/**
  * @brief  PID 初始化（对齐 SguanFOC PID_Init）
  * @note   使用 double 精度计算离散化系数，提高精度
  * @param  pid: PID 控制器指针
  * @retval 无
  *
  * @discrete:
  *   Integral: I_num = T*Ki/2
  *   Derivative: D_num = 2*Kd*Wc/(Wc*T-2), D_den = (Wc*T+2)/(Wc*T-2)
  */
void PID_Init(PID_Controller_t *pid);

/**
  * @brief  PID 计算（对齐 SguanFOC PID_Loop）
  * @note   位置式 PID + 积分抗饱和 + 微分低通滤波
  * @param  pid: PID 控制器指针
  * @retval PID 输出（即 pid->run.Output）
  *
  * @algorithm:
  *   1. 计算误差：err = Ref - Fbk
  *   2. 积分项：带抗饱和（积分冻结机制）
  *   3. 微分项：带一阶低通滤波
  *   4. 输出：Out = Kp*err + Io + Do
  *   5. 输出限幅
  */
float PID_Calc(PID_Controller_t *pid);

/**
  * @brief  重置 PID 控制器（清零历史数据）
  * @param  pid: PID 控制器指针
  * @retval 无
  */
void PID_Reset(PID_Controller_t *pid);

/**
  * @brief  设置 PID 输出限幅
  * @param  pid: PID 控制器指针
  * @param  max: 上限
  * @param  min: 下限
  * @retval 无
  */
void PID_Set_OutputLimit(PID_Controller_t *pid, float max, float min);

/**
  * @brief  设置 PID 积分限幅
  * @param  pid: PID 控制器指针
  * @param  max: 上限
  * @param  min: 下限
  * @retval 无
  */
void PID_Set_IntegralLimit(PID_Controller_t *pid, float max, float min);

/**
  * @brief  设置 PID 参数
  * @param  pid: PID 控制器指针
  * @param  kp: 比例增益
  * @param  ki: 积分增益
  * @param  kd: 微分增益
  * @retval 无
  */
void PID_Set_Param(PID_Controller_t *pid, float kp, float ki, float kd);

#endif /* __MINIFOC_PID_H__ */
