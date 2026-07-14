#include "button_app.h"
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "FOC_Model.h"

static Button btn1;
static uint8_t motor_enable = 1;

static uint8_t button_read_GPIO(uint8_t button_id)
{
    switch (button_id) {
    case SW_S1:
        return HAL_GPIO_ReadPin(GPIOC, KEY_Pin);
    default:
        return 0;
    }
}

static void button_callback(Button *handle, void *user_data)
{
    (void)user_data;

    if (motor_enable)
    {
        HAL_GPIO_WritePin(GPIOC, LED2_Pin|LED3_Pin, GPIO_PIN_SET);
        TIM1->CCR1=4000;
        TIM1->CCR2=4000;
        TIM1->CCR3=4000;
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
        HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
        HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
        HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);
        BSP_ADC_Recalibrate();
        rtU.Motor_OnOff = 1.0F;
        HAL_TIMEx_HallSensor_Start_IT(&htim4);
        motor_enable = 0;
    }
    else
    {
        HAL_GPIO_WritePin(GPIOC, LED2_Pin|LED3_Pin, GPIO_PIN_RESET);
        rtU.Motor_OnOff = 0.0F;
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
        HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
        HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
        HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
        HAL_TIMEx_HallSensor_Stop_IT(&htim4);
        TIM1->CCR1=4000;
        TIM1->CCR2=4000;
        TIM1->CCR3=4000;
        motor_enable = 1;
    }

}

void BSP_Button_Init(void)
{
    button_init(&btn1, button_read_GPIO, 1, SW_S1);
    button_attach(&btn1, BTN_PRESS_DOWN, button_callback, NULL);
    button_start(&btn1);
}
