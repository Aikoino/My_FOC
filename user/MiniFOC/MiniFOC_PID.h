/**
  ******************************************************************************
  * @file           : MiniFOC_PID.h
  * @brief          : MiniFOC PID控制器
  * @attention
  *
  * 支持：
  * - 增量式PID
  * - 位置式PID
  * - 抗积分饱和
  * - 输出限幅
  *
  ******************************************************************************
  */
#ifndef __MINIFOC_PID_H__
#define __MINIFOC_PID_H__

#include <stdint.h>
#include "MiniFOC_math.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== PID结构体 ========== */

/**
  * @brief  PID参数结构体
  */
typedef struct {
    float Kp;             /* 比例增益 */
    float Ki;             /* 积分增益 */
    float Kd;             /* 微分增益 */

    float out_max;        /* 输出上限 */
    float out_min;        /* 输出下限 */

    float integrator;     /* 积分累积值 */
    float prev_error_1;   /* 上一次误差 e(k-1) */
    float prev_error_2;   /* 上上次误差 e(k-2) */
} PID_Controller_t;

/* ========== PID函数声明 ========== */

/**
  * @brief  PID初始化
  * @param  pid: PID控制器指针
  * @param  kp: 比例增益
  * @param  ki: 积分增益
  * @param  kd: 微分增益
  */
void PID_Init(PID_Controller_t *pid, float kp, float ki, float kd);

/**
  * @brief  位置式PID计算
  * @param  pid: PID控制器指针
  * @param  ref: 目标值
  * @param  meas: 测量值
  * @retval PID输出
  * @note   带抗积分饱和
  */
float PID_Pos_Calc(PID_Controller_t *pid, float ref, float meas);

/**
  * @brief  增量式PID计算
  * @param  pid: PID控制器指针
  * @param  ref: 目标值
  * @param  meas: 测量值
  * @retval PID增量输出
  * @note   增量式适合积分饱和严重的场景
  */
float PID_Inc_Calc(PID_Controller_t *pid, float ref, float meas);

/**
  * @brief  重置PID控制器
  * @param  pid: PID控制器指针
  */
void PID_Reset(PID_Controller_t *pid);

/**
  * @brief  设置PID输出限幅
  * @param  pid: PID控制器指针
  * @param  max: 上限
  * @param  min: 下限
  */
void PID_Set_Limit(PID_Controller_t *pid, float max, float min);

#ifdef __cplusplus
}
#endif

#endif /* __MINIFOC_PID_H__ */
