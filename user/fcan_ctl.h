#ifndef __FCAN_CTL_H_
#define __FCAN_CTL_H_
#include "main.h"
#include "adc.h"
#include "fdcan.h"
extern FDCAN_TxHeaderTypeDef TxHeader;
extern FDCAN_RxHeaderTypeDef RxHeader;
extern	float target_speed_ref;


void Motor_Start_Stop(uint8_t on);
void FDCAN_Control(void);
void SpeedRamp_Update(void);

#endif
