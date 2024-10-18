/*
 * @Author: Cisjy
 * @Date: 2024-07-11 19:22:21
 * @Last Modified by: Cisjy
 * @Last Modified time: 2024-07-14 16:36:45
 */
#ifndef __I2C_C_H__
#define __I2C_C_H__
#include "delay.h"

#define SYS_CLK 72
#define I2C_WRITE 0
#define I2C_READ 1
#define I2C_ACK 0
#define I2C_NACK 1

typedef struct
{
    GPIO_TypeDef *SCL_Port;
    uint16_t SCL_Pin;
    GPIO_TypeDef *SDA_Port;
    uint16_t SDA_Pin;
} i2c_gpio_pin_t;

typedef struct
{
    i2c_gpio_pin_t *i2c_gpio_pin;
    void (*Init)(i2c_gpio_pin_t *i2c_gpio_pin);
    void (*DeInit)(i2c_gpio_pin_t *i2c_gpio_pin);
    void (*Start)(i2c_gpio_pin_t *i2c_gpio_pin);
    void (*Stop)(i2c_gpio_pin_t *i2c_gpio_pin);
    uint8_t (*WaitAck)(i2c_gpio_pin_t *i2c_gpio_pin);
    void (*SendByte)(i2c_gpio_pin_t *i2c_gpio_pins, uint8_t Data);
    uint8_t (*ReadByte)(i2c_gpio_pin_t *i2c_gpio_pin, uint8_t ack);
} i2c_driver_interface_t;

void I2C_Init(i2c_gpio_pin_t *gpio_pin);
void I2C_Start(i2c_gpio_pin_t *gpio_pin);
void I2C_Stop(i2c_gpio_pin_t *gpio_pin);
void I2C_SendByte(i2c_gpio_pin_t *gpio_pin, uint8_t data);
uint8_t I2C_ReadByte(i2c_gpio_pin_t *gpio_pin, uint8_t ack);
uint8_t I2C_WaitAck(i2c_gpio_pin_t *gpio_pin);
void I2C_Ack(i2c_gpio_pin_t *gpio_pin);
void I2C_Nack(i2c_gpio_pin_t *gpio_pin);

#endif /* __I2C_C_H */
