/**
  ******************************************************************************
  * @file           : MiniFOC_Config.h
  * @brief          : MiniFOC 用户配置头文件
  * @attention
  *
  * 在此配置你的电机参数和控制参数
  * 根据实际硬件修改以下参数
  *
  ******************************************************************************
  */
#ifndef __MINIFOC_CONFIG_H__
#define __MINIFOC_CONFIG_H__

/* ========== 硬件配置 ========== */

/* 主频 (Hz) */
#define SYSTEM_CLOCK_HZ     170000000UL   /* STM32G431 170MHz */

/* ADC参数 */
#define ADC_VOLTAGE_REF      3.3f          /* ADC参考电压 (V) */
#define ADC_RESOLUTION       12            /* ADC位数 */
#define ADC_MAX_VALUE        4095          /* 2^12 - 1 */

/* 母线电压分压系数（根据实际分压电阻计算）*/
/* 公式：(R4 + R9) / R9 = (75kΩ + 3kΩ) / 3kΩ = 26 */
#define VBUS_DIVIDER_RATIO    26.0f        /* 分压系数 26:1 */

/* PWM参数 */
#define PWM_FREQUENCY        20000         /* PWM频率 (Hz) */
#define PWM_PERIOD           (SYSTEM_CLOCK_HZ / 2 / PWM_FREQUENCY)  /* TIM1时钟二分频 */
#define DEAD_TIME_NS         120           /* 死区时间 (ns) - FD6288 推荐 100~200ns */

/* 电流采样参数 */
#define CURRENT_SAMPLE_RATE  20000         /* 电流采样频率 (Hz) */
#define CURRENT_SENSOR_GAIN  20.0f         /* 电流传感器增益 (V/A) */
#define SHUNT_RESISTOR      0.01f          /* 采样电阻 (Ω) */
#define CURRENT_SAMPLE_RES  0.021972f      /* 电流转换系数 (A/bit) = 3.3V/4096 * SHUNT_RESISTOR * GAIN */

/* ========== 电机参数（LA034 040NN07A）========== */

/* 电机类型 */
#define MOTOR_TYPE           MOTOR_PMSM    /* PMSM永磁同步电机 */

/* 电机电气参数 - LA034 典型值 */
#define MOTOR_POLE_PAIRS     7             /* 极对数 (14极) */
#define MOTOR_RATED_CURRENT  4.5f          /* 额定电流 (A) */
#define MOTOR_RATED_SPEED    3000.0f       /* 额定转速 (rpm) */
#define MOTOR_MAX_SPEED      5000.0f       /* 最大转速 (rpm) */
#define MOTOR_MAX_CURRENT    8.0f          /* 最大电流 (A) */
#define MOTOR_RATED_VOLTAGE  24.0f         /* 额定电压 (V) */

/* 电机电阻/电感 - LA034 典型值 */
#define MOTOR_PHASE_RESISTANCE  0.5f       /* 相电阻 (Ω) - 保守值 */
#define MOTOR_PHASE_INDUCTANCE  0.001f     /* 相电感 (H) - 保守值 */

/* ========== 控制参数 ========== */

/* 默认控制模式（可在UserData_Config.h中修改）*/
#define DEFAULT_CONTROL_MODE     MODE_Current_SINGLE   /* 电流单闭环 */

/* 速度指令源 */
#define DEFAULT_CMD_SOURCE       CMD_SOURCE_UART       /* 串口调速 */

/* PID参数 - 电流环（保守调试参数）*/
#define DEFAULT_CURRENT_KP       0.5f      /* 增大 Kp（原来是 0.1）*/
#define DEFAULT_CURRENT_KI       0.05f     /* 增大 Ki（原来是 0.01）*/
#define DEFAULT_CURRENT_KD       0.0f
#define DEFAULT_CURRENT_LIMIT    8.0f      /* 降低输出限幅（原来是 5A）*/

/* PID参数 - 速度环 */
#define DEFAULT_SPEED_KP         1.0f
#define DEFAULT_SPEED_KI         0.5f
#define DEFAULT_SPEED_KD         0.0f
#define DEFAULT_SPEED_LIMIT      5.0f      /* 降低限幅（原来是 8A）*/

/* 电流限幅 */
#define CURRENT_LIMIT            8.0f   /* 硬件限流 (A) */

/* ========== 传感器配置 ========== */

/* 编码器配置（如果使用）*/
#define ENCODER_PPR              1000   /* 编码器线数 (Pulse Per Revolution) */
#define ENCODER_POLE_PAIRS       1      /* 编码器倍频 */

/* 霍尔传感器配置（如果使用）*/
#define USE_HALL_SENSOR          1      /* 1=启用霍尔传感器, 0=禁用 */
#define HALL_SENSOR_POLARITY     1      /* 霍尔极性 (1或-1) */

/* ========== 无感观测器配置（可选）========== */

/* SMO滑模观测器 */
#define SMO_SLIDE_GAIN           0.1f
#define SMO_FILTER_CUTOFF        50.0f  /* 滤波器截止频率 (Hz) */

/* HFI高频注入 */
#define HFI_INJECTION_VOLTAGE    2.0f   /* 注入电压幅值 (V) */
#define HFI_INJECTION_FREQ       1000.0f/* 注入频率 (Hz) */

/* ========== 保护参数 ========== */

/* 过流保护 (A) */
#define OVER_CURRENT_LIMIT       10.0f
#define OVER_CURRENT_TIME        100    /* 持续时间 (ms) */

/* 欠压保护 (V) */
#define UNDER_VOLTAGE_LIMIT      18.0f
#define OVER_VOLTAGE_LIMIT       30.0f

/* 过温保护 (°C) - 如果支持温度采样 */
#define OVER_TEMP_LIMIT          80

/* ========== 调试配置 ========== */

/* 调试输出 */
#define DEBUG_VOFA_ENABLE         1     /* 1=启用VOFA+, 0=禁用 */
#define DEBUG_VOFA_FREQ           100   /* VOFA+发送频率 (Hz) */

/* CAN调试 */
#define DEBUG_CAN_ENABLE          1     /* 1=启用CAN调试, 0=禁用 */
#define DEBUG_CAN_TX_ID           0x125
#define DEBUG_CAN_RX_ID           0x126

#endif /* __MINIFOC_CONFIG_H__ */
