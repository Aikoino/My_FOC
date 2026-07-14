#include "uart_flash.h"

/* Global receive data buffers */
RxData_t rxdata = {0};
RxData_t recv = {0};

/* Flash parameter cache (to avoid unnecessary writes) */
static FlashParam_t flash_param_cache;


/* ================================================================
 * Motor Start/Stop wrapper functions
 * ================================================================ */
void Motor_Start(void)
{
    Motor_Start_Stop(1);
}

void Motor_Stop(void)
{
    Motor_Start_Stop(0);
}

/* ================================================================
 * USART3 DMA TX Complete Callback
 * ================================================================ */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &huart3)
        uart_tx_flag = 0;
}

/* ================================================================
 * BSP Vofa (USART3 DMA RX) Initialization
 * ================================================================ */
void BSP_Vofa_Init(void)
{
    HAL_UART_Receive_DMA(&huart3, rxdata.buff, RXLEN);
    __HAL_UART_CLEAR_IDLEFLAG(&huart3);
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
}

/* ================================================================
 * USART3 Idle Interrupt Handler (call in USART3_IRQHandler)
 * ================================================================ */
void UART3_RxHandler(void)
{
    if(__HAL_UART_GET_FLAG(&huart3, UART_FLAG_IDLE) == SET)
    {
        __HAL_UART_CLEAR_IDLEFLAG(&huart3);
        HAL_UART_DMAStop(&huart3);
        rxdata.cnt = RXLEN - __HAL_DMA_GET_COUNTER(huart3.hdmarx);
        rxdata.flag = 1;
    }
}

/* ================================================================
 * CRC32 Calculation (for Flash parameter storage verification)
 * ================================================================ */
static uint32_t CRC32_Calculate(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFF;
    for(uint32_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for(uint8_t j = 0; j < 8; j++)
        {
            if(crc & 0x80000000)
                crc = (crc << 1) ^ 0x04C11DB7;
            else
                crc <<= 1;
        }
    }
    return crc;
}

/* ================================================================
 * Save Parameters to Internal Flash
 * ================================================================ */
void Flash_SaveParams(void)
{
    FlashParam_t params;
    uint32_t page_error = 0;

    /* Read current parameters */
    params.speed_kp   = rt_Simulink_Struct.spd_kp;
    params.speed_ki   = rt_Simulink_Struct.spd_ki;
    params.curr_kp    = rt_Simulink_Struct.CurrKp;
    params.curr_ki    = rt_Simulink_Struct.CurrKi;
    params.flux       = motor.flux;
    params.rs         = motor.Rs;
    params.ls         = motor.L;
    params.pn         = motor.Pn;
    params.hall_state = Hal_State;
    params.volt_up    = flash_param_cache.volt_up;
    params.volt_low   = flash_param_cache.volt_low;

    /* Calculate CRC */
    params.crc = CRC32_Calculate((uint8_t *)&params, sizeof(FlashParam_t) - 4);

    /* Compare with cache to avoid unnecessary writes */
    if(memcmp(&params, &flash_param_cache, sizeof(FlashParam_t)) == 0)
        return;

    /* Unlock Flash */
    HAL_FLASH_Unlock();

    /* Erase target page (Page 63, last page of 128KB Flash on STM32G431CB) */
    FLASH_EraseInitTypeDef erase_init = {0};
    erase_init.TypeErase   = FLASH_TYPEERASE_PAGES;
    erase_init.Banks       = FLASH_BANK_1;
    erase_init.Page        = 63;
    erase_init.NbPages     = 1;
    HAL_FLASHEx_Erase(&erase_init, &page_error);

    /* Write as double-words (64-bit) to Flash */
    uint32_t addr = FLASH_PARAM_ADDR;
    uint32_t *pSrc = (uint32_t *)&params;
    uint32_t word_count = sizeof(FlashParam_t) / 8;
    if(sizeof(FlashParam_t) % 8) word_count++;

    for(uint32_t i = 0; i < word_count; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, addr,
                          *(uint64_t *)(pSrc + i * 2));
        addr += 8;
    }

    /* Lock Flash */
    HAL_FLASH_Lock();

    /* Update cache */
    memcpy(&flash_param_cache, &params, sizeof(FlashParam_t));
}

/* ================================================================
 * Load Parameters from Internal Flash
 * ================================================================ */
void Flash_LoadParams(void)
{
    FlashParam_t *pFlash = (FlashParam_t *)FLASH_PARAM_ADDR;

    /* Check if Flash has been written (0xFFFFFFFF means erased/empty) */
    uint32_t *pCheck = (uint32_t *)FLASH_PARAM_ADDR;
    if(pCheck[0] == 0xFFFFFFFF)
        return;

    /* Verify CRC */
    uint32_t crc_calc = CRC32_Calculate((uint8_t *)pFlash, sizeof(FlashParam_t) - 4);
    if(crc_calc != pFlash->crc)
        return;

    /* Load parameters into FOC model */
    rt_Simulink_Struct.spd_kp = pFlash->speed_kp;
    rt_Simulink_Struct.spd_ki = pFlash->speed_ki;
    rt_Simulink_Struct.CurrKp = pFlash->curr_kp;
    rt_Simulink_Struct.CurrKi = pFlash->curr_ki;
    motor.flux  = pFlash->flux;
    motor.Rs    = pFlash->rs;
    motor.L     = pFlash->ls;
    motor.Pn    = pFlash->pn;
    Hal_State   = pFlash->hall_state;

    /* Update cache */
    memcpy(&flash_param_cache, pFlash, sizeof(FlashParam_t));
}

/* ================================================================
 * USART3 Data Analyze (called periodically in main loop)
 *
 * Communication Protocol:
 *   Frame Header: 0xAA 0x55 (2 bytes)
 *   Command:      1 byte (0x01~0x0E)
 *   Data:         4 bytes (float, little-endian)
 *   Frame Tail:   0x55 0xAA (2 bytes)
 *
 * Total frame length:
 *   9 bytes (0x02~0x0D with data)
 *   5 bytes (0x01 / 0x0E without data)
 * ================================================================ */
void USART3_Anylze(void)
{
    static uint8_t motor_run_flag = 0;

    if(rxdata.flag != 1)
        return;

    /* Verify frame header */
    if(rxdata.buff[0] != 0xAA || rxdata.buff[1] != 0x55)
    {
        memset(&rxdata, 0, sizeof(rxdata));
        return;
    }

    uint8_t cmd = rxdata.buff[2];

    /* Extract data field (little-endian: buff[3]=LSB ... buff[6]=MSB) */
    recv.buff[0] = rxdata.buff[3];
    recv.buff[1] = rxdata.buff[4];
    recv.buff[2] = rxdata.buff[5];
    recv.buff[3] = rxdata.buff[6];

    /* Interpret 4-byte data as float (little-endian) */
    float fval;
    memcpy(&fval, recv.buff, sizeof(float));

    switch(cmd)
    {
    /* --------------------------------------------------------
     * 0x01: Motor Start/Stop toggle
     *   Send once to start, send again to stop
     *   Frame: AA 55 01 55 AA (no data)
     * -------------------------------------------------------- */
    case 0x01:
        motor_run_flag = !motor_run_flag;
        if(motor_run_flag)
            Motor_Start();
        else
            Motor_Stop();
        break;

    /* --------------------------------------------------------
     * 0x02: Speed Setting (4-byte float, RPM)
     *   Example: set 1000 RPM -> AA 55 02 00 00 7A 44 55 AA
     * -------------------------------------------------------- */
    case 0x02:
        target_speed_ref = fval;
        break;

    /* --------------------------------------------------------
     * 0x03: Speed Loop Kp
     * -------------------------------------------------------- */
    case 0x03:
        rt_Simulink_Struct.spd_kp = fval;
        break;

    /* --------------------------------------------------------
     * 0x04: Speed Loop Ki
     * -------------------------------------------------------- */
    case 0x04:
        rt_Simulink_Struct.spd_ki = fval;
        break;

    /* --------------------------------------------------------
     * 0x05: Current Loop Kp
     * -------------------------------------------------------- */
    case 0x05:
        rt_Simulink_Struct.CurrKp = fval;
        break;

    /* --------------------------------------------------------
     * 0x06: Current Loop Ki
     * -------------------------------------------------------- */
    case 0x06:
        rt_Simulink_Struct.CurrKi = fval;
        break;

    /* --------------------------------------------------------
     * 0x07: Flux Constant (Wb)
     * -------------------------------------------------------- */
    case 0x07:
        motor.flux = fval;
        break;

    /* --------------------------------------------------------
     * 0x08: Phase Resistance (Ohm)
     * -------------------------------------------------------- */
    case 0x08:
        motor.Rs = fval;
        break;

    /* --------------------------------------------------------
     * 0x09: Phase Inductance (H)
     * -------------------------------------------------------- */
    case 0x09:
        motor.L = fval;
        break;

    /* --------------------------------------------------------
     * 0x0A: Pole Pairs
     * -------------------------------------------------------- */
    case 0x0A:
        motor.Pn = fval;
        break;

    /* --------------------------------------------------------
     * 0x0B: Open-Loop Force Speed (RPM)
     * -------------------------------------------------------- */
    case 0x0B:
        /* Open-loop forced speed, used during startup sequence */
        break;

    /* --------------------------------------------------------
     * 0x0C: Sensored / Sensorless Selection
     *   Send 0 = Sensored (HALL), 1 = Sensorless (Observer)
     * -------------------------------------------------------- */
    case 0x0C:
        float sel;
        memcpy(&sel, recv.buff, 4);
        Hal_State = (uint8_t)sel;                       // ‘À–– ±«–ªª
        break;

    /* --------------------------------------------------------
     * 0x0D: Power Supply Voltage Threshold
     *   First 2 bytes: upper limit (uint16, unit: 0.1V)
     *   Last 2 bytes:  lower limit (uint16, unit: 0.1V)
     *   Example: AA 55 0D 00 24 00 18 55 AA
     *     Upper: 0x0024=36
     *     Lower: 0x0018=24
     * -------------------------------------------------------- */
    case 0x0D:
    {
        uint16_t volt_up  = recv.buff[0] | ((uint16_t)recv.buff[1] << 8);
        uint16_t volt_low = recv.buff[2] | ((uint16_t)recv.buff[3] << 8);
        flash_param_cache.volt_up  = volt_up;
        flash_param_cache.volt_low = volt_low;
        break;
    }

    /* --------------------------------------------------------
     * 0x0E: Save Parameters to Internal Flash
     *   Frame: AA 55 0E 55 AA (no data)
     * -------------------------------------------------------- */
    case 0x0E:
        Flash_SaveParams();
        break;

    default:
        break;
    }

    /* Clear receive buffer and restart DMA reception */
    memset(&rxdata, 0, sizeof(rxdata));
    HAL_UART_Receive_DMA(&huart3, rxdata.buff, RXLEN);
}
