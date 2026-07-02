/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : test_gpio.c
  * @brief          : 最小 GPIO 测试 - 只测试 PC13 按键输入
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "test_gpio.h"

extern UART_HandleTypeDef huart3;

/**
  * @brief  测试 PC13 输入
  * @retval None
  */
void Test_GPIO_Input(void)
{
    // 打印测试开始
    char msg[50];
    sprintf(msg, "\r\n=== GPIO Test Start ===\r\n");
    HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), 1000);

    // 测试 1: 读取 PC13
    for (int i = 0; i < 10; i++)
    {
        uint8_t level = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
        sprintf(msg, "Test %d: PC13=%d\r\n", i, level);
        HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), 1000);
        HAL_Delay(500);
    }

    // 提示用户按下按键
    sprintf(msg, "\r\nPress KEY1 now...\r\n");
    HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), 1000);

    // 测试 2: 持续读取，等待按键按下
    for (int i = 0; i < 20; i++)
    {
        uint8_t level = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
        uint8_t pc14 = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_14);
        uint8_t pc15 = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15);

        sprintf(msg, "PC13=%d PC14=%d PC15=%d\r\n", level, pc14, pc15);
        HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), 1000);

        HAL_Delay(200);
    }

    sprintf(msg, "\r\n=== GPIO Test End ===\r\n\r\n");
    HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), 1000);
}
