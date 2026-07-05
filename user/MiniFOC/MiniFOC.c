/**
  ******************************************************************************
  * @file           : MiniFOC.c
  * @brief          : MiniFOC 核心实现（完全按照 BLOC FOC_Core.c 实现）
  * @attention
  *
  ******************************************************************************
  */
#include "MiniFOC.h"
#include "MiniFOC_Config.h"

/* 引入BSP模块 */
#include "../bsp_adc.h"
#include "stm32g4xx_hal.h"
#include <stdlib.h>
#include <string.h>

/* ========== 全局变量 ========== */
MiniFOC_t foc;
extern TIM_HandleTypeDef htim1;

/* VF 控制变量 */
static float vf_theta = 0.0f;

/* ========== 初始化函数 ========== */

void MiniFOC_Init(void)
{
    memset(&foc, 0, sizeof(foc));
    foc.mode = DEFAULT_CONTROL_MODE;
    foc.cmd_source = DEFAULT_CMD_SOURCE;
    foc.motor_running = false;
    foc.fault_flag = false;

    PID_Init(&foc.current_pid, DEFAULT_CURRENT_KP, DEFAULT_CURRENT_KI, DEFAULT_CURRENT_KD);
    PID_Set_Limit(&foc.current_pid, DEFAULT_CURRENT_LIMIT, -DEFAULT_CURRENT_LIMIT);

    PID_Init(&foc.speed_pid, DEFAULT_SPEED_KP, DEFAULT_SPEED_KI, DEFAULT_SPEED_KD);
    PID_Set_Limit(&foc.speed_pid, DEFAULT_SPEED_LIMIT, -DEFAULT_SPEED_LIMIT);

    srand((unsigned int)HAL_GetTick());
    TIM1->CCR1 = 0; TIM1->CCR2 = 0; TIM1->CCR3 = 0;
}

/**
  * @brief  高频任务 (40kHz，在ADC中断中调用)
  */
void MiniFOC_HighFreqLoop(void)
{
    if (!foc.motor_running) return;
    if (foc.mode != MODE_VF_OPENLOOP) return;

    /* V/F 控制（完全按照 BLOC FOC_Core.c VF_Step()）*/
    /* BLOC 用固定 24V，不用动态 Vbus（避免 Vbus=0 导致异常）*/
    float Vbus = 24.0f;

    /* V/F 参数：根据目标转速计算 */
    float vf_freq = foc.target_speed * MOTOR_POLE_PAIRS / 60.0f;  /* 转速→电频率 */
    float vf_voltage = foc.target_speed * MOTOR_RATED_VOLTAGE / MOTOR_RATED_SPEED;  /* V/f 曲线 */

    /* 限制最小电压/频率（避免低速启动失败）*/
    if (vf_voltage < 3.0f) vf_voltage = 3.0f;
    if (vf_freq < 2.0f) vf_freq = 2.0f;

    vf_theta += 2.0f * 3.14159f * vf_freq * 0.000025f;
    if (vf_theta > 6.283185f) vf_theta -= 6.283185f;

    float sine = sinf(vf_theta);
    float cosine = cosf(vf_theta);
    float Ualpha = -vf_voltage * sine;
    float Ubeta  =  vf_voltage * cosine;

    float Ua = Ualpha;
    float Ub = -0.5f * Ualpha + 0.8660254037844386f * Ubeta;
    float Uc = -0.5f * Ualpha - 0.8660254037844386f * Ubeta;

    float Umax = fmaxf(fmaxf(Ua, Ub), Uc);
    float Umin = fminf(fminf(Ua, Ub), Uc);
    float U0 = -0.5f * (Umax + Umin);

    float duty_a = (-(U0 + Ua) / Vbus + 0.5f) * 8000.0f;
    float duty_b = (-(U0 + Ub) / Vbus + 0.5f) * 8000.0f;
    float duty_c = (-(U0 + Uc) / Vbus + 0.5f) * 8000.0f;

    if (duty_a > 7999.0f) duty_a = 7999.0f; if (duty_a < 0.0f) duty_a = 0.0f;
    if (duty_b > 7999.0f) duty_b = 7999.0f; if (duty_b < 0.0f) duty_b = 0.0f;
    if (duty_c > 7999.0f) duty_c = 7999.0f; if (duty_c < 0.0f) duty_c = 0.0f;

    TIM1->CCR1 = (uint32_t)duty_c;
    TIM1->CCR2 = (uint32_t)duty_b;
    TIM1->CCR3 = (uint32_t)duty_a;
}

/* ========== 用户API ========== */

void MiniFOC_SetMode(ControlMode_t mode)
{
    foc.mode = mode;
}

void MiniFOC_SetTargetSpeed(float speed)
{
    foc.target_speed = Limit(speed, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
}

void MiniFOC_SetTargetCurrent(float current)
{
    foc.target_current = Limit(current, -MOTOR_MAX_CURRENT, MOTOR_MAX_CURRENT);
}

void MiniFOC_MotorEnable(bool enable)
{
    foc.motor_running = enable;

    if (enable) {
        vf_theta = (float)rand() / RAND_MAX * TWO_PI;
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    } else {
        TIM1->CCR1 = 0; TIM1->CCR2 = 0; TIM1->CCR3 = 0;
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
        PID_Reset(&foc.current_pid);
        PID_Reset(&foc.speed_pid);
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
  * @brief  主循环任务 (1kHz)
  * @note   当前 VF 开环模式下不需要，保留接口
  */
void MiniFOC_MainLoop(void)
{
    /* 可以在这里添加 1kHz 任务：速度环、状态机等 */
}
