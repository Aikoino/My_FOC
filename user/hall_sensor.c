#include "hall_sensor.h"

HALL_Handle_t HALL_Handle = {0};


//定时器 4 的输入捕获中断回调函数。160Mhz 50 分频 1/3200000s
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    /* Prevent unused argument(s) compilation warning */
    UNUSED(htim);
    if (htim == &htim4)
    {
        HALL_Get_Electrical_Angle((void *)&HALL_Handle);/*添加获取角度和速度的代码*/
    }
}



/*获取初始化角度*/
void HALL_Init_Electrical_Angle(void)
{
    HALL_Handle_t *phandle = &HALL_Handle;

    /* 读取三个霍尔传感器接口，确定当前转子所在扇区 */
    /* PB8=bit0, PB7=bit1, PB6=bit2 */
    phandle->HallState = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8);
    phandle->HallState |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) << 1;
    phandle->HallState |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) << 2;

    /*
     * 初始化：根据当前霍尔状态的绝对值计算扇区角度
     * 基础角度 PHASE_SHIFT_ANGLE + 扇区偏移 PI/6
     * 注意：初始化时电机静止，不需要判断方向！
     */
    switch (phandle->HallState)
    {
    case STATE_5:
    {
        /* 扇区 5: 基础角度 + PI/6 */
        phandle->HallElAngle = PHASE_SHIFT_ANGLE + PI / 6.0f;
        break;
    }
    case STATE_4:
    {
        /* 扇区 4: PI/3 + 基础角度 + PI/6 */
        phandle->HallElAngle = (PI / 3.0f) + PHASE_SHIFT_ANGLE + PI / 6.0f;
        break;
    }
    case STATE_6:
    {
        /* 扇区 6: 2*PI/3 + 基础角度 + PI/6 */
        phandle->HallElAngle = (PI * 2.0f / 3.0f) + PHASE_SHIFT_ANGLE + PI / 6.0f;
        break;
    }
    case STATE_2:
    {
        /* 扇区 2: PI + 基础角度 + PI/6 */
        phandle->HallElAngle = PI + PHASE_SHIFT_ANGLE + PI / 6.0f;
        break;
    }
    case STATE_3:
    {
        /* 扇区 3: 4*PI/3 + 基础角度 + PI/6 */
        phandle->HallElAngle = (PI * 4.0f / 3.0f) + PHASE_SHIFT_ANGLE + PI / 6.0f;
        break;
    }
    case STATE_1:
    {
        /* 扇区 1: 5*PI/3 + 基础角度 + PI/6 */
        phandle->HallElAngle = (PI * 5.0f / 3.0f) + PHASE_SHIFT_ANGLE + PI / 6.0f;
        break;
    }
    default:
    {
        break;
    }
    }

    /* 初始化 MeasuredElAngle 与 HallElAngle 一致 */
    phandle->MeasuredElAngle = phandle->HallElAngle;
}



void HALL_Get_Electrical_Angle(void *pHandleVoid)
{
    HALL_Handle_t *phandle = (HALL_Handle_t *)pHandleVoid;
    phandle->hHighSpeedCapture = HAL_TIM_ReadCapturedValue(&htim4, TIM_CHANNEL_1);
    phandle->bPrevHallState = phandle->HallState;
    phandle->HallState = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8);
    phandle->HallState |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) << 1;
    phandle->HallState |= HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) << 2;
    switch (phandle->HallState)
    {
    case STATE_5:
    {
        if (STATE_1 == phandle->bPrevHallState)
        {
            phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE;
            phandle->Direction = POSITIVE;
        }
        else if (STATE_4 == phandle->bPrevHallState)
        {
            phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + PI / 3.0f;
            phandle->Direction = NEGATIVE;
        }
        else
        {
        }; // nothing
        break;
    }
    case STATE_4:
    {
        if (STATE_5 == phandle->bPrevHallState)
        {
            phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + PI / 3.0f;
            phandle->Direction = POSITIVE;
        }
        else if (STATE_6 == phandle->bPrevHallState)
        {
            phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + 2 * PI / 3.0f;
            phandle->Direction = NEGATIVE;
        }
        else
        {
        }; // nothing
        break;
    }
    case STATE_6:
    {
        if (STATE_4 == phandle->bPrevHallState)
        {
            phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + 2 * PI / 3.0f;
            phandle->Direction = POSITIVE;
        }
        else if (STATE_2 == phandle->bPrevHallState)
        {
            phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + PI;
            phandle->Direction = NEGATIVE;
        }
        else
        {
        }; // nothing
        break;
    }
    case STATE_2:
    {
        if (STATE_6 == phandle->bPrevHallState)
        {
            phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + PI;
            phandle->Direction = POSITIVE;
        }
        else if (STATE_3 == phandle->bPrevHallState)
        {
            phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + 4 * PI / 3.0f;
            phandle->Direction = NEGATIVE;
        }
        else
        {
        }; // nothing
        break;
    }
    case STATE_3:
    {
        if (STATE_2 == phandle->bPrevHallState)
        {
            phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + 4 * PI / 3.0f;
            phandle->Direction = POSITIVE;
        }
        else if (STATE_1 == phandle->bPrevHallState)
        {
            phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + 5 * PI / 3.0f;
            phandle->Direction = NEGATIVE;
        }
        else
        {
        }; // nothing
        break;
    }
    case STATE_1:
    {
        if (STATE_3 == phandle->bPrevHallState)
        {
            phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + 5 * PI / 3.0f;
            phandle->Direction = POSITIVE;
        }
        else if (STATE_5 == phandle->bPrevHallState)
        {
            phandle->MeasuredElAngle = PHASE_SHIFT_ANGLE + 2.0f * PI;
            phandle->Direction = NEGATIVE;
        }
        else
        {
        }; // nothing
        break;
    }
    default:
    {
        break;
    }
    }
    if (phandle->MeasuredElAngle < 0.0f)
    {
        phandle->MeasuredElAngle += 2.0f * PI;
    }
    else if (phandle->MeasuredElAngle > (2.0f * PI))
    {
        phandle->MeasuredElAngle -= 2.0f * PI;
    }
//10KHz 下的平均电角速度 FOC 模型在 ADC 中断里执行，所以频率是 10000 一圈 2PI 分成 6 个扇区，每个扇占 PI/3
    phandle->AvrElSpeedDpp = (PI/3)/((phandle->hHighSpeedCapture/3200000.0f)*10000);
//将 rad/s 转换位 rpm/min
    phandle->TempSpeed = (PI/3)/(phandle->hHighSpeedCapture/3200000.0f)*30/(2*PI);
    phandle->TempSpeed = phandle->TempSpeed * phandle->Direction;
    FirstOrderRC_LPF(&phandle->HallSpeed,phandle->TempSpeed,0.2379f);  /* 一阶滤波 */
    phandle->AvrElSpeedDpp = phandle->AvrElSpeedDpp * phandle->Direction;
//当前方法是基于之前 60°的霍尔时间去处理的，并不是当前的值，我们默认每个扇区速度一样。但霍尔测量总归是有误差的，所以要进行补偿，补偿方法的话就是霍尔测量的角度减去电流环中累加的角度再除 10000；
    phandle->DeltaAngle = (phandle->MeasuredElAngle - phandle->HallElAngle) / 10000;
}
void FirstOrderRC_LPF(float *out, float in, float alpha)
{
    *out = (1-alpha)*(*out) + alpha*in;
}
