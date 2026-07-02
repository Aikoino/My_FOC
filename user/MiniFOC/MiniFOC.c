/**
  ******************************************************************************
  * @file           : MiniFOC.c
  * @brief          : MiniFOC 核心实现
  * @attention
  *
  * 核心功能：
  * - 电流环控制 (10kHz)
  * - 速度环控制 (1kHz)
  * - SVPWM生成
  * - Clark/Park坐标变换
  *
  ******************************************************************************
  */
#include "MiniFOC.h"
#include "MiniFOC_Config.h"

/* 引入BSP模块（你需要保持这些模块的接口） */
#include "../bsp_adc.h"
#include "../bsp_uart_vofta.h"
#include "../bsp_can.h"
#include "stm32g4xx_hal.h"

/* ========== 全局变量 ========== */

MiniFOC_t foc;

/* PWM句柄（需要与你的硬件匹配）*/
extern TIM_HandleTypeDef htim1;

/* ========== 内部函数声明 ========== */

static void MiniFOC_CurrentLoop(void);
static void MiniFOC_SpeedLoop(void);
static void MiniFOC_ApplyPWM(float pwm_a, float pwm_b, float pwm_c);
static void MiniFOC_UpdateSensor(void);
static void MiniFOC_CurrentCalibration(void);

/* ========== 初始化函数 ========== */

/**
  * @brief  MiniFOC初始化
  */
void MiniFOC_Init(void)
{
    /* 清空结构体 */
    memset(&foc, 0, sizeof(foc));

    /* 默认配置 */
    foc.mode = DEFAULT_CONTROL_MODE;
    foc.cmd_source = DEFAULT_CMD_SOURCE;
    foc.motor_running = false;
    foc.fault_flag = false;

    /* 初始化PID */
    PID_Init(&foc.current_pid,
              DEFAULT_CURRENT_KP,
              DEFAULT_CURRENT_KI,
              DEFAULT_CURRENT_KD);
    PID_Set_Limit(&foc.current_pid, DEFAULT_CURRENT_LIMIT, -DEFAULT_CURRENT_LIMIT);

    PID_Init(&foc.speed_pid,
              DEFAULT_SPEED_KP,
              DEFAULT_SPEED_KI,
              DEFAULT_SPEED_KD);
    PID_Set_Limit(&foc.speed_pid, DEFAULT_SPEED_LIMIT, -DEFAULT_SPEED_LIMIT);

    /* 初始化ADC（如果尚未初始化）*/
    BSP_ADC_Init();

    /* 电流采样零点校准 (电机必须静止！) */
    MiniFOC_CurrentCalibration();

    /* 启停PWM输出（输出全0）*/
    MiniFOC_ApplyPWM(0.0f, 0.0f, 0.0f);
}

/**
  * @brief  主循环任务 (1kHz)
  */
void MiniFOC_MainLoop(void)
{
    /* 更新传感器数据 */
    MiniFOC_UpdateSensor();

    /* 速度环控制 */
    MiniFOC_SpeedLoop();

    /* 故障检测 */
    if (foc.fault_flag) {
        MiniFOC_MotorEnable(false);
    }
}

/**
  * @brief  高频任务 (10kHz，在TIM1中断中调用)
  */
void MiniFOC_HighFreqLoop(void)
{
    if (!foc.motor_running) return;

    /* 电流环控制 */
    MiniFOC_CurrentLoop();
}

/* ========== 控制函数 ========== */

/**
  * @brief  电流环 (10kHz)
  */
static void MiniFOC_CurrentLoop(void)
{
    /* 1. 读取三相电流 (零点校准后) */
    float Iu = BSP_ADC_GetCurrentU() - foc.current_offset_u;
    float Iv = BSP_ADC_GetCurrentV() - foc.current_offset_v;
    float Iw = BSP_ADC_GetCurrentW() - foc.current_offset_w;

    /* 保存到结构体 */
    foc.phase_current_u = Iu;
    foc.phase_current_v = Iv;
    foc.phase_current_w = Iw;

    /* 2. Clark变换 (abc → αβ) */
    float ialpha, ibeta;
    Clark_Transform(foc.phase_current_u,
                    foc.phase_current_v,
                    foc.phase_current_w,
                    &ialpha, &ibeta);

    /* 3. Park变换 (αβ → dq) */
    Park_Transform(ialpha, ibeta, foc.rotor_angle, &foc.Id, &foc.Iq);

    /* 4. 电流环PID（控制q轴电流产生转矩）*/
    float Iq_ref = PID_Pos_Calc(&foc.current_pid, foc.target_current, foc.Iq);

    /* 5. 限制Vq电压 */
    foc.Vq = Limit(Iq_ref, -foc.bus_voltage * 0.5f, foc.bus_voltage * 0.5f);
    foc.Vd = 0;  /* d轴电压设为0（最大转矩电流比）*/

    /* 6. 逆Park变换 (dq → αβ) */
    float ualpha, ubeta;
    Park_Inv_Transform(foc.Vd, foc.Vq, foc.rotor_angle, &ualpha, &ubeta);

    /* 7. SVPWM生成 */
    float pwm_a, pwm_b, pwm_c;
    SVPWM_Generate(ualpha / foc.bus_voltage, ubeta / foc.bus_voltage,
                   &pwm_a, &pwm_b, &pwm_c);

    /* 8. 更新PWM */
    MiniFOC_ApplyPWM(pwm_a, pwm_b, pwm_c);
}

/**
  * @brief  速度环 (1kHz)
  */
static void MiniFOC_SpeedLoop(void)
{
    /* 读取速度 */
    foc.rotor_speed = MiniFOC_GetSpeed();

    /* 速度环PID */
    float current_ref = PID_Pos_Calc(&foc.speed_pid,
                                      foc.target_speed,
                                      foc.rotor_speed);

    foc.target_current = current_ref;
}

/**
  * @brief  更新传感器数据
  */
static void MiniFOC_UpdateSensor(void)
{
    /* 母线电压（包含分压系数）*/
    foc.bus_voltage = BSP_ADC_GetVbus() * (ADC_VOLTAGE_REF / ADC_MAX_VALUE) * VBUS_DIVIDER_RATIO;

    /* 转子角度和速度（根据传感器类型实现）*/
    // TODO: 根据实际传感器实现
    // - 编码器：读取TIM2 CNT
    // - 霍尔：解码霍尔状态
    // - 无感：调用观测器

    /* 当前简单实现：开环估算 */
    if (foc.motor_running) {
        static float openloop_angle = 0;
        float speed_rad_per_s = foc.target_speed * TWO_PI / 60.0f;
        openloop_angle += speed_rad_per_s * 0.001f;  /* 1kHz */
        openloop_angle = Normalize_Angle(openloop_angle);
        foc.rotor_angle = openloop_angle;
    }
}

/**
  * @brief  应用PWM到TIM1
  */
static void MiniFOC_ApplyPWM(float pwm_a, float pwm_b, float pwm_c)
{
    /* 转换为定时器比较值 */
    uint32_t ccr_a = (uint32_t)(pwm_a * PWM_PERIOD);
    uint32_t ccr_b = (uint32_t)(pwm_b * PWM_PERIOD);
    uint32_t ccr_c = (uint32_t)(pwm_c * PWM_PERIOD);

    /* 更新比较寄存器 */
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr_a);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, ccr_b);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, ccr_c);
}

/* ========== 用户API ========== */

void MiniFOC_SetMode(ControlMode_t mode)
{
    foc.mode = mode;
}

void MiniFOC_SetTargetSpeed(float speed)
{
    foc.target_speed = Limit(speed, -MOTOR_MAX_SPEED, MOTOR_MAX_SPEED);
}

void MiniFOC_SetTargetCurrent(float current)
{
    foc.target_current = Limit(current, -MOTOR_MAX_CURRENT, MOTOR_MAX_CURRENT);
}

void MiniFOC_MotorEnable(bool enable)
{
    foc.motor_running = enable;

    if (enable) {
        /* 使能PWM输出 */
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
        HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    } else {
        /* 关闭PWM输出 */
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);

        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
        HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);

        PID_Reset(&foc.current_pid);
        PID_Reset(&foc.speed_pid);
    }
}

float MiniFOC_GetSpeed(void)
{
    return foc.rotor_speed;
}

float MiniFOC_GetCurrent(void)
{
    return foc.Iq;
}

void MiniFOC_FaultHandler(uint32_t fault_code)
{
    foc.fault_flag = true;
    foc.fault_code = fault_code;
    MiniFOC_MotorEnable(false);

    /* TODO: 根据故障码处理 */
    switch (fault_code) {
        case 0x01:  /* 过流 */
            break;
        case 0x02:  /* 欠压 */
            break;
        case 0x03:  /* 过压 */
            break;
    }
}

/**
  * @brief  电流采样零点校准
  * @note   电机必须处于静止状态，上电时调用一次
  */
static void MiniFOC_CurrentCalibration(void)
{
    /* 电流零点（ADC 原始值）*/
    static float current_offset_u = 0.0f;
    static float current_offset_v = 0.0f;
    static float current_offset_w = 0.0f;

    /* 采样次数 */
    #define CALIBRATION_SAMPLES  100

    /* 累计采样值 */
    int32_t sum_u = 0, sum_v = 0, sum_w = 0;
    int16_t adc_u = 0, adc_v = 0, adc_w = 0;

    /* 采样 N 次取平均 */
    for (uint16_t i = 0; i < CALIBRATION_SAMPLES; i++) {
        /* 读取注入 ADC 原始值 */
        adc_u = (int16_t)HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
        adc_v = (int16_t)HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1);
        adc_w = (int16_t)HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2);

        sum_u += adc_u;
        sum_v += adc_v;
        sum_w += adc_w;

        HAL_Delay(1);
    }

    /* 计算平均值 */
    current_offset_u = (float)(sum_u / CALIBRATION_SAMPLES) * CURRENT_SAMPLE_RES;
    current_offset_v = (float)(sum_v / CALIBRATION_SAMPLES) * CURRENT_SAMPLE_RES;
    current_offset_w = (float)(sum_w / CALIBRATION_SAMPLES) * CURRENT_SAMPLE_RES;

    /* 保存到全局变量（供电流环使用）*/
    foc.current_offset_u = current_offset_u;
    foc.current_offset_v = current_offset_v;
    foc.current_offset_w = current_offset_w;
}
