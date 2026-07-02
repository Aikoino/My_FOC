#!/usr/bin/env python3
import re

# 读取当前 main.c
with open('d:/My_FOC/Core/Src/main.c', 'r', encoding='utf-8') as f:
    content = f.read()

# 在 MX_DMA_Init 函数结束后添加其他初始化函数
insert_after = content.find('}\n\n/* USER CODE BEGIN 4 */')

if insert_after == -1:
    print("ERROR: Cannot find insertion point")
    exit(1)

# 读取 cb_test 的其他函数定义
with open('E:/cb_test/cb_test/Core/Src/main.c', 'r', encoding='latin-1') as f:
    cb_content = f.read()

# 提取 MX_TIM1_Init 到 MX_OPAMP3_Init 的函数定义
tim1_start = cb_content.find('static void MX_TIM1_Init(void)')
opamp3_end = cb_content.find('}', cb_content.find('static void MX_OPAMP3_Init(void)')) + 1

if tim1_start == -1 or opamp3_end == -1:
    print("ERROR: Cannot find functions in cb_test")
    exit(1)

additional_functions = cb_content[tim1_start:opamp3_end]

# 插入函数定义
new_content = content[:insert_after] + additional_functions + '\n\n' + content[insert_after:]

# 保存
with open('d:/My_FOC/Core/Src/main.c', 'w', encoding='utf-8') as f:
    f.write(new_content)

print(f"Added {len(additional_functions)} chars of initialization functions")
