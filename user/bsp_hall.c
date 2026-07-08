/**
  ******************************************************************************
  * @file           : bsp_hall.c
  * @brief          : 霍尔传感器驱动（已迁移到 tim.c，此文件仅保留兼容接口）
  * @note           : 霍尔传感器的核心实现在 Core/Src/tim.c 中
  *                   使用 TIM4 硬件捕获 + ADC 中断累加角度
  ******************************************************************************
  */
#include "bsp_hall.h"
#include "../Core/Inc/tim.h"  /* 引用 HALL_Handle_t 和 HALL_Handle */
#include <string.h>

/* ========== 全局变量（兼容接口）========== */

BSP_Hall_t hall_sensor;

/**
  * @brief  霍尔传感器初始化（兼容接口）
  */
void BSP_Hall_Init(void)
{
    memset(&hall_sensor, 0, sizeof(hall_sensor));
    hall_sensor.filter_alpha = 1.0f;
    HALL_Init_Electrical_Angle();
}

/**
  * @brief  读取霍尔传感器状态（从 HALL_Handle 同步）
  */
void BSP_Hall_Read(void)
{
    hall_sensor.ha_raw = (HALL_Handle.HallState >> 2) & 0x01;
    hall_sensor.hb_raw = (HALL_Handle.HallState >> 1) & 0x01;
    hall_sensor.hc_raw = HALL_Handle.HallState & 0x01;
    hall_sensor.sector = HALL_Handle.HallState;
    hall_sensor.elec_angle = HALL_Handle.HallElAngle;
    hall_sensor.rotor_speed = HALL_Handle.HallSpeed;
    hall_sensor.ha_filtered = hall_sensor.ha_raw;
    hall_sensor.hb_filtered = hall_sensor.hb_raw;
    hall_sensor.hc_filtered = hall_sensor.hc_raw;
}

/**
  * @brief  计算电角度（兼容接口，实际在 tim.c 中计算）
  */
void BSP_Hall_CalculateAngle(void)
{
    /* 空函数：角度计算已在 HALL_Init_Electrical_Angle() 中完成 */
}

/**
  * @brief  获取电角度
  */
float BSP_Hall_GetAngle(void)
{
    return hall_sensor.elec_angle;
}

/**
  * @brief  获取扇区编号
  */
uint8_t BSP_Hall_GetSector(void)
{
    return hall_sensor.sector;
}

/**
  * @brief  设置滤波系数
  */
void BSP_Hall_SetFilter(float alpha)
{
    if (alpha > 1.0f) alpha = 1.0f;
    if (alpha < 0.0f) alpha = 0.0f;
    hall_sensor.filter_alpha = alpha;
}

/**
  * @brief  获取估算转速
  */
float BSP_Hall_GetSpeed(void)
{
    return hall_sensor.rotor_speed;
}

/**
  * @brief  重置 Hall 跟踪状态
  */
void BSP_Hall_ResetTracking(void)
{
    memset(&hall_sensor, 0, sizeof(hall_sensor));
    hall_sensor.filter_alpha = 1.0f;
    HALL_Init_Electrical_Angle();
    hall_sensor.last_change_tick = HAL_GetTick();
    hall_sensor.last_sector = HALL_Handle.HallState;
    hall_sensor.last_track_angle = HALL_Handle.HallElAngle;
    hall_sensor.rotor_speed = 0.0f;
}

/**
  * @brief  获取原始信号（调试用）
  */
void BSP_Hall_GetRawSignal(uint8_t *ha, uint8_t *hb, uint8_t *hc)
{
    if (ha) *ha = hall_sensor.ha_raw;
    if (hb) *hb = hall_sensor.hb_raw;
    if (hc) *hc = hall_sensor.hc_raw;
}

/**
  * @brief  获取滤波后信号（调试用）
  */
void BSP_Hall_GetFilteredSignal(uint8_t *ha, uint8_t *hb, uint8_t *hc)
{
    if (ha) *ha = hall_sensor.ha_filtered;
    if (hb) *hb = hall_sensor.hb_filtered;
    if (hc) *hc = hall_sensor.hc_filtered;
}

/**
  * @brief  TIM4捕获回调（HAL库标准回调，在HAL_TIM_IRQHandler中自动调用）
  * @param  htim: TIM handle
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    /* 判断是否是TIM4 */
    if (htim == &htim4) {
        /* 调用霍尔处理函数 */
        HALL_Get_Electrical_Angle(&HALL_Handle);
    }
}
