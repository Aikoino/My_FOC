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
  * @brief  轮询方式发送单字节
  * @param  data: 要发送的字节
  * @retval None
  */
static void VOFA_UART_SendByte(uint8_t data)
{
    /* 等待发送缓冲区空 (TXE = Transmit Empty) */
    while (!(USART3->ISR & USART_ISR_TXE)) {
        /* wait */
    }

    USART3->TDR = data;
}

/**
  * @brief  VOFA+ 串口初始化
  * @retval None
  */
void BSP_UART_VOFA_Init(void)
{
    /* VOFA+ 初始化完成，USART3 已在 CubeMX 中配置 */
}

/**
  * @brief  通过串口发送四个浮点数到 VOFA+ (JustFloat协议)
  * @param  a: 参数 A (U相电流)
  * @param  b: 参数 B (V相电流)
  * @param  c: 参数 C (W相电流)
  * @param  d: 参数 D (母线电压)
  * @retval HAL_StatusTypeDef: HAL_OK=成功, HAL_ERROR=失败
  *
  * JustFloat协议格式：
  * - 4个 float，小端格式，每个4字节 = 16字节
  * - 帧尾固定: 0x00, 0x00, 0x80, 0x7F = 4字节
  * - 总长度: 20字节
  */
HAL_StatusTypeDef BSP_UART_VOFA_SendFloats(float a, float b, float c, float d)
{
    uint8_t i;

    /* 发送 4 个 float (16 字节) */
    uint8_t *ptr = (uint8_t *)&a;
    for (i = 0; i < 4; i++) {
        VOFA_UART_SendByte(ptr[i]);
    }

    ptr = (uint8_t *)&b;
    for (i = 0; i < 4; i++) {
        VOFA_UART_SendByte(ptr[i]);
    }

    ptr = (uint8_t *)&c;
    for (i = 0; i < 4; i++) {
        VOFA_UART_SendByte(ptr[i]);
    }

    ptr = (uint8_t *)&d;
    for (i = 0; i < 4; i++) {
        VOFA_UART_SendByte(ptr[i]);
    }

    /* 发送帧尾: 0x00, 0x00, 0x80, 0x7F */
    VOFA_UART_SendByte(0x00);
    VOFA_UART_SendByte(0x00);
    VOFA_UART_SendByte(0x80);
    VOFA_UART_SendByte(0x7F);

    return HAL_OK;
}
