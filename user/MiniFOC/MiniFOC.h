/**
  ******************************************************************************
  * @file           : MiniFOC.h
  * @brief          : MiniFOC 核心头文件
  * @version        : 1.0.0
  * @author         : YourName
  * @date           : 2026-06-30
  *
  * 基于 SguanFOC v3.1.0 裁剪优化
  * 适用于 STM32G431CBT6 + FD6288 + L9616CAN
  *
  ******************************************************************************
  */
#ifndef __MINIFOC_H__
#define __MINIFOC_H__

#include <stdint.h>
#include <stdbool.h>
#include "MiniFOC_math.h"
#include "MiniFOC_PID.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========== 版本信息 ========== */
#define MINIFOC_VERSION_MAJOR  1
#define MINIFOC_VERSION_MINOR  0
#define MINIFOC_VERSION_PATCH  0

/* ========== 控制模式选择 ========== */
typedef enum {
    MODE_VF_OPENLOOP      = 0,   /* VF压频比开环（开环角度） */
    MODE_Current_SINGLE   = 3,   /* 电流单闭环（VF角度） */
    MODE_VelCur_DOUBLE    = 4,   /* 速度-电流串级闭环（VF角度+速度环） */
    MODE_Sensor_Hall_I    = 6,   /* 霍尔电流环（霍尔角度，电流闭环） */
    MODE_Sensor_Hall_S    = 7,   /* 霍尔速度环（霍尔角度+速度环） */
} ControlMode_t;

/* ========== 速度指令源 ========== */
typedef enum {
    CMD_SOURCE_UART       = 0,   /* 串口指令 */
    CMD_SOURCE_CAN        = 1,   /* CAN指令 */
    CMD_SOURCE_POT        = 2,   /* 电位器 */
    CMD_SOURCE_ENCODER    = 3,   /* 编码器 */
} SpeedCmdSource_t;

/* ========== FOC状态结构体 ========== */

/**
  * @brief  FOC主状态结构体
  */
typedef struct {
    /* 控制模式 */
    ControlMode_t mode;              /* 当前控制模式 */
    SpeedCmdSource_t cmd_source;     /* 速度指令源 */

    /* 目标值 */
    float target_speed;              /* 目标转速 (rpm) */
    float target_current;            /* 目标电流 (A) */
    float target_position;           /* 目标位置 (可选) */

    /* 传感器反馈 */
    float rotor_angle;               /* 转子机械角度 (rad) */
    float rotor_speed;               /* 转子转速 (rpm) */
    float bus_voltage;               /* 母线电压 (V) */
    float phase_current_u;           /* U相电流 (A) */
    float phase_current_v;           /* V相电流 (A) */
    float phase_current_w;           /* W相电流 (A) */

    /* dq轴变量 */
    float Id, Iq;                    /* d轴/q轴电流反馈 */
    float Ud, Uq;                    /* d轴/q轴电压输出 */
    float Target_Id;                 /* d轴电流给定 */
    float Target_Iq;                 /* q轴电流给定 */

    /* SVPWM占空比（用于VOFA+显示）*/
    float duty_a, duty_b, duty_c;    /* U/V/W相占空比 (0-8000) */

    /* 电流采样零点校准 */
    float current_offset_u;          /* U相电流偏移 (A) */
    float current_offset_v;          /* V相电流偏移 (A) */
    float current_offset_w;          /* W相电流偏移 (A) */

    /* PID控制器 */
    PID_Controller_t current_pid;    /* 电流环PID（Q轴）*/
    PID_Controller_t current_pid_d;  /* 电流环PID（D轴）*/
    PID_Controller_t speed_pid;      /* 速度环PID */

    /* 状态标志 */
    bool motor_running;              /* 电机运行标志 */
    bool fault_flag;                 /* 故障标志 */
    uint32_t fault_code;             /* 故障码 */

    /* VF 开环 */
    float vf_elec_angle;             /* VF 开环电气角 (rad) */
    uint32_t vf_start_time;          /* 启动时间戳（用于 Kick-start） */

    /* 预定位（Hall 模式启动用）*/
    uint32_t pre_position_start;     /* 预定位开始时间戳 */
    bool pre_position_active;        /* 是否在预定位阶段 */

    float rotor_sine;                /* 电角度 sin 值 */
    float rotor_cosine;              /* 电角度 cos 值 */

    /* 霍尔角度校准偏移（rad）*/
    float hall_angle_offset;         /* Hall 模式电角度校准偏移 */

    /* ========== 无感观测器（暂未使用）========== */
    bool observer_enabled;         /* 无感观测器使能标志 */
    uint8_t observer_mode;         /* 0=霍尔, 1=无感, 2=切换中 */
    float observer_speed_threshold; /* 切换速度阈值 (rpm) */

} MiniFOC_t;

/* ========== 全局变量声明 ========== */
extern MiniFOC_t foc;

/* ========== 函数声明 ========== */

/**
  * @brief  MiniFOC初始化
  */
void MiniFOC_Init(void);

/**
  * @brief  主循环任务 (1kHz调用)
  * @note   处理速度环、状态机、指令接收
  */
void MiniFOC_MainLoop(void);

/**
  * @brief  高频任务 (10kHz调用，在TIM1中断中)
  * @note   处理电流环、SVPWM、坐标变换
  */
void MiniFOC_HighFreqLoop(void);

/**
  * @brief  设置控制模式
  */
void MiniFOC_SetMode(ControlMode_t mode);

/**
  * @brief  设置目标速度
  * @param  speed: 目标转速 (rpm)
  */
void MiniFOC_SetTargetSpeed(float speed);

/**
  * @brief  设置目标电流
  * @param  current: 目标电流 (A)
  */
void MiniFOC_SetTargetCurrent(float current);

/**
  * @brief  电机启停控制
  * @param  enable: true=启动, false=停止
  */
void MiniFOC_MotorEnable(bool enable);

/**
  * @brief  获取当前转速
  * @retval 转速 (rpm)
  */
float MiniFOC_GetSpeed(void);

/**
  * @brief  获取当前电流
  * @retval 电流 (A)
  */
float MiniFOC_GetCurrent(void);

/**
  * @brief  故障处理
  */
void MiniFOC_FaultHandler(uint32_t fault_code);

/**
  * @brief  更新转子角度和转速（由 BSP 调用）
  * @param  angle: 转子机械角度（rad）
  * @param  speed: 转子转速（rpm）
  */
void MiniFOC_UpdateRotorState(float angle, float speed);

/**
  * @brief  更新三相电流（由 BSP 调用）
  * @param  iu: U 相电流（A）
  * @param  iv: V 相电流（A）
  */
void MiniFOC_UpdateCurrent(float iu, float iv);

/**
  * @brief  V/F 开环控制步骤（内部函数）
  */
void MiniFOC_VF_Step(void);

/**
  * @brief  电流环闭环控制（内部函数）
  */
void MiniFOC_CurrentLoop(void);

/**
  * @brief  速度环控制（内部函数）
  */
void MiniFOC_SpeedLoop(void);

/**
  * @brief  设置霍尔角度校准偏移（用于修正霍尔传感器零点偏差）
  * @param  offset_rad: 偏移角度（rad），例如 PI/2 = 1.5708f
  */
void MiniFOC_SetHallAngleOffset(float offset_rad);

/* ========== 无感观测器接口 ========== */

/**
  * @brief  更新无感观测器（在ADC中断中调用）
  * @param  I_alpha: α轴电流 (A)
  * @param  I_beta: β轴电流 (A)
  * @param  U_alpha: α轴电压 (V)
  * @param  U_beta: β轴电压 (V)
  * @retval None
  */
void MiniFOC_UpdateObserver(float I_alpha, float I_beta, float U_alpha, float U_beta);

/**
  * @brief  获取观测器电角度
  * @retval 电角度 (rad)
  */
float MiniFOC_GetObserverAngle(void);

/**
  * @brief  获取观测器转速
  * @retval 转速 (rpm)
  */
float MiniFOC_GetObserverSpeed(void);

/**
  * @brief  获取当前观测器模式
  * @retval 0=霍尔, 1=无感, 2=切换中
  */
uint8_t MiniFOC_GetObserverMode(void);

#ifdef __cplusplus
}
#endif

#endif /* __MINIFOC_H__ */
