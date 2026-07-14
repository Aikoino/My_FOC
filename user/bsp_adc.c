#include "BSP_ADC.h"


uint16_t cnt = 0;
float offset[3] = {0};
uint8_t ADC_OffSet = 0;
float ADC_Data[4];
float CurrlValue[3];


void BSP_ADC_Init(void)
{
    HAL_OPAMP_Start(&hopamp1);
    HAL_OPAMP_Start(&hopamp2);
    HAL_OPAMP_Start(&hopamp3);
    HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2,ADC_SINGLE_ENDED);
    __HAL_ADC_CLEAR_FLAG( &hadc1, ADC_FLAG_EOC);
    __HAL_ADC_CLEAR_FLAG( &hadc1, ADC_FLAG_JEOC);
    __HAL_ADC_CLEAR_FLAG( &hadc2, ADC_FLAG_JEOC);
    HAL_ADCEx_InjectedStart_IT(&hadc1);
    HAL_ADCEx_InjectedStart(&hadc2);
}



void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(hadc);
    if(hadc->Instance == ADC1)
    {
        if(ADC_OffSet == 0)
        {
            cnt++;
            offset[0] += ADC1->JDR1;
            offset[1] += ADC2->JDR1;
            offset[2] += ADC1->JDR2;
            if(cnt >= 10)
            {
                offset[0] /= 10.0f;
                offset[1] /= 10.0f;
                offset[2] /= 10.0f;
                ADC_OffSet = 1;
            }
        } else
        {
            extern uint8_t Hal_State;
            if (Hal_State == 1) {
                if (HALL_Handle.MeasuredElAngle == PHASE_SHIFT_ANGLE && HALL_Handle.Direction == 1)
                {
                    HALL_Handle.HallElAngle = HALL_Handle.MeasuredElAngle;
                    HALL_Handle.MeasuredElAngle = HALL_Handle.MeasuredElAngle + HALL_Handle.AvrElSpeedDpp;
                    HALL_Handle.HallElAngle = HALL_Handle.HallElAngle + HALL_Handle.AvrElSpeedDpp;
                }
                else if (HALL_Handle.MeasuredElAngle == PHASE_SHIFT_ANGLE + PI / 3 && HALL_Handle.Direction == -1)
                {
                    HALL_Handle.HallElAngle = HALL_Handle.MeasuredElAngle;
                    HALL_Handle.MeasuredElAngle = HALL_Handle.MeasuredElAngle + HALL_Handle.AvrElSpeedDpp;
                    HALL_Handle.HallElAngle = HALL_Handle.HallElAngle + HALL_Handle.AvrElSpeedDpp;
                }
                else
                {
                    HALL_Handle.MeasuredElAngle = HALL_Handle.MeasuredElAngle + HALL_Handle.AvrElSpeedDpp;
                    HALL_Handle.HallElAngle = HALL_Handle.HallElAngle + HALL_Handle.AvrElSpeedDpp + HALL_Handle.DeltaAngle;
                }
                if (HALL_Handle.HallElAngle < 0.0f)
                {
                    HALL_Handle.HallElAngle += 2.0f * PI;
                }
                else if (HALL_Handle.HallElAngle > (2.0f * PI))
                {
                    HALL_Handle.HallElAngle -= 2.0f * PI;
                }
            }
// ===== HALL end =====

            ADC_Data[0] = ADC1->JDR1;
            ADC_Data[1] = ADC2->JDR1;
            ADC_Data[2] = ADC1->JDR2;
            ADC_Data[3] = (ADC1->JDR3) *0.020947;
            CurrlValue[0] = (ADC_Data[0]-offset[0])*0.01933593f;
            CurrlValue[1] = (ADC_Data[1]-offset[1])*0.01933593f;
            CurrlValue[2] = (ADC_Data[2]-offset[2])*0.01933593f;
            FirstOrderRC_LPF(&rtU.ia, CurrlValue[0], 0.3f);
            FirstOrderRC_LPF(&rtU.ib, CurrlValue[1], 0.3f);
            FirstOrderRC_LPF(&rtU.ic, CurrlValue[2], 0.3f);
            rtU.v_bus = ADC_Data[3];
            FOC_Model_step();
            TIM1->CCR1 = rtY.Tcmp1;
            TIM1->CCR2 = rtY.Tcmp2;
            TIM1->CCR3 = rtY.Tcmp3;
        }
    }
}
void BSP_ADC_Recalibrate(void)
{
    cnt = 0;
    offset[0] = 0;
    offset[1] = 0;
    offset[2] = 0;
    ADC_OffSet = 0;
}
//电位器设置速度 speed 600~2300
void Potentiometer_SpeedSet(void)
{
    static uint16_t vres[10] = {0};
    uint16_t Vres = 0;
    static uint8_t pot_cnt = 0;
    if(pot_cnt<10) {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 10);
        vres[pot_cnt] = HAL_ADC_GetValue(&hadc1);
        pot_cnt++;
    } else
    {
        pot_cnt = 0;
        for(uint8_t i=0; i<10; i++)
        {
            Vres += vres[i];
        }
        Vres /= 10;
        if(Vres>2048)
        {
            rtU.SpeedRef = (int)((Vres-2048)*0.83)+600;
        } else
        {
            rtU.SpeedRef = -((int)((2048-Vres)*0.83)+600);
        }
    }
}
