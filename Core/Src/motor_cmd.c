/**
 * @file  motor_cmd.c
 * @brief 电机控制命令解析和处理
 * @date   2026-07-02
 */

/* Includes ------------------------------------------------------------------*/
#include "motor_cmd.h"
#include "bsp_motor.h"
#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/* Private defines ------------------------------------------------------------*/
#define MOTOR_CMD_FW_VERSION     0x0100  // 固件版本 1.0.0
#define FLASH_PARAM_ADDR         0x0807F000  // Flash 参数存储地址（最后一页）

/* 电流环默认参数（与 MiniFOC_Config.h 保持一致）*/
#define CURRENT_KP_DEFAULT       0.1f
#define CURRENT_KI_DEFAULT       0.01f

/* Private variables ----------------------------------------------------------*/
static MotorParams_t motor_params;
static uint8_t param_modified = 0;  // 参数修改标志

/* 函数声明（外部） -----------------------------------------------------------*/
extern void Error_Handler(void);

/**
 * @brief  简单的 CRC8 校验
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @retval CRC8 值
 */
static uint8_t CRC8(uint8_t *data, uint16_t len)
{
    uint8_t crc = 0x00;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/**
 * @brief  加载默认参数
 * @retval None
 */
static void Load_DefaultParams(void)
{
    motor_params.speed_kp        = DEFAULT_SPEED_KP;
    motor_params.speed_ki        = DEFAULT_SPEED_KI;
    motor_params.current_kp      = CURRENT_KP_DEFAULT;
    motor_params.current_ki      = CURRENT_KI_DEFAULT;
    motor_params.flux            = DEFAULT_FLUX;
    motor_params.resistance      = DEFAULT_RESISTANCE;
    motor_params.inductance      = DEFAULT_INDUCTANCE;
    motor_params.pole_pairs      = DEFAULT_POLE_PAIRS;
    motor_params.openloop_speed  = DEFAULT_OPENLOOP_SPD;
    motor_params.vbus_max        = DEFAULT_VBUS_MAX;
    motor_params.vbus_min        = DEFAULT_VBUS_MIN;
    motor_params.mode            = DEFAULT_MODE;
    memset(motor_params.reserved, 0, sizeof(motor_params.reserved));
    motor_params.crc             = 0;  // 重新计算
}

/**
 * @brief  计算参数 CRC32
 * @retval CRC32 值
 */
static uint32_t Calc_ParamCRC(void)
{
    uint32_t crc = 0;
    uint8_t *p = (uint8_t *)&motor_params;
    for (uint16_t i = 0; i < offsetof(MotorParams_t, crc); i++) {
        crc ^= p[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80000000) {
                crc = (crc << 1) ^ 0xEDB88320;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/**
 * @brief  初始化命令模块
 * @retval None
 */
void MotorCmd_Init(void)
{
    // 尝试从 Flash 加载参数
    memcpy(&motor_params, (void *)FLASH_PARAM_ADDR, sizeof(MotorParams_t));

    // 验证 CRC
    if (motor_params.crc != Calc_ParamCRC()) {
        // CRC 校验失败，加载默认参数
        Load_DefaultParams();
    }

    // 调试：注释掉printf看是不是这里卡住
    // printf("Motor CMD Init: Speed=%.2f, Kp=%.2f, Ki=%.2f\r\n",
    //        motor_params.speed_kp, motor_params.speed_ki, motor_params.mode);
}

/**
 * @brief  解析命令数据（通用）
 * @param  cmd_id: 命令 ID
 * @param  data: 数据缓冲区（8 字节）
 * @retval 响应状态
 */
static ResponseStatus_t Parse_Command(uint8_t cmd_id, uint8_t *data)
{
    uint32_t param_u32;
    float param_f32;
    uint16_t param_u16;
    uint8_t param_u8;

    switch (cmd_id) {
        /* === 电机控制 === */
        case CMD_MOTOR_START:
            BSP_Motor_Start();
            return RESP_OK;

        case CMD_MOTOR_STOP:
            BSP_Motor_Stop();
            return RESP_OK;

        case CMD_SET_SPEED:
            memcpy(&param_u32, data, 4);
            BSP_Motor_SetSpeed((float)param_u32 / 10.0f);  // 输入 0.1 RPM
            return RESP_OK;

        case CMD_SET_MODE:
            param_u8 = data[0];
            if (param_u8 <= MOTOR_MODE_SENSORLESS) {
                motor_params.mode = (MotorMode_t)param_u8;
                BSP_Motor_SetMode(motor_params.mode);
                param_modified = 1;
                return RESP_OK;
            }
            return RESP_PARAM_ERR;

        /* === 速度环参数 === */
        case CMD_SET_SPEED_KP:
            memcpy(&param_f32, data, 4);
            motor_params.speed_kp = param_f32;
            BSP_Motor_SetSpeedKp(param_f32);
            param_modified = 1;
            return RESP_OK;

        case CMD_SET_SPEED_KI:
            memcpy(&param_f32, data, 4);
            motor_params.speed_ki = param_f32;
            BSP_Motor_SetSpeedKi(param_f32);
            param_modified = 1;
            return RESP_OK;

        /* === 电流环参数 === */
        case CMD_SET_CURRENT_KP:
            memcpy(&param_f32, data, 4);
            motor_params.current_kp = param_f32;
            BSP_Motor_SetCurrentKp(param_f32);
            param_modified = 1;
            return RESP_OK;

        case CMD_SET_CURRENT_KI:
            memcpy(&param_f32, data, 4);
            motor_params.current_ki = param_f32;
            BSP_Motor_SetCurrentKi(param_f32);
            param_modified = 1;
            return RESP_OK;

        /* === 电机参数 === */
        case CMD_SET_FLUX:
            memcpy(&param_f32, data, 4);
            motor_params.flux = param_f32;
            BSP_Motor_SetFlux(param_f32);
            param_modified = 1;
            return RESP_OK;

        case CMD_SET_RESISTANCE:
            memcpy(&param_f32, data, 4);
            motor_params.resistance = param_f32;
            BSP_Motor_SetResistance(param_f32);
            param_modified = 1;
            return RESP_OK;

        case CMD_SET_INDUCTANCE:
            memcpy(&param_f32, data, 4);
            motor_params.inductance = param_f32;
            BSP_Motor_SetInductance(param_f32);
            param_modified = 1;
            return RESP_OK;

        case CMD_SET_POLE_PAIRS:
            memcpy(&param_u16, data, 2);
            motor_params.pole_pairs = param_u16;
            BSP_Motor_SetPolePairs(param_u16);
            param_modified = 1;
            return RESP_OK;

        case CMD_SET_OPENLOOP_SPD:
            memcpy(&param_u16, data, 2);
            motor_params.openloop_speed = param_u16;
            BSP_Motor_SetOpenloopSpeed(param_u16);
            param_modified = 1;
            return RESP_OK;

        /* === 电源参数 === */
        case CMD_SET_VBUS_MAX:
            memcpy(&param_u16, data, 2);
            motor_params.vbus_max = param_u16;
            param_modified = 1;
            return RESP_OK;

        case CMD_SET_VBUS_MIN:
            memcpy(&param_u16, data, 2);
            motor_params.vbus_min = param_u16;
            param_modified = 1;
            return RESP_OK;

        /* === Flash 操作 === */
        case CMD_SAVE_TO_FLASH:
            motor_params.crc = Calc_ParamCRC();
            HAL_FLASH_Unlock();
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, FLASH_PARAM_ADDR, *(uint64_t *)&motor_params);
            HAL_FLASH_Lock();
            param_modified = 0;
            return RESP_OK;

        case CMD_LOAD_FROM_FLASH:
            memcpy(&motor_params, (void *)FLASH_PARAM_ADDR, sizeof(MotorParams_t));
            if (motor_params.crc == Calc_ParamCRC()) {
                // 应用参数到电机驱动
                BSP_Motor_SetSpeedKp(motor_params.speed_kp);
                BSP_Motor_SetSpeedKi(motor_params.speed_ki);
                BSP_Motor_SetCurrentKp(motor_params.current_kp);
                BSP_Motor_SetCurrentKi(motor_params.current_ki);
                return RESP_OK;
            }
            return RESP_READ_ERR;

        case CMD_RESET_DEFAULT:
            Load_DefaultParams();
            return RESP_OK;

        default:
            return RESP_INVALID;
    }
}

/**
 * @brief  读取参数数据（通用）
 * @param  cmd_id: 命令 ID
 * @param  data: 数据缓冲区（用于填充返回值）
 * @retval 响应状态
 */
static ResponseStatus_t Read_Parameter(uint8_t cmd_id, uint8_t *data)
{
    uint32_t param_u32;
    float param_f32;
    uint16_t param_u16;
    uint8_t param_u8;

    switch (cmd_id) {
        case CMD_GET_SPEED:
            param_u32 = (uint32_t)(BSP_Motor_GetSpeed() * 10.0f);
            memcpy(data, &param_u32, 4);
            return RESP_OK;

        case CMD_GET_SPEED_KP:
            memcpy(&param_f32, &motor_params.speed_kp, 4);
            memcpy(data, &param_f32, 4);
            return RESP_OK;

        case CMD_GET_SPEED_KI:
            memcpy(&param_f32, &motor_params.speed_ki, 4);
            memcpy(data, &param_f32, 4);
            return RESP_OK;

        case CMD_GET_CURRENT_KP:
            memcpy(&param_f32, &motor_params.current_kp, 4);
            memcpy(data, &param_f32, 4);
            return RESP_OK;

        case CMD_GET_CURRENT_KI:
            memcpy(&param_f32, &motor_params.current_ki, 4);
            memcpy(data, &param_f32, 4);
            return RESP_OK;

        case CMD_GET_FLUX:
            memcpy(&param_f32, &motor_params.flux, 4);
            memcpy(data, &param_f32, 4);
            return RESP_OK;

        case CMD_GET_RESISTANCE:
            memcpy(&param_f32, &motor_params.resistance, 4);
            memcpy(data, &param_f32, 4);
            return RESP_OK;

        case CMD_GET_INDUCTANCE:
            memcpy(&param_f32, &motor_params.inductance, 4);
            memcpy(data, &param_f32, 4);
            return RESP_OK;

        case CMD_GET_POLE_PAIRS:
            memcpy(&param_u16, &motor_params.pole_pairs, 2);
            memcpy(data, &param_u16, 2);
            return RESP_OK;

        case CMD_GET_OPENLOOP_SPD:
            memcpy(&param_u16, &motor_params.openloop_speed, 2);
            memcpy(data, &param_u16, 2);
            return RESP_OK;

        case CMD_GET_VBUS_MAX:
            memcpy(&param_u16, &motor_params.vbus_max, 2);
            memcpy(data, &param_u16, 2);
            return RESP_OK;

        case CMD_GET_VBUS_MIN:
            memcpy(&param_u16, &motor_params.vbus_min, 2);
            memcpy(data, &param_u16, 2);
            return RESP_OK;

        case CMD_GET_MODE:
            param_u8 = (uint8_t)motor_params.mode;
            memcpy(data, &param_u8, 1);
            return RESP_OK;

        case CMD_GET_VERSION:
            param_u16 = MOTOR_CMD_FW_VERSION;
            memcpy(data, &param_u16, 2);
            return RESP_OK;

        case CMD_GET_STATUS:
            param_u8 = BSP_Motor_IsRunning() ? MOTOR_STATUS_RUNNING : MOTOR_STATUS_STOPPED;
            memcpy(data, &param_u8, 1);
            return RESP_OK;

        default:
            return RESP_INVALID;
    }
}

/**
 * @brief  处理串口命令
 * @param  data: 数据缓冲区（8 字节）
 * @param  len: 数据长度
 * @retval None
 */
void MotorCmd_ProcessUart(uint8_t *data, uint16_t len)
{
    if (data == NULL || len < 2) {
        return;
    }

    uint8_t cmd_id = data[0];
    uint8_t rx_crc = data[1];
    uint8_t calc_crc = CRC8(data, 1);

    // CRC 校验
    if (rx_crc != calc_crc) {
        printf("CMD CRC Error: rx=0x%02X, calc=0x%02X\r\n", rx_crc, calc_crc);
        return;
    }

    // 解析命令
    ResponseStatus_t status;
    uint8_t resp_data[8] = {0};

    // 判断是读命令还是写命令
    if (cmd_id % 2 == 0) {  // 读命令（偶数）
        status = Read_Parameter(cmd_id, resp_data);
    } else {  // 写命令（奇数）
        status = Parse_Command(cmd_id, &data[2]);
    }

    // 发送响应
    uint8_t resp[11];
    resp[0] = 0xAA;  // 帧头
    resp[1] = cmd_id;
    resp[2] = status;
    memcpy(&resp[3], resp_data, 8);

    HAL_UART_Transmit(&huart3, resp, 11, 100);
}

/**
 * @brief  处理 CAN 命令
 * @param  id: CAN ID
 * @param  data: 数据缓冲区（8 字节）
 * @param  len: 数据长度
 * @retval None
 */
void MotorCmd_ProcessCAN(uint32_t id, uint8_t *data, uint8_t len)
{
    if (data == NULL || len < 2) {
        return;
    }

    uint8_t cmd_id = data[0];

    // 解析命令
    ResponseStatus_t status;
    uint8_t resp_data[8] = {0};

    if (cmd_id % 2 == 0) {  // 读命令
        status = Read_Parameter(cmd_id, resp_data);
    } else {  // 写命令
        status = Parse_Command(cmd_id, &data[1]);
    }

    // 发送 CAN 响应（ID+1）
    uint8_t tx_data[8];
    tx_data[0] = cmd_id;
    tx_data[1] = status;
    memcpy(&tx_data[2], resp_data, 6);

    CAN_Send(id + 1, tx_data, 8);
}

/**
 * @brief  发送 CAN 响应（兼容旧接口）
 * @param  can_id: 响应 CAN ID
 * @param  cmd_id: 命令 ID
 * @param  status: 响应状态
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @retval None
 */
void MotorCmd_SendResponse(uint32_t can_id, uint8_t cmd_id, uint8_t status, uint8_t *data, uint8_t len)
{
    uint8_t tx_data[8];
    tx_data[0] = cmd_id;
    tx_data[1] = status;
    if (data && len > 0) {
        memcpy(&tx_data[2], data, (len < 6) ? len : 6);
    }

    CAN_Send(can_id, tx_data, 8);
}

/**
 * @brief  获取当前电机参数
 * @retval 参数指针
 */
MotorParams_t *MotorCmd_GetParams(void)
{
    return &motor_params;
}

/**
 * @brief  检查参数是否被修改
 * @retval 1=已修改, 0=未修改
 */
uint8_t MotorCmd_IsParamModified(void)
{
    return param_modified;
}
