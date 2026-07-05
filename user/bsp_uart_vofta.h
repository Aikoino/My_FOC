/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_uart_vofta.h
  * @brief          : VOFA+ 串口数据发送头文件
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* 防止重复包含 */
#ifndef __BSP_UART_VOFA_H__
#define __BSP_UART_VOFA_H__

#include <stdint.h>
#include "stm32g4xx_hal.h"

/* VOFA+ 协议宏定义 */

/**
  * @brief  VOFA+ 串口初始化
  */
void BSP_UART_VOFA_Init(void);
/**
  * @brief  通过串口发送六个浮点数到 VOFA+
  * @param  a: 参数 A（U 相电流）
  * @param  b: 参数 B（V 相电流）
  * @param  c: 参数 C（W 相电流）
  * @param  d: 参数 D（CCR1 占空比）
  * @param  e: 参数 E（CCR2 占空比）
  * @param  f: 参数 F（CCR3 占空比）
  * @retval HAL_StatusTypeDef: HAL_OK=成功, 其他=失败
  */
HAL_StatusTypeDef BSP_UART_VOFA_SendFloats(float a, float b, float c, float d, float e, float f);

#endif
