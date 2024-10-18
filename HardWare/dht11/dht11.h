#ifndef __DHT11_H__
#define __DHT11_H__
#include "stm32f1xx_hal.h"
#include "delay.h"

#define DHT11_DATA_PORT GPIOA
#define DHT11_DATA_PIN GPIO_PIN_1
#define DHT11_DATA_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

#define dht11_ReadData HAL_GPIO_ReadPin(DHT11_DATA_PORT, DHT11_DATA_PIN)
#define dht11_high HAL_GPIO_WritePin(DHT11_DATA_PORT, DHT11_DATA_PIN, GPIO_PIN_SET)
#define dht11_low HAL_GPIO_WritePin(DHT11_DATA_PORT, DHT11_DATA_PIN, GPIO_PIN_RESET)

uint8_t DHT11_Init(void);
uint8_t DHT11_Read_Data(uint8_t *data);
#endif
