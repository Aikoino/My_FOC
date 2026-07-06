/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : MiniFOC_Transform.h
 * @brief          : MiniFOC 坐标变换库（对齐 SguanFOC v3.1.0）
 * @description    : 包含 Clarke/Park 正逆变换、快速三角函数
 * @dependencies   : 无
 * @note           : 完全对齐 SguanFOC_Library-main/SguanFOC库v3.1.0
 ******************************************************************************
 * @attention
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 ******************************************************************************
 */
/* USER CODE END Header */

#ifndef __MINIFOC_TRANSFORM_H__
#define __MINIFOC_TRANSFORM_H__

#include <stdint.h>
#include <stdbool.h>

/* ========== 常量定义（对齐 SguanFOC）========== */

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#define TWO_PI_F             6.283185307179586f      /* 2π */
#define INV_SQRT3_F          0.5773502691896257f     /* 1/√3 */
#define SQRT3_2_F            0.8660254037844386f     /* √3/2 */
#define INV_SQRT2_F          0.7071067811865475f     /* 1/√2 */
#define PI_2_F               1.570796326794896f      /* π/2 */

/* ========== 基础数学函数 ========== */

/**
 * @brief  数值限幅
 * @param  val: 待限幅值
 * @param  max: 上限
 * @param  min: 下限
 * @retval 限幅后的值
 */
static inline float Limit_float(float val, float max, float min)
{
    if (val > max) return max;
    if (val < min) return min;
    return val;
}

/**
 * @brief  数值绝对值
 * @param  x: 输入值
 * @retval 绝对值
 */
static inline float fabs_float(float x)
{
    union {
        float f;
        uint32_t i;
    } u;
    u.f = x;
    u.i &= 0x7FFFFFFF;
    return u.f;
}

/**
 * @brief  角度归一化到 [0, 2π)
 * @param  angle: 输入角度（rad）
 * @retval 归一化后的角度
 */
static inline float normalize_angle(float angle)
{
    while (angle >= TWO_PI_F) angle -= TWO_PI_F;
    while (angle < 0.0f) angle += TWO_PI_F;
    return angle;
}

/**
 * @brief  角度插值（用于跟踪连续角度变化）
 * @param  Output: 输出累计角度
 * @param  Input: 当前输入角度
 * @param  Last_in: 上次输入角度
 * @param  T: 时间间隔
 */
static inline void angle_integrate(float *Output, float Input, float Last_in, float T)
{
    *Output += (Input + Last_in) * (T / 2.0f);
    *Output = normalize_angle(*Output);
}

/* ========== 坐标变换函数 ========== */

/**
 * @brief  Clarke 正变换（3相 → αβ 轴）
 * @note   将三相电流 Ia/Ib/Ic 转换为两相静止坐标系 Ialpha/Ibeta
 *         第三相电流 Ic = -(Ia + Ib)（无传感器时省略）
 * @param  i_alpha: 输出 α 轴电流
 * @param  i_beta: 输出 β 轴电流
 * @param  i_a: U 相电流（A）
 * @param  i_b: V 相电流（A）
 * @retval 无
 *
 * @formula:
 *   Ialpha = Ia
 *   Ibeta = (Ia + 2*Ib) / √3
 */
void clarke(float *i_alpha, float *i_beta, float i_a, float i_b);

/**
 * @brief  Park 正变换（αβ 轴 → dq 轴）
 * @note   将两相静止坐标系转换为两相旋转坐标系（D/Q 轴）
 *         需要电角度 sine/cosine 值
 * @param  i_d: 输出 D 轴电流（A）
 * @param  i_q: 输出 Q 轴电流（A）
 * @param  i_alpha: α 轴电流（A）
 * @param  i_beta: β 轴电流（A）
 * @param  sine: 电角度 sin 值
 * @param  cosine: 电角度 cos 值
 * @retval 无
 *
 * @formula:
 *   Id = Ialpha * cos(θ) + Ibeta * sin(θ)
 *   Iq = Ibeta * cos(θ) - Ialpha * sin(θ)
 */
void park(float *i_d, float *i_q, float i_alpha, float i_beta, float sine, float cosine);

/**
 * @brief  Park 逆变换（dq 轴 → αβ 轴）
 * @note   将两相旋转坐标系转换回两相静止坐标系
 * @param  u_alpha: 输出 α 轴电压（V）
 * @param  u_beta: 输出 β 轴电压（V）
 * @param  u_d: D 轴电压（V）
 * @param  u_q: Q 轴电压（V）
 * @param  sine: 电角度 sin 值
 * @param  cosine: 电角度 cos 值
 * @retval 无
 *
 * @formula:
 *   Ualpha = Ud * cos(θ) - Uq * sin(θ)
 *   Ubeta = Uq * cos(θ) + Ud * sin(θ)
 */
void ipark(float *u_alpha, float *u_beta, float u_d, float u_q, float sine, float cosine);

/**
 * @brief  Clarke 逆变换（αβ 轴 → 3相）
 * @note   将两相静止坐标系转换回三相电压 Ua/Ub/Uc
 * @param  u_a: 输出 U 相电压（V）
 * @param  u_b: 输出 V 相电压（V）
 * @param  u_c: 输出 W 相电压（V）
 * @param  u_alpha: α 轴电压（V）
 * @param  u_beta: β 轴电压（V）
 * @retval 无
 *
 * @formula:
 *   Ua = Ualpha
 *   Ub = -0.5 * Ualpha + (√3/2) * Ubeta
 *   Uc = -0.5 * Ualpha - (√3/2) * Ubeta
 */
void iclarke(float *u_a, float *u_b, float *u_c, float u_alpha, float u_beta);

/**
 * @brief  快速计算 sin 和 cos（查表法，对齐 SguanFOC）
 * @note   使用 512 点查找表，精度约 0.012°（约 1LSB@12bit ADC）
 *         比标准库 sinf/cosf 快 5-10 倍
 * @param  x: 输入角度（rad），自动归一化到 [0, 2π)
 * @param  sin_x: 输出 sin(x)
 * @param  cos_x: 输出 cos(x)
 * @retval 无
 */
void fast_sin_cos(float x, float *sin_x, float *cos_x);

/**
 * @brief  快速正弦函数（查表法，仅返回 sin）
 * @note   对齐 SguanFOC v3.1.0 Sguan_math.c:257
 */
float fast_sin(float theta);

/**
 * @brief  快速余弦函数（查表法）
 * @note   cos(x) = sin(x + π/2)
 */
static inline float fast_cos(float x)
{
    return fast_sin(PI_2_F - x);
}

/* ========== 单位转换函数 ========== */

/**
 * @brief  机械角度 → 电角度
 * @note   根据电机极对数转换：θ_elec = θ_mech * pole_pairs
 * @param  mech_angle: 机械角度（rad）
 * @param  pole_pairs: 电机极对数
 * @retval 电角度（rad）
 */
static inline float mech_to_elec_angle(float mech_angle, uint8_t pole_pairs)
{
    return mech_angle * (float)pole_pairs;
}

/**
 * @brief  电角度 → 机械角度
 * @param  elec_angle: 电角度（rad）
 * @param  pole_pairs: 电机极对数
 * @retval 机械角度（rad）
 */
static inline float elec_to_mech_angle(float elec_angle, uint8_t pole_pairs)
{
    return elec_angle / (float)pole_pairs;
}

/**
 * @brief  转速（rpm）→ 电频率（Hz）
 * @param  speed_rpm: 机械转速（rpm）
 * @param  pole_pairs: 电机极对数
 * @retval 电频率（Hz）
 */
static inline float speed_rpm_to_freq(float speed_rpm, uint8_t pole_pairs)
{
    return speed_rpm * (float)pole_pairs / 60.0f;
}

/**
 * @brief  电频率（Hz）→ 转速（rpm）
 * @param  freq_hz: 电频率（Hz）
 * @param  pole_pairs: 电机极对数
 * @retval 机械转速（rpm）
 */
static inline float freq_to_speed_rpm(float freq_hz, uint8_t pole_pairs)
{
    return freq_hz * 60.0f / (float)pole_pairs;
}

/**
 * @brief  角速度（rad/s）→ 频率（Hz）
 * @param  omega: 角速度（rad/s）
 * @retval 频率（Hz）
 */
static inline float rad_s_to_hz(float omega)
{
    return omega / TWO_PI_F;
}

/**
 * @brief  频率（Hz）→ 角速度（rad/s）
 * @param  freq: 频率（Hz）
 * @retval 角速度（rad/s）
 */
static inline float hz_to_rad_s(float freq)
{
    return freq * TWO_PI_F;
}

#endif /* __MINIFOC_TRANSFORM_H__ */
