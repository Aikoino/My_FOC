#ifndef __BSP_ADC_H_
#define __BSP_ADC_H_
#include "main.h"
#include "opamp.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"
#include "button_app.h"
#include "FOC_Model.h"
#include "adc.h"
#include "hall_sensor.h"

void BSP_ADC_Init(void);
void BSP_ADC_Recalibrate(void);
extern float CurrlValue[3];
void Potentiometer_SpeedSet(void);

#endif
