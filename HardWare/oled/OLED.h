#ifndef __OLED_H__
#define __OLED_H__
#include "i2c_c.h"

#define OLED_SLAVE_ADDR 0x3C // OLED原始地址

typedef struct oledHandleTypeDef oledHandleTypeDef;
struct oledHandleTypeDef
{
  i2c_driver_interface_t *i2c_driver_interface;
  void (*Clear)(oledHandleTypeDef *oled_handle);
  /**
   * @brief oled清除指定行
   * @param {oledHandleTypeDef} *oled_handle
   * @param {uint8_t} line  1~4
   * @return {*}
   */
  void (*ClearLine)(oledHandleTypeDef *oled_handle, uint8_t line);
  /**
   * @brief oled清除指定位置
   * @param {oledHandleTypeDef} *oled_handle
   * @param {uint8_t} line  1~4
   * @param {uint8_t} column  1~16
   * @return {*}
   */
  void (*ClearPoint)(oledHandleTypeDef *oled_handle, uint8_t line, uint8_t column);
  void (*ShowChar)(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, char Char);
  void (*ShowString)(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, char *String);
  void (*ShowNum)(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
  void (*ShowSignedNum)(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
  void (*ShowHexNum)(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
  void (*ShowBinNum)(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
};

uint8_t OLED_Init(oledHandleTypeDef *oled_handle,
                  i2c_driver_interface_t *i2c_driver_interface);

/*how to use----------------------------------------*/
/*
oledHandleTypeDef oled_handle;
i2c_driver_interface_t i2c_driver = {
    .i2c_gpio_pin = &(i2c_gpio_pin_t){.SCL_Pin = GPIO_PIN_8,
                                      .SCL_Port = GPIOB,
                                      .SDA_Pin = GPIO_PIN_9,
                                      .SDA_Port = GPIOB},
    .Init = I2C_Init,
    .Start = I2C_Start,
    .Stop = I2C_Stop,
    .WaitAck = I2C_WaitAck,
    .SendByte = I2C_SendByte};

  OLED_Init(&oled_handle, &i2c_driver);
*/
#endif
