#include "fcan_ctl.h"

FDCAN_TxHeaderTypeDef TxHeader= {0};
FDCAN_RxHeaderTypeDef RxHeader= {0};
float target_speed_ref = 0.0f;

uint8_t FDCan_RxData[8]= {0};
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hfdcan);
    UNUSED(RxFifo0ITs);
    if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
    {
        if(hfdcan->Instance == FDCAN1)
        {
            /* Retrieve Rx messages from RX FIFO0 */
            if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, FDCan_RxData) != HAL_OK)
            {
                Error_Handler();
            }
            FDCAN_Control();
        }
    }
}

void Motor_Start_Stop(uint8_t on)
{
    if (on) {
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
    } else {
        rtU.Motor_OnOff = 0.0F;
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
        HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
        HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
        HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);
        HAL_TIMEx_HallSensor_Stop_IT(&htim4);
        HAL_GPIO_WritePin(GPIOC, LED2_Pin|LED3_Pin, GPIO_PIN_RESET);
    }
}
void SpeedRamp_Update(void)
{
    static uint32_t last_tick = 0;
    if (HAL_GetTick() - last_tick < 10) return;   // 100Hz
    last_tick = HAL_GetTick();

    float target = target_speed_ref;
    float current = rtU.SpeedRef;
    const float step = 30.0f;                     // 每 10ms 改 30 rpm = 3000 rpm/s

    if (fabsf(target - current) <= step) {
        rtU.SpeedRef = target;
    } else if (target > current) {
        rtU.SpeedRef = current + step;
    } else {
        rtU.SpeedRef = current - step;
    }
}

void FDCAN_Control(void)
{
    uint16_t speed_temp = 0;
    if (FDCan_RxData[0] == 0x01) {                       // 启停信号
        speed_temp = (FDCan_RxData[2] << 8) | FDCan_RxData[3];   // 拼速度
        if (FDCan_RxData[1] == 0x01) {                   // 正反转
            target_speed_ref = (float)speed_temp;            // ← 大写 R
        } else {
            target_speed_ref = -(float)speed_temp;
        }
        Motor_Start_Stop(1);                             // ← 用 button 里的启停逻辑
    } else {
        Motor_Start_Stop(0);
    }
}
