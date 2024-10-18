#ifndef __BH1750_H__
#define __BH1750_H__
#include "i2c_c.h"
// BH1750的地址
#define BH1750_Addr 0x23 // BH1750原始地址 7位
// BH1750指令码
#define POWER_OFF 0x00
#define POWER_ON 0x01
#define MODULE_RESET 0x07
#define CONTINUE_H_MODE 0x10
#define CONTINUE_H_MODE2 0x11
#define CONTINUE_L_MODE 0x13
#define ONE_TIME_H_MODE 0x20
#define ONE_TIME_H_MODE2 0x21
#define ONE_TIME_L_MODE 0x23

// 测量模式
#define Measure_Mode CONTINUE_H_MODE

// 分辨率	光照强度（单位lx）=（High Byte  + Low Byte）/ 1.2 * 测量精度
#if ((Measure_Mode == CONTINUE_H_MODE2) | (Measure_Mode == ONE_TIME_H_MODE2))
#define Resolurtion 0.5
#elif ((Measure_Mode == CONTINUE_H_MODE) | (Measure_Mode == ONE_TIME_H_MODE))
#define Resolurtion 1
#elif ((Measure_Mode == CONTINUE_L_MODE) | (Measure_Mode == ONE_TIME_L_MODE))
#define Resolurtion 4
#endif
typedef struct _BH1750HandleTypeDef BH1750HandleTypeDef;
struct _BH1750HandleTypeDef
{
    i2c_driver_interface_t *bh1750_i2c_driver_interface;
    void (*BH1750_Power_ON)(BH1750HandleTypeDef *bh1750_handle);
    void (*BH1750_Power_OFF)(BH1750HandleTypeDef *bh1750_handle);
    void (*BH1750_Reset)(BH1750HandleTypeDef *bh1750_handle);
    uint8_t (*BH1750_Byte_Write)(BH1750HandleTypeDef *bh1750_handle, uint8_t data);
    uint8_t (*BH1750_Read_Measure)(BH1750HandleTypeDef *bh1750_handle, uint16_t *light_intensity);
    void (*Light_Intensity)(BH1750HandleTypeDef *bh1750_handle, float *LightIntensity);
};

uint8_t BH1750_Init(BH1750HandleTypeDef *bh1750_handle, i2c_driver_interface_t *i2c_driver_interface);
#endif

/**
 *
 *
BH1750HandleTypeDef bh1750_handle;
i2c_driver_interface_t bh1750_i2c_driver = {
    .i2c_gpio_pin = &(i2c_gpio_pin_t){
        .SCL_Port = GPIOB,
        .SCL_Pin = GPIO_PIN_14,
        .SDA_Port = GPIOB,
        .SDA_Pin = GPIO_PIN_15,
    },
    .Init = I2C_Init,
    .Start = I2C_Start,
    .Stop = I2C_Stop,
    .WaitAck = I2C_WaitAck,
    .SendByte = I2C_SendByte,
    .ReadByte = I2C_ReadByte,
};
BH1750_Init(&bh1750_handle, &bh1750_i2c_driver);
 */
