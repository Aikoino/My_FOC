/**
  ******************************************************************************
  * @file           : UserData_Config.h
  * @brief          : MiniFOC 用户配置 - 电机参数和控制模式
  * @attention
  *
  * 在此配置你的电机和控制参数
  * 修改后重新编译即可生效
  *
  ******************************************************************************
  */
#ifndef __USERDATA_CONFIG_H__
#define __USERDATA_CONFIG_H__

#include "MiniFOC_Config.h"

/* ========== 电机参数（根据你的实际电机修改）========== */

/* 极对数 - LA034-040NN07A 为8极（4对极） */
#define MOTOR_POLE_PAIRS     4

/* 额定参数 */
#define MOTOR_RATED_CURRENT  5.0f       /* 额定电流 (A) */
#define MOTOR_RATED_SPEED    3000.0f    /* 额定转速 (rpm) */
#define MOTOR_RATED_VOLTAGE  24.0f      /* 额定电压 (V) */

/* 极限参数 */
#define MOTOR_MAX_SPEED      5000.0f    /* 最大转速 (rpm) */
#define MOTOR_MAX_CURRENT    8.0f       /* 最大允许电流 (A) */

/* 电机电气参数（可选，用于观测器）*/
#define MOTOR_PHASE_RESISTANCE  0.5f    /* 相电阻 (Ω) */
#define MOTOR_PHASE_INDUCTANCE  0.001f  /* 相电感 (H) */

/* ========== 控制模式（单选）========== */

/* 选择你的控制模式 */
#define USER_CONTROL_MODE    MODE_Current_SINGLE   /* 电流单闭环（推荐先测试） */
// #define USER_CONTROL_MODE  MODE_VelCur_DOUBLE    /* 速度-电流双闭环 */
// #define USER_CONTROL_MODE  MODE_Sensor_Hall      /* 霍尔传感器 */
// #define USER_CONTROL_MODE  MODE_Sensorless_SMO   /* 无感SMO */
// #define USER_CONTROL_MODE  MODE_VF_OPENLOOP      /* VF开环（调试用） */

/* ========== PID参数（需要根据电机调整）========== */

/* 电流环PID */
#define USER_CURRENT_KP      0.1f
#define USER_CURRENT_KI      0.01f
#define USER_CURRENT_KD      0.0f
#define USER_CURRENT_LIMIT   5.0f        /* 电流限幅 (A) */

/* 速度环PID（仅在速度环模式使用）*/
#define USER_SPEED_KP        0.5f
#define USER_SPEED_KI        0.1f
#define USER_SPEED_KD        0.0f
#define USER_SPEED_LIMIT     8.0f        /* 速度环输出限幅 (A) */

/* ========== 调速方式（可选，多选）========== */

/* 默认调速方式 */
#define USER_CMD_SOURCE      CMD_SOURCE_UART  /* 串口调速（用VOFA+） */
// #define USER_CMD_SOURCE    CMD_SOURCE_CAN   /* CAN调速 */

/* ========== 传感器配置 ========== */

/* 编码器（如果使用）*/
#define USER_ENCODER_PPR     1000         /* 编码器线数 */
#define USER_ENCODER_MODE    ENCODER_ABZ  /* 编码器类型 */

/* 霍尔传感器（如果使用）*/
#define USER_HALL_POLARITY   1            /* 霍尔极性 */

/* ========== 保护参数 ========== */

/* 过流保护 */
#define USER_OVER_CURRENT_LIMIT    10.0f   /* 过流阈值 (A) */
#define USER_OVER_CURRENT_TIME     100     /* 持续时间 (ms) */

/* 电压保护 */
#define USER_UNDER_VOLTAGE_LIMIT   18.0f   /* 欠压阈值 (V) */
#define USER_OVER_VOLTAGE_LIMIT    30.0f   /* 过压阈值 (V) */

/* 过温保护（如果支持）*/
#define USER_OVER_TEMP_LIMIT       80      /* 过温阈值 (°C) */

/* ========== 调试参数 ========== */

#define USER_DEBUG_VOFA_ENABLE      1      /* 启用VOFA+ */
#define USER_DEBUG_CAN_ENABLE       1      /* 启用CAN调试 */
#define USER_DEBUG_LOG_ENABLE       0      /* 启用串口日志 */

#endif /* __USERDATA_CONFIG_H__ */
