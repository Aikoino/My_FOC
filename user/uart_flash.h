#ifndef __UART_FLASH_H_
#define __UART_FLASH_H_
#include "main.h"
#include "opamp.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"
#include "button_app.h"
#include "FOC_Model.h"
#include "adc.h"
#include "fdcan.h"
#include "usart.h"

/* DMA接收缓冲区长度 */
#define RXLEN  128

/* 接收数据结构体 */
typedef struct {
    uint8_t buff[RXLEN];
    uint16_t cnt;
    uint8_t flag;
} RxData_t;

/* Flash参数存储地址 (使用最后一个Flash页) */
#define FLASH_PARAM_ADDR    0x0801F800  /* STM32G431CB: 128KB Flash, Page 63 @ 0x0801F800 */

/* Flash参数存储结构体 */
typedef struct {
    float speed_kp;       /* 速度环Kp */
    float speed_ki;       /* 速度环Ki */
    float curr_kp;        /* 电流环Kp */
    float curr_ki;        /* 电流环Ki */
    float flux;           /* 磁链常数 */
    float rs;             /* 相电阻 */
    float ls;             /* 相电感 */
    float pn;             /* 极对数 */
    float open_speed;     /* 开环强拖速度 */
    uint8_t hall_state;   /* 有感/无感选择: 0=有感, 1=无感 */
    uint16_t volt_up;     /* 电压上限 */
    uint16_t volt_low;    /* 电压下限 */
    uint32_t crc;         /* CRC校验 */
} FlashParam_t;

extern RxData_t rxdata;
extern RxData_t recv;

void BSP_Vofa_Init(void);
void UART3_RxHandler(void);
void USART3_Anylze(void);
void Motor_Start(void);
void Motor_Stop(void);
void Flash_SaveParams(void);
void Flash_LoadParams(void);
#endif
