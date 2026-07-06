/**
  ******************************************************************************
  * @file           : MiniFOC_Debug.h
  * @brief          : MiniFOC 调试辅助宏
  ******************************************************************************
  */
#ifndef __MINIFOC_DEBUG_H__
#define __MINIFOC_DEBUG_H__

#include <stdio.h>

/* 调试输出开关 */
#define DEBUG_ENABLE      1

/* 调试宏 */
#if DEBUG_ENABLE
    #define DEBUG_PRINT(fmt, ...)  printf(fmt, ##__VA_ARGS__)
#else
    #define DEBUG_PRINT(fmt, ...)  do {} while (0)
#endif

#endif /* __MINIFOC_DEBUG_H__ */
