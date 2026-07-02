/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_adc.c
  * @brief          : ADC 采样驱动，使用 OPAMP + 注入通道采集三相电流
  ******************************************************************************
  * @attention
  *
  * 说明：
  * - 使用 OPAMP 配合 ADC 注入通道采集 U/V/W 三相电流
  * - 上电后先做偏置采样，再换算为实际电流值
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "bsp_adc.h"
#include "main.h"

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

extern OPAMP_HandleTypeDef hopamp1;
extern OPAMP_HandleTypeDef hopamp2;
extern OPAMP_HandleTypeDef hopamp3;

static int32_t ADC_OffSet = 0;
static uint32_t cnt = 0;
static int32_t offset[3] = {0};
static int16_t ADC_Data[3] = {0};
static float CurrlValue[3] = {0.0f};

/**
  * @brief  ADC 初始化
  * @retval None
  */
void BSP_ADC_Init(void)
{
    HAL_OPAMP_Start(&hopamp1);
    HAL_OPAMP_Start(&hopamp2);
    HAL_OPAMP_Start(&hopamp3);

    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);

    __HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_EOC);
    __HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_JEOC);
    __HAL_ADC_CLEAR_FLAG(&hadc2, ADC_FLAG_EOC);
    __HAL_ADC_CLEAR_FLAG(&hadc2, ADC_FLAG_JEOC);

    HAL_ADCEx_InjectedStart_IT(&hadc1);
    HAL_ADCEx_InjectedStart_IT(&hadc2);
}

/**
  * @brief  启动 ADC（预留）
  * @retval None
  */
void BSP_ADC_Start(void)
{
}

/**
  * @brief  注入转换完成回调
  * @param  hadc: ADC 句柄
  * @retval None
  */
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (ADC_OffSet == 0)
    {
        cnt++;
        /* 直接读取ADC数据寄存器 */
        offset[0] += ADC1->JDR1;        /* U相偏置 (ADC1注入通道1) */
        offset[1] += ADC2->JDR1;        /* V相偏置 (ADC2注入通道1) */
        offset[2] += ADC1->JDR2;        /* W相偏置 (ADC1注入通道2) */

        if (cnt >= 10)
        {
            offset[0] /= 10.0f;
            offset[1] /= 10.0f;
            offset[2] /= 10.0f;
            ADC_OffSet = 1;
        }
    }
    else
    {
        /* 直接读取ADC数据寄存器 */
        ADC_Data[0] = (int16_t)ADC1->JDR1;   /* U相电流 */
        ADC_Data[1] = (int16_t)ADC2->JDR1;   /* V相电流 */
        ADC_Data[2] = (int16_t)ADC1->JDR2;   /* W相电流 */

        /* 实际电流 = (ADC值 - 偏置) * 增益系数 */
        CurrlValue[0] = (float)(ADC_Data[0] - offset[0]) * 0.021972f;
        CurrlValue[1] = (float)(ADC_Data[1] - offset[1]) * 0.021972f;
        CurrlValue[2] = (float)(ADC_Data[2] - offset[2]) * 0.021972f;
    }
}

/**
  * @brief  读取 U 相电流
  * @retval U 相电流值（单位根据实际采样换算）
  */
float BSP_ADC_GetCurrentU(void)
{
    return CurrlValue[0];
}

/**
  * @brief  读取 V 相电流
  * @retval V 相电流值
  */
float BSP_ADC_GetCurrentV(void)
{
    return CurrlValue[1];
}

/**
  * @brief  读取 W 相电流
  * @retval W 相电流值
  */
float BSP_ADC_GetCurrentW(void)
{
    return CurrlValue[2];
}

/**
  * @brief  获取电流采样零点偏移 (用于 MiniFOC)
  * @param  ch: 通道 (0=U, 1=V, 2=W)
  * @retval 零点偏移值 (A)
  */
float BSP_ADC_GetCurrentOffset(uint8_t ch)
{
    if (ch > 2) return 0.0f;
    return (float)offset[ch] * 0.021972f;  /* 转换为安培 */
}

/**
  * @brief  读取母线电压原始值
  * @retval 母线电压 ADC 采样值
  */
uint16_t BSP_ADC_GetVbus(void)
{
    return (uint16_t)ADC1->JDR3;  /* 读取ADC1注入通道3 (母线电压 PA0) */
}
