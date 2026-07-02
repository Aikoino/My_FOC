/**
  ******************************************************************************
  * @file           : MiniFOC_math.h
  * @brief          : MiniFOC 数学运算库 - 坐标变换
  * @attention
  *
  * 包含：
  * - Clark变换（abc → αβ）
  * - Park变换（αβ → dq）
  * - 逆Park变换（dq → αβ）
  * - SVPWM生成
  * - 三角函数、限幅等工具函数
  *
  ******************************************************************************
  */
#ifndef __MINIFOC_MATH_H__
#define __MINIFOC_MATH_H__

#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 常量定义 ========== */
#ifndef M_PI
#define M_PI  3.14159265358979323846f
#endif

#define TWO_PI       (2.0f * M_PI)
#define HALF_PI      (0.5f * M_PI)
#define SQRT3        1.73205080757f
#define SQRT3_INV    0.57735026919f

/* ========== Clark变换 (abc → αβ) ========== */

/**
  * @brief  Clark变换 (Irvine公式)
  * @param  ia: U相电流
  * @param  ib: V相电流
  * @param  ic: W相电流
  * @param  ialpha: α轴电流输出
  * @param  ibeta: β轴电流输出
  */
static inline void Clark_Transform(float ia, float ib, float ic,
                                   float *ialpha, float *ibeta)
{
    *ialpha = ia;
    *ibeta = (ia + 2.0f * ib) * SQRT3_INV;
}

/**
  * @brief  Clark逆变换 (αβ → abc)
  */
static inline void Clark_Inv_Transform(float ialpha, float ibeta,
                                       float *ia, float *ib, float *ic)
{
    *ia = ialpha;
    *ib = -0.5f * ialpha + SQRT3_INV * ibeta;
    *ic = -0.5f * ialpha - SQRT3_INV * ibeta;
}

/* ========== Park变换 (αβ → dq) ========== */

/**
  * @brief  Park变换
  * @param  ialpha: α轴电流
  * @param  ibeta: β轴电流
  * @param  theta: 转子电角度 (rad)
  * @param  id: d轴电流输出
  * @param  iq: q轴电流输出
  */
static inline void Park_Transform(float ialpha, float ibeta, float theta,
                                  float *id, float *iq)
{
    float cos_theta = cosf(theta);
    float sin_theta = sinf(theta);

    *id =  ialpha * cos_theta + ibeta * sin_theta;
    *iq = -ialpha * sin_theta + ibeta * cos_theta;
}

/**
  * @brief  逆Park变换 (dq → αβ)
  */
static inline void Park_Inv_Transform(float id, float iq, float theta,
                                      float *ialpha, float *ibeta)
{
    float cos_theta = cosf(theta);
    float sin_theta = sinf(theta);

    *ialpha =  id * cos_theta - iq * sin_theta;
    *ibeta  =  id * sin_theta + iq * cos_theta;
}

/* ========== SVPWM 生成 ========== */

/**
  * @brief  SVPWM生成 - 计算三相PWM占空比
  * @param  ualpha: α轴电压
  * @param  ubeta: β轴电压
  * @param  pwm_a: A相占空比输出 (0~1)
  * @param  pwm_b: B相占空比输出 (0~1)
  * @param  pwm_c: C相占空比输出 (0~1)
  * @note   输出范围已经限幅在 [0, 1]
  */
static inline void SVPWM_Generate(float ualpha, float ubeta,
                                  float *pwm_a, float *pwm_b, float *pwm_c)
{
    float u1 = ubeta;
    float u2 = (-0.5f * ualpha + SQRT3_INV * ubeta);
    float u3 = (-0.5f * ualpha - SQRT3_INV * ubeta);

    // 扇区判断
    uint8_t sector = 0;
    if (u1 > 0) sector |= 1;
    if (u2 > 0) sector |= 2;
    if (u3 > 0) sector |= 4;

    float tx = 0, ty = 0, tz = 0;
    float t1 = 0, t2 = 0;

    switch (sector) {
        case 1:  // 扇区1
            t1 = u2;
            t2 = u1;
            break;
        case 2:  // 扇区2
            t1 = u1;
            t2 = -u3;
            break;
        case 3:  // 扇区3
            t1 = -u3;
            t2 = u2;
            break;
        case 4:  // 扇区4
            t1 = -u2;
            t2 = u3;
            break;
        case 5:  // 扇区5
            t1 = u3;
            t2 = -u1;
            break;
        case 6:  // 扇区6
            t1 = -u1;
            t2 = -u2;
            break;
        default:
            break;
    }

    // 计算PWM (0.5是死区补偿的一半)
    float half_pwm = 0.5f;

    *pwm_a = half_pwm + 0.5f * (t1 - t2);
    *pwm_b = half_pwm + 0.5f * (t1 + t2);
    *pwm_c = half_pwm - t1;

    // 限幅 [0, 1]
    if (*pwm_a > 1.0f) *pwm_a = 1.0f;
    if (*pwm_a < 0.0f) *pwm_a = 0.0f;
    if (*pwm_b > 1.0f) *pwm_b = 1.0f;
    if (*pwm_b < 0.0f) *pwm_b = 0.0f;
    if (*pwm_c > 1.0f) *pwm_c = 1.0f;
    if (*pwm_c < 0.0f) *pwm_c = 0.0f;
}

/* ========== 工具函数 ========== */

/**
  * @brief  角度归一化到 [0, 2π)
  */
static inline float Normalize_Angle(float angle)
{
    while (angle >= TWO_PI) angle -= TWO_PI;
    while (angle < 0.0f) angle += TWO_PI;
    return angle;
}

/**
  * @brief  角度归一化到 [-π, π)
  */
static inline float Normalize_Angle_PI(float angle)
{
    while (angle >= M_PI) angle -= TWO_PI;
    while (angle < -M_PI) angle += TWO_PI;
    return angle;
}

/**
  * @brief  限幅函数
  */
static inline float Limit(float value, float min_val, float max_val)
{
    if (value > max_val) return max_val;
    if (value < min_val) return min_val;
    return value;
}

/**
  * @brief  快速归一化 (使用查表法，比atan2快)
  * @param  sin_val: sin值
  * @param  cos_val: cos值
  * @retval 归一化角度 [0, 2π)
  */
float Fast_Normalize_Angle(float sin_val, float cos_val);

/**
  * @brief  线性插值
  */
static inline float Linear_Interpolate(float x, float x1, float y1, float x2, float y2)
{
    if (x2 == x1) return y1;
    return y1 + (x - x1) * (y2 - y1) / (x2 - x1);
}

#ifdef __cplusplus
}
#endif

#endif /* __MINIFOC_MATH_H__ */
