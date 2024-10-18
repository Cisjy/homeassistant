#ifndef __OLED_DISPLAY_H__
#define __OLED_DISPLAY_H__
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "OLED.h"
#include "usart_print.h"
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "hard_init.h"
#include "stdio.h"
#include "usart.h"

#define OLED_DISPLAY_DEBUG 0

typedef struct oled_page_t oled_page_t;
typedef struct
{
    char *text;
    uint8_t hasSubMenu;
    void (*action)(uint8_t index);
    oled_page_t *subMenu;
} oled_option_t;

struct oled_page_t
{
    __IO uint8_t index;      // 当前选择的行 1-4
    __IO uint8_t indexFlag;  // 选择的行是否发生变化 一共8位 0位上:0没有变化 1变化  1位上:0表示按下了up 1表示按下了down   2位上:0没有按下确认 1按下确认 3位上:0没有按下取消 1按下取消
    uint8_t numOptions;      // 选项个数
    oled_option_t optins[5]; // 0保留  1-4显示选项
    oled_page_t *parent;
};

extern oled_page_t *oled_page;
extern __IO uint8_t oled_comfirm_flag;
extern bool is_bluetooth_debug;
void oled_display_init(void);
void oled_display(oled_page_t **currentPage);
#endif
