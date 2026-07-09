/**
  ******************************************************************************
  * @file           : MiniFOC_SMO.h
  * @brief          : MiniFOC 无感SMO滑模观测器 + PLL锁相环
  * @version        : 1.0.0
  * @description    : 基于SguanFOC v3.1.0 SMO+PLL算法提取，全部改为MiniFOC命名
  *
  * 依赖:
  *   MiniFOC_Transform.h (fast_sin/fast_cos, normalize_angle)
  *   MiniFOC_Config.h    (电机参数宏定义)
  *
  * 使用流程:
  *   1. MiniFOC_SMO_Init()     — 初始化SMO+PLL(在MiniFOC_Init中调用)
  *   2. MiniFOC_SMO_Loop()     — 每个PWM周期调用(喂入Iα/Iβ, Uα/Uβ)
  *   3. MiniFOC_PLL_Loop()     — 每个PWM周期调用(从SMO反电动势提取角度速度)
  *   4. MiniFOC_GetAngle()     — 获取当前电角度
  *   5. MiniFOC_GetSpeed()     — 获取当前电角速度
  *
  * 启动流程:
  *   IF强拖(虚拟角度) → SMO+PLL锁定 → 切换闭环
  ******************************************************************************
  */
#ifndef __MINIFOC_SMO_H__
#define __MINIFOC_SMO_H__

#include <stdint.h>
#include <stdbool.h>
#include "MiniFOC_math.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== SMO 单轴观测器结构体 (α或β轴) ========== */
typedef struct {
    float I_i;              /* 积分项历史输入 */
    float F_i;              /* 滤波项历史输入 */
    float F_o;              /* 滤波项历史输出 */

    float Input_Ix;         /* x轴电流输入 (实际值) */
    float Input_Ux;         /* x轴电压输入 (实际值) */
    float Input_Iy;         /* y轴电流输入 (用于交叉耦合) */
    float Output_Ex;        /* x轴反电动势输出 (预测值) */
    float Output_Ix;        /* x轴电流输出 (预测值) */

    uint8_t IntegralFrozen_flag; /* 积分抗饱和标志 */
} MiniFOC_SMO_Axis_t;

/* ========== SMO 观测器主结构体 ========== */
typedef struct {
    MiniFOC_SMO_Axis_t alpha;   /* α轴数据 */
    MiniFOC_SMO_Axis_t beta;    /* β轴数据 */

    float T;                    /* 离散周期 (s) */
    float Rs;                   /* 相电阻 (Ω) */
    float Ld;                   /* D轴电感 (H) */
    float Lq;                   /* Q轴电感 (H) */

    float Wc;                   /* 低通滤波截止频率 (rad/s) */
    float h;                    /* 滑模解调增益 */

    float IntMax;               /* 积分项上限 */
    float IntMin;               /* 积分项下限 */

    /* ====== 预计算系数 (Init时自动计算) ====== */
    float I_num;                /* 积分项传递函数分子 */
    float F_num;                /* 滤波项传递函数分子 */
    float F_den;                /* 滤波项传递函数分母 */
    float Gain0;                /* 滑模增益 1/Ld */
    float Gain1;                /* 滑模增益 Rs/Ld */
    float Gain2;                /* 滑模增益 (Ld-Lq)/Ld */

    float Input_We;             /* 电机电角速度输入 (由PLL反馈) */
} MiniFOC_SMO_Controller_t;

/* ========== PLL 锁相环结构体 ========== */
typedef struct {
    float We_i;                 /* 速度历史输入 */
    float Re_i;                 /* 角度历史输入 */
    float X_num[2];             /* PI传递函数分子系数 */
    float Y_num;                /* 积分项分母系数 */

    float Error;                 /* 角度误差输入 */
    float OutWe;                /* 输出电角速度 (rad/s) */
    float OutRe;                /* 输出电角度 (rad, 0~2π) */
} MiniFOC_PLL_Internal_t;

typedef struct {
    MiniFOC_PLL_Internal_t go;

    float T;                    /* 离散周期 (s) */
    float Kp;                   /* 比例增益 */
    float Ki;                   /* 积分增益 */
} MiniFOC_PLL_Controller_t;

/* ========== 低通滤波器结构体 (二阶巴特沃斯) ========== */
typedef struct {
    float Input;                /* 输入 */
    float Output;               /* 输出 */
    float i[2];                 /* 历史输入 [n-1, n-2] */
    float o[2];                 /* 历史输出 [n-1, n-2] */
    float a[3];                 /* 分子系数 b0,b1,b2 */
    float b[2];                 /* 分母系数 a1,a2 */
} MiniFOC_LPF_t;

/* ========== 无感SMO状态机 ========== */
typedef enum {
    SENSORLESS_STATE_IF_STARTUP    = 0,  /* IF开环强拖 (虚拟角度) */
    SENSORLESS_STATE_SMO_LOCKING   = 1,  /* SMO+PLL锁定中 (角度融合过渡) */
    SENSORLESS_STATE_CLOSED_LOOP   = 2,  /* SMO闭环运行 */
} MiniFOC_SensorlessState_t;

/* ========== 无感SMO全局数据 ========== */
typedef struct {
    /* SMO+PLL */
    MiniFOC_SMO_Controller_t smo;
    MiniFOC_PLL_Controller_t pll;
    MiniFOC_LPF_t lpf_speed;

    /* 状态机 */
    MiniFOC_SensorlessState_t state;
    uint32_t state_transition_tick;     /* 状态切换计时 */

    /* IF开环启动 */
    float if_target_speed;              /* IF开环目标转速 (rpm) */
    float if_elec_angle;                /* IF虚拟电角度 (rad) */
    float if_iq_setpoint;               /* IF开环Iq电流 (A) */
    float if_we;                        /* IF开环角速度 (rad/s电角) */

    /* 角度融合 */
    float smo_gain;                     /* SMO权重 0~1 (0=纯IF, 1=纯SMO) */
    float low_speed_angle;              /* 低速域虚拟角度 */
    float low_speed_we;                 /* 低速域虚拟角速度 */
    float high_speed_angle;             /* 高速域SMO角度 */
    float high_speed_we;                /* 高速域SMO角速度 */

    /* 切换阈值 (rpm 机械转速) */
    float switch_speed_min;             /* 开始切换到SMO的最低转速 */
    float switch_speed_max;             /* 完全切到SMO的转速 */
    float switch_speed_hyst;            /* 切换滞环 */

    /* 上一拍电压 (用于SMO迭代) */
    float prev_Ualpha;
    float prev_Ubeta;
} MiniFOC_Sensorless_t;

/* ========== 全局变量声明 ========== */
extern MiniFOC_Sensorless_t sensorless;

/* ========== 函数声明 ========== */

/* --- SMO 观测器 --- */
void MiniFOC_SMO_Init(MiniFOC_SMO_Controller_t *smo, float T, float Rs, float Ld, float Lq);
void MiniFOC_SMO_Loop(MiniFOC_SMO_Controller_t *smo);

/* --- PLL 锁相环 --- */
void MiniFOC_PLL_Init(MiniFOC_PLL_Controller_t *pll, float T, float Kp, float Ki);
void MiniFOC_PLL_Loop(MiniFOC_PLL_Controller_t *pll, float error, uint8_t position_mode);

/* --- 低通滤波器 --- */
void MiniFOC_LPF_Init(MiniFOC_LPF_t *lpf, float T, float Wc);
float MiniFOC_LPF_Update(MiniFOC_LPF_t *lpf, float input);

/* --- 无感SMO主控 --- */
void MiniFOC_Sensorless_Init(void);
void MiniFOC_Sensorless_Loop(float I_alpha, float I_beta, float U_alpha, float U_beta);
float MiniFOC_Sensorless_GetAngle(void);
float MiniFOC_Sensorless_GetSpeed(void);
float MiniFOC_Sensorless_GetWe(void);

#ifdef __cplusplus
}
#endif

#endif /* __MINIFOC_SMO_H__ */
