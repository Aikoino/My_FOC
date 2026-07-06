/**
  ******************************************************************************
  * @file           : MiniFOC.c
  * @brief          : MiniFOC 核心实现（电流环 + 速度环双闭环）
  * @version        : 2.0.0
  * @description    : 对齐 SguanFOC v3.1.0，支持电流环/速度环/霍尔模式
  ******************************************************************************
  * @attention
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  ******************************************************************************
  */
#include "MiniFOC.h"
#include "MiniFOC_Config.h"

/* 引入BSP模块 */
#include "../bsp_adc.h"
#include "../bsp_hall.h"
#include "MiniFOC_Transform.h"
#include "MiniFOC_SVPWM.h"
#include "stm32g4xx_hal.h"
#include <stdlib.h>
#include <string.h>

/* ========== 全局变量 ========== */
MiniFOC_t foc;
extern TIM_HandleTypeDef htim1;

/* 速度环分频计数器 */
static uint8_t speed_loop_count = 0;
#define SPEED_LOOP_DIVIDER  10  /* 速度环频率：40kHz / 10 = 4kHz */

/* 电流环采样周期 (s) */
#define CURRENT_LOOP_T      0.00005f  /* 20kHz（与 ADC 注入中断频率匹配）*/

/* ========== 初始化函数 ========== */

void MiniFOC_Init(void)
{
    memset(&foc, 0, sizeof(foc));
    foc.mode = DEFAULT_CONTROL_MODE;
    foc.cmd_source = DEFAULT_CMD_SOURCE;
    foc.motor_running = false;
    foc.fault_flag = false;

    /* 母线电压（临时固定，后续可改为实时读取）*/
    foc.bus_voltage = 24.0f;

    /* 霍尔角度校准偏移（调试中：0=无偏移，逐步测试）*/
    foc.hall_angle_offset = 0.0f;

    /* 电流环 PID 初始化（先设参数，再Init，确保 Ki 正确计算 I_num）*/
    foc.current_pid.T = CURRENT_LOOP_T;
    PID_Set_Param(&foc.current_pid, DEFAULT_CURRENT_KP, DEFAULT_CURRENT_KI, DEFAULT_CURRENT_KD);
    PID_Init(&foc.current_pid);
    PID_Set_OutputLimit(&foc.current_pid, DEFAULT_CURRENT_LIMIT, -DEFAULT_CURRENT_LIMIT);
    /* 积分限幅设为输出限幅的 50%，防止积分饱和过深 */
    PID_Set_IntegralLimit(&foc.current_pid, DEFAULT_CURRENT_LIMIT * 0.5f, -DEFAULT_CURRENT_LIMIT * 0.5f);

    /* 速度环 PID 初始化（先设参数，再Init）*/
    foc.speed_pid.T = 0.01f;  /* 100Hz 执行周期（1kHz主循环 / 10分频 = 100Hz）*/
    PID_Set_Param(&foc.speed_pid, DEFAULT_SPEED_KP, DEFAULT_SPEED_KI, DEFAULT_SPEED_KD);
    PID_Init(&foc.speed_pid);
    PID_Set_OutputLimit(&foc.speed_pid, DEFAULT_SPEED_LIMIT, -DEFAULT_SPEED_LIMIT);
    PID_Set_IntegralLimit(&foc.speed_pid, DEFAULT_SPEED_LIMIT, -DEFAULT_SPEED_LIMIT);

    /* 目标值初始化 */
    foc.target_speed = 0.0f;
    foc.target_current = 0.0f;

    /* 电流环给定初始化（MTPA：Id=0, Iq=0）*/
    foc.Target_Id = 0.0f;
    foc.Target_Iq = 0.0f;

    /* D/Q 轴电压初始化 */
    foc.Ud = 0.0f;
    foc.Uq = 0.0f;

    /* VF 开环初始化 */
    foc.vf_elec_angle = 0.0f;
    foc.vf_start_time = 0;

    /* 转子角度/转速初始化 */
    foc.rotor_angle = 0.0f;
    foc.rotor_speed = 0.0f;

    /* 清空电流采样偏移 */
    foc.current_offset_u = 0.0f;
    foc.current_offset_v = 0.0f;

    /* PWM 清零 */
    TIM1->CCR1 = 0; TIM1->CCR2 = 0; TIM1->CCR3 = 0;
}

/**
  * @brief  高频任务 (40kHz，在ADC中断中调用)
  * @note   根据控制模式执行不同的控制算法
  */
void MiniFOC_HighFreqLoop(void)
{
    if (!foc.motor_running) return;

    /* 转子状态在电流环中统一读取（避免双重调用）*/

    switch (foc.mode) {
        case MODE_VF_OPENLOOP:
            /* V/F 开环控制 */
            MiniFOC_VF_Step();
            break;

        case MODE_Current_SINGLE:
            /* 电流单闭环 */
            MiniFOC_CurrentLoop();
            break;

        case MODE_VelCur_DOUBLE:
            /* 速度-电流双闭环 */
            MiniFOC_CurrentLoop();
            break;

        case MODE_Sensor_Hall:
            /* 霍尔有感控制 */
            MiniFOC_CurrentLoop();
            break;

        default:
            /* 其他模式暂未实现，停止输出 */
            SVPWM_EmergencyStop();
            break;
    }
}

/**
  * @brief  V/F 开环控制步骤（对齐 BLOC FOC_Core.c）
  * @note   使用动态 V/f 曲线，对齐 BLOC 的 V/f = 3V/Hz
  *
  * @fix_history:
  *   2026-07-05: 修复转速过慢问题
  *     - 原来：固定 4Hz → 34rpm（太慢）
  *     - 现在：根据 target_speed 动态计算
  *     - V/f 曲线：3V/Hz（保守值，避免过压）
  */
void MiniFOC_VF_Step(void)
{
    /* V/f 曲线参数（对齐 BLOC：固定电压频率比）*/
    const float VF_VOLTAGE_PER_HZ = 3.0f;  /* V/f = 3V/Hz（保守值）*/

    /* 根据目标转速计算 V/f 参数 */
    float vf_freq = foc.target_speed * (float)MOTOR_POLE_PAIRS / 60.0f;  /* 转速→电频率 */
    float vf_voltage = vf_freq * VF_VOLTAGE_PER_HZ;  /* V/f 曲线 */

    /* 限制最小电压/频率（避免低速启动失败）*/
    if (vf_voltage < 5.0f) vf_voltage = 5.0f;  /* 最小 5V */
    if (vf_freq < 5.0f) vf_freq = 5.0f;        /* 最小 5Hz */

    /* 限制最大电压（不超过母线电压 90%）*/
    float vbus_actual = (foc.bus_voltage > 20.0f) ? foc.bus_voltage : 24.0f;
    if (vf_voltage > vbus_actual * 0.9f) {
        vf_voltage = vbus_actual * 0.9f;
    }

    /* 积分更新电角度 */
    foc.vf_elec_angle += 2.0f * 3.14159f * vf_freq * CURRENT_LOOP_T;
    if (foc.vf_elec_angle > 6.283185f) foc.vf_elec_angle -= 6.283185f;

    /* 计算正弦/余弦 */
    float sine = sinf(foc.vf_elec_angle);
    float cosine = cosf(foc.vf_elec_angle);

    /* 生成电压矢量（Ud=0, Uq=vf_voltage）*/
    float Ualpha = -vf_voltage * sine;
    float Ubeta  =  vf_voltage * cosine;

    /* 马鞍波 SVPWM */
    uint32_t duty_a, duty_b, duty_c;
    SVPWM_SaddleWave(Ualpha, Ubeta, vbus_actual, &duty_a, &duty_b, &duty_c);

    /* 设置 PWM */
    TIM1->CCR3 = duty_a;  /* U 相 → CH3 */
    TIM1->CCR2 = duty_b;  /* V 相 → CH2 */
    TIM1->CCR1 = duty_c;  /* W 相 → CH1 */
}

/**
  * @brief  电流环闭环控制（40kHz 实时控制）
  * @note   实现 D/Q 轴电流 PI 控制（优化霍尔模式兼容性）
  *
  * @control_flow:
  *   1. 读取三相电流 → Clarke 变换 → αβ 轴
  *   2. Park 变换 → dq 轴
  *   3. 电流环 PI 控制 → D/Q 轴电压
  *   4. 前馈补偿（可选）
  *   5. 电压限幅
  *   6. 逆 Park + 逆 Clarke + SVPWM
  */
void MiniFOC_CurrentLoop(void)
{
    if (!foc.motor_running) return;

    /* 预定位阶段：给固定电压矢量，把转子拉到已知位置 */
    if (foc.pre_position_active) {
        uint32_t elapsed = HAL_GetTick() - foc.pre_position_start;
        if (elapsed < 300) {  /* 预定位持续 300ms */
            /* 读取 Hall 角度 */
            if (foc.mode == MODE_Sensor_Hall) {
                BSP_Hall_Read();
            }
            float elec_angle = foc.rotor_angle * (float)MOTOR_POLE_PAIRS;

            /* Ud=0, Uq=5V（固定电压矢量，产生最大转矩）*/
            foc.Ud = 0.0f;
            foc.Uq = 5.0f;

            float sine = fast_sin(elec_angle);
            float cosine = fast_cos(elec_angle);

            /* 逆 Park + SVPWM */
            float Ualpha, Ubeta;
            ipark(&Ualpha, &Ubeta, foc.Ud, foc.Uq, sine, cosine);

            uint32_t duty_a, duty_b, duty_c;
            SVPWM_SaddleWave(Ualpha, Ubeta, foc.bus_voltage, &duty_a, &duty_b, &duty_c);

            TIM1->CCR3 = duty_a;
            TIM1->CCR2 = duty_b;
            TIM1->CCR1 = duty_c;
            return;
        } else {
            /* 预定位结束，切回 PID 控制 */
            foc.pre_position_active = false;
            PID_Reset(&foc.current_pid);
        }
    }

    /* 1. 读取三相电流（已校准，单位：A）*/
    float Ia = foc.phase_current_u - foc.current_offset_u;
    float Ib = foc.phase_current_v - foc.current_offset_v;
    float Ic = -Ia - Ib;  /* 第三相电流 */

    /* 2. Clarke 正变换：3相 → αβ 轴 */
    float Ialpha, Ibeta;
    clarke(&Ialpha, &Ibeta, Ia, Ib);

    /* 3. Park 正变换：αβ → dq 轴 */
    float sine, cosine;

    /* 根据控制模式选择角度来源 */
    if (foc.mode == MODE_Sensor_Hall) {
        /* 霍尔模式：使用 BSP_Hall_Read() 已更新的插值角度 */
        BSP_Hall_Read();  /* 刷新角度 + 线性插值 */

        /* 检查霍尔信号是否有效（非全 0 且非全 1）*/
        uint8_t ha = hall_sensor.ha_raw;
        uint8_t hb = hall_sensor.hb_raw;
        uint8_t hc = hall_sensor.hc_raw;
        uint8_t all_zero = ((ha | hb | hc) == 0);
        uint8_t all_one  = ((ha & hb & hc) == 1);

        static uint32_t hall_invalid_counter = 0;

        if (all_zero || all_one) {
            /* 霍尔信号无效，保持上次角度不做更新 */
            hall_invalid_counter++;
            if (hall_invalid_counter > 200) {
                foc.mode = MODE_VF_OPENLOOP;
                MiniFOC_VF_Step();
                return;
            }
        } else {
            /* 角度有效，使用插值后的角度 */
            foc.rotor_angle = BSP_Hall_GetAngle();
            foc.rotor_speed = BSP_Hall_GetSpeed();
            hall_invalid_counter = 0;
        }
    }

    float elec_angle;

    /* Hall 传感器输出的是机械角度（6扇区=360°机械角），乘以极对数得到电角度 */
    if (foc.mode == MODE_Sensor_Hall) {
        elec_angle = foc.rotor_angle * (float)MOTOR_POLE_PAIRS;
    } else {
        elec_angle = foc.rotor_angle * (float)MOTOR_POLE_PAIRS;
    }

    /* 霍尔角度校准偏移（sin/cos 天然支持大角度，不需要归一化）*/
    elec_angle += foc.hall_angle_offset;

    /* 预计算 sin/cos*/
    foc.rotor_sine = sine = fast_sin(elec_angle);
    foc.rotor_cosine = cosine = fast_cos(elec_angle);

    /* Park 变换*/
    park(&foc.Id, &foc.Iq, Ialpha, Ibeta, sine, cosine);

    /* 4. 电流环 PI 控制（MTPA：Id=0, 只控制 Iq）*/
    foc.Target_Id = 0.0f;

    if (foc.mode == MODE_Sensor_Hall) {
        foc.Target_Iq = foc.target_current;
    }

    foc.Ud = 0.0f;

    /* Q 轴电流环*/
    foc.current_pid.run.Ref = foc.Target_Iq;
    foc.current_pid.run.Fbk = foc.Iq;
    foc.Uq = PID_Calc(&foc.current_pid);

    /* 5. 电压限幅*/
    float Vmax = foc.bus_voltage * 0.95f;
    float voltage_mag = sqrtf(foc.Ud * foc.Ud + foc.Uq * foc.Uq);
    if (voltage_mag > Vmax) {
        float ratio = Vmax / voltage_mag;
        foc.Ud *= ratio;
        foc.Uq *= ratio;
    }

    /* 6. 逆 Park + SVPWM（用马鞍波，与 VF 模式一致，确认可用）*/
    float Ualpha, Ubeta;
    ipark(&Ualpha, &Ubeta, foc.Ud, foc.Uq, sine, cosine);

    uint32_t duty_a, duty_b, duty_c;
    SVPWM_SaddleWave(Ualpha, Ubeta, foc.bus_voltage, &duty_a, &duty_b, &duty_c);

    /* 7. 设置 PWM*/
    TIM1->CCR3 = duty_a;
    TIM1->CCR2 = duty_b;
    TIM1->CCR1 = duty_c;
}

/**
  * @brief  主循环任务 (1kHz)
  * @note   处理速度环、状态机、指令接收
  */
void MiniFOC_MainLoop(void)
{
    if (!foc.motor_running) return;

    switch (foc.mode) {
        case MODE_VelCur_DOUBLE:
            /* 速度-电流双闭环：速度环在 1kHz 执行 */
            MiniFOC_SpeedLoop();
            break;

        default:
            /* 其他模式不需要主循环任务 */
            break;
    }
}

/**
  * @brief  速度环控制（1kHz 执行）
  * @note   输出为 Iq 给定值
  */
void MiniFOC_SpeedLoop(void)
{
    if (foc.mode != MODE_VelCur_DOUBLE) return;

    /* 降低速度环执行频率（10 分频：1kHz → 100Hz）*/
    if (++speed_loop_count >= SPEED_LOOP_DIVIDER) {
        speed_loop_count = 0;

        /* 速度环 PI 控制 */
        foc.Target_Iq = PID_Calc(&foc.speed_pid);

        /* Id 给定始终为 0（MTPA 控制）*/
        foc.Target_Id = 0.0f;

        /* 更新电流环 PID 的给定值 */
        foc.current_pid.run.Ref = foc.Target_Iq;
    }
}

/* ========== 用户API ========== */

void MiniFOC_SetMode(ControlMode_t mode)
{
    foc.mode = mode;

    /* 模式切换时的特殊处理 */
    if (mode == MODE_VelCur_DOUBLE) {
        /* 切换到双闭环模式时，速度环给定为当前转速 */
        foc.speed_pid.run.Ref = foc.rotor_speed;
    }
}

void MiniFOC_SetTargetSpeed(float speed)
{
    foc.target_speed = Limit(speed, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);

    /* 更新速度环给定 */
    foc.speed_pid.run.Ref = foc.target_speed;
}

void MiniFOC_SetTargetCurrent(float current)
{
    foc.target_current = Limit(current, -MOTOR_MAX_CURRENT, MOTOR_MAX_CURRENT);

    /* 更新电流环给定 */
    foc.Target_Iq = foc.target_current;
    foc.current_pid.run.Ref = foc.target_current;
}

/**
  * @brief  设置霍尔角度校准偏移（运行时动态调整）
  * @param  offset_rad: 偏移角度（rad），每次加约 0.1745 = 10°
  */
void MiniFOC_SetHallAngleOffset(float offset_rad)
{
    foc.hall_angle_offset = offset_rad;
}

void MiniFOC_MotorEnable(bool enable)
{
    foc.motor_running = enable;

    if (enable) {
        /* 启动电机 */
        #if 0
        /* VF 开环需要随机角度（避免启动时堵转）*/
        vf_theta = (float)rand() / RAND_MAX * TWO_PI;
        #endif

        /* 启动预定位（Hall 模式：先给固定电压拉转子到正确位置）*/
        foc.pre_position_active = true;
        foc.pre_position_start = HAL_GetTick();

        /* 启动 PWM */
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

        /* 重置 PID */
        PID_Reset(&foc.current_pid);
        PID_Reset(&foc.speed_pid);

        /* 电流环给定初始化 */
        foc.Target_Id = 0.0f;
        foc.Target_Iq = foc.target_current;
        foc.current_pid.run.Ref = foc.Target_Iq;

        /* 霍尔模式：重置跟踪状态 */
        #ifdef USE_HALL_SENSOR
        BSP_Hall_ResetTracking();
        #endif
    } else {
        /* 停止电机 */
        SVPWM_EmergencyStop();
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);

        /* 重置 PID */
        PID_Reset(&foc.current_pid);
        PID_Reset(&foc.speed_pid);

        /* 清零给定 */
        foc.Target_Id = 0.0f;
        foc.Target_Iq = 0.0f;
    }
}

float MiniFOC_GetSpeed(void)
{
    return foc.rotor_speed;
}

float MiniFOC_GetCurrent(void)
{
    return foc.Iq;
}

void MiniFOC_FaultHandler(uint32_t fault_code)
{
    foc.fault_flag = true;
    foc.fault_code = fault_code;
    MiniFOC_MotorEnable(false);
}

/**
  * @brief  更新转子角度和转速（由 BSP 调用）
  * @param  angle: 转子机械角度（rad）
  * @param  speed: 转子转速（rpm）
  */
void MiniFOC_UpdateRotorState(float angle, float speed)
{
    foc.rotor_angle = angle;
    foc.rotor_speed = speed;
}

/**
  * @brief  更新三相电流（由 BSP 调用）
  * @param  iu: U 相电流（A）
  * @param  iv: V 相电流（A）
  */
void MiniFOC_UpdateCurrent(float iu, float iv)
{
    foc.phase_current_u = iu;
    foc.phase_current_v = iv;
}
