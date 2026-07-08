/**
  ******************************************************************************
  * @file    Flux_Observer.h
  * @brief   磁链观测器头文件
  * @version 1.0.0
  * @date    2026-07-08
  ******************************************************************************
  */
#ifndef __FLUX_OBSERVER_H__
#define __FLUX_OBSERVER_H__

#include <stdint.h>
#include <stdbool.h>
#include "MiniFOC_Config.h"

/* ========== 磁链观测器句柄 ========== */
typedef struct {
    /* 电机参数 */
    float Rs;                  /* 相电阻 (Ω) */
    float Ld;                  /* D轴电感 (H) */
    float Lq;                  /* Q轴电感 (H) */
    float psi_f;               /* 磁链 (Wb) */
    float sigma;               /* 漏磁系数 */

    /* 观测结果 */
    float theta_mech;          /* 机械角度 (rad) */
    float theta_elec;          /* 电角度 (rad) */
    float speed_mech;          /* 机械转速 (rpm) */
    float speed_elec;          /* 电角速度 (rad/s) */

    /* 磁链 */
    float psi_alpha;           /* α轴磁链 */
    float psi_beta;            /* β轴磁链 */
    float psi_alpha_filt;      /* α轴磁链（滤波后）*/
    float psi_beta_filt;       /* β轴磁链（滤波后）*/

    /* 内部状态 */
    float I_alpha;             /* α轴电流 */
    float I_beta;              /* β轴电流 */
    float U_alpha;             /* α轴电压 */
    float U_beta;              /* β轴电压 */

    /* 观测器参数 */
    float K;                   /* 观测器增益 */
    float lpf_alpha;           /* 低通滤波系数 */
} Flux_Observer_t;

/* ========== 函数声明 ========== */

/**
  * @brief  磁链观测器初始化
  * @param  flux: 磁链观测器句柄
  * @retval None
  */
void Flux_Observer_Init(Flux_Observer_t *flux);

/**
  * @brief  磁链观测器主循环
  * @param  flux: 磁链观测器句柄
  * @param  I_alpha: α轴电流 (A)
  * @param  I_beta: β轴电流 (A)
  * @param  U_alpha: α轴电压 (V)
  * @param  U_beta: β轴电压 (V)
  * @param  T: 采样周期 (s)
  * @retval None
  */
void Flux_Observer_Loop(Flux_Observer_t *flux,
                        float I_alpha, float I_beta,
                        float U_alpha, float U_beta,
                        float T);

/**
  * @brief  获取电角度
  * @param  flux: 磁链观测器句柄
  * @retval 电角度 (rad)
  */
float Flux_Observer_GetElecAngle(Flux_Observer_t *flux);

/**
  * @brief  获取机械角度
  * @param  flux: 磁链观测器句柄
  * @retval 机械角度 (rad)
  */
float Flux_Observer_GetMechAngle(Flux_Observer_t *flux);

/**
  * @brief  获取转速
  * @param  flux: 磁链观测器句柄
  * @retval 转速 (rpm)
  */
float Flux_Observer_GetSpeed(Flux_Observer_t *flux);

#endif /* __FLUX_OBSERVER_H__ */
