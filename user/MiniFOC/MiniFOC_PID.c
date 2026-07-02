/**
  ******************************************************************************
  * @file           : MiniFOC_PID.c
  * @brief          : MiniFOC PID控制器实现
  ******************************************************************************
  */
#include "MiniFOC_PID.h"

/**
  * @brief  PID初始化
  */
void PID_Init(PID_Controller_t *pid, float kp, float ki, float kd)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;

    pid->out_max = 1.0f;
    pid->out_min = -1.0f;

    pid->integrator = 0.0f;
    pid->prev_error_1 = 0.0f;
    pid->prev_error_2 = 0.0f;
}

/**
  * @brief  位置式PID计算（带抗积分饱和）
  */
float PID_Pos_Calc(PID_Controller_t *pid, float ref, float meas)
{
    float error = ref - meas;
    float p_term = pid->Kp * error;

    /* 积分项（带饱和处理） */
    pid->integrator += pid->Ki * error;
    Limit(pid->integrator, pid->out_min, pid->out_max);

    /* 微分项（对误差微分） */
    float d_term = pid->Kd * (error - pid->prev_error_1);

    float output = p_term + pid->integrator + d_term;

    /* 输出限幅 */
    output = Limit(output, pid->out_min, pid->out_max);

    pid->prev_error_1 = error;

    return output;
}

/**
  * @brief  增量式PID计算
  * @note   公式: Δu(k) = Kp*[e(k)-e(k-1)] + Ki*e(k) + Kd*[e(k)-2e(k-1)+e(k-2)]
  */
float PID_Inc_Calc(PID_Controller_t *pid, float ref, float meas)
{
    float error = ref - meas;

    float delta_p = pid->Kp * (error - pid->prev_error_1);
    float delta_i = pid->Ki * error;
    float delta_d = pid->Kd * (error - 2.0f * pid->prev_error_1 + pid->prev_error_2);

    float delta_output = delta_p + delta_i + delta_d;

    /* 保存历史误差 */
    pid->prev_error_2 = pid->prev_error_1;
    pid->prev_error_1 = error;

    return delta_output;
}

/**
  * @brief  重置PID
  */
void PID_Reset(PID_Controller_t *pid)
{
    pid->integrator = 0.0f;
    pid->prev_error_1 = 0.0f;
    pid->prev_error_2 = 0.0f;
}

/**
  * @brief  设置输出限幅
  */
void PID_Set_Limit(PID_Controller_t *pid, float max, float min)
{
    pid->out_max = max;
    pid->out_min = min;
}
