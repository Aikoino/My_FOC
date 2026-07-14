/**
 * @file  motor_cmd.h
 * @brief 电机控制命令协议（串口 + CAN 通用）
 * @date   2026-07-02
 *
 * 命令格式（8 字节）：
 * [CMD_ID][PARAM1][PARAM2][PARAM3][PARAM4][PARAM5][PARAM6][CHECKSUM]
 *
 * 支持串口和 CAN 总线控制
 */

#ifndef __MOTOR_CMD_H
#define __MOTOR_CMD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>

/* 命令 ID 定义 --------------------------------------------------------------*/
typedef enum {
    CMD_GET_VERSION      = 0x00,  // 获取版本号
    CMD_MOTOR_START      = 0x01,  // 电机启动
    CMD_MOTOR_STOP       = 0x02,  // 电机停止
    CMD_SET_SPEED        = 0x03,  // 设置转速 (RPM)
    CMD_GET_SPEED        = 0x04,  // 读取转速
    CMD_SET_SPEED_KP     = 0x05,  // 设置速度环 Kp
    CMD_GET_SPEED_KP     = 0x06,  // 读取速度环 Kp
    CMD_SET_SPEED_KI     = 0x07,  // 设置速度环 Ki
    CMD_GET_SPEED_KI     = 0x08,  // 读取速度环 Ki
    CMD_SET_CURRENT_KP   = 0x09,  // 设置电流环 Kp
    CMD_GET_CURRENT_KP   = 0x0A,  // 读取电流环 Kp
    CMD_SET_CURRENT_KI   = 0x0B,  // 设置电流环 Ki
    CMD_GET_CURRENT_KI   = 0x0C,  // 读取电流环 Ki
    CMD_SET_FLUX         = 0x0D,  // 设置磁链常数
    CMD_GET_FLUX         = 0x0E,  // 读取磁链常数
    CMD_SET_RESISTANCE   = 0x0F,  // 设置相电阻
    CMD_GET_RESISTANCE   = 0x10,  // 读取相电阻
    CMD_SET_INDUCTANCE   = 0x11,  // 设置相电感
    CMD_GET_INDUCTANCE   = 0x12,  // 读取相电感
    CMD_SET_POLE_PAIRS   = 0x13,  // 设置极对数
    CMD_GET_POLE_PAIRS   = 0x14,  // 读取极对数
    CMD_SET_OPENLOOP_SPD = 0x15,  // 设置开环强拖速度
    CMD_GET_OPENLOOP_SPD = 0x16,  // 读取开环强拖速度
    CMD_SET_VBUS_MAX     = 0x17,  // 设置母线电压上限
    CMD_GET_VBUS_MAX     = 0x18,  // 读取母线电压上限
    CMD_SET_VBUS_MIN     = 0x19,  // 设置母线电压下限
    CMD_GET_VBUS_MIN     = 0x1A,  // 读取母线电压下限
    CMD_SET_MODE         = 0x1B,  // 设置控制模式 (0=有感, 1=无感)
    CMD_GET_MODE         = 0x1C,  // 读取控制模式
    CMD_SAVE_TO_FLASH    = 0x1D,  // 保存参数到 Flash
    CMD_LOAD_FROM_FLASH  = 0x1E,  // 从 Flash 加载参数
    CMD_RESET_DEFAULT    = 0x1F,  // 恢复默认参数
    CMD_GET_STATUS       = 0x20,  // 获取电机状态
    CMD_GET_ALL_PARAMS   = 0x21,  // 获取所有参数
    CMD_SET_ALL_PARAMS   = 0x22,  // 设置所有参数
} MotorCmdID_t;

/* 控制模式定义 --------------------------------------------------------------*/
typedef enum {
    MOTOR_MODE_SENSORED   = 0x00,  // 有感控制
    MOTOR_MODE_SENSORLESS = 0x01,  // 无感控制
} MotorMode_t;

/* 电机状态定义 --------------------------------------------------------------*/
typedef enum {
    MOTOR_STATUS_STOPPED = 0x00,  // 停止
    MOTOR_STATUS_RUNNING = 0x01,  // 运行
    MOTOR_STATUS_FAULT   = 0x02,  // 故障
} MotorStatus_t;

/* 响应状态定义 --------------------------------------------------------------*/
typedef enum {
    RESP_OK        = 0x00,  // 成功
    RESP_INVALID   = 0x01,  // 无效命令
    RESP_PARAM_ERR = 0x02,  // 参数错误
    RESP_WRITE_ERR = 0x03,  // 写入失败
    RESP_READ_ERR  = 0x04,  // 读取失败
    RESP_BUSY      = 0x05,  // 忙（电机运行中）
} ResponseStatus_t;

/* 默认参数值 ----------------------------------------------------------------*/
#define DEFAULT_SPEED_KP        0.5f    // 速度环 Kp
#define DEFAULT_SPEED_KI        0.1f    // 速度环 Ki
// DEFAULT_CURRENT_KP / DEFAULT_CURRENT_KI 定义在 MiniFOC_Config.h 中，此处省略
#define DEFAULT_FLUX            0.016884f  // 磁链常数 - 参考模型验证值
#define DEFAULT_RESISTANCE      6.97f      // 相电阻 (Ohm) - 参考模型验证值
#define DEFAULT_INDUCTANCE      0.00535f   // 相电感 (H) - 参考模型验证值
#define DEFAULT_POLE_PAIRS      2          // 极对数 - 参考模型验证值
#define DEFAULT_OPENLOOP_SPD    1000    // 开环强拖速度 (RPM)
#define DEFAULT_VBUS_MAX        450     // 母线电压上限 (0.1V) = 45.0V
#define DEFAULT_VBUS_MIN        250     // 母线电压下限 (0.1V) = 25.0V
#define DEFAULT_MODE            MOTOR_MODE_SENSORLESS  // 默认无感

/**
 * @brief 电机参数结构体
 */
typedef struct {
    float   speed_kp;         // 速度环 Kp
    float   speed_ki;         // 速度环 Ki
    float   current_kp;       // 电流环 Kp
    float   current_ki;       // 电流环 Ki
    float   flux;             // 磁链常数
    float   resistance;       // 相电阻
    float   inductance;       // 相电感
    uint16_t pole_pairs;      // 极对数
    uint16_t openloop_speed;  // 开环强拖速度 (RPM)
    uint16_t vbus_max;        // 母线电压上限 (0.1V)
    uint16_t vbus_min;        // 母线电压下限 (0.1V)
    MotorMode_t mode;         // 控制模式
    uint8_t  reserved[3];     // 保留
    uint32_t crc;             // CRC32 校验
} MotorParams_t;

/* 函数声明 ------------------------------------------------------------------*/
void MotorCmd_Init(void);
void MotorCmd_ProcessUart(uint8_t *data, uint16_t len);
void MotorCmd_ProcessCAN(uint32_t id, uint8_t *data, uint8_t len);
void MotorCmd_SendResponse(uint32_t can_id, uint8_t cmd_id, uint8_t status, uint8_t *data, uint8_t len);

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_CMD_H */
