#ifndef __BSP_CAN_H__
#define __BSP_CAN_H__

#include <stdint.h>
#include "stm32g4xx_hal.h"
#include "main.h"

/* CAN 报文 ID 定义 */
#define CAN_TX_ID   0x125    /* 发送 ID */
#define CAN_RX_ID   0x126    /* 接收 ID，根据你的需求改 */

/* CAN 初始化（完整版） */
void CAN_Init(void);

/* CAN 初始化（无中断版 - 测试用） */
void CAN_Init_NoIRQ(void);

/* CAN 发送 */
uint8_t CAN_Send(uint32_t id, uint8_t *data, uint8_t len);

/* CAN 接收（返回 1=收到新报文，0=无） */
uint8_t CAN_Receive(uint32_t *id, uint8_t *data, uint8_t *len);

#endif
