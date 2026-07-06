/**
  ******************************************************************************
  * @file           : MiniFOC_SVPWM.c
  * @brief          : MiniFOC SVPWM 实现（对齐 SguanFOC v3.1.0）
  * @description    : 七段式空间矢量 PWM + 马鞍波生成
  * @dependencies   : MiniFOC_SVPWM.h
  * @note           : 完全对齐 SguanFOC_Library-main v3.1.0/Sguan_SVPWM.c
  ******************************************************************************
  * @attention
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  ******************************************************************************
  */
#include "MiniFOC_SVPWM.h"
#include "MiniFOC_Config.h"
#include "stm32g4xx_hal.h"

extern TIM_HandleTypeDef htim1;

/**
  * @brief  SVPWM 七段式空间矢量合成
  * @note   对齐 SguanFOC v3.1.0 Sguan_SVPWM.c
  *
  * @扇区分布：
  *   Sector 1: 0°~60°   (V4, V6, V0, V7, V0, V4, V6)
  *   Sector 2: 60°~120°  (V6, V2, V0, V7, V0, V6, V2)
  *   Sector 3: 120°~180° (V2, V3, V0, V7, V0, V2, V3)
  *   Sector 4: 180°~240° (V3, V1, V0, V7, V0, V3, V1)
  *   Sector 5: 240°~300° (V1, V5, V0, V7, V0, V1, V5)
  *   Sector 6: 300°~360° (V5, V4, V0, V7, V0, V5, V4)
  */
void SVPWM_SevenSegment(float u_alpha, float u_beta,
                        float *d_u, float *d_v, float *d_w)
{
    const float ts = 1.0f;  /* 周期归一化 */

    /* 1. 扇区判断信号 */
    float u1 = u_beta;
    float u2 = -SQRT3_2 * u_alpha - 0.5f * u_beta;
    float u3 = SQRT3_2 * u_alpha - 0.5f * u_beta;

    /* 2. 计算扇区编号（0~5） */
    uint8_t sector = (u1 > 0.0f) + ((u2 > 0.0f) << 1) + ((u3 > 0.0f) << 2);

    float t_a, t_b, t_c;
    float k_svpwm;

    /* 3. 根据扇区计算 PWM 时间 */
    switch (sector) {
        case 5:  /* Sector 1: V4, V6, V0, V7, V0, V4, V6 */
        {
            float t4 = u3;
            float t6 = u1;
            float sum = t4 + t6;
            if (sum > ts) {
                k_svpwm = ts / sum;
                t4 *= k_svpwm;
                t6 *= k_svpwm;
            }
            float t7 = (ts - t4 - t6) / 2.0f;
            t_a = t4 + t6 + t7;
            t_b = t6 + t7;
            t_c = t7;
            break;
        }
        case 1:  /* Sector 2: V6, V2, V0, V7, V0, V6, V2 */
        {
            float t2 = -u3;
            float t6 = -u2;
            float sum = t2 + t6;
            if (sum > ts) {
                k_svpwm = ts / sum;
                t2 *= k_svpwm;
                t6 *= k_svpwm;
            }
            float t7 = (ts - t2 - t6) / 2.0f;
            t_a = t6 + t7;
            t_b = t2 + t6 + t7;
            t_c = t7;
            break;
        }
        case 3:  /* Sector 3: V2, V3, V0, V7, V0, V2, V3 */
        {
            float t2 = u1;
            float t3 = u2;
            float sum = t2 + t3;
            if (sum > ts) {
                k_svpwm = ts / sum;
                t2 *= k_svpwm;
                t3 *= k_svpwm;
            }
            float t7 = (ts - t2 - t3) / 2.0f;
            t_a = t7;
            t_b = t2 + t3 + t7;
            t_c = t3 + t7;
            break;
        }
        case 2:  /* Sector 4: V3, V1, V0, V7, V0, V3, V1 */
        {
            float t1 = -u1;
            float t3 = -u3;
            float sum = t1 + t3;
            if (sum > ts) {
                k_svpwm = ts / sum;
                t1 *= k_svpwm;
                t3 *= k_svpwm;
            }
            float t7 = (ts - t1 - t3) / 2.0f;
            t_a = t7;
            t_b = t3 + t7;
            t_c = t1 + t3 + t7;
            break;
        }
        case 6:  /* Sector 5: V1, V5, V0, V7, V0, V1, V5 */
        {
            float t1 = u2;
            float t5 = u3;
            float sum = t1 + t5;
            if (sum > ts) {
                k_svpwm = ts / sum;
                t1 *= k_svpwm;
                t5 *= k_svpwm;
            }
            float t7 = (ts - t1 - t5) / 2.0f;
            t_a = t5 + t7;
            t_b = t7;
            t_c = t1 + t5 + t7;
            break;
        }
        case 4:  /* Sector 6: V5, V4, V0, V7, V0, V5, V4 */
        {
            float t4 = -u2;
            float t5 = -u1;
            float sum = t4 + t5;
            if (sum > ts) {
                k_svpwm = ts / sum;
                t4 *= k_svpwm;
                t5 *= k_svpwm;
            }
            float t7 = (ts - t4 - t5) / 2.0f;
            t_a = t4 + t5 + t7;
            t_b = t7;
            t_c = t5 + t7;
            break;
        }
        default:  /* 扇区 0：零矢量 */
        {
            t_a = 0.5f;
            t_b = 0.5f;
            t_c = 0.5f;
            break;
        }
    }

    /* 4. 输出归一化占空比（0~1） */
    *d_u = t_a;
    *d_v = t_b;
    *d_w = t_c;
}

/**
  * @brief  SVPWM 马鞍波生成（简化版，用于 VF 开环）
  * @note   零序注入法，直接输出电压值并计算 CCR
  */
void SVPWM_SaddleWave(float u_alpha, float u_beta, float vbus,
                      uint32_t *duty_a, uint32_t *duty_b, uint32_t *duty_c)
{
    /* 1. 逆 Clarke 变换 */
    float Ua = u_alpha;
    float Ub = -0.5f * u_alpha + SQRT3_2 * u_beta;
    float Uc = -0.5f * u_alpha - SQRT3_2 * u_beta;

    /* 2. 计算零序注入量（马鞍波） */
    float Umax = (Ua > Ub) ? ((Ua > Uc) ? Ua : Uc) : ((Ub > Uc) ? Ub : Uc);
    float Umin = (Ua < Ub) ? ((Ua < Uc) ? Ua : Uc) : ((Ub < Uc) ? Ub : Uc);
    float U0 = -0.5f * (Umax + Umin);

    /* 3. 计算最终占空比（归一化到 0~1） */
    float duty_a_f = (-(U0 + Ua) / vbus + 0.5f);
    float duty_b_f = (-(U0 + Ub) / vbus + 0.5f);
    float duty_c_f = (-(U0 + Uc) / vbus + 0.5f);

    /* 4. 限幅到 0~1 */
    if (duty_a_f > 1.0f) duty_a_f = 1.0f;
    if (duty_a_f < 0.0f) duty_a_f = 0.0f;
    if (duty_b_f > 1.0f) duty_b_f = 1.0f;
    if (duty_b_f < 0.0f) duty_b_f = 0.0f;
    if (duty_c_f > 1.0f) duty_c_f = 1.0f;
    if (duty_c_f < 0.0f) duty_c_f = 0.0f;

    /* 5. 转换为 CCR 值（0~PWM_PERIOD）*/
    *duty_a = (uint32_t)(duty_a_f * PWM_PERIOD);
    *duty_b = (uint32_t)(duty_b_f * PWM_PERIOD);
    *duty_c = (uint32_t)(duty_c_f * PWM_PERIOD);
}

/**
  * @brief  直接设置三相 PWM 占空比
  */
void SVPWM_SetDuty(uint32_t duty_a, uint32_t duty_b, uint32_t duty_c)
{
    /* 限幅 */
    if (duty_a > (uint32_t)PWM_PERIOD) duty_a = (uint32_t)PWM_PERIOD;
    if (duty_b > (uint32_t)PWM_PERIOD) duty_b = (uint32_t)PWM_PERIOD;
    if (duty_c > (uint32_t)PWM_PERIOD) duty_c = (uint32_t)PWM_PERIOD;

    /* 设置 CCR（注意相序：CH1=W, CH2=V, CH3=U）*/
    TIM1->CCR3 = duty_a;  /* U 相 → CH3 */
    TIM1->CCR2 = duty_b;  /* V 相 → CH2 */
    TIM1->CCR1 = duty_c;  /* W 相 → CH1 */
}

/**
  * @brief  紧急停止（所有 PWM 清零）
  */
void SVPWM_EmergencyStop(void)
{
    TIM1->CCR1 = 0;
    TIM1->CCR2 = 0;
    TIM1->CCR3 = 0;
}
