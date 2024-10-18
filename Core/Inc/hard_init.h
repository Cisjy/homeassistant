#ifndef __HARD_INIT_H__
#define __HARD_INIT_H__
#include "OLED.h"
#include "OLED_Display.h"
#include "BH1750.h"
#include "esp8266.h"
#include "main.h"
extern oledHandleTypeDef oled_handle;
extern mpu6050HandleTypeDef mpu6050_handle;
extern BH1750HandleTypeDef bh1750_handle;

void hard_init(void);

#endif
