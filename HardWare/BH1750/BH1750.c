#include "BH1750.h"

static uint8_t BH1750_Byte_Write(BH1750HandleTypeDef *bh1750_handle, uint8_t data)
{
    bh1750_handle->bh1750_i2c_driver_interface->Start(bh1750_handle->bh1750_i2c_driver_interface->i2c_gpio_pin);
    bh1750_handle->bh1750_i2c_driver_interface->SendByte(bh1750_handle->bh1750_i2c_driver_interface->i2c_gpio_pin, BH1750_Addr << 1 | I2C_WRITE);
    if (I2C_NACK == bh1750_handle->bh1750_i2c_driver_interface->WaitAck(bh1750_handle->bh1750_i2c_driver_interface->i2c_gpio_pin))
    {
        bh1750_handle->bh1750_i2c_driver_interface->Stop(bh1750_handle->bh1750_i2c_driver_interface->i2c_gpio_pin);
        return 1;
    }
    bh1750_handle->bh1750_i2c_driver_interface->SendByte(bh1750_handle->bh1750_i2c_driver_interface->i2c_gpio_pin, data);
    if (I2C_NACK == bh1750_handle->bh1750_i2c_driver_interface->WaitAck(bh1750_handle->bh1750_i2c_driver_interface->i2c_gpio_pin))
    {
        bh1750_handle->bh1750_i2c_driver_interface->Stop(bh1750_handle->bh1750_i2c_driver_interface->i2c_gpio_pin);
        return 2;
    }
    bh1750_handle->bh1750_i2c_driver_interface->Stop(bh1750_handle->bh1750_i2c_driver_interface->i2c_gpio_pin);
    return 0;
}

/**
 * @brief BH1750的光照强度
 * @param {BH1750HandleTypeDef} *bh1750_handle
 * @param {uint16_t} *light_intensity
 * @return 0 成功，1 失败
 */
uint8_t BH1750_Read_Measure(BH1750HandleTypeDef *bh1750_handle, uint16_t *light_intensity)
{
    uint16_t receive_data = 0;
    bh1750_handle->bh1750_i2c_driver_interface->Start(bh1750_handle->bh1750_i2c_driver_interface->i2c_gpio_pin);
    // 发送读地址
    bh1750_handle->bh1750_i2c_driver_interface->SendByte(bh1750_handle->bh1750_i2c_driver_interface->i2c_gpio_pin, BH1750_Addr << 1 | I2C_READ);
    if (I2C_NACK == bh1750_handle->bh1750_i2c_driver_interface->WaitAck(bh1750_handle->bh1750_i2c_driver_interface->i2c_gpio_pin))
    {
        bh1750_handle->bh1750_i2c_driver_interface->Stop(bh1750_handle->bh1750_i2c_driver_interface->i2c_gpio_pin);
        return 1;
    }
    // 读取高八位
    receive_data = bh1750_handle->bh1750_i2c_driver_interface->ReadByte(bh1750_handle->bh1750_i2c_driver_interface->i2c_gpio_pin, I2C_ACK);
    // 读取低八位
    receive_data = (receive_data << 8) + bh1750_handle->bh1750_i2c_driver_interface->ReadByte(bh1750_handle->bh1750_i2c_driver_interface->i2c_gpio_pin, I2C_NACK);
    bh1750_handle->bh1750_i2c_driver_interface->Stop(bh1750_handle->bh1750_i2c_driver_interface->i2c_gpio_pin);
    *light_intensity = receive_data;
    return 0;
}

// 获取光照强度
void Light_Intensity(BH1750HandleTypeDef *bh1750_handle, float *LightIntensity)
{
    uint16_t light_intensity = 0;
    bh1750_handle->BH1750_Read_Measure(bh1750_handle, &light_intensity);
    *LightIntensity = (float)(light_intensity / 1.1f * Resolurtion);
}

// BH1750s上电
void BH1750_Power_ON(BH1750HandleTypeDef *bh1750_handle)
{
    bh1750_handle->BH1750_Byte_Write(bh1750_handle, POWER_ON);
}

// BH1750s断电
void BH1750_Power_OFF(BH1750HandleTypeDef *bh1750_handle)
{
    bh1750_handle->BH1750_Byte_Write(bh1750_handle, POWER_OFF);
}

// BH1750复位   仅在上电时有效
void BH1750_Reset(BH1750HandleTypeDef *bh1750_handle)
{
    bh1750_handle->BH1750_Byte_Write(bh1750_handle, RESET);
}

uint8_t BH1750_Init(BH1750HandleTypeDef *bh1750_handle, i2c_driver_interface_t *i2c_driver_interface)
{
    bh1750_handle->bh1750_i2c_driver_interface = i2c_driver_interface;
    bh1750_handle->BH1750_Byte_Write = BH1750_Byte_Write;
    bh1750_handle->BH1750_Power_ON = BH1750_Power_ON;
    bh1750_handle->BH1750_Power_OFF = BH1750_Power_OFF;
    bh1750_handle->BH1750_Reset = BH1750_Reset;
    bh1750_handle->BH1750_Read_Measure = BH1750_Read_Measure;
    bh1750_handle->Light_Intensity = Light_Intensity;
    // 配置GPIO
    bh1750_handle->bh1750_i2c_driver_interface->Init(bh1750_handle->bh1750_i2c_driver_interface->i2c_gpio_pin);

    bh1750_handle->BH1750_Power_ON(bh1750_handle); // BH1750s上电
    bh1750_handle->BH1750_Byte_Write(bh1750_handle, Measure_Mode);
}
