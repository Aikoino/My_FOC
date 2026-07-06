/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : bsp_hall.c
 * @brief          : 霍尔传感器驱动实现（对齐 SguanFOC v3.1.0）
 * @description    : 三路霍尔信号读取 + 6扇区判断 + 电角度映射 + 线性插值
 * @dependencies   : bsp_hall.h, stm32g4xx_hal.h
 * @note           : 完全对齐 SguanFOC_Library-main v3.1.0/Sguan_Hall.c
 ******************************************************************************
 * @attention
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 ******************************************************************************
 */
/* USER CODE END Header */

#include "bsp_hall.h"
#include "MiniFOC/MiniFOC_Transform.h"
#include "MiniFOC/MiniFOC_Config.h"
#include "stm32g4xx_hal.h"
#include <string.h>
#include <math.h>

/* ========== 全局变量 ========== */

BSP_Hall_t hall_sensor;

/* ========== 内部宏定义 ========== */

/* 霍尔扇区映射表（根据注释修正：Sector 5 → 180° = 3*60°，map[5] = 3）*/
/* 索引：扇区 1~6，值：0~5 扇区编号 */
static const uint8_t hall_sector_map[7] = {0, 5, 0, 1, 4, 3, 2};

/* 扇区中心电角度（rad）：扇区 1~6 → 300°, 0°, 60°, 240°, 180°, 120° */
static const float sector_center_angle[7] = {
    0.0f,                      /* 索引 0 不使用 */
    5.2359878f,                /* 扇区 1 → 300° = 5*π/3 */
    0.0f,                      /* 扇区 2 → 0°   = 0 */
    1.0471976f,                /* 扇区 3 → 60°  = π/3 */
    4.1887902f,                /* 扇区 4 → 240° = 4*π/3 */
    3.1415927f,                /* 扇区 5 → 180° = π   */
    2.0943951f                 /* 扇区 6 → 120° = 2*π/3 */
};

/* 正转顺序（扇区编号）：2→3→6→5→4→1→2... */
static const uint8_t fwd_order[6] = {2, 3, 6, 5, 4, 1};

/* ========== 内部函数声明 ========== */

/**
  * @brief  霍尔传感器 GPIO 初始化
  * @note   配置 PB6/PB7/PB8 为浮空输入
  *         外部已有 10kΩ 上拉到 3.3V，不需要内部上拉
  */
static void BSP_Hall_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 1. 使能 GPIOB 时钟 */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* 2. 配置 PB6/PB7/PB8 为浮空输入
     *    外部已有 10kΩ 上拉电阻到 3.3V
     *    所以这里用 GPIO_NOPULL，避免与外部上拉冲突
     */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;      /* 输入模式 */
    GPIO_InitStruct.Pull = GPIO_NOPULL;           /* 无上下拉（外部已有 10kΩ 上拉）*/
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* ========== 内部辅助函数 ========== */

/**
  * @brief  查找扇区在正转序列中的索引（0~5）
  * @param  sector: 扇区编号（1~6）
  * @retval 索引（0~5），无效扇区返回 255
  */
static uint8_t hall_fwd_index(uint8_t sector)
{
    for (uint8_t i = 0; i < 6; i++) {
        if (fwd_order[i] == sector) return i;
    }
    return 255;
}

/* 扇区 from index 函数已移除（未使用） */

/* ========== 函数实现 ========== */

/**
  * @brief  霍尔传感器初始化
  */
void BSP_Hall_Init(void)
{
    /* 1. 初始化 GPIO（重要！）*/
    BSP_Hall_GPIO_Init();

    /* 2. 清空数据结构体 */
    memset(&hall_sensor, 0, sizeof(hall_sensor));

    /* 3. 初始化滤波系数（无滤波）*/
    hall_sensor.filter_alpha = 1.0f;

    /* 4. 读取初始状态 */
    BSP_Hall_Read();

    /* 5. 计算初始电角度 */
    BSP_Hall_CalculateAngle();

    /* 6. 初始化跟踪状态 */
    hall_sensor.last_change_tick = HAL_GetTick();
    hall_sensor.last_sector = hall_sensor.sector;
    hall_sensor.last_track_angle = hall_sensor.elec_angle;
    hall_sensor.rotor_speed = 0.0f;
}

/**
  * @brief  读取霍尔传感器状态 + 更新跟踪角度
  * @note   读取 GPIO → 极性处理 → 扇区判断 → 角度线性插值 → 速度估算
  */
void BSP_Hall_Read(void)
{
    /* 1. 读取原始 GPIO 状态 */
    uint8_t ha = (HAL_GPIO_ReadPin(HALL_HA_GPIO_Port, HALL_HA_Pin) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t hb = (HAL_GPIO_ReadPin(HALL_HB_GPIO_Port, HALL_HB_Pin) == GPIO_PIN_SET) ? 1 : 0;
    uint8_t hc = (HAL_GPIO_ReadPin(HALL_HC_GPIO_Port, HALL_HC_Pin) == GPIO_PIN_SET) ? 1 : 0;

    /* 2. 极性处理 */
    hall_sensor.ha_raw = (HALL_POLARITY_A) ? ha : (1 - ha);
    hall_sensor.hb_raw = (HALL_POLARITY_B) ? hb : (1 - hb);
    hall_sensor.hc_raw = (HALL_POLARITY_C) ? hc : (1 - hc);

    /* 3. 软件低通滤波（一阶 IIR 滤波）*/
    /* y[k] = α·x[k] + (1-α)·y[k-1] */
    if (hall_sensor.filter_alpha < 1.0f) {
        hall_sensor.ha_filtered = (uint8_t)(hall_sensor.filter_alpha * hall_sensor.ha_raw +
                                             (1.0f - hall_sensor.filter_alpha) * hall_sensor.ha_filtered);
        hall_sensor.hb_filtered = (uint8_t)(hall_sensor.filter_alpha * hall_sensor.hb_raw +
                                             (1.0f - hall_sensor.filter_alpha) * hall_sensor.hb_filtered);
        hall_sensor.hc_filtered = (uint8_t)(hall_sensor.filter_alpha * hall_sensor.hc_raw +
                                             (1.0f - hall_sensor.filter_alpha) * hall_sensor.hc_filtered);
    } else {
        /* 无滤波（α=1.0）*/
        hall_sensor.ha_filtered = hall_sensor.ha_raw;
        hall_sensor.hb_filtered = hall_sensor.hb_raw;
        hall_sensor.hc_filtered = hall_sensor.hc_raw;
    }

    /* 4. 扇区判断：HA=bit2, HB=bit1, HC=bit0 */
    uint8_t sector_raw = (hall_sensor.ha_filtered << 2) |
                         (hall_sensor.hb_filtered << 1) |
                         (hall_sensor.hc_filtered);

    uint8_t new_sector = hall_sensor.sector;
    if (sector_raw >= 1 && sector_raw <= 6) {
        new_sector = sector_raw;
    }

    /* 5. 扇区切换检测 → 更新插值锚点 + 速度估算 */
    uint32_t now = HAL_GetTick();
    uint8_t last_idx = hall_fwd_index(hall_sensor.last_sector);
    uint8_t new_idx  = hall_fwd_index(new_sector);

    if (new_sector != hall_sensor.last_sector && last_idx != 255 && new_idx != 255) {
        /* 计算扇区间跳跃方向（取最短路径，+1=正转，-1=反转） */
        int8_t delta = (int8_t)new_idx - (int8_t)last_idx;
        if (delta > 3)  delta -= 6;
        if (delta < -3) delta += 6;

        /* 经过的时间（ms） */
        uint32_t dt = now - hall_sensor.last_change_tick;

        if (dt > 0 && dt < 500) {
            /* 计算转速（rpm）= 每步 60°电角度 / dt(ms) × 极对数 */
            hall_sensor.rotor_speed = (60000.0f / (float)dt) / (float)MOTOR_POLE_PAIRS;
            hall_sensor.rotor_speed *= delta;  /* 保留方向 */
        } else {
            hall_sensor.rotor_speed = 0.0f;
        }

        /* 更新插值锚点：累计角度（连续不跳变），而不是跳转到扇区中心角 */
        hall_sensor.last_track_angle += delta * (TWO_PI_F / 6.0f);
        /* 归一化到 [0, 2π) */
        while (hall_sensor.last_track_angle >= TWO_PI_F) hall_sensor.last_track_angle -= TWO_PI_F;
        while (hall_sensor.last_track_angle < 0.0f)  hall_sensor.last_track_angle += TWO_PI_F;

        hall_sensor.last_change_tick = now;
        hall_sensor.last_sector = new_sector;
    }

    /* 6. 线性插值（使用连续累计角度，不跳变）*/
    {
        float dt = (float)(now - hall_sensor.last_change_tick);
        float mech_period;
        if (hall_sensor.rotor_speed != 0.0f) {
            mech_period = 60000.0f / fabsf(hall_sensor.rotor_speed);
        } else {
            mech_period = 1000.0f;
        }
        float sector_period = mech_period / 6.0f;
        float ratio = dt / sector_period;
        if (ratio > 1.0f) ratio = 1.0f;

        float sign = (hall_sensor.rotor_speed > 0.0f) ? 1.0f : -1.0f;
        float interpolated = hall_sensor.last_track_angle + sign * ratio * (TWO_PI_F / 6.0f);

        while (interpolated >= TWO_PI_F) interpolated -= TWO_PI_F;
        while (interpolated < 0.0f)  interpolated += TWO_PI_F;

        hall_sensor.elec_angle = interpolated;
    }

    /* 7. 更新扇区 */
    hall_sensor.sector = new_sector;
}

/**
  * @brief  计算电角度（保留原版，用于初始化和 BSP_Hall_Read 中的基础值）
  */
void BSP_Hall_CalculateAngle(void)
{
    /* 扇区判断：HA=bit2, HB=bit1, HC=bit0 */
    uint8_t sector_raw = (hall_sensor.ha_filtered << 2) |
                         (hall_sensor.hb_filtered << 1) |
                         (hall_sensor.hc_filtered);

    /* 映射到 1~6 扇区 */
    if (sector_raw >= 1 && sector_raw <= 6) {
        hall_sensor.sector = sector_raw;
    } else {
        /* 无效状态（全 0 或全 1），保持上次扇区 */
        hall_sensor.sector = hall_sensor.sector;
    }

    /* 查表映射到扇区值（0~5）*/
    uint8_t sector_value = hall_sector_map[hall_sensor.sector];

    /* 计算电角度（对齐 SguanFOC）*/
    hall_sensor.elec_angle = (float)sector_value * TWO_PI_F / 6.0f;
}

/**
  * @brief  获取电角度
  * @retval 电角度（rad, 0~2π）
  */
float BSP_Hall_GetAngle(void)
{
    return hall_sensor.elec_angle;
}

/**
  * @brief  获取扇区编号
  * @retval 扇区（1~6）
  */
uint8_t BSP_Hall_GetSector(void)
{
    return hall_sensor.sector;
}

/**
  * @brief  设置滤波系数
  * @param  alpha: 滤波系数（0~1）
  *         - 1.0 = 无滤波（实时响应）
  *         - 0.5 = 中等滤波
  *         - 0.1 = 强滤波（抗干扰）
  */
void BSP_Hall_SetFilter(float alpha)
{
    if (alpha > 1.0f) alpha = 1.0f;
    if (alpha < 0.0f) alpha = 0.0f;
    hall_sensor.filter_alpha = alpha;
}

/**
  * @brief  获取估算转速（rpm）
  * @retval 转速（rpm，正=正转方向）
  */
float BSP_Hall_GetSpeed(void)
{
    return hall_sensor.rotor_speed;
}

/**
  * @brief  重置 Hall 跟踪状态（启动电机前调用）
  */
void BSP_Hall_ResetTracking(void)
{
    BSP_Hall_Read();
    hall_sensor.last_sector = hall_sensor.sector;
    hall_sensor.last_track_angle = hall_sensor.elec_angle;
    hall_sensor.last_change_tick = HAL_GetTick();
    hall_sensor.rotor_speed = 0.0f;
}

/**
  * @brief  获取原始信号（调试用）
  * @param  ha: HA 原始信号指针
  * @param  hb: HB 原始信号指针
  * @param  hc: HC 原始信号指针
  */
void BSP_Hall_GetRawSignal(uint8_t *ha, uint8_t *hb, uint8_t *hc)
{
    if (ha) *ha = hall_sensor.ha_raw;
    if (hb) *hb = hall_sensor.hb_raw;
    if (hc) *hc = hall_sensor.hc_raw;
}

/**
  * @brief  获取滤波后信号（调试用）
  * @param  ha: HA 滤波后信号指针
  * @param  hb: HB 滤波后信号指针
  * @param  hc: HC 滤波后信号指针
  */
void BSP_Hall_GetFilteredSignal(uint8_t *ha, uint8_t *hb, uint8_t *hc)
{
    if (ha) *ha = hall_sensor.ha_filtered;
    if (hb) *hb = hall_sensor.hb_filtered;
    if (hc) *hc = hall_sensor.hc_filtered;
}
