#include "bsp_can.h"

/* 接收缓冲区 */
static uint8_t s_rx_data[8];
static uint32_t s_rx_id = 0;
static uint8_t s_rx_len = 0;
static uint8_t s_rx_flag = 0;

/**
  * @brief  CAN 过滤器配置
  * @param  None
  * @retval None
  */
static void CAN_Filter_Config(void)
{
    FDCAN_FilterTypeDef Filter = {0};

    Filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;      /* 匹配的报文存入 FIFO0 */
    Filter.FilterID1 = 0x000;                            /* 标准 ID 起点 */
    Filter.FilterID2 = 0x7FF;                            /* 掩码：接收所有标准 ID (11位全开) */
    Filter.FilterIndex = 0;                              /* 滤波器索引 0 */
    Filter.FilterType = FDCAN_FILTER_MASK;               /* 掩码过滤模式 */
    Filter.IdType = FDCAN_STANDARD_ID;                   /* 标准标识符 */

    if (HAL_FDCAN_ConfigFilter(&hfdcan1, &Filter) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief  CAN 初始化（完整版，含中断激活）
  * @param  None
  * @retval None
  */
void CAN_Init(void)
{
    /* 配置滤波器 */
    CAN_Filter_Config();

    /* 启动 FDCAN */
    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
    {
        Error_Handler();
    }

    /* 激活接收 FIFO0 新消息中断 */
    if (HAL_FDCAN_ActivateNotification(&hfdcan1,
                                        FDCAN_IT_RX_FIFO0_NEW_MESSAGE,
                                        0) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief  CAN 初始化（不激活中断 - 测试用）
  * @param  None
  * @retval None
  */
void CAN_Init_NoIRQ(void)
{
    /* 配置滤波器 */
    CAN_Filter_Config();

    /* 启动 FDCAN */
    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
    {
        Error_Handler();
    }

    /* 不激活中断！ */
}

/**
  * @brief  CAN 发送数据
  * @param  id:   报文 ID
  * @param  data: 数据缓冲区
  * @param  len:  数据长度（经典 CAN 最大 8）
  * @retval 0=失败, 1=成功
  */
uint8_t CAN_Send(uint32_t id, uint8_t *data, uint8_t len)
{
    FDCAN_TxHeaderTypeDef TxHeader = {0};

    if (len > 8) return 0;   /* 经典 CAN 最多 8 字节 */

    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.DataLength = len;  /* DLC 直接赋值，不要左移！ */
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.Identifier = id;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.MessageMarker = 0x02;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;

    if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, data) != HAL_OK)
    {
        return 0;
    }
    return 1;
}

/**
  * @brief  CAN 接收数据（轮询调用）
  * @param  id:   保存接收到的 ID
  * @param  data: 保存接收到的数据
  * @param  len:  保存接收到的长度
  * @retval 1=有新报文, 0=无
  */
uint8_t CAN_Receive(uint32_t *id, uint8_t *data, uint8_t *len)
{
    if (s_rx_flag)
    {
        *id = s_rx_id;
        *len = s_rx_len;
        for (int i = 0; i < s_rx_len; i++)
        {
            data[i] = s_rx_data[i];
        }
        s_rx_flag = 0;
        return 1;
    }
    return 0;
}

/**
  * @brief  FDCAN 接收 FIFO0 回调（由中断调用）
  * @param  hfdcan:      FDCAN 句柄
  * @param  RxFifo0ITs:  FIFO0 中断状态
  * @retval None
  */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    UNUSED(hfdcan);

    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
    {
        if (HAL_FDCAN_GetRxMessage(hfdcan,
                                    FDCAN_RX_FIFO0,
                                    &RxHeader,
                                    s_rx_data) == HAL_OK)
        {
            s_rx_id = RxHeader.Identifier;
            s_rx_len = (uint8_t)(RxHeader.DataLength & 0xF);  /* 经典 CAN: DLC 在低4位 */
            s_rx_flag = 1;    /* 标记有新报文 */
        }
    }
}
