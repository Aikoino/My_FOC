/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_adc.c
  * @brief          : ADC sampling driver using OPAMP + injected channels for 3-phase current
  ******************************************************************************
  * @attention
  *
  * Description:
  * - Uses OPAMP with ADC injected channels to sample U/V/W 3-phase currents
  * - Performs offset calibration on power-up
  * - Uses software trigger instead of hardware trigger (TIM1 CC4)
  * - NO DEBUG OUTPUT (to avoid interfering with VOFA+ JustFloat protocol)
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "bsp_adc.h"
#include "main.h"
#include "MiniFOC/MiniFOC.h"   /* 添加：访问 foc 结构体 */

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

extern OPAMP_HandleTypeDef hopamp1;
extern OPAMP_HandleTypeDef hopamp2;
extern OPAMP_HandleTypeDef hopamp3;

/* 私有变量：电流采样相关 */
static int32_t ADC_OffSet = 0;
static uint32_t cnt = 0;
static int32_t offset[3] = {0};
static int16_t ADC_Data[3] = {0};
static float curr_value[3] = {0.0f};  /* 重命名为 curr_value 避免与 adc.c 的 CurrlValue 冲突 */

/**
  * @brief  ADC initialization
  * @retval None
  */
void BSP_ADC_Init(void)
{
    /* NO DEBUG OUTPUT - keep USART3 clean for VOFA+ */

    HAL_OPAMP_Start(&hopamp1);
    HAL_OPAMP_Start(&hopamp2);
    HAL_OPAMP_Start(&hopamp3);

    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);

    __HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_EOC);
    __HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_JEOC);
    __HAL_ADC_CLEAR_FLAG(&hadc2, ADC_FLAG_EOC);
    __HAL_ADC_CLEAR_FLAG(&hadc2, ADC_FLAG_JEOC);

    /* 启动注入通道（电流采样） */
    HAL_ADCEx_InjectedStart_IT(&hadc1);
    HAL_ADCEx_InjectedStart_IT(&hadc2);

    /* 启动规则通道（母线电压采样） */
    HAL_ADC_Start(&hadc1);
}

/**
  * @brief  Start ADC (reserved)
  * @retval None
  */
void BSP_ADC_Start(void)
{
}

/**
  * @brief  Injected conversion complete callback
  * @param  hadc: ADC handle
  * @retval None
  */
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (ADC_OffSet == 0)
    {
        cnt++;
        offset[0] += ADC1->JDR1;
        offset[1] += ADC2->JDR1;
        offset[2] += ADC1->JDR2;

        if (cnt >= 10)
        {
            offset[0] /= 10;
            offset[1] /= 10;
            offset[2] /= 10;
            ADC_OffSet = 1;
        }
    }
    else
    {
        ADC_Data[0] = (int16_t)ADC1->JDR1;
        ADC_Data[1] = (int16_t)ADC2->JDR1;
        ADC_Data[2] = (int16_t)ADC1->JDR2;

        curr_value[0] = (float)(ADC_Data[0] - offset[0]) * 0.021972f;
        curr_value[1] = (float)(ADC_Data[1] - offset[1]) * 0.021972f;
        curr_value[2] = (float)(ADC_Data[2] - offset[2]) * 0.021972f;

        /* VF 开环控制：在 ADC 中断中执行（40kHz 硬件同步）*/
        if (foc.motor_running && foc.mode == MODE_VF_OPENLOOP) {
            MiniFOC_HighFreqLoop();
        }
    }
}

/**
  * @brief  Software trigger for ADC injected conversion
  * @retval None
  */
void BSP_ADC_SoftwareTrigger(void)
{
    if (ADC_OffSet == 0) return;

    HAL_ADCEx_InjectedStart_IT(&hadc1);
    HAL_ADCEx_InjectedStart_IT(&hadc2);
}

/**
  * @brief  Read U phase current
  * @retval U phase current value
  */
float BSP_ADC_GetCurrentU(void)
{
    return curr_value[0];
}

/**
  * @brief  Read V phase current
  * @retval V phase current value
  */
float BSP_ADC_GetCurrentV(void)
{
    return curr_value[1];
}

/**
  * @brief  Read W phase current
  * @retval W phase current value
  */
float BSP_ADC_GetCurrentW(void)
{
    return curr_value[2];
}

/**
  * @brief  Get current sampling offset (for MiniFOC)
  * @param  ch: channel (0=U, 1=V, 2=W)
  * @retval Offset value in Amperes
  */
float BSP_ADC_GetCurrentOffset(uint8_t ch)
{
    if (ch > 2) return 0.0f;
    return (float)offset[ch] * 0.021972f;
}

/**
  * @brief  Read bus voltage raw value
  * @retval Bus voltage ADC sample
  */
uint16_t BSP_ADC_GetVbus(void)
{
    /* PA0 -> ADC1_IN1 -> ADC_CHANNEL_1
     * 分压系数：26:1（26k 上拉，1k 下拉）
     */
    /* 等待转换完成 */
    if (__HAL_ADC_GET_FLAG(&hadc1, ADC_FLAG_EOC)) {
        uint16_t adc_val = HAL_ADC_GetValue(&hadc1);
        /* 重新启动下一次转换 */
        HAL_ADC_Start(&hadc1);
        return adc_val;
    }
    return 0;
}
