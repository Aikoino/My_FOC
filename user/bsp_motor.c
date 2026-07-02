/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_motor.c
  * @brief          : 电机控制核心，提供启停接口、运行状态查询与 LED 指示
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "bsp_motor.h"
#include "main.h"

extern TIM_HandleTypeDef htim1;

static uint8_t s_motor_running = 0;
static MotorCtrlMode_t s_motor_mode = MOTOR_CTRL_SENSORLESS;
static float s_target_speed = 0.0f;
static float s_speed_kp = 0.5f;
static float s_speed_ki = 0.1f;
static float s_current_kp = 0.1f;
static float s_current_ki = 0.01f;
static float s_flux = 0.012f;
static float s_resistance = 0.5f;
static float s_inductance = 0.001f;
static uint16_t s_pole_pairs = 7;
static uint16_t s_openloop_speed = 1000;

/**
  * @brief  初始化电机
  * @retval None
  */
void BSP_Motor_Init(void)
{
    BSP_Motor_Stop();
}

/**
  * @brief  启动电机
  * @retval None
  */
void BSP_Motor_Start(void)
{
    if (s_motor_running == 0)
    {
        s_motor_running = 1;
    }
}

/**
  * @brief  停止电机
  * @retval None
  */
void BSP_Motor_Stop(void)
{
    if (s_motor_running == 1)
    {
        s_motor_running = 0;
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
    }
}

/**
  * @brief  查询电机是否正在运行
  * @retval 1: 正在运行  0: 已停止
  */
uint8_t BSP_Motor_IsRunning(void)
{
    return s_motor_running;
}

/**
  * @brief  切换电机启停状态
  * @retval None
  */
void BSP_Motor_Toggle(void)
{
    if (s_motor_running)
    {
        BSP_Motor_Stop();
    }
    else
    {
        BSP_Motor_Start();
    }
}

/**
  * @brief  设置电机转速
  * @param  speed: 转速 (RPM)
  * @retval None
  */
void BSP_Motor_SetSpeed(float speed)
{
    s_target_speed = speed;
}

/**
  * @brief  获取电机转速
  * @retval 转速 (RPM)
  */
float BSP_Motor_GetSpeed(void)
{
    return s_target_speed;
}

/**
  * @brief  设置控制模式
  * @param  mode: 控制模式
  * @retval None
  */
void BSP_Motor_SetMode(MotorCtrlMode_t mode)
{
    s_motor_mode = mode;
}

/**
  * @brief  获取控制模式
  * @retval 控制模式
  */
MotorCtrlMode_t BSP_Motor_GetMode(void)
{
    return s_motor_mode;
}

/**
  * @brief  设置速度环 Kp
  * @param  kp: Kp 值
  * @retval None
  */
void BSP_Motor_SetSpeedKp(float kp)
{
    s_speed_kp = kp;
}

/**
  * @brief  设置速度环 Ki
  * @param  ki: Ki 值
  * @retval None
  */
void BSP_Motor_SetSpeedKi(float ki)
{
    s_speed_ki = ki;
}

/**
  * @brief  设置电流环 Kp
  * @param  kp: Kp 值
  * @retval None
  */
void BSP_Motor_SetCurrentKp(float kp)
{
    s_current_kp = kp;
}

/**
  * @brief  设置电流环 Ki
  * @param  ki: Ki 值
  * @retval None
  */
void BSP_Motor_SetCurrentKi(float ki)
{
    s_current_ki = ki;
}

/**
  * @brief  设置磁链常数
  * @param  flux: 磁链常数
  * @retval None
  */
void BSP_Motor_SetFlux(float flux)
{
    s_flux = flux;
}

/**
  * @brief  设置相电阻
  * @param  resistance: 相电阻 (Ohm)
  * @retval None
  */
void BSP_Motor_SetResistance(float resistance)
{
    s_resistance = resistance;
}

/**
  * @brief  设置相电感
  * @param  inductance: 相电感 (H)
  * @retval None
  */
void BSP_Motor_SetInductance(float inductance)
{
    s_inductance = inductance;
}

/**
  * @brief  设置极对数
  * @param  pole_pairs: 极对数
  * @retval None
  */
void BSP_Motor_SetPolePairs(uint16_t pole_pairs)
{
    s_pole_pairs = pole_pairs;
}

/**
  * @brief  设置开环强拖速度
  * @param  speed: 速度 (RPM)
  * @retval None
  */
void BSP_Motor_SetOpenloopSpeed(uint16_t speed)
{
    s_openloop_speed = speed;
}
