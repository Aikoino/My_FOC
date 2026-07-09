/**
  ******************************************************************************
  * @file           : MiniFOC_SMO.c
  * @brief          : MiniFOC 无感SMO滑模观测器 + PLL锁相环 + LPF低通滤波器
  * @version        : 1.0.0
  * @description    : 基于SguanFOC v3.1.0 提取，全改为MiniFOC命名
  *
  * 算法原理:
  *   SMO基于静止坐标系下的电机数学模型，通过滑模控制观测反电动势，
  *   再由PLL锁相环提取转子角度和速度。
  *
  * 参考:
  *   SguanFOC_Library-main/SguanFOC库v3.1.0/Sguan_SMO.png (Simulink原理图)
  *   SguanFOC_Library-main/SguanFOC库v3.1.0/Sguan_PLL.png (Simulink原理图)
  ******************************************************************************
  */
#include "MiniFOC_SMO.h"
#include "MiniFOC_Config.h"
#include "MiniFOC_Transform.h"
#include "stm32g4xx_hal.h"
#include <stdio.h>
#include <string.h>

/* ========== 局部宏定义（如果MiniFOC_Config.h中未定义）========== */
#ifndef CURRENT_LOOP_T
#define CURRENT_LOOP_T      0.00005f
#endif

#ifndef M_PI
#define M_PI                3.14159265358979323846f
#endif

#ifndef SMO_GAIN
#define SMO_GAIN            3.5f
#endif

#ifndef SMO_LPF_CUTOFF
#define SMO_LPF_CUTOFF      10000.0f
#endif

#ifndef SMO_PLL_KP
#define SMO_PLL_KP          150.0f
#endif

#ifndef SMO_PLL_KI
#define SMO_PLL_KI          80000.0f
#endif

/* ========== 全局变量 ========== */
MiniFOC_Sensorless_t sensorless;

/* ========== SMO 观测器实现 ========== */

/**
  * @brief  静止坐标系下单轴观测器运算 (α或β轴)
  * @param  smo: SMO控制器句柄
  * @param  axis: 轴数据 (alpha 或 beta)
  * @retval None
  */
static void MiniFOC_SMO_Run(MiniFOC_SMO_Controller_t *smo, MiniFOC_SMO_Axis_t *axis)
{
    float I_input, part0, part1, part2, part3;

    /* 电流观测器: di/dt = (1/Ld)*Ux - (Rs/Ld)*Ix - We*(Ld-Lq)/Ld*Iy - (1/Ld)*Ex */
    part0 = axis->Input_Ux * smo->Gain0;
    part1 = axis->Output_Ix * smo->Gain1;
    part2 = smo->Input_We * smo->Gain2 * axis->Input_Iy;
    part3 = axis->Output_Ex * smo->Gain0;
    I_input = part0 - part1 - part2 - part3;

    /* 积分电流预测 (带抗饱和) */
    if (axis->IntegralFrozen_flag) {
        /* 冻结中：检查是否可以解除 */
        if ((I_input * axis->Output_Ix < 0) ||
            ((axis->Output_Ix < smo->IntMax) && (axis->Output_Ix > smo->IntMin))) {
            axis->IntegralFrozen_flag = 0;
        }
    } else {
        /* 正常积分 (双线性变换) */
        axis->Output_Ix += smo->I_num * (I_input + axis->I_i);

        /* 限幅检测 → 冻结积分 */
        if (axis->Output_Ix > smo->IntMax) {
            axis->Output_Ix = smo->IntMax;
            axis->IntegralFrozen_flag = 1;
        } else if (axis->Output_Ix < smo->IntMin) {
            axis->Output_Ix = smo->IntMin;
            axis->IntegralFrozen_flag = 1;
        }
    }

    /* 滑模计算: 电流误差 → 反电动势 */
    float error = axis->Output_Ix - axis->Input_Ix;
    if (error > 1.0f) error = 1.0f;
    if (error < -1.0f) error = -1.0f;
    float Ex = error * smo->h;

    /* 低通滤波反电动势 (一阶) */
    axis->Output_Ex = smo->F_num * (Ex + axis->F_i) - smo->F_den * axis->F_o;

    /* 更新历史值 */
    axis->I_i = I_input;
    axis->F_i = Ex;
    axis->F_o = axis->Output_Ex;
}

/**
  * @brief  SMO 初始化
  * @param  smo: SMO句柄
  * @param  T: 离散周期 (s)
  * @param  Rs: 相电阻 (Ω)
  * @param  Ld: D轴电感 (H)
  * @param  Lq: Q轴电感 (H)
  * @retval None
  *
  * @note   预计算传递函数系数 (使用double提高精度)
  */
void MiniFOC_SMO_Init(MiniFOC_SMO_Controller_t *smo, float T, float Rs, float Ld, float Lq)
{
    memset(smo, 0, sizeof(MiniFOC_SMO_Controller_t));

    smo->T = T;
    smo->Rs = Rs;
    smo->Ld = Ld;
    smo->Lq = Lq;

    /* 使用默认参数 (可在之后覆盖) */
    smo->Wc = 10000.0f;     /* 反电动势滤波截止频率 10kHz */
    smo->h = 3.5f;          /* 滑模解调增益 */
    smo->IntMax = 1e10f;    /* 积分限幅 (极大, 实际不限) */
    smo->IntMin = -1e10f;

    /* 预计算系数 (double精度) */
    double temp0 = (double)(smo->T) * (double)(smo->Wc);
    double den = 2.0 + temp0;

    smo->I_num = (float)((double)(smo->T) / 2.0);
    smo->F_num = (float)(temp0 / den);
    smo->F_den = (float)((-2.0 + temp0) / den);

    smo->Gain0 = (float)(1.0 / (double)(smo->Ld));
    smo->Gain1 = (float)((double)(smo->Rs) / (double)(smo->Ld));
    smo->Gain2 = (float)(((double)(smo->Ld) - (double)(smo->Lq)) / (double)(smo->Ld));
}

/**
  * @brief  SMO 离散运行 (每个PWM周期调用)
  * @param  smo: SMO句柄
  * @retval None
  *
  * @note   调用前需设置:
  *   smo->alpha.Input_Ix = Ialpha
  *   smo->alpha.Input_Ux = Ualpha
  *   smo->beta.Input_Ix  = Ibeta
  *   smo->beta.Input_Ux  = Ubeta
  *   smo->data.Input_We  = PLL输出的电角速度
  */
void MiniFOC_SMO_Loop(MiniFOC_SMO_Controller_t *smo)
{
    MiniFOC_SMO_Run(smo, &smo->alpha);
    MiniFOC_SMO_Run(smo, &smo->beta);

    /* 交叉耦合: α轴Iy用β轴Output_Ix, β轴Iy用α轴Output_Ix */
    smo->alpha.Input_Iy = smo->beta.Output_Ix;
    smo->beta.Input_Iy = smo->alpha.Output_Ix;
}

/* ========== PLL 锁相环实现 ========== */

/**
  * @brief  PLL 初始化
  * @param  pll: PLL句柄
  * @param  T: 离散周期 (s)
  * @param  Kp: 比例增益
  * @param  Ki: 积分增益
  * @retval None
  */
void MiniFOC_PLL_Init(MiniFOC_PLL_Controller_t *pll, float T, float Kp, float Ki)
{
    memset(pll, 0, sizeof(MiniFOC_PLL_Controller_t));

    pll->T = T;
    pll->Kp = Kp;
    pll->Ki = Ki;

    /* 预计算PI系数 (双线性变换, double精度) */
    double temp0 = (double)pll->T * (double)pll->Ki;
    pll->go.X_num[0] = (float)((2.0 * (double)pll->Kp + temp0) / 2.0);
    pll->go.X_num[1] = (float)((-2.0 * (double)pll->Kp + temp0) / 2.0);
    pll->go.Y_num = (float)((double)pll->T / 2.0);
}

/**
  * @brief  PLL 离散运行
  * @param  pll: PLL句柄
  * @param  error: 角度误差输入 (rad)
  * @param  position_mode: 0=速度模式(角度归一化), 1=位置模式(连续角度)
  * @retval None
  *
  * @note   输出: pll->go.OutWe = 电角速度 (rad/s)
  *                pll->go.OutRe = 电角度 (rad, 0~2π)
  */
void MiniFOC_PLL_Loop(MiniFOC_PLL_Controller_t *pll, float error, uint8_t position_mode)
{
    /* 1. PI控制器 → 输出电角速度 */
    pll->go.Error = error;
    pll->go.OutWe += pll->go.X_num[0] * pll->go.Error +
                     pll->go.X_num[1] * pll->go.We_i;

    /* 限幅：防止PLL速度过大（±5000 rad/s）*/
    if (pll->go.OutWe > 5000.0f) pll->go.OutWe = 5000.0f;
    if (pll->go.OutWe < -5000.0f) pll->go.OutWe = -5000.0f;

    /* 2. 积分器 → 输出电角度 */
    pll->go.OutRe += pll->go.Y_num * (pll->go.OutWe + pll->go.Re_i);

    if (!position_mode) {
        /* 速度模式: 归一化到 [0, 2π) */
        while (pll->go.OutRe >= 6.283185307179586f) pll->go.OutRe -= 6.283185307179586f;
        while (pll->go.OutRe < 0.0f) pll->go.OutRe += 6.283185307179586f;
    }

    /* 3. 更新历史值 */
    pll->go.We_i = pll->go.Error;
    pll->go.Re_i = pll->go.OutWe;
}

/* ========== 低通滤波器实现 ========== */

/**
  * @brief  二阶巴特沃斯低通滤波器初始化
  * @param  lpf: 滤波器句柄
  * @param  T: 采样周期 (s)
  * @param  Wc: 截止频率 (rad/s)
  * @retval None
  */
void MiniFOC_LPF_Init(MiniFOC_LPF_t *lpf, float T, float Wc)
{
    memset(lpf, 0, sizeof(MiniFOC_LPF_t));

    /* 预计算巴特沃斯二阶系数 */
    double omega = (double)Wc;
    double ts = (double)T;
    double K = 2.0 * omega * ts;  /* 近似: 2*Wc*T / 4 归一化 */
    double K2 = K * K;
    double D = 4.0 + 2.0*K + K2;  /* 分母 */

    lpf->a[0] = (float)(K2 / D);           /* b0 */
    lpf->a[1] = (float)(2.0 * K2 / D);     /* b1 */
    lpf->a[2] = (float)(K2 / D);           /* b2 */
    lpf->b[0] = (float)((2.0*K2 - 8.0) / D);  /* -a1 */
    lpf->b[1] = (float)((4.0 - 2.0*K + K2) / D); /* -a2 */
}

/**
  * @brief  二阶巴特沃斯低通滤波器更新
  * @param  lpf: 滤波器句柄
  * @param  input: 输入值
  * @retval 滤波后的输出值
  */
float MiniFOC_LPF_Update(MiniFOC_LPF_t *lpf, float input)
{
    lpf->Input = input;

    /* 直接IIR结构: y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2] */
    float output = lpf->a[0] * input +
                   lpf->a[1] * lpf->i[0] +
                   lpf->a[2] * lpf->i[1] -
                   lpf->b[0] * lpf->o[0] -
                   lpf->b[1] * lpf->o[1];

    /* 更新历史值 */
    lpf->i[1] = lpf->i[0];
    lpf->i[0] = input;
    lpf->o[1] = lpf->o[0];
    lpf->o[0] = output;

    lpf->Output = output;
    return output;
}

/* ========== 无感SMO主控实现 ========== */

/**
  * @brief  无感SMO初始化
  * @retval None
  *
  * @note   在 MiniFOC_Init() 中调用
  */
void MiniFOC_Sensorless_Init(void)
{
    memset(&sensorless, 0, sizeof(MiniFOC_Sensorless_t));

    /* 初始化SMO */
    MiniFOC_SMO_Init(&sensorless.smo,
                     CURRENT_LOOP_T,           /* 离散周期 = 电流环周期 */
                     MOTOR_PHASE_RESISTANCE,   /* 相电阻 */
                     MOTOR_PHASE_INDUCTANCE,   /* D轴电感 (SPMSM: Ld=Lq) */
                     MOTOR_PHASE_INDUCTANCE);  /* Q轴电感 */

    /* 覆盖默认参数 (可从MiniFOC_Config.h读入) */
    sensorless.smo.Wc = SMO_LPF_CUTOFF;
    sensorless.smo.h = SMO_GAIN;

    /* 重新计算系数 (因为覆盖了Wc) */
    {
        double temp0 = (double)(sensorless.smo.T) * (double)(sensorless.smo.Wc);
        double den = 2.0 + temp0;
        sensorless.smo.F_num = (float)(temp0 / den);
        sensorless.smo.F_den = (float)((-2.0 + temp0) / den);
    }

    /* 初始化PLL */
    MiniFOC_PLL_Init(&sensorless.pll,
                     CURRENT_LOOP_T,
                     SMO_PLL_KP,
                     SMO_PLL_KI);

    /* 初始化速度低通滤波器 */
    MiniFOC_LPF_Init(&sensorless.lpf_speed,
                     CURRENT_LOOP_T,
                     100.0f);   /* 速度滤波截止频率 100 rad/s (~16Hz) */

    /* 状态机 */
    sensorless.state = SENSORLESS_STATE_IF_STARTUP;
    sensorless.if_target_speed = 0.0f;
    sensorless.if_elec_angle = 0.0f;
    sensorless.if_iq_setpoint = 2.0f;              /* IF启动电流 2A */
    sensorless.smo_gain = 0.0f;

    /* 切换阈值 (rad/s 机械角速度) */
    sensorless.switch_speed_min = 50.0f;            /* ~80 RPM (7极对) */
    sensorless.switch_speed_max = 100.0f;           /* ~140 RPM */
    sensorless.switch_speed_hyst = 20.0f;
    sensorless.prev_Ualpha = 0.0f;
    sensorless.prev_Ubeta = 0.0f;
}

/* ========== 辅助函数 ========== */

/**
  * @brief  根据速度计算增益（低速域→高速域的线性过渡）
  * @param  gain: 输出增益指针 (0~1)
  * @param  real_speed: 实际速度 (rpm)
  * @param  abs_max: 完全切换到SMO的速度阈值 (rpm)
  * @param  abs_min: 开始切换的速度阈值 (rpm)
  * @retval 速度绝对值
  *
  * @note   速度在 [abs_min, abs_max] 区间内线性过渡：
  *         gain = (|speed| - abs_min) / (abs_max - abs_min)
  */
void MiniFOC_Value_Gain_Get(float *gain, float real_speed, float abs_max, float abs_min)
{
    float abs_real = fabsf(real_speed);

    if (abs_real <= abs_min) {
        *gain = 0.0f;               /* 纯IF */
    } else if (abs_real >= abs_max) {
        *gain = 1.0f;               /* 纯SMO */
    } else {
        *gain = (abs_real - abs_min) / (abs_max - abs_min);  /* 线性过渡 */
    }
}

/**
  * @brief  角度积分（低速域虚拟角度生成）
  * @param  Output: 输出角度 (rad)
  * @param  Input: 输入角速度 (rpm)
  * @param  Last_in: 上次输入速度
  * @param  T: 采样周期 (s)
  * @retval None
  */
void MiniFOC_Value_Rad_Loop(float *Output, float Input, float Last_in, float T)
{
    /* 一阶积分：angle += (we_elec + we_last) * T / 2 */
    float we_elec = Input * (2.0f * M_PI * MOTOR_POLE_PAIRS / 60.0f);  /* rpm → rad/s电角 */
    *Output += (we_elec + Last_in * (2.0f * M_PI * MOTOR_POLE_PAIRS / 60.0f)) * T / 2.0f;

    /* 归一化到 [0, 2π) */
    while (*Output >= 2.0f * M_PI) *Output -= 2.0f * M_PI;
    while (*Output < 0.0f) *Output += 2.0f * M_PI;
}

/**
  * @brief  角度误差归一化到 [-π, π)
  * @param  error: 角度误差 (rad)
  * @retval 归一化后的误差
  */
static float Normalize_Error(float error)
{
    while (error > 3.14159265f) error -= 6.2831853f;
    while (error < -3.14159265f) error += 6.2831853f;
    return error;
}

/**
  * @brief  无感SMO主运算 (在ADC中断中调用)
  * @param  I_alpha: α轴电流 (A)
  * @param  I_beta: β轴电流 (A)
  * @param  U_alpha: α轴电压 (V)
  * @param  U_beta: β轴电压 (V)
  * @retval None
  *
  * @note   调用频率 = 电流环频率 (20kHz)
  *         每次调用内部执行SMO+PLL一步迭代
  */
void MiniFOC_Sensorless_Loop(float I_alpha, float I_beta,
                              float U_alpha, float U_beta)
{
    static uint32_t loop_cnt = 0;
    float sine, cosine, pll_error, pll_angle, current_speed;

    loop_cnt++;

    /* ========== 调试输出（每10000次，约0.5秒）========== */
    if (loop_cnt % 40000 == 0) {
        extern UART_HandleTypeDef huart3;
        char buf[256];
        int len = sprintf(buf, "[SMO_DEBUG] State=%d IF_Angle=%.2f IF_We=%.2f "
                               "PLL_Angle=%.2f PLL_We=%.2f "
                               "SMO_Gain=%.3f LoopCnt=%lu\r\n",
                         sensorless.state,
                         sensorless.if_elec_angle * 180.0f / M_PI,
                         sensorless.if_we,
                         sensorless.pll.go.OutRe * 180.0f / M_PI,
                         sensorless.pll.go.OutWe,
                         sensorless.smo_gain,
                         loop_cnt);
        HAL_UART_Transmit(&huart3, (uint8_t*)buf, len, 10);
    }

    /* ========== 状态机处理 ========== */
    switch (sensorless.state) {
        case SENSORLESS_STATE_IF_STARTUP:
            /* IF开环强拖：生成虚拟角度 */
            if (sensorless.if_target_speed > 0.0f) {
                /* IF角度递增（电角速度 = rpm → rad/s → 电角速度）*/
                sensorless.if_we = sensorless.if_target_speed * (2.0f * M_PI * MOTOR_POLE_PAIRS / 60.0f);
                sensorless.if_elec_angle += sensorless.if_we * CURRENT_LOOP_T;

                /* 归一化 */
                while (sensorless.if_elec_angle >= 2.0f * M_PI) sensorless.if_elec_angle -= 2.0f * M_PI;
                while (sensorless.if_elec_angle < 0.0f) sensorless.if_elec_angle += 2.0f * M_PI;

                /* 赋值给PLL（供外部调用MiniFOC_Sensorless_GetAngle()获取）*/
                sensorless.pll.go.OutRe = sensorless.if_elec_angle;
                sensorless.pll.go.OutWe = sensorless.if_we;

                /* 切换到SMO模式 */
                if (loop_cnt % 40000 == 0) {  /* 每0.5秒检查一次 */
                    sensorless.state = SENSORLESS_STATE_SMO_LOCKING;
                    sensorless.state_transition_tick = loop_cnt;
                    sensorless.smo_gain = 0.0f;  /* 开始融合 */
                }
            }
            break;

        case SENSORLESS_STATE_SMO_LOCKING:
            /* SMO+PLL锁定中：IF和SMO角度融合 */

            /* 1. 运行SMO+PLL */
            sensorless.smo.alpha.Input_Ix = I_alpha;
            sensorless.smo.alpha.Input_Ux = U_alpha;
            sensorless.smo.beta.Input_Ix  = I_beta;
            sensorless.smo.beta.Input_Ux  = U_beta;
            sensorless.smo.Input_We = sensorless.pll.go.OutWe;

            MiniFOC_SMO_Loop(&sensorless.smo);

            /* 2. 计算PLL误差（关键：乘以极对数！）*/
            pll_angle = sensorless.pll.go.OutRe * MOTOR_POLE_PAIRS;  /* ← 乘以极对数 */
            fast_sin_cos(pll_angle, &sine, &cosine);

            pll_error = -(sensorless.smo.alpha.Output_Ex * cosine +
                          sensorless.smo.beta.Output_Ex * sine);
            pll_error = Normalize_Error(pll_error);

            MiniFOC_PLL_Loop(&sensorless.pll, pll_error, 0);

            /* 3. 计算SMO权重（基于速度的线性过渡）*/
            MiniFOC_Value_Gain_Get(&sensorless.smo_gain,
                                   sensorless.pll.go.OutWe * 60.0f / (2.0f * M_PI * MOTOR_POLE_PAIRS),  /* rad/s → rpm */
                                   sensorless.switch_speed_max,
                                   sensorless.switch_speed_min);

            /* 4. 角度融合（混合IF和SMO）*/
            sensorless.pll.go.OutRe = sensorless.if_elec_angle * (1.0f - sensorless.smo_gain) +
                                      sensorless.pll.go.OutRe * sensorless.smo_gain;

            /* 5. 切换到闭环 */
            if (sensorless.smo_gain >= 0.99f ||
                (loop_cnt - sensorless.state_transition_tick > 50000)) {  /* 超时强制切换 */
                sensorless.state = SENSORLESS_STATE_CLOSED_LOOP;
            }
            break;

        case SENSORLESS_STATE_CLOSED_LOOP:
            /* SMO闭环运行 */

            /* 1. 运行SMO+PLL */
            sensorless.smo.alpha.Input_Ix = I_alpha;
            sensorless.smo.alpha.Input_Ux = U_alpha;
            sensorless.smo.beta.Input_Ix  = I_beta;
            sensorless.smo.beta.Input_Ux  = U_beta;
            sensorless.smo.Input_We = sensorless.pll.go.OutWe;

            MiniFOC_SMO_Loop(&sensorless.smo);

            /* 2. 计算PLL误差（关键：乘以极对数！）*/
            fast_sin_cos(sensorless.pll.go.OutRe * MOTOR_POLE_PAIRS, &sine, &cosine);

            pll_error = -(sensorless.smo.alpha.Output_Ex * cosine +
                          sensorless.smo.beta.Output_Ex * sine);
            pll_error = Normalize_Error(pll_error);

            MiniFOC_PLL_Loop(&sensorless.pll, pll_error, 0);

            /* 3. 速度过低时切回IF模式（滞环判断）*/
            current_speed = sensorless.pll.go.OutWe * 60.0f / (2.0f * M_PI * MOTOR_POLE_PAIRS);
            if (fabsf(current_speed) < (sensorless.switch_speed_min - sensorless.switch_speed_hyst)) {
                sensorless.state = SENSORLESS_STATE_SMO_LOCKING;
                sensorless.state_transition_tick = loop_cnt;
            }
            break;

        default:
            sensorless.state = SENSORLESS_STATE_IF_STARTUP;
            break;
    }
}

/**
  * @brief  获取SMO估算的电角度
  * @retval 电角度 (rad, 0~2π)
  */
float MiniFOC_Sensorless_GetAngle(void)
{
    return sensorless.pll.go.OutRe;
}

/**
  * @brief  获取SMO估算的机械转速
  * @retval 转速 (rpm)
  */
float MiniFOC_Sensorless_GetSpeed(void)
{
    /* OutWe 是电角速度 (rad/s) → 除以极对数 → 机械角速度 (rad/s) → RPM */
    float mech_rad_s = sensorless.pll.go.OutWe / (float)MOTOR_POLE_PAIRS;
    return mech_rad_s * 60.0f / 6.2831853f;       /* rad/s → RPM */
}

/**
  * @brief  获取SMO估算的电角速度
  * @retval 电角速度 (rad/s)
  */
float MiniFOC_Sensorless_GetWe(void)
{
    return sensorless.pll.go.OutWe;
}
