/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : bsp_uart_vofta.c
  * @brief          : VOFA+ 串口数据发送驱动
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "bsp_uart_vofta.h"
#include "main.h"

extern UART_HandleTypeDef huart3;

/**
  * @brief  VOFA+ 串口初始化
  * @retval None
  */
void BSP_UART_VOFA_Init(void)
{
}

/**
  * @brief  通过串口发送四个浮点数到 VOFA+ (JustFloat协议)
  * @param  a: 参数 A (U相电流)
  * @param  b: 参数 B (V相电流)
  * @param  c: 参数 C (W相电流)
  * @param  d: 参数 D (母线电压)
  * @retval None
  *
  * JustFloat协议格式：
  * - 小端浮点数组
  * - 帧尾固定: 0x00, 0x00, 0x80, 0x7F (float 1.0)
  * - 示例: 4个float = 4*4 + 4 = 20字节
  */
void BSP_UART_VOFA_SendFloats(float a, float b, float c, float d)
{
    /* 数据数组: 4个float(16字节) + 帧尾(4字节) = 20字节 */
    static uint8_t buf[20] = {0};

    /* 清零数据区域（保留帧尾） */
    memset(buf, 0, 16);

    /* 拷贝4个float到缓冲区（小端格式） */
    memcpy(buf, &a, sizeof(float));
    memcpy(buf + 4, &b, sizeof(float));
    memcpy(buf + 8, &c, sizeof(float));
    memcpy(buf + 12, &d, sizeof(float));

    /* 设置帧尾: 0x00, 0x00, 0x80, 0x7F */
    buf[16] = 0x00;
    buf[17] = 0x00;
    buf[18] = 0x80;
    buf[19] = 0x7F;

    /* 发送20字节 */
    HAL_UART_Transmit(&huart3, buf, 20, 1);
}

