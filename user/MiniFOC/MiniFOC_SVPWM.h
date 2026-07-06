/**
  ******************************************************************************
  * @file           : MiniFOC_SVPWM.h
  * @brief          : MiniFOC SVPWM 模块（对齐 SguanFOC v3.1.0）
  * @description    : 七段式空间矢量 PWM + 马鞍波生成
  * @dependencies   : MiniFOC_Config.h
  * @note           : 完全对齐 SguanFOC_Library-main v3.1.0
  ******************************************************************************
  * @attention
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  ******************************************************************************
  */
#ifndef __MINIFOC_SVPWM_H__
#define __MINIFOC_SVPWM_H__

#include <stdint.h>

/* ========== 宏定义 ========== */


#define SQRT3_2             0.8660254037844386f  /* √3/2 */
#define INV_SQRT3           0.5773502691896257f  /* 1/√3 */

/* ========== 函数声明 ========== */

/**
  * @brief  SVPWM 七段式空间矢量合成
  * @note   对齐 SguanFOC v3.1.0 Sguan_SVPWM.c
  *
  * @algorithm:
  *   1. 计算扇区判断信号 u1, u2, u3
  *   2. 确定电压矢量所在扇区
  *   3. 计算两个相邻有效矢量的作用时间 t_a, t_b
  *   4. 计算零矢量作用时间 t7
  *   5. 根据扇区分配三相 PWM 时间
  *   6. 过调制处理（当 t_a + t_b > 1 时缩放）
  *
  * @param  u_alpha: α 轴电压（归一化 -1~1）
  * @param  u_beta: β 轴电压（归一化 -1~1）
  * @param  d_u: 输出 U 相占空比（归一化 0~1）
  * @param  d_v: 输出 V 相占空比（归一化 0~1）
  * @param  d_w: 输出 W 相占空比（归一化 0~1）
  * @retval 无
  */
void SVPWM_SevenSegment(float u_alpha, float u_beta,
                        float *d_u, float *d_v, float *d_w);

/**
  * @brief  SVPWM 马鞍波生成（简化版，用于 VF 开环）
  * @note   零序注入法，适合 V/F 开环控制
  *
  * @algorithm:
  *   1. 逆 Clarke 变换：Ua/Ub/Uc
  *   2. 计算最大/最小值 Umax/Umin
  *   3. 计算零序注入量 U0 = -(Umax + Umin)/2
  *   4. 计算最终占空比并限幅
  *
  * @param  u_alpha: α 轴电压（V）
  * @param  u_beta: β 轴电压（V）
  * @param  vbus: 母线电压（V）
  * @param  duty_a: 输出 U 相 CCR 值（0~PWM_PERIOD）
  * @param  duty_b: 输出 V 相 CCR 值（0~PWM_PERIOD）
  * @param  duty_c: 输出 W 相 CCR 值（0~PWM_PERIOD）
  * @retval 无
  */
void SVPWM_SaddleWave(float u_alpha, float u_beta, float vbus,
                      uint32_t *duty_a, uint32_t *duty_b, uint32_t *duty_c);

/**
  * @brief  直接设置三相 PWM 占空比
  * @note   用于测试或特殊场景
  *
  * @param  duty_a: U 相 CCR 值（0~PWM_PERIOD）
  * @param  duty_b: V 相 CCR 值（0~PWM_PERIOD）
  * @param  duty_c: W 相 CCR 值（0~PWM_PERIOD）
  * @retval 无
  */
void SVPWM_SetDuty(uint32_t duty_a, uint32_t duty_b, uint32_t duty_c);

/**
  * @brief  紧急停止（所有 PWM 清零）
  * @retval 无
  */
void SVPWM_EmergencyStop(void);

#endif /* __MINIFOC_SVPWM_H__ */
