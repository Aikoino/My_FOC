/**
  ******************************************************************************
  * @file    Flux_Observer.c
  * @brief   磁链观测器实现（参考曹志洋项目）
  * @version 1.0.0
  * @date    2026-07-08
  *
  * 功能：基于电机磁链方程估算转子位置
  * 适用：低速霍尔 + 高速无感混合控制
  ******************************************************************************
  */
#include "Flux_Observer.h"
#include "MiniFOC_math.h"
#include <math.h>

/**
  * @brief  磁链观测器初始化
  * @param  flux: 磁链观测器句柄
  * @retval None
  */
void Flux_Observer_Init(Flux_Observer_t *flux)
{
    /* 参数初始化 */
    flux->Rs = MOTOR_PHASE_RESISTANCE;       /* 相电阻 (Ω) */
    flux->Ld = MOTOR_D_AXIS_INDUCTANCE;      /* D轴电感 (H) */
    flux->Lq = MOTOR_Q_AXIS_INDUCTANCE;      /* Q轴电感 (H) */
    flux->psi_f = MOTOR_FLUX_LINKAGE;        /* 磁链 (Wb) */

    /* 计算电机参数 */
    flux->sigma = 1.0f - (flux->Ld * flux->Lq) / (flux->Ld + flux->Lq) / (flux->Ld + flux->Lq);

    /* 状态变量清零 */
    flux->theta_mech = 0.0f;          /* 机械角度 */
    flux->theta_elec = 0.0f;          /* 电角度 */
    flux->speed_mech = 0.0f;          /* 机械转速 (rpm) */
    flux->speed_elec = 0.0f;          /* 电角速度 (rad/s) */

    /* αβ轴磁链 */
    flux->psi_alpha = 0.0f;
    flux->psi_beta = 0.0f;

    /* 内部状态 */
    flux->I_alpha = 0.0f;
    flux->I_beta = 0.0f;
    flux->U_alpha = 0.0f;
    flux->U_beta = 0.0f;

    /* 观测器增益 */
    flux->K = 10.0f;                  /* 观测器增益（可调） */

    /* 低通滤波器 */
    flux->lpf_alpha = 0.1f;           /* 滤波系数（可调） */
    flux->psi_alpha_filt = 0.0f;
    flux->psi_beta_filt = 0.0f;
}

/**
  * @brief  磁链观测器主循环
  * @param  flux: 磁链观测器句柄
  * @param  I_alpha: α轴电流 (A)
  * @param  I_beta: β轴电流 (A)
  * @param  U_alpha: α轴电压 (V)
  * @param  U_beta: β轴电压 (V)
  * @param  T: 采样周期 (s)
  * @retval None
  *
  * 算法原理：
  * 1. 磁链观测：ψ = ∫(U - Rs*I) dt
  * 2. 位置提取：theta = atan2(-ψβ, ψα)
  * 3. 低通滤波平滑磁链
  */
void Flux_Observer_Loop(Flux_Observer_t *flux,
                        float I_alpha, float I_beta,
                        float U_alpha, float U_beta,
                        float T)
{
    /* 保存输入 */
    flux->I_alpha = I_alpha;
    flux->I_beta = I_beta;
    flux->U_alpha = U_alpha;
    flux->U_beta = U_beta;

    /* ========== 1. 磁链观测（积分法）========== */
    /* 磁链微分方程：dψ/dt = U - Rs*I - ψ/L */
    /* 简化版：dψ/dt = U - Rs*I（忽略电感压降）*/

    float d_psi_alpha = U_alpha - flux->Rs * I_alpha;
    float d_psi_beta  = U_beta  - flux->Rs * I_beta;

    /* 数值积分（欧拉法）*/
    flux->psi_alpha += d_psi_alpha * T;
    flux->psi_beta  += d_psi_beta  * T;

    /* ========== 2. 低通滤波（平滑磁链）========== */
    flux->psi_alpha_filt = flux->lpf_alpha * flux->psi_alpha +
                           (1.0f - flux->lpf_alpha) * flux->psi_alpha_filt;
    flux->psi_beta_filt  = flux->lpf_alpha * flux->psi_beta +
                           (1.0f - flux->lpf_alpha) * flux->psi_beta_filt;

    /* ========== 3. 位置提取 ========== */
    /* 电角度 = atan2(-ψβ, ψα) */
    float theta_elec = atan2f(-flux->psi_beta_filt, flux->psi_alpha_filt);

    /* 归一化到 [0, 2π) */
    if (theta_elec < 0.0f) {
        theta_elec += 2.0f * M_PI;
    }

    /* ========== 4. 速度计算 ========== */
    float delta_theta = theta_elec - flux->theta_elec;

    /* 角度跳变检测（处理 0/2π 跳变）*/
    if (delta_theta > M_PI) {
        delta_theta -= 2.0f * M_PI;
    } else if (delta_theta < -M_PI) {
        delta_theta += 2.0f * M_PI;
    }

    /* 电角速度 (rad/s) */
    flux->speed_elec = delta_theta / T;

    /* 机械转速 (rpm) */
    flux->speed_mech = flux->speed_elec * 60.0f / (2.0f * M_PI * MOTOR_POLE_PAIRS);

    /* ========== 5. 更新状态 ========== */
    flux->theta_elec = theta_elec;
    flux->theta_mech = theta_elec / MOTOR_POLE_PAIRS;
}

/**
  * @brief  获取电角度
  * @param  flux: 磁链观测器句柄
  * @retval 电角度 (rad)
  */
float Flux_Observer_GetElecAngle(Flux_Observer_t *flux)
{
    return flux->theta_elec;
}

/**
  * @brief  获取机械角度
  * @param  flux: 磁链观测器句柄
  * @retval 机械角度 (rad)
  */
float Flux_Observer_GetMechAngle(Flux_Observer_t *flux)
{
    return flux->theta_mech;
}

/**
  * @brief  获取转速
  * @param  flux: 磁链观测器句柄
  * @retval 转速 (rpm)
  */
float Flux_Observer_GetSpeed(Flux_Observer_t *flux)
{
    return flux->speed_mech;
}
