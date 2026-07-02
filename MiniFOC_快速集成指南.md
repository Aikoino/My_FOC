# MiniFOC 快速集成指南

## 📋 你的硬件配置

| 模块 | 配置 |
|------|------|
| **主控** | STM32G431CBT6 @ 170MHz |
| **栅极驱动** | FD6288 × 1（三路半桥） |
| **CAN收发** | L9616CAN |
| **电流采样** | ADC1 注入组 + OPAMP × 3 |
| **母线电压** | ADC1 规则组 |
| **PWM输出** | TIM1 CH1/1N, CH2/2N, CH3/3N |
| **人机界面** | LED2/3 + KEY |

---

## 🚀 5分钟快速开始

### **第1步：添加文件到工程**

将以下文件添加到你的工程：

```
user/MiniFOC/
├── MiniFOC.h
├── MiniFOC.c
├── MiniFOC_math.h
├── MiniFOC_PID.h
├── MiniFOC_PID.c
├── MiniFOC_Config.h
└── UserData_Config.h
```

---

### **第2步：修改 main.c**

已为你修改完成！

**关键改动**：
- ✅ 引入 MiniFOC 头文件
- ✅ 添加 `MiniFOC_Init()` 初始化
- ✅ 主循环调用 `MiniFOC_MainLoop()` 1kHz
- ✅ 按键改为控制电机启停
- ✅ CAN接收解析速度指令

---

### **第3步：TIM1 中断配置（重要！）**

**当前状态**：TIM1 配置了 PWM，但没有中断

**需要添加**：TIM1 更新中断（10kHz 用于电流环）

#### **方法A：在 CubeMX 中配置（推荐）**

1. 打开 `.ioc` 文件
2. 进入 **TIM1** 配置
3. **Parameter Settings**：
   - Prescaler: 169 (时钟170MHz/170 = 1MHz)
   - Period: 99 (1MHz/100 = 10kHz)
   - Counter Mode: Center-aligned mode 1（已配置）
4. **NVIC Settings**：
   - 勾选 `TIM1 update interrupt`
5. 重新生成代码

#### **方法B：手动修改代码**

在 `tim.c` 的 `MX_TIM1_Init()` 末尾添加：

```c
/* 使能TIM1更新中断 */
HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 1, 0);
HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
```

然后在 `stm32g4xx_it.c` 中添加中断处理函数：

```c
/**
  * @brief TIM1更新中断（10kHz）
  */
void TIM1_UP_TIM16_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_UP_TIM16_IRQn 0 */

  /* USER CODE END TIM1_UP_TIM16_IRQn 0 */
  HAL_TIM_IRQHandler(&htim1);
  /* USER CODE BEGIN TIM1_UP_TIM16_IRQn 1 */
  MiniFOC_HighFreqLoop();  /* ← 电流环10kHz */
  /* USER CODE END TIM1_UP_TIM16_IRQn 1 */
}
```

---

### **第4步：配置电机参数**

打开 `UserData_Config.h`，修改以下参数：

```c
/* 极对数 - 查看你的电机铭牌，常见的是 7 或 8 */
#define MOTOR_POLE_PAIRS     7

/* 额定电流 - 根据你的电机修改 */
#define MOTOR_RATED_CURRENT  5.0f  /* 单位：A */

/* 控制模式 - 建议先用电流单闭环测试 */
#define USER_CONTROL_MODE    MODE_Current_SINGLE

/* PID参数 - 后续根据调试结果调整 */
#define USER_CURRENT_KP      0.1f
#define USER_CURRENT_KI      0.01f
```

---

### **第5步：编译下载**

1. 编译工程
2. 下载到板子
3. 打开 VOFA+ 查看电流波形
4. 打开 PCAN-View 查看 CAN 数据

---

## 🎛️ 控制方式

### **按键控制**
- **按下 KEY** → LED2 亮 → 电机启动
- **松开 KEY** → LED2 灭 → 电机停止

### **CAN 遥控**（发送到 ID=0x126）
```
数据格式：4字节float（目标转速 rpm）
示例：0x00 0x00 0x70 0x42 = 50.0 rpm
```

### **后续可扩展**
- 串口指令控制
- 电位器调速
- 编码器速度闭环

---

## 🔧 调试流程

### **第1阶段：硬件验证（第1天）**
1. ✅ 编译下载
2. ✅ 确认 LED3 闪烁（程序运行）
3. ✅ 确认按键控制 LED2
4. ✅ 用万用表测量 PWM 输出（应该是互补波形）

### **第2阶段：开环测试（第2-3天）**
1. 修改 `MiniFOC.c` 中的 `MiniFOC_UpdateSensor()`
2. 实现开环 VF 控制（不依赖传感器）
3. 用 VOFA+ 观察电流波形
4. 调整 PWM 频率和死区时间

### **第3阶段：电流环（第4-5天）**
1. 确认电流采样正确
2. 整定电流环 PID 参数
3. 测试电流阶跃响应
4. 确认过流保护正常

### **第4阶段：速度环（第6-7天）**
1. 加编码器或霍尔传感器
2. 实现速度估算
3. 整定速度环 PID 参数
4. 测试转速响应

### **第5阶段：无感模式（可选）**
1. 集成 SMO 滑模观测器
2. 或 HFI 高频注入
3. 全速域无感运行

---

## 📊 关键参数

### **TIM1 配置**
```c
Prescaler: 169         // 时钟 170MHz/170 = 1MHz
Period: 7999           // PWM 频率 1MHz/(7999+1) = 100Hz (中心对齐)
                       // 实际频率 = 200Hz
```

**PWM 频率计算**：
- 中心对齐模式下，一个周期包含 2×Period 个时钟
- 实际 PWM 频率 = 1MHz / (2 × 7999) = 62.5Hz
- ⚠️ **这太低了！需要调整**

**建议修改为 20kHz**：
```c
Prescaler: 0           // 时钟 170MHz/1 = 170MHz
Period: 8499           // 170MHz/(8499+1) = 20kHz
```

### **ADC 采样**
- **注入组**：UVW 三相电流（10kHz 同步采样）
- **规则组**：母线电压（可更低频率）

### **控制频率**
- **电流环**：10kHz（TIM1 中断）
- **速度环**：1kHz（主循环）
- **主循环**：100Hz（10ms）

---

## ⚠️ 注意事项

### **1. 电流采样校准**
首次上电必须做电流偏置校准：
```c
// MiniFOC.c 中的 HAL_ADCEx_InjectedConvCpltCallback
// 上电10次采样计算偏置
```

### **2. 死区时间**
FD6288 的死区时间需要配置：
```c
sBreakDeadTimeConfig.DeadTime = 500;  // 500ns
```

### **3. 母线电压保护**
如果母线电压过高/过低，MiniFOC 会自动停止

### **4. 首次测试建议**
- 先断开电机，用示波器看 PWM
- 确认波形正确后再接电机
- 电流环测试时用小电流（0.5A 以内）

---

## 📚 文件说明

| 文件 | 功能 | 是否可修改 |
|------|------|-----------|
| `MiniFOC.h/c` | 核心框架 | ⚠️ 高级功能再改 |
| `MiniFOC_math.h` | 坐标变换 | ❌ 不需要改 |
| `MiniFOC_PID.h/c` | PID控制器 | ✅ 可调整参数 |
| `MiniFOC_Config.h` | 硬件配置 | ✅ 根据硬件修改 |
| `UserData_Config.h` | 电机参数 | ✅ 根据电机修改 |

---

## 🆘 常见问题

### **Q1: 编译错误：找不到 MiniFOC.h**
**A**: 检查 #include 路径，确保 `user/MiniFOC/` 目录在工程中

### **Q2: PWM 输出正常，但电机不转**
**A**: 检查：
1. 电流采样是否正确（看 VOFA+ 波形）
2. 电流环 PID 参数是否合适
3. 电机相序是否正确

### **Q3: 电机抖动但转不起来**
**A**:
1. 检查转子角度是否正确
2. 电流环增益太低
3. 死区时间过大

### **Q4: 过流保护频繁触发**
**A**:
1. 降低目标电流
2. 检查电流传感器增益
3. 增大电流环输出限幅

---

## 📞 获取帮助

- 查看 `MiniFOC_Config.h` 中的详细注释
- 用 VOFA+ 观察关键变量
- 用 CAN 发送调试命令

---

**祝你调试顺利！** 🚀
