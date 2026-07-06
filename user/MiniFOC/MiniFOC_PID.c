/**
  ******************************************************************************
  * @file           : MiniFOC_PID.c
  * @brief          : MiniFOC PID控制器实现（对齐 SguanFOC v3.1.0）
  * @description    : 位置式PID + 积分抗饱和 + 微分滤波
  * @dependencies   : MiniFOC_PID.h
  * @note           : 完全对齐 SguanFOC_Library-main v3.1.0/Sguan_PID.c
  ******************************************************************************
  * @attention
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  ******************************************************************************
  */
#include "MiniFOC_PID.h"
#include "MiniFOC_Config.h"

/* ========== PID 实现 ========== */

/**
  * @brief  PID 初始化（对齐 SguanFOC PID_Init）
  * @note   使用 double 精度计算离散化系数，提高精度
  */
void PID_Init(PID_Controller_t *pid)
{
    double temp0 = ((double)pid->T) * ((double)pid->Ki) / 2.0;
    double temp1 = ((double)pid->T) * ((double)pid->Wc);
    double temp2 = ((double)pid->Kd) * ((double)pid->Wc);
    double den = -2.0 + temp1;

    pid->run.I_num = (float)temp0;                           /* T*Ki/2 */
    pid->run.D_num = (float)((2.0 * temp2) / den);          /* 2*Kd*Wc/(Wc*T-2) */
    pid->run.D_den = (float)((2.0 + temp1) / den);          /* (Wc*T+2)/(Wc*T-2) */

    /* 初始化为零 */
    pid->run.i[0] = 0.0f;
    pid->run.i[1] = 0.0f;
    pid->run.Ref = 0.0f;
    pid->run.Fbk = 0.0f;
    pid->run.Output = 0.0f;
    pid->run.Io = 0.0f;
    pid->run.Do = 0.0f;
    pid->run.IntegralFrozen_flag = 0;
}

/**
  * @brief  PID 计算（对齐 SguanFOC PID_Loop）
  * @note   位置式 PID + 积分抗饱和 + 微分低通滤波
  *
  * @formula:
  *   err(k) = Ref(k) - Fbk(k)
  *
  *   Integral:
  *     Io(k) = Io(k-1) + I_num * [err(k) + err(k-1)]
  *     （带积分冻结抗饱和）
  *
  *   Derivative:
  *     Do(k) = D_num * [err(k) + err(k-1)] - D_den * Do(k-1)
  *     （带一阶低通滤波）
  *
  *   Output:
  *     Out(k) = Kp * err(k) + Io(k) + Do(k)
  *     （带输出限幅）
  */
float PID_Calc(PID_Controller_t *pid)
{
    /* 1. 计算误差 */
    pid->run.i[0] = pid->run.Ref - pid->run.Fbk;

    /* 2. 积分项（带抗饱和 - 积分冻结机制） */
    if (pid->Ki != 0.0f) {
        /* 检查是否需要冻结积分 */
        if (pid->run.IntegralFrozen_flag) {
            /* 积分已冻结，保持上次值 */
            /* 检查是否可以解除冻结：
             * 条件1：误差反向（误差符号与积分输出符号相反）
             * 条件2：积分值回到限幅范围内
             */
            if ((pid->run.i[0] * pid->run.Io < 0.0f) ||                          /* 误差反向 */
                ((pid->run.Io < pid->IntMax) && (pid->run.Io > pid->IntMin))) {  /* 积分在限幅内 */
                pid->run.IntegralFrozen_flag = 0;                                 /* 解除冻结 */
            }
        } else {
            /* 正常计算积分（梯形积分） */
            pid->run.Io += pid->run.I_num * (pid->run.i[0] + pid->run.i[1]);

            /* 积分限幅并冻结 */
            if (pid->run.Io > pid->IntMax) {
                pid->run.Io = pid->IntMax;
                pid->run.IntegralFrozen_flag = 1;  /* 冻结积分 */
            } else if (pid->run.Io < pid->IntMin) {
                pid->run.Io = pid->IntMin;
                pid->run.IntegralFrozen_flag = 1;  /* 冻结积分 */
            }
        }
    } else {
        /* Ki = 0 时，积分项清零 */
        pid->run.Io = 0.0f;
    }

    /* 3. 微分项（带一阶低通滤波） */
    if (pid->Kd != 0.0f) {
        pid->run.Do = pid->run.D_num * (pid->run.i[0] + pid->run.i[1]) -
                      pid->run.D_den * pid->run.Do;
    } else {
        pid->run.Do = 0.0f;
    }

    /* 4. 计算输出：Out = Kp*err + Io + Do */
    pid->run.Output = pid->run.i[0] * pid->Kp + pid->run.Io + pid->run.Do;

    /* 5. 输出限幅 */
    if (pid->run.Output > pid->OutMax) {
        pid->run.Output = pid->OutMax;
    } else if (pid->run.Output < pid->OutMin) {
        pid->run.Output = pid->OutMin;
    }

    /* 6. 更新历史误差 */
    pid->run.i[1] = pid->run.i[0];

    return pid->run.Output;
}

/**
  * @brief  重置 PID 控制器（清零历史数据）
  */
void PID_Reset(PID_Controller_t *pid)
{
    pid->run.i[0] = 0.0f;
    pid->run.i[1] = 0.0f;
    pid->run.Io = 0.0f;
    pid->run.Do = 0.0f;
    pid->run.Output = 0.0f;
    pid->run.IntegralFrozen_flag = 0;
}

/**
  * @brief  设置 PID 输出限幅
  */
void PID_Set_OutputLimit(PID_Controller_t *pid, float max, float min)
{
    pid->OutMax = max;
    pid->OutMin = min;
}

/**
  * @brief  设置 PID 积分限幅
  */
void PID_Set_IntegralLimit(PID_Controller_t *pid, float max, float min)
{
    pid->IntMax = max;
    pid->IntMin = min;
}

/**
  * @brief  设置 PID 参数
  * @note   修改参数后会自动重新计算离散化系数
  */
void PID_Set_Param(PID_Controller_t *pid, float kp, float ki, float kd)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    PID_Init(pid);  /* 重新计算离散化系数 */
}


