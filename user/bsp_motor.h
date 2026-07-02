/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_motor.h
  * @brief          : 电机控制头文件，提供启停接口与状态查询
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* 防止重复包含 */
#ifndef __BSP_MOTOR_H__
#define __BSP_MOTOR_H__

#include <stdint.h>

/* 控制模式枚举 */
typedef enum {
    MOTOR_CTRL_SENSORED   = 0,  // 有感控制
    MOTOR_CTRL_SENSORLESS = 1,  // 无感控制
} MotorCtrlMode_t;

/* 电机控制接口 */

/**
  * @brief  初始化电机
  */
void BSP_Motor_Init(void);

/**
  * @brief  启动电机
  */
void BSP_Motor_Start(void);

/**
  * @brief  停止电机
  */
void BSP_Motor_Stop(void);

/**
  * @brief  查询电机是否正在运行
  * @retval 1: 正在运行  0: 已停止
  */
uint8_t BSP_Motor_IsRunning(void);

/**
  * @brief  切换电机启停状态
  */
void BSP_Motor_Toggle(void);

/**
  * @brief  设置电机转速
  * @param  speed: 转速 (RPM)
  */
void BSP_Motor_SetSpeed(float speed);

/**
  * @brief  获取电机转速
  * @retval 转速 (RPM)
  */
float BSP_Motor_GetSpeed(void);

/**
  * @brief  设置控制模式
  * @param  mode: 控制模式
  */
void BSP_Motor_SetMode(MotorCtrlMode_t mode);

/**
  * @brief  获取控制模式
  * @retval 控制模式
  */
MotorCtrlMode_t BSP_Motor_GetMode(void);

/**
  * @brief  设置速度环 Kp
  * @param  kp: Kp 值
  */
void BSP_Motor_SetSpeedKp(float kp);

/**
  * @brief  设置速度环 Ki
  * @param  ki: Ki 值
  */
void BSP_Motor_SetSpeedKi(float ki);

/**
  * @brief  设置电流环 Kp
  * @param  kp: Kp 值
  */
void BSP_Motor_SetCurrentKp(float kp);

/**
  * @brief  设置电流环 Ki
  * @param  ki: Ki 值
  */
void BSP_Motor_SetCurrentKi(float ki);

/**
  * @brief  设置磁链常数
  * @param  flux: 磁链常数
  */
void BSP_Motor_SetFlux(float flux);

/**
  * @brief  设置相电阻
  * @param  resistance: 相电阻 (Ohm)
  */
void BSP_Motor_SetResistance(float resistance);

/**
  * @brief  设置相电感
  * @param  inductance: 相电感 (H)
  */
void BSP_Motor_SetInductance(float inductance);

/**
  * @brief  设置极对数
  * @param  pole_pairs: 极对数
  */
void BSP_Motor_SetPolePairs(uint16_t pole_pairs);

/**
  * @brief  设置开环强拖速度
  * @param  speed: 速度 (RPM)
  */
void BSP_Motor_SetOpenloopSpeed(uint16_t speed);

#endif

