/**
  ******************************************************************************
  * @file           : MiniFOC.c
  * @brief          : MiniFOC 核心实现（电流环 + 速度环）
  * @version        : 2.1.0
  * @description    : 4种控制模式：VF开环/双闭环/霍尔电流环/霍尔速度环
  ******************************************************************************
  * @attention
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  ******************************************************************************
  */
#include "MiniFOC.h"
#include "MiniFOC_Config.h"
#include <math.h>
#include <stdio.h>

/* 引入BSP模块 */
#include "../bsp_adc.h"
#include "../bsp_hall.h"
#include "MiniFOC_Transform.h"
#include "MiniFOC_SVPWM.h"
#include "stm32g4xx_hal.h"
#include <string.h>

/* ========== 全局变量 ========== */
MiniFOC_t foc;
extern TIM_HandleTypeDef htim1;

/* 速度环分频计数器 */
static uint8_t speed_loop_count = 0;
#define SPEED_LOOP_DIVIDER  10

/* 电流环采样周期 (s) */
#define CURRENT_LOOP_T      0.00005f

/* ========== 初始化函数 ========== */

void MiniFOC_Init(void)
{
    memset(&foc, 0, sizeof(foc));
    foc.mode = DEFAULT_CONTROL_MODE;
    foc.cmd_source = DEFAULT_CMD_SOURCE;
    foc.motor_running = false;
    foc.fault_flag = false;
    foc.bus_voltage = 24.0f;
    foc.hall_angle_offset = 0.0f;

    /* 电流环 PID 初始化 */
    foc.current_pid.T = CURRENT_LOOP_T;
    PID_Set_Param(&foc.current_pid, DEFAULT_CURRENT_KP, DEFAULT_CURRENT_KI, DEFAULT_CURRENT_KD);
    PID_Init(&foc.current_pid);
    PID_Set_OutputLimit(&foc.current_pid, DEFAULT_CURRENT_LIMIT, -DEFAULT_CURRENT_LIMIT);
    PID_Set_IntegralLimit(&foc.current_pid, DEFAULT_CURRENT_LIMIT * 0.5f, -DEFAULT_CURRENT_LIMIT * 0.5f);

    /* D轴电流环 PID 初始化 */
    foc.current_pid_d.T = CURRENT_LOOP_T;
    PID_Set_Param(&foc.current_pid_d, DEFAULT_CURRENT_KP, DEFAULT_CURRENT_KI, DEFAULT_CURRENT_KD);
    PID_Init(&foc.current_pid_d);
    PID_Set_OutputLimit(&foc.current_pid_d, DEFAULT_CURRENT_LIMIT, -DEFAULT_CURRENT_LIMIT);
    PID_Set_IntegralLimit(&foc.current_pid_d, DEFAULT_CURRENT_LIMIT * 0.5f, -DEFAULT_CURRENT_LIMIT * 0.5f);

    /* 速度环 PID 初始化 */
    foc.speed_pid.T = 0.01f;
    PID_Set_Param(&foc.speed_pid, DEFAULT_SPEED_KP, DEFAULT_SPEED_KI, DEFAULT_SPEED_KD);
    PID_Init(&foc.speed_pid);
    PID_Set_OutputLimit(&foc.speed_pid, DEFAULT_SPEED_LIMIT, -DEFAULT_SPEED_LIMIT);
    PID_Set_IntegralLimit(&foc.speed_pid, DEFAULT_SPEED_LIMIT, -DEFAULT_SPEED_LIMIT);

    foc.target_speed = 0.0f;
    foc.target_current = 0.0f;
    foc.Target_Id = 0.0f;
    foc.Target_Iq = 0.0f;
    foc.Ud = 0.0f;
    foc.Uq = 0.0f;
    foc.vf_elec_angle = 0.0f;
    foc.vf_start_time = 0;
    foc.rotor_angle = 0.0f;
    foc.rotor_speed = 0.0f;
    foc.current_offset_u = 0.0f;
    foc.current_offset_v = 0.0f;
    foc.duty_a = 0.0f;
    foc.duty_b = 0.0f;
    foc.duty_c = 0.0f;

    TIM1->CCR1 = 0; TIM1->CCR2 = 0; TIM1->CCR3 = 0;
}

void MiniFOC_HighFreqLoop(void)
{
    if (!foc.motor_running) return;
    MiniFOC_CurrentLoop();
}

void MiniFOC_VF_Step(void)
{
    const float VF_VOLTAGE_PER_HZ = 3.0f;
    float vf_freq = foc.target_speed * (float)MOTOR_POLE_PAIRS / 60.0f;
    float vf_voltage = vf_freq * VF_VOLTAGE_PER_HZ;
    vf_voltage = fabsf(vf_voltage);

    if (vf_voltage < 5.0f) vf_voltage = 5.0f;
    if (vf_freq < 5.0f) vf_freq = 5.0f;

    float vbus_actual = (foc.bus_voltage > 20.0f) ? foc.bus_voltage : 24.0f;
    if (vf_voltage > vbus_actual * 0.9f) {
        vf_voltage = vbus_actual * 0.9f;
    }

    foc.vf_elec_angle += 2.0f * 3.14159f * vf_freq * CURRENT_LOOP_T;

    if (foc.vf_elec_angle >= 6.283185f) {
        foc.vf_elec_angle -= 6.283185f;
    } else if (foc.vf_elec_angle < 0.0f) {
        foc.vf_elec_angle += 6.283185f;
    }

    foc.rotor_angle = foc.vf_elec_angle / (float)MOTOR_POLE_PAIRS;
    foc.rotor_speed = vf_freq * 60.0f / (float)MOTOR_POLE_PAIRS;

    float sine = sinf(foc.vf_elec_angle);
    float cosine = cosf(foc.vf_elec_angle);

    float Ualpha = -vf_voltage * sine;
    float Ubeta  =  vf_voltage * cosine;

    uint32_t duty_a, duty_b, duty_c;
    SVPWM_SaddleWave(Ualpha, Ubeta, vbus_actual, &duty_a, &duty_b, &duty_c);

    TIM1->CCR3 = duty_a;
    TIM1->CCR2 = duty_b;
    TIM1->CCR1 = duty_c;

    foc.duty_a = (float)duty_a;
    foc.duty_b = (float)duty_b;
    foc.duty_c = (float)duty_c;
}

void MiniFOC_CurrentLoop(void)
{
    if (!foc.motor_running) return;

    /* ========== VF/I/F/双闭环模式（开环角度）========== */
    if (foc.mode == MODE_VF_OPENLOOP || foc.mode == MODE_Current_SINGLE ||
        foc.mode == MODE_VelCur_DOUBLE) {
        /* 读取三相电流 */
        float Ia = foc.phase_current_u - foc.current_offset_u;
        float Ib = foc.phase_current_v - foc.current_offset_v;
        float Ic = -Ia - Ib;

        /* Clarke 变换 */
        float Ialpha, Ibeta;
        clarke(&Ialpha, &Ibeta, Ia, Ib);

        /* Park 变换（使用 VF 开环角度）*/
        float sine = fast_sin(foc.vf_elec_angle);
        float cosine = fast_cos(foc.vf_elec_angle);
        park(&foc.Id, &foc.Iq, Ialpha, Ibeta, sine, cosine);

        /* 电流环 PI 控制 */
        foc.Target_Id = 0.0f;

        if (foc.mode == MODE_Current_SINGLE) {
            foc.Target_Iq = foc.target_current;
        } else if (foc.mode == MODE_VelCur_DOUBLE) {
            /* 双闭环：Target_Iq 由速度环更新 */
        } else {
            foc.Target_Iq = 0.0f;
        }

        foc.Ud = 0.0f;
        foc.current_pid.run.Ref = foc.Target_Iq;
        foc.current_pid.run.Fbk = foc.Iq;
        foc.Uq = PID_Calc(&foc.current_pid);

        /* 电压限幅 */
        float Vmax = foc.bus_voltage * 0.95f;
        float voltage_mag = sqrtf(foc.Ud * foc.Ud + foc.Uq * foc.Uq);
        if (voltage_mag > Vmax) {
            float ratio = Vmax / voltage_mag;
            foc.Ud *= ratio;
            foc.Uq *= ratio;
        }

        /* 逆 Park + SVPWM */
        float Ualpha, Ubeta;
        ipark(&Ualpha, &Ubeta, foc.Ud, foc.Uq, sine, cosine);

        uint32_t duty_a, duty_b, duty_c;
        SVPWM_SaddleWave(Ualpha, Ubeta, foc.bus_voltage, &duty_a, &duty_b, &duty_c);

        TIM1->CCR3 = duty_a;
        TIM1->CCR2 = duty_b;
        TIM1->CCR1 = duty_c;

        foc.duty_a = (float)duty_a;
        foc.duty_b = (float)duty_b;
        foc.duty_c = (float)duty_c;

        /* VF/双闭环模式更新电角度 */
        if (foc.mode == MODE_VF_OPENLOOP || foc.mode == MODE_VelCur_DOUBLE) {
            MiniFOC_VF_Step();
        }

        return;
    }

    /* ========== 霍尔传感器模式（霍尔角度）========== */
    if (foc.mode == MODE_Sensor_Hall_I || foc.mode == MODE_Sensor_Hall_S) {
        /* 读取三相电流 */
        float Ia = foc.phase_current_u - foc.current_offset_u;
        float Ib = foc.phase_current_v - foc.current_offset_v;
        float Ic = -Ia - Ib;

        /* Clarke 变换 */
        float Ialpha, Ibeta;
        clarke(&Ialpha, &Ibeta, Ia, Ib);

        /* 读取霍尔传感器 */
        BSP_Hall_Read();

        uint8_t ha = hall_sensor.ha_raw;
        uint8_t hb = hall_sensor.hb_raw;
        uint8_t hc = hall_sensor.hc_raw;
        uint8_t hall_invalid = ((ha | hb | hc) == 0) || ((ha & hb & hc) == 1);
        float hall_angle = BSP_Hall_GetAngle();  /* 已经是电角度（tim.c中已乘极对数）*/

        static uint32_t hall_invalid_counter = 0;

        if (hall_invalid) {
            hall_invalid_counter++;
            if (hall_invalid_counter > 200) {
                foc.mode = MODE_VF_OPENLOOP;
                MiniFOC_VF_Step();
                return;
            }
            foc.rotor_speed = 0.0f;
        } else {
            foc.rotor_angle = hall_angle;
            foc.rotor_speed = BSP_Hall_GetSpeed();
            hall_invalid_counter = 0;
        }

        /* 电角度（hall_angle 已是电角度，无需再乘极对数）*/
        float elec_angle = foc.rotor_angle + foc.hall_angle_offset;

        /* Park 变换 */
        foc.rotor_sine = fast_sin(elec_angle);
        foc.rotor_cosine = fast_cos(elec_angle);
        park(&foc.Id, &foc.Iq, Ialpha, Ibeta, foc.rotor_sine, foc.rotor_cosine);

        /* 电流环 PI 控制 */
        foc.Target_Id = 0.0f;
        foc.Target_Iq = foc.target_current;
        foc.Ud = 0.0f;

        foc.current_pid.run.Ref = foc.Target_Iq;
        foc.current_pid.run.Fbk = foc.Iq;
        foc.Uq = PID_Calc(&foc.current_pid);

        /* 电压限幅 */
        float Vmax = foc.bus_voltage * 0.95f;
        float voltage_mag = sqrtf(foc.Ud * foc.Ud + foc.Uq * foc.Uq);
        if (voltage_mag > Vmax) {
            float ratio = Vmax / voltage_mag;
            foc.Ud *= ratio;
            foc.Uq *= ratio;
        }

        /* 逆 Park + SVPWM */
        float Ualpha, Ubeta;
        ipark(&Ualpha, &Ubeta, foc.Ud, foc.Uq, foc.rotor_sine, foc.rotor_cosine);

        uint32_t duty_a, duty_b, duty_c;
        SVPWM_SaddleWave(Ualpha, Ubeta, foc.bus_voltage, &duty_a, &duty_b, &duty_c);

        TIM1->CCR3 = duty_a;
        TIM1->CCR2 = duty_b;
        TIM1->CCR1 = duty_c;

        foc.duty_a = (float)duty_a;
        foc.duty_b = (float)duty_b;
        foc.duty_c = (float)duty_c;
    }
}

void MiniFOC_MainLoop(void)
{
    if (!foc.motor_running) return;

    switch (foc.mode) {
        case MODE_VelCur_DOUBLE:
            MiniFOC_SpeedLoop();
            break;

        case MODE_Sensor_Hall_S:
            MiniFOC_SpeedLoop();
            break;

        default:
            break;
    }
}

void MiniFOC_SpeedLoop(void)
{
    if (foc.mode != MODE_VelCur_DOUBLE && foc.mode != MODE_Sensor_Hall_S) return;

    if (++speed_loop_count >= SPEED_LOOP_DIVIDER) {
        speed_loop_count = 0;

        /* 速度环 PI 控制 */
        foc.speed_pid.run.Ref = foc.target_speed;
        foc.speed_pid.run.Fbk = foc.rotor_speed;  /* 关键：设置速度反馈 */
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
    if (mode == MODE_VelCur_DOUBLE) {
        foc.speed_pid.run.Ref = foc.rotor_speed;
    }
}

void MiniFOC_SetTargetSpeed(float speed)
{
    foc.target_speed = Limit(speed, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
    foc.speed_pid.run.Ref = foc.target_speed;
}

void MiniFOC_SetTargetCurrent(float current)
{
    foc.target_current = Limit(current, -MOTOR_MAX_CURRENT, MOTOR_MAX_CURRENT);
    foc.Target_Iq = foc.target_current;
    foc.current_pid.run.Ref = foc.target_current;
}

void MiniFOC_SetHallAngleOffset(float offset_rad)
{
    foc.hall_angle_offset = offset_rad;
}

void MiniFOC_MotorEnable(bool enable)
{
    foc.motor_running = enable;

    if (enable) {
        __disable_irq();
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
        HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
        HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
        HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
        __enable_irq();

        PID_Reset(&foc.current_pid);
        PID_Reset(&foc.current_pid_d);
        PID_Reset(&foc.speed_pid);

        foc.Target_Id = 0.0f;
        foc.Target_Iq = foc.target_current;
        foc.current_pid.run.Ref = foc.Target_Iq;
        foc.current_pid_d.run.Ref = 0.0f;

        #ifdef USE_HALL_SENSOR
        BSP_Hall_ResetTracking();
        #endif
    } else {
        SVPWM_EmergencyStop();
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
        HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
        HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
        HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);

        PID_Reset(&foc.current_pid);
        PID_Reset(&foc.speed_pid);

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

void MiniFOC_UpdateRotorState(float angle, float speed)
{
    foc.rotor_angle = angle;
    foc.rotor_speed = speed;
}

void MiniFOC_UpdateCurrent(float iu, float iv)
{
    foc.phase_current_u = iu;
    foc.phase_current_v = iv;
}

/* 无感观测器接口（暂未使用） */
void MiniFOC_UpdateObserver(float I_alpha, float I_beta, float U_alpha, float U_beta)
{
}

float MiniFOC_GetObserverAngle(void)
{
    return foc.vf_elec_angle;
}

float MiniFOC_GetObserverSpeed(void)
{
    return foc.rotor_speed;
}

uint8_t MiniFOC_GetObserverMode(void)
{
    return 0;
}
