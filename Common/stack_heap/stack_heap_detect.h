#ifndef __STACK_HEAP_DETECT_H__
#define __STACK_HEAP_DETECT_H__
#include "stm32f1xx_hal.h"
int stack_set_guard(void);
int stack_detect_guard(void);
// 如果是rtos heap不使用
int heap_set_guard(void);
int heap_detect_guard(void);
#endif
