#include "button_app.h"
#include "main.h"

/* 按键GPIO读取函数 - 供外部调用获取按键状态
 * 硬件: SW2 接 PC13 (KEY_Pin)
 * 逻辑: 未按下 = 低电平(0), 按下 = 高电平(1) --> 高电平有效 */
uint8_t BSP_Button_Read(void)
{
    return (uint8_t)HAL_GPIO_ReadPin(GPIOC, KEY_Pin);
}

/* 按键模块初始化 - 仅配置GPIO，不启用multi_button回调
 * 原因：main.c使用直接GPIO读取控制LED，避免multi_button回调冲突
 * 如果未来需要使用multi_button高级功能（长按、双击等），可使用 BSP_Button_Init_Advanced() */
void BSP_Button_Init(void)
{
    /* GPIO已在MX_GPIO_Init()中配置为输入模式，无需额外操作
     * 此函数保留用于兼容性和未来扩展 */
}

/* 高级按键初始化（可选）- 启用multi_button回调功能
 * 按 BTN_PRESS_DOWN 事件翻转 LED2 和 LED3 */
void BSP_Button_Init_Advanced(void)
{
#if 0  /* 调试时可取消注释测试multi_button回调功能 */
    static Button btn1;
    static uint8_t button_read_GPIO(uint8_t button_id) {
        if (button_id == SW_S1) {
            return (uint8_t)HAL_GPIO_ReadPin(GPIOC, KEY_Pin);
        }
        return 0;
    }

    static void button_callback(Button *handle, void *user_data) {
        (void)user_data;
        if (button_get_event(handle) == BTN_PRESS_DOWN) {
            HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
            HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
        }
    }

    button_init(&btn1, button_read_GPIO, 1, SW_S1);
    button_attach(&btn1, BTN_PRESS_DOWN, button_callback, NULL);
    button_start(&btn1);
#endif
}
