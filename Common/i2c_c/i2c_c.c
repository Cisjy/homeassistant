/*
 * @Descripttion:
 * @Author: Cisjy
 * @Date: 2024-07-11 19:18:19
 * @LastEditors: Cisjy
 * @LastEditTime: 2024-10-13 11:30:48
 */

#include "i2c_c.h"

/**
 * @brief: enable the clock of the specified GPIO port
 * @param {GPIO_TypeDef} *GPIOx
 * @return {*}
 */
void Enable_GPIO_Clock(GPIO_TypeDef *GPIOx)
{
    if (GPIOx == GPIOA)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    else if (GPIOx == GPIOB)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    else if (GPIOx == GPIOC)
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    else if (GPIOx == GPIOD)
    {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
    else if (GPIOx == GPIOE)
    {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    }
}
// #define I2C_W_SCL(x) HAL_GPIO_WritePin(I2C_GPIO_Port, I2C_SCL_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)
// #define I2C_W_SDA(x) HAL_GPIO_WritePin(I2C_GPIO_Port, I2C_SDA_Pin, (x) ? GPIO_PIN_SET : GPIO_PIN_RESET)

void I2C_W_SCL(i2c_gpio_pin_t *gpio_pin, uint8_t state)
{

    state ? HAL_GPIO_WritePin(gpio_pin->SCL_Port, gpio_pin->SCL_Pin, GPIO_PIN_SET)
          : HAL_GPIO_WritePin(gpio_pin->SCL_Port, gpio_pin->SCL_Pin, GPIO_PIN_RESET);
}
void I2C_W_SDA(i2c_gpio_pin_t *gpio_pin, uint8_t state)
{
    state ? HAL_GPIO_WritePin(gpio_pin->SDA_Port, gpio_pin->SDA_Pin, GPIO_PIN_SET)
          : HAL_GPIO_WritePin(gpio_pin->SDA_Port, gpio_pin->SDA_Pin, GPIO_PIN_RESET);
}
void I2C_Init(i2c_gpio_pin_t *gpio_pin)
{
    Enable_GPIO_Clock(gpio_pin->SCL_Port);
    Enable_GPIO_Clock(gpio_pin->SDA_Port);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin = gpio_pin->SCL_Pin | gpio_pin->SDA_Pin;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
    HAL_GPIO_Init(gpio_pin->SCL_Port, &GPIO_InitStructure);
    HAL_GPIO_Init(gpio_pin->SDA_Port, &GPIO_InitStructure);

    delay_init(SYS_CLK);

    I2C_W_SCL(gpio_pin, 1);
    I2C_W_SDA(gpio_pin, 1);
}

void I2C_Start(i2c_gpio_pin_t *gpio_pin)
{

    I2C_W_SDA(gpio_pin, 1);
    I2C_W_SCL(gpio_pin, 1);
    delay_us(2);
    I2C_W_SDA(gpio_pin, 0);
    delay_us(2);
    I2C_W_SCL(gpio_pin, 0);
}

void I2C_Stop(i2c_gpio_pin_t *gpio_pin)
{
    I2C_W_SDA(gpio_pin, 0);
    delay_us(2);
    I2C_W_SCL(gpio_pin, 1);
    delay_us(2);
    I2C_W_SDA(gpio_pin, 1);
}

void I2C_SendByte(i2c_gpio_pin_t *gpio_pin, uint8_t data)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        I2C_W_SDA(gpio_pin, (data & 0x80) >> 7);
        delay_us(2);
        I2C_W_SCL(gpio_pin, 1);
        delay_us(2);
        I2C_W_SCL(gpio_pin, 0);
        data <<= 1;
    }
    I2C_W_SDA(gpio_pin, 1);
}

uint8_t I2C_ReadByte(i2c_gpio_pin_t *gpio_pin, uint8_t ack)
{
    uint8_t i, data = 0;
    for (i = 0; i < 8; i++)
    {
        data <<= 1;
        I2C_W_SCL(gpio_pin, 1);
        delay_us(2);
        if (GPIO_PIN_SET == HAL_GPIO_ReadPin(gpio_pin->SDA_Port, gpio_pin->SDA_Pin))
        {
            data++;
        }
        I2C_W_SCL(gpio_pin, 0);
        delay_us(1);
    }
    if (I2C_ACK == ack)
    {
        I2C_Ack(gpio_pin);
    }
    else
    {
        I2C_Nack(gpio_pin);
    }
    return data;
}

/**
 * @brief :
 * @param {i2c_gpio_pin_t} *gpio_pin
 * @return uint8_t  1:nack 0:ack
 */
uint8_t I2C_WaitAck(i2c_gpio_pin_t *gpio_pin)
{
    uint8_t ack = 0;
    I2C_W_SDA(gpio_pin, 1);
    delay_us(2);
    I2C_W_SCL(gpio_pin, 1);
    delay_us(2);
    if (GPIO_PIN_SET == HAL_GPIO_ReadPin(gpio_pin->SDA_Port, gpio_pin->SDA_Pin))
    {
        ack = 1;
    }
    I2C_W_SCL(gpio_pin, 0);
    delay_us(2);
    return ack;
}

/**
 * @brief : i2c master send ack
 * @param {i2c_gpio_pin_t} *gpio_pin
 * @return {*}
 */
void I2C_Ack(i2c_gpio_pin_t *gpio_pin)
{
    I2C_W_SDA(gpio_pin, 0);
    I2C_W_SCL(gpio_pin, 1);
    delay_us(2);
    I2C_W_SCL(gpio_pin, 0);
    I2C_W_SDA(gpio_pin, 1);
    delay_us(2);
}

/**
 * @brief : i2c master send nack
 * @param {i2c_gpio_pin_t} *gpio_pin
 * @return {*}
 */
void I2C_Nack(i2c_gpio_pin_t *gpio_pin)
{
    I2C_W_SDA(gpio_pin, 1);
    I2C_W_SCL(gpio_pin, 1);
    delay_us(2);
    I2C_W_SCL(gpio_pin, 0);
    delay_us(2);
}
