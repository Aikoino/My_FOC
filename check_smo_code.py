#!/usr/bin/env python3
"""
SMO代码语法和逻辑检查脚本
不依赖ARM编译器，只检查代码结构和潜在问题
"""

import os
import sys
import re

sys.stdout.reconfigure(encoding='utf-8')

def check_file_exists(filepath):
    """检查文件是否存在"""
    if not os.path.exists(filepath):
        print(f"❌ 文件不存在: {filepath}")
        return False
    print(f"✅ 文件存在: {filepath}")
    return True

def check_function_exists(filepath, func_name):
    """检查函数是否定义"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
            if f'void {func_name}' in content or f'float {func_name}' in content:
                print(f"✅ 函数存在: {func_name}()")
                return True
            else:
                print(f"❌ 函数不存在: {func_name}()")
                return False
    except Exception as e:
        print(f"❌ 读取文件失败: {e}")
        return False

def check_macro_defined(filepath, macro_name):
    """检查宏是否定义"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
            pattern = rf'#define\s+{macro_name}\s+'
            if re.search(pattern, content):
                print(f"✅ 宏已定义: {macro_name}")
                return True
            else:
                print(f"❌ 宏未定义: {macro_name}")
                return False
    except Exception as e:
        print(f"❌ 读取文件失败: {e}")
        return False

def check_enum_value(filepath, enum_name):
    """检查枚举值是否定义"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
            if enum_name in content:
                print(f"✅ 枚举值存在: {enum_name}")
                return True
            else:
                print(f"❌ 枚举值不存在: {enum_name}")
                return False
    except Exception as e:
        print(f"❌ 读取文件失败: {e}")
        return False

def check_include(filepath, include_name):
    """检查是否包含某个头文件"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
            if f'#include "{include_name}"' in content or f'#include <{include_name}>' in content:
                print(f"✅ 已包含头文件: {include_name}")
                return True
            else:
                print(f"⚠️  未找到包含: {include_name}")
                return False
    except Exception as e:
        print(f"❌ 读取文件失败: {e}")
        return False

print("=" * 60)
print("SMO代码检查报告")
print("=" * 60)

# 1. 检查文件存在性
print("\n📁 文件存在性检查")
print("-" * 60)
files = [
    'user/MiniFOC/MiniFOC_SMO.c',
    'user/MiniFOC/MiniFOC_SMO.h',
    'user/MiniFOC/MiniFOC.c',
    'user/MiniFOC/MiniFOC.h',
    'user/MiniFOC/MiniFOC_Config.h'
]
for f in files:
    check_file_exists(f)

# 2. 检查SMO核心函数
print("\n🔧 SMO核心函数检查")
print("-" * 60)
smo_functions = [
    'MiniFOC_SMO_Init',
    'MiniFOC_SMO_Loop',
    'MiniFOC_PLL_Init',
    'MiniFOC_PLL_Loop',
    'MiniFOC_LPF_Init',
    'MiniFOC_LPF_Update',
    'MiniFOC_Sensorless_Init',
    'MiniFOC_Sensorless_Loop',
    'MiniFOC_Sensorless_GetAngle',
    'MiniFOC_Sensorless_GetSpeed',
    'MiniFOC_Sensorless_GetWe',
    'MiniFOC_Value_Gain_Get',
    'MiniFOC_Value_Rad_Loop'
]
for func in smo_functions:
    check_function_exists('user/MiniFOC/MiniFOC_SMO.c', func)

# 3. 检查模式定义
print("\n🎛️  控制模式检查")
print("-" * 60)
check_enum_value('user/MiniFOC/MiniFOC.h', 'MODE_Sensorless_I')
check_enum_value('user/MiniFOC/MiniFOC.h', 'MODE_Sensorless_S')

# 4. 检查SMO配置宏
print("\n⚙️  SMO配置宏检查")
print("-" * 60)
smo_macros = [
    'SMO_GAIN',
    'SMO_LPF_CUTOFF',
    'SMO_PLL_KP',
    'SMO_PLL_KI',
    'SMO_IF_IQ_SETPOINT',
    'SMO_SWITCH_SPEED_MIN',
    'SMO_SWITCH_SPEED_MAX',
    'SMO_SWITCH_HYSTERESIS'
]
for macro in smo_macros:
    check_macro_defined('user/MiniFOC/MiniFOC_Config.h', macro)

# 5. 检查结构体字段
print("\n📊 结构体字段检查")
print("-" * 60)
struct_fields = [
    'if_elec_angle',
    'if_we',
    'smo_gain',
    'switch_speed_min',
    'switch_speed_max',
    'switch_speed_hyst'
]
for field in struct_fields:
    try:
        with open('user/MiniFOC/MiniFOC_SMO.h', 'r', encoding='utf-8') as f:
            content = f.read()
            if field in content:
                print(f"✅ 字段存在: {field}")
            else:
                print(f"❌ 字段不存在: {field}")
    except:
        print(f"❌ 检查失败: {field}")

# 6. 检查MiniFOC.c集成
print("\n🔗 MiniFOC.c集成检查")
print("-" * 60)
check_include('user/MiniFOC/MiniFOC.c', 'MiniFOC_SMO.h')

# 7. 检查常见问题
print("\n🐛 常见问题检查")
print("-" * 60)

with open('user/MiniFOC/MiniFOC_SMO.c', 'r', encoding='utf-8') as f:
    smo_content = f.read()

# 检查是否还有×0.1的错误
if 'if_elec_angle +=' in smo_content and '* 0.1f' in smo_content:
    print("❌ 发现IF角度×0.1的错误")
else:
    print("✅ IF角度计算正确（无×0.1错误）")

# 检查GetSpeed是否使用正确的字段
if 'high_speed_we' in smo_content:
    print("⚠️  仍有high_speed_we引用（应改为pll.go.OutWe）")
else:
    print("✅ GetSpeed()已修复（无high_speed_we引用）")

# 检查是否有完整的main函数调用（main.c）
print("\n📝 main.c集成检查")
print("-" * 60)
if os.path.exists('Core/Src/main.c'):
    with open('Core/Src/main.c', 'r', encoding='utf-8') as f:
        main_content = f.read()
        if 'MiniFOC_Sensorless_Init' in main_content:
            print("✅ main.c中调用了MiniFOC_Sensorless_Init()")
        else:
            print("⚠️  main.c中未调用MiniFOC_Sensorless_Init()（集成阶段需要添加）")

print("\n" + "=" * 60)
print("检查完成！")
print("=" * 60)
