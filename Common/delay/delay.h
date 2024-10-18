/*
 * @Descripttion:
 * @Author: Cisjy
 * @Date: 2024-08-15 13:01:43
 * @LastEditors: Cisjy
 * @LastEditTime: 2024-08-21 16:13:50
 */
#ifndef __DELAY_H__
#define __DELAY_H__

#include "stm32f1xx_hal.h"

#define SYS_SUPPORT_OS 1 /* 支持OS */

void delay_init(uint16_t sysclk); /* 初始化延迟函数 */
void delay_ms(uint16_t nms);      /* 延时nms */
void delay_us(uint32_t nus);      /* 延时nus */

#if (!SYS_SUPPORT_OS)           /* 没有使用Systick中断 */
void HAL_Delay(uint32_t Delay); /* HAL库的延时函数，SDIO等需要用到 */
#endif

#endif
