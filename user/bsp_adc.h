/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_adc.h
  * @brief          : ADC 采样驱动头文件
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* 防止重复包含 */
#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__

#include <stdint.h>

/* 电流与电压读取接口 */

/**
  * @brief  ADC 初始化
  */
void BSP_ADC_Init(void);
/**
  * @brief  启动 ADC（预留）
  */
void BSP_ADC_Start(void);

/**
  * @brief  读取 U 相电流
  * @retval U 相电流值
  */
float BSP_ADC_GetCurrentU(void);
/**
  * @brief  读取 V 相电流
  * @retval V 相电流值
  */
float BSP_ADC_GetCurrentV(void);
/**
  * @brief  读取 W 相电流
  * @retval W 相电流值
  */
float BSP_ADC_GetCurrentW(void);
/**
  * @brief  读取母线电压原始值
  * @retval 母线电压 ADC 采样值
  */
uint16_t BSP_ADC_GetVbus(void);

#endif
