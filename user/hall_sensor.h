#ifndef __HALL_SENSOR_H_
#define __HALL_SENSOR_H_
#include "main.h"
#include "tim.h"

#define STATE_1    0x01  // 001
#define STATE_2    0x02  // 010
#define STATE_3    0x03  // 011
#define STATE_4    0x04  // 100
#define STATE_5    0x05  // 101
#define STATE_6    0x06  // 110

#define POSITIVE   1  
#define NEGATIVE  -1   

#define PI         3.1415926535f
#define PHASE_SHIFT_ANGLE (float)(218.0f/360.0f*2.0f*PI)

typedef struct
{
 uint8_t HallState;        
 float AvrElSpeedDpp;      
 float MeasuredElAngle;    
 float HallElAngle;        
 int8_t Direction;         
 float HallSpeed;          
 float TempSpeed;         
 float DeltaAngle;         
 uint8_t bPrevHallState;   
 float hHighSpeedCapture;  
 float MeasureTest;        
}HALL_Handle_t;

extern HALL_Handle_t hHall;
extern HALL_Handle_t HALL_Handle;
void FirstOrderRC_LPF(float *out, float in, float alpha);
void HALL_Get_Electrical_Angle(void *pHandleVoid);
void HALL_Init_Electrical_Angle(void);
#endif
