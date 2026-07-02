# FOC无刷电机驱动器开发计划 (v2.0)

**项目名称**: My_FOC (STM32G431 电机控制系统)
**目标芯片**: STM32G431CBT6 @ 170MHz
**开发周期**: 15天
**创建时间**: 2026-07-02
**更新时间**: 2026-07-02 20:30
**状态**: ✅ 方案已确认，准备开始

---

## 📋 目录

1. [项目概述](#1-项目概述)
2. [当前工程功能](#2-当前工程功能)
3. [上位机功能需求](#3-上位机功能需求)
4. [FOC库选型决策](#4-foc库选型决策)
5. [详细开发计划](#5-详细开发计划)
6. [进度跟踪表](#6-进度跟踪表)
7. [里程碑节点](#7-里程碑节点)
8. [风险与应对](#8-风险与应对)
9. [参考资源](#9-参考资源)
10. [附录](#10-附录)

---

## 1. 项目概述

### 1.1 项目背景
基于 STM32G431CBT6 的无刷电机 FOC 控制系统，集成完整的参数配置、通信和保护功能。

### 1.2 硬件配置

| 模块 | 型号/参数 |
|------|----------|
| **主控** | STM32G431CBT6 (Cortex-M4, FPU, 170MHz) |
| **栅极驱动** | FD6288 (三路半桥) |
| **CAN收发** | L9616CAN (500kbps) |
| **电流采样** | ADC1 注入组 + OPAMP1/2/3 (三相电流) |
| **母线电压** | ADC1 规则组 (分压 26:1) |
| **PWM输出** | TIM1 CH1/1N, CH2/2N, CH3/3N @ 20kHz |
| **调试串口** | USART3 @ 921600 baud |
| **人机界面** | LED2/3 (PC14/15) + KEY (PC13) |

### 1.3 现有资源

| 资源 | 状态 | 说明 |
|------|------|------|
| **当前项目** | ✅ 可用 | 基础功能已验证 (按键、ADC、VOFA+、CAN) |
| **MiniFOC库** | ✅ 当前项目 | v1.0.0, 1,036 行，8种控制模式 |
| **SguanFOC库** | ✅ E盘已有 | v3.0.0/v3.0.1/v3.1.0，**备用** |
| **上位机** | ⏳ 待开发 | 需求已明确 (见第3节) |
| **硬件电路** | ✅ 已完成 | L9616CAN、FD6288、采样电路 |

---

## 2. 当前工程功能

### 2.1 已验证功能清单

| 功能模块 | 状态 | 验证时间 |
|---------|------|---------|
| **系统初始化** | ✅ 完成 | 2026-06-30 |
| **按键检测** | ✅ 完成 | 2026-06-30 17:40 |
| **LED指示** | ✅ 完成 | 2026-06-30 |
| **ADC采样** | ✅ 完成 | 2026-07-01 |
| **OPAMP放大** | ✅ 完成 | 2026-07-01 |
| **VOFA+波形** | ✅ 完成 | 2026-07-01 18:30 |
| **CAN通信** | ✅ 完成 | 2026-07-02 |
| **TIM1 PWM** | ✅ 完成 | 2026-07-02 |

### 2.2 硬件资源分配

#### GPIO
- **PC13**: KEY (按键输入)
- **PC14**: LED2 (状态指示)
- **PC15**: LED3 (心跳闪烁)

#### ADC (ADC1 + ADC2)
- **ADC1 JDR1** (PA2): U相电流 (OPAMP1)
- **ADC1 JDR2** (PB1): W相电流 (OPAMP3)
- **ADC1 JDR3** (PA0): 母线电压 (分压 26:1)
- **ADC2 JDR1** (PA6): V相电流 (OPAMP2)

#### TIM1 (PWM + ADC触发)
- **CH1/CH1N** (PE8/PE7): U相 PWM
- **CH2/CH2N** (PE9/PE10): V相 PWM
- **CH3/CH3N** (PE11/PE12): W相 PWM
- **CH4** (PE13): ADC 注入触发
- **频率**: 20kHz (ARR=7999, 中心对齐)
- **死区时间**: 120ns

#### FDCAN1
- **TX** (PA12): CAN 发送
- **RX** (PA11): CAN 接收
- **波特率**: 500 kbps (标准帧)

#### USART3 (调试)
- **TX** (PB10): 调试输出
- **RX** (PB11): 命令输入 (可选)
- **波特率**: 921600

### 2.3 软件架构

```
main.c
├── 初始化阶段
│   ├── HAL_Init()
│   ├── SystemClock_Config() → 160MHz
│   ├── MX_GPIO_Init()
│   ├── MX_USART3_UART_Init()
│   ├── MX_TIM1_Init()
│   ├── MX_ADC1_Init() + MX_ADC2_Init()
│   ├── MX_OPAMP1/2/3_Init()
│   ├── MX_FDCAN1_Init()
│   ├── BSP_Init() (ADC/UART/Button/Motor)
│   ├── CAN_Init()
│   └── MiniFOC_Init() ← 新增
│
└── 主循环 (while(1) @ 1kHz)
    ├── 按键扫描 (10ms)
    ├── ADC 采样 (100ms)
    ├── VOFA+ 发送 (100ms)
    ├── CAN 发送 (500ms)
    ├── CAN 接收 (轮询)
    ├── MiniFOC_MainLoop() ← 新增
    └── LED 闪烁 (500ms)

TIM1 中断 (10kHz)
└── MiniFOC_HighFreqLoop() ← 新增
    ├── 电流环 PID
    ├── Clarke/Park 变换
    ├── SVPWM 生成
    └── PWM 更新
```

---

## 3. 上位机功能需求

### 3.1 控制功能

#### 电机运动控制
- ✅ **启停控制**: 一键启动/停止电机
- ✅ **转速设定**: 目标转速实时调节 (RPM)
- ⏳ **启停方式**: 平滑斜坡加减速 (可配置时间)

#### FOC 参数在线整定

| 参数分类 | 可配置参数 | 数据类型 | 范围 |
|---------|----------|---------|------|
| **闭环PI参数** | 速度环 KP | float | 0 ~ 10 |
| | 速度环 KI | float | 0 ~ 1 |
| | 电流环 KP | float | 0 ~ 10 |
| | 电流环 KI | float | 0 ~ 1 |
| **电机本体参数** | 磁链常数 | float | 0.001 ~ 0.1 Wb |
| | 相电阻 | float | 0.01 ~ 5 Ω |
| | 相电感 | float | 0.0001 ~ 0.01 H |
| | 极对数 | uint16 | 1 ~ 20 |
| **启动控制参数** | 开环强拖速度 | uint16 | 100 ~ 3000 RPM |
| **保护阈值参数** | 母线电压上限 | uint16 | 0.1V 单位 (如 450=45.0V) |
| | 母线电压下限 | uint16 | 0.1V 单位 (如 250=25.0V) |

#### 参数存储
- ✅ **保存到 Flash**: 一键保存所有参数到 STM32 片内 Flash
- ✅ **断电保护**: 参数不丢失，上电自动加载
- ✅ **CRC 校验**: 防止 Flash 数据损坏

### 3.2 串口通信配置

支持自定义串口底层通信参数:

| 参数 | 可选值 | 默认值 |
|------|--------|--------|
| **端口号** | COM1 ~ COM256 | COM3 |
| **波特率** | 9600 ~ 921600 | 921600 |
| **数据位** | 7/8 | 8 |
| **停止位** | 1/2 | 1 |
| **校验位** | None/Odd/Even | None |

- ✅ **打开串口**: 建立通信连接
- ✅ **关闭串口**: 断开连接释放端口

### 3.3 CAN 通信功能

| 功能 | 说明 |
|------|------|
| **CAN 发送** | STM32 → 上位机 (状态回传、响应) |
| **CAN 接收** | 上位机 → STM32 (控制命令) |
| **波特率** | 500 kbps (固定) |
| **ID 格式** | 标准帧 (11位) |
| **发送 ID** | 0x125 |
| **接收 ID** | 0x100/0x126 |

### 3.4 状态监控

- ⏳ **实时数据回传**:
  - 实际转速 (RPM)
  - 三相电流 (A)
  - 母线电压 (V)
  - 电机状态 (停止/运行/故障)

- ⏳ **故障报警**:
  - 过流保护 (OCP)
  - 过压保护 (OVP)
  - 欠压保护 (UVP)
  - 过温保护 (OTP)

---

## 4. FOC库选型决策

### 4.1 对比分析 (已评估)

#### MiniFOC vs SguanFOC v3.1.0

| 对比项 | MiniFOC | SguanFOC v3.1.0 |
|--------|---------|-----------------|
| **代码规模** | 1,036 行 | 7,151 行 |
| **文件数量** | 7 个 | 42 个 |
| **控制模式** | 8 种基础 | 20+ 种高级 |
| **无感算法** | ❌ 无 | ✅ SMO + HFI |
| **高级控制** | ❌ 基础PID | ✅ LADRC + 滤波器 |
| **当前适配性** | ✅ 已适配 | ⚠️ 需移植 |

详见: `MiniFOC_vs_SguanFOC_对比分析.md`

### 4.2 最终决策

**✅ 采用 MiniFOC v1.0.0**

#### 核心理由

1. **够用就好**
   - 当前需求: 电流环 + 速度环 + CAN/串口控制
   - MiniFOC **完全满足**这些需求
   - 不需要过度设计

2. **时间成本**
   - 节省 3-4 天 (不需要移植代码)
   - 可用于完善保护功能、调试 PID、编写文档

3. **风险可控**
   - MiniFOC 已验证可用
   - 不需要重新移植和调试

4. **可扩展**
   - 后续如需 SMO/HFI/LADRC 可随时升级 SguanFOC
   - 约 2 天即可完成升级
   - 不损失任何现有代码

#### 后续升级路径

```
当前: MiniFOC (基础FOC控制)
  ↓
如需要高级功能:
  ├── 评估需求 (第15天)
  ├── 升级到 SguanFOC (2天)
  └── 获得 SMO/HFI/LADRC/滤波器
```

---

## 5. 详细开发计划

### 🔴 阶段1: MiniFOC 基础功能 (第1-4天)

#### 第1天: MiniFOC 配置优化 + 硬件适配

**目标**: 优化 MiniFOC 配置，确保硬件适配正确

**任务清单**:

- [ ] **1.1 验证 MiniFOC_Config.h**
  ```c
  ✅ 系统时钟: 170MHz (已配置)
  ✅ ADC 参数: 3.3V, 12bit, 4095 (正确)
  ✅ 分压系数: 26.0f (75k+3k)/3k (正确)
  ✅ PWM 频率: 20kHz (已配置)
  ✅ 死区时间: 500ns (Current: 120ns, 需确认)
  ✅ 电机参数: 极对数7, 额定电流5A (需根据实际电机修改)
  ✅ PID 参数: 电流环0.1/0.01, 速度环0.5/0.1 (保守值)
  ```

- [ ] **1.2 验证电流采样适配**
  ```c
  // MiniFOC 需要的接口 (已在 bsp_adc.c 实现)
  float BSP_ADC_GetCurrentU(void);  // ✅ PA2, ADC1 JDR1
  float BSP_ADC_GetCurrentV(void);  // ✅ PA6, ADC2 JDR1
  float BSP_ADC_GetCurrentW(void);  // ✅ PB1, ADC1 JDR2
  float BSP_ADC_GetVbus(void);      // ✅ PA0, ADC1 JDR3 (需实现)

  // 电流转换系数 (从 bsp_adc.c 复制)
  #define CURRENT_SAMPLE_RES  0.021972f  // A/bit
  ```

- [ ] **1.3 验证 PWM 输出接口**
  ```c
  // MiniFOC 需要的接口
  void MiniFOC_ApplyPWM(float pwm_a, float pwm_b, float pwm_c);
  // 实现: 更新 TIM1 CCR1/2/3
  ```

- [ ] **1.4 配置 TIM1 中断 (10kHz)**
  ```c
  // 在 tim.c 的 MX_TIM1_Init() 末尾添加
  HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

  // 在 stm32g4xx_it.c 中添加
  void TIM1_UP_TIM16_IRQHandler(void)
  {
      HAL_TIM_IRQHandler(&htim1);
      MiniFOC_HighFreqLoop();  // 电流环 10kHz
  }
  ```

- [ ] **1.5 编译测试**
  - 添加 MiniFOC 到 Keil 工程
  - 确认编译通过
  - 下载测试

**验收标准**:
- ✅ Keil 编译通过，无错误
- ✅ MiniFOC_Config.h 配置正确
- ✅ TIM1 中断配置正确
- ✅ 工程大小增加 < 2KB

---

#### 第2天: 电流环 PID 整定

**目标**: 实现稳定的电流闭环控制

**任务清单**:

- [ ] **2.1 实现电流采样零点校准**
  ```c
  // MiniFOC.c 中添加
  static float current_offset_u = 0.0f;
  static float current_offset_v = 0.0f;
  static float current_offset_w = 0.0f;

  void MiniFOC_CurrentCalibration(void)
  {
      // 上电时电机静止，采样100次平均
      uint32_t sum_u = 0, sum_v = 0, sum_w = 0;
      for (int i = 0; i < 100; i++) {
          sum_u += HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
          sum_v += HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1);
          sum_w += HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2);
          HAL_Delay(1);
      }
      current_offset_u = (sum_u / 100.0f) * CURRENT_SAMPLE_RES;
      current_offset_v = (sum_v / 100.0f) * CURRENT_SAMPLE_RES;
      current_offset_w = (sum_w / 100.0f) * CURRENT_SAMPLE_RES;
  }
  ```

- [ ] **2.2 电流环控制逻辑 (10kHz)**
  ```c
  // MiniFOC_HighFreqLoop() 中
  static void MiniFOC_CurrentLoop(void)
  {
      // 1. 读取三相电流 (零点校准后)
      float Iu = BSP_ADC_GetCurrentU() - current_offset_u;
      float Iv = BSP_ADC_GetCurrentV() - current_offset_v;
      float Iw = BSP_ADC_GetCurrentW() - current_offset_w;

      // 2. Clarke 变换 (abc → αβ)
      float I_alpha = Iu;
      float I_beta = (Iv - Iw) / 1.73205080757f;

      // 3. Park 变换 (αβ → dq)
      float cos_theta = cosf(foc.rotor_angle);
      float sin_theta = sinf(foc.rotor_angle);
      foc.Id = I_alpha * cos_theta + I_beta * sin_theta;
      foc.Iq = -I_alpha * sin_theta + I_beta * cos_theta;

      // 4. 电流环 PID (控制 Iq, Id 保持 0)
      float Iq_ref = foc.target_current;
      float V_q = PID_Pos_Calc(&foc.current_pid, Iq_ref, foc.Iq);
      float V_d = PID_Pos_Calc(&foc.current_pid, 0.0f, foc.Id);

      // 5. 限制电压 (90% 母线电压)
      float v_limit = foc.bus_voltage * 0.9f;
      foc.Vq = Limit(V_q, -v_limit, v_limit);
      foc.Vd = Limit(V_d, -v_limit, v_limit);

      // 6. 逆 Park 变换 (dq → αβ)
      float V_alpha = foc.Vd * cos_theta - foc.Vq * sin_theta;
      float V_beta  = foc.Vd * sin_theta + foc.Vq * cos_theta;

      // 7. SVPWM 生成
      MiniFOC_SVPWMGenerate(V_alpha, V_beta);
  }
  ```

- [ ] **2.3 实现 SVPWM**
  ```c
  static void MiniFOC_SVPWMGenerate(float U_alpha, float U_beta)
  {
      // 1. 计算扇区
      float U_out = sqrtf(U_alpha*U_alpha + U_beta*U_beta);
      float theta = atan2f(U_beta, U_alpha);
      if (theta < 0) theta += 2.0f * PI;

      uint8_t sector = (uint8_t)(theta / (PI / 3.0f)) + 1;

      // 2. 计算 T1, T2 (简化版)
      float T1 = U_out * sinf((float)sector * PI / 3.0f - theta);
      float T2 = U_out * sinf(theta - (float)(sector-1) * PI / 3.0f);
      float T = PWM_PERIOD;

      float Ta = (T - T1 - T2) / 2.0f;
      float Tb = Ta + T1;
      float Tc = Ta + T1 + T2;

      // 3. 更新 CCR
      switch(sector) {
          case 1:  // U_alpha > 0, U_beta > 0
              __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, Tb);
              __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, Ta);
              __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, Tc);
              break;
          // ... 其他扇区
      }
  }
  ```

- [ ] **2.4 调试电流环**
  - 从开环 VF 切换闭环
  - VOFA+ 监控 Iq 响应
  - 调整 Kp/Ki 直到稳定

**验收标准**:
- ✅ 静止时电流 < ±0.05A
- ✅ 电流阶跃响应时间 < 5ms
- ✅ 稳态误差 < 5%

---

#### 第3天: 速度环 PID 整定

**目标**: 实现速度闭环控制

**任务清单**:

- [ ] **3.1 速度计算**
  ```c
  // 方法1: 编码器 (如果安装)
  float MiniFOC_GetSpeedFromEncoder(void)
  {
      static int32_t last_cnt = 0;
      int32_t diff = encoder_cnt - last_cnt;
      last_cnt = encoder_cnt;
      return (float)diff * 60.0f / (MOTOR_POLE_PAIRS * ENCODER_LINES * 0.001f);
  }

  // 方法2: 观测器估算 (当前使用开环)
  float MiniFOC_GetSpeedFromObserver(void)
  {
      // 先用开环角度差分估算
      static float last_angle = 0.0f;
      static uint32_t last_time = 0;

      uint32_t now = HAL_GetTick();
      float dt = (now - last_time) * 0.001f;
      last_time = now;

      float speed = (foc.rotor_angle - last_angle) / dt;
      last_angle = foc.rotor_angle;

      return speed * 60.0f / (2.0f * PI);  // 转换为 RPM
  }
  ```

- [ ] **3.2 速度环控制逻辑 (1kHz)**
  ```c
  // 主循环调用
  void MiniFOC_SpeedLoop(void)
  {
      // 1. 读取实际转速
      foc.rotor_speed = MiniFOC_GetSpeedFromObserver();

      // 2. 速度环 PID (输出作为 Iq 参考)
      float speed_error = foc.target_speed - foc.rotor_speed;
      float Iq_ref = PID_Pos_Calc(&foc.speed_pid, speed_error, 0.001f);

      // 3. 限幅 (电流环输入)
      foc.target_current = Limit(Iq_ref, -CURRENT_LIMIT, +CURRENT_LIMIT);
  }
  ```

- [ ] **3.3 调试速度环**
  - 设定目标转速 100 RPM
  - VOFA+ 监控速度响应
  - 调整 Kp/Ki 直到稳态误差 < 2%

**验收标准**:
- ✅ 速度响应时间 < 200ms
- ✅ 稳态误差 < 2%
- ✅ 无振荡

---

#### 第4天: 开环测试 + 闭环验证

**目标**: 验证 MiniFOC 功能，确保电机可控

**任务清单**:

- [ ] **4.1 开环 VF 测试**
  ```c
  // 在 main.c 中测试开环
  while(1) {
      // 固定电压 2V, 频率 10Hz
      static float angle = 0.0f;
      angle += 2.0f * PI * 10.0f * 0.001f;  // 10Hz
      if (angle > 2.0f * PI) angle -= 2.0f * PI;

      // 开环输出
      float U = 2.0f * cosf(angle);
      float V = 2.0f * cosf(angle - 2.0f*PI/3.0f);
      float W = 2.0f * cosf(angle + 2.0f*PI/3.0f);

      MiniFOC_SVPWMGenerate(U, V);

      HAL_Delay(1);
  }
  ```

- [ ] **4.2 验证 PWM 波形**
  - 用示波器查看三相 PWM
  - 确认互补、120° 相位差

- [ ] **4.3 闭环测试**
  - 从开环切换闭环
  - 设定目标电流 1A
  - 观察电流响应

- [ ] **4.4 电机空载测试**
  - 缓慢增加转速 (100 → 500 → 1000 RPM)
  - 确认电机平稳运行
  - 记录电流和速度波形

**验收标准**:
- ✅ 三相 PWM 波形正确
- ✅ 开环电压输出正确
- ✅ 闭环电流控制稳定
- ✅ 电机能平稳运行

---

### 🟡 阶段2: 核心控制功能 (第5-8天)

#### 第5天: 电机启停控制 + 保护功能

**目标**: 实现平滑启停和保护功能

**任务清单**:
- [ ] **5.1 平滑启停 (斜坡函数)**
  ```c
  typedef enum {
      MOTOR_STATE_STOP = 0,
      MOTOR_STATE_STARTING,
      MOTOR_STATE_RUNNING,
      MOTOR_STATE_STOPPING
  } MotorState_t;

  #define RAMP_STEP    0.5f   // 每 1ms 步进 (1s 从 0→500 RPM)

  void Motor_RampUpdate(void)
  {
      switch (motor_state) {
          case MOTOR_STATE_STARTING:
              if (ramp_current < foc.target_speed) {
                  ramp_current += RAMP_STEP;
                  foc.target_speed = MIN(ramp_current, foc.target_speed);
              }
              break;
          case MOTOR_STATE_STOPPING:
              if (ramp_current > 0) {
                  ramp_current -= RAMP_STEP;
                  foc.target_speed = MAX(ramp_current, 0);
              }
              break;
      }
  }
  ```

- [ ] **5.2 过流保护 (OCP)**
  ```c
  #define OCP_THRESHOLD     10.0f    // 10A
  #define OCP_DELAY_MS      10       // 持续 10ms

  static uint16_t ocp_timer = 0;

  void Protection_CheckOverCurrent(void)
  {
      float I_max = MAX(MAX(fabsf(I_u), fabsf(I_v)), fabsf(I_w));
      if (I_max > OCP_THRESHOLD) {
          ocp_timer++;
          if (ocp_timer > OCP_DELAY_MS) {
              MiniFOC_MotorEnable(false);
              CAN_SendAlarm(ALARM_OVERCURRENT);
          }
      } else {
          ocp_timer = 0;
      }
  }
  ```

- [ ] **5.3 母线电压保护**
  ```c
  void Protection_CheckVbus(void)
  {
      if (vbus_voltage > 45.0f || vbus_voltage < 18.0f) {
          MiniFOC_MotorEnable(false);
          CAN_SendAlarm(ALARM_VOLTAGE);
      }
  }
  ```

---

#### 第6天: CAN 速度指令解析

**目标**: 实现 CAN 远程调速和参数设置

**任务清单**:
- [ ] **6.1 扩展 CAN 命令**
  ```c
  void CAN_RxCallback(uint32_t id, uint8_t *data, uint8_t len)
  {
      switch (data[0]) {
          case 0x01:  // 启动
              MiniFOC_MotorEnable(true);
              break;
          case 0x02:  // 停止
              MiniFOC_MotorEnable(false);
              break;
          case 0x03:  // 设定转速
              memcpy(&speed_target, data+1, 4);
              MiniFOC_SetTargetSpeed(speed_target);
              break;
          case 0x05:  // 设置速度环 KP
              memcpy(&kp, data+1, 4);
              PID_SetKp(&foc.speed_pid, kp);
              break;
          // ... 其他命令
      }
  }
  ```

- [ ] **6.2 状态回传**
  ```c
  void CAN_SendTelemetry(void)
  {
      uint8_t data[8];
      int16_t speed = (int16_t)(foc.rotor_speed * 10.0f);
      int16_t current = (int16_t)(foc.Iq * 100.0f);
      uint16_t vbus = (uint16_t)(vbus_voltage * 10.0f);

      data[0] = speed >> 8;
      data[1] = speed & 0xFF;
      data[2] = current >> 8;
      data[3] = current & 0xFF;
      data[4] = vbus >> 8;
      data[5] = vbus & 0xFF;
      data[6] = (uint8_t)motor_state;
      data[7] = 0;

      CAN_Send(0x125, 0xF0, data, 8);
  }
  ```

---

#### 第7天: 参数管理 + Flash 存储

**目标**: 参数保存到 Flash，断电不丢失

**任务清单**:
- [ ] **7.1 定义参数结构体**
  ```c
  typedef struct {
      float speed_kp, speed_ki;
      float current_kp, current_ki;
      uint16_t pole_pairs;
      uint16_t vbus_max, vbus_min;
      float ocp_threshold;
      uint32_t crc;
  } MotorParams_t;

  MotorParams_t motor_params;
  ```

- [ ] **7.2 Flash 存储实现**
  ```c
  #define FLASH_PARAM_ADDR  0x0807F000

  uint8_t MotorParams_SaveToFlash(void)
  {
      motor_params.crc = CalcCRC();
      HAL_FLASH_Unlock();
      // 擦除 + 写入
      HAL_FLASH_Lock();
      return 0;
  }
  ```

---

#### 第8天: 串口通信协议

**目标**: 实现串口参数配置

**任务清单**:
- [ ] **8.1 串口参数配置**
  ```c
  // 支持动态修改波特率、数据位、停止位、校验位
  uint8_t Uart_SetConfig(uint32_t baudrate, uint8_t data_bits, ...);
  ```

- [ ] **8.2 串口命令解析**
  ```c
  // 复用 CAN 命令解析
  void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
  {
      if (huart->Instance == USART3) {
          CAN_RxCallback(0x126, rx_buf, 8);
      }
  }
  ```

---

### 🟢 阶段3: 完善功能 + 测试 (第9-15天)

#### 第9-10天: CAN 命令扩展 + 状态回传

#### 第11天: 故障保护完善

#### 第12天: 功能测试清单

#### 第13天: 性能优化

#### 第14-15天: 文档编写 + 最终验证

---

## 6. 进度跟踪表

### 总体进度

| 阶段 | 任务 | 工期 | 状态 | 完成时间 |
|------|------|------|------|---------|
| **阶段1** | MiniFOC 基础功能 | 4天 | ⏳ 待开始 | - |
| **阶段2** | 核心控制功能 | 4天 | ⏳ 待开始 | - |
| **阶段3** | 完善测试 | 7天 | ⏳ 待开始 | - |
| **总计** | - | **15天** | - | - |

### 每日进度

| 日期 | 计划任务 | 实际完成 | 备注 |
|------|---------|---------|------|
| 第1天 | MiniFOC 配置优化 | ⏳ 未开始 | - |
| 第2天 | 电流环 PID | ⏳ 未开始 | - |
| 第3天 | 速度环 PID | ⏳ 未开始 | - |
| 第4天 | 开环+闭环测试 | ⏳ 未开始 | - |
| 第5天 | 启停控制+保护 | ⏳ 未开始 | - |
| 第6天 | CAN 速度指令 | ⏳ 未开始 | - |
| 第7天 | Flash 存储 | ⏳ 未开始 | - |
| 第8天 | 串口通信 | ⏳ 未开始 | - |
| 第9-10天 | CAN 扩展 | ⏳ 未开始 | - |
| 第11天 | 保护完善 | ⏳ 未开始 | - |
| 第12天 | 功能测试 | ⏳ 未开始 | - |
| 第13天 | 性能优化 | ⏳ 未开始 | - |
| 第14-15天 | 文档验证 | ⏳ 未开始 | - |

---

## 7. 里程碑节点

```
✅ 第1天末 (Day 1 End)
   📍 里程碑0: MiniFOC 配置完成
   - ✅ MiniFOC_Config.h 配置正确
   - ✅ 硬件接口适配完成
   - ✅ TIM1 中断配置完成

✅ 第4天末 (Day 4 End)
   📍 里程碑1: 电机闭环控制
   - ✅ 电流环 PID 整定完成
   - ✅ 速度环 PID 整定完成
   - ✅ 电机能平稳运行

✅ 第8天末 (Day 8 End)
   📍 里程碑2: 通信功能完成
   - ✅ CAN 速度指令响应正常
   - ✅ Flash 参数保存/加载
   - ✅ 串口参数配置

✅ 第11天末 (Day 11 End)
   📍 里程碑3: 保护功能完善
   - ✅ 过流/过压/欠压保护
   - ✅ CAN 遥测回传
   - ✅ 异常报警

✅ 第15天末 (Day 15 End)
   📍 里程碑4: 产品级固件
   - ✅ 所有功能测试通过
   - ✅ 性能指标达标
   - ✅ 文档完整
   - 🎉 可交付使用
```

---

## 8. 风险与应对

| 风险 | 概率 | 影响 | 风险等级 | 应对方案 |
|------|------|------|---------|---------|
| **电流采样误差大** | 高 | 中 | 🟡 中 | 充分校准零点;调整采样电路 |
| **PID 参数难整定** | 中 | 中 | 🟡 中 | 参考FOC教程;从小参数开始 |
| **电机不转或抖动** | 中 | 高 | 🔴 高 | 检查相序;死区;降低增益 |
| **过流保护误触发** | 中 | 中 | 🟡 中 | 增大延迟;降低阈值 |
| **Flash 写入失败** | 低 | 高 | 🟡 中 | CRC校验;失败回滚 |
| **CAN 通信不稳定** | 低 | 中 | 🟢 低 | 增加重试;检查终端电阻 |

---

## 9. 参考资源

### 9.1 项目文档
- ✅ `完整调试日志.md` - 历史调试经验
- ✅ `MiniFOC_快速集成指南.md` - MiniFOC 集成参考
- ✅ `MiniFOC_vs_SguanFOC_对比分析.md` - FOC库选型评估

### 9.2 E 盘资料 (备用)
- ⚠️ `SguanFOC_Library-main/` - 后续升级可用
- ⚠️ `配套QT上位机` - 可选
- ⚠️ `配套Simulink模型` - 参考用

---

## 10. 附录

### 10.1 MiniFOC 控制模式

| 模式 | 值 | 说明 |
|------|-----|------|
| MODE_VF_OPENLOOP | 0 | VF压频比开环 |
| MODE_IF_OPENLOOP | 1 | IF流频比开环 |
| MODE_Current_SINGLE | 3 | 电流单闭环 |
| MODE_VelCur_DOUBLE | 4 | 速度-电流串级闭环 |
| MODE_Sensor_Hall | 6 | 有感霍尔 |
| MODE_Sensorless_SMO | 8 | 无感SMO (MiniFOC未实现) |

### 10.2 关键参数默认值

| 参数 | 默认值 | 说明 |
|------|--------|------|
| 速度环 KP | 0.5 | 中等响应速度 |
| 速度环 KI | 0.1 | 消除稳态误差 |
| 电流环 KP | 0.1 | 保守值,防振荡 |
| 电流环 KI | 0.01 | 很小,几乎无积分 |
| 极对数 | 7 | 需根据电机修改 |
| 过流阈值 | 10A | 根据电机调整 |

---

**文档版本**: v2.0
**最后更新**: 2026-07-02 20:30
**状态**: ✅ 方案已确认，准备开始第1天任务
