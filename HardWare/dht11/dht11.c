#include "dht11.h"

static void DHT11_GPIO_Init(void)
{
    DHT11_DATA_GPIO_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pin = DHT11_DATA_PIN;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_DATA_PORT, &GPIO_InitStruct);
}

/**
 * @brief DHT11_GPIO_Direction 1:input 0:output
 * @param {uint8_t} dir
 * @return {*}
 */
static void DHT11_GPIO_Direction(uint8_t dir)
{
    DHT11_DATA_GPIO_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_Initstrtct;
    if (dir == 1)
    {
        GPIO_Initstrtct.Mode = GPIO_MODE_INPUT;
        GPIO_Initstrtct.Pull = GPIO_NOPULL;
    }
    else
    {
        GPIO_Initstrtct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_Initstrtct.Pull = GPIO_PULLUP;
    }
    GPIO_Initstrtct.Pin = DHT11_DATA_PIN;
    GPIO_Initstrtct.Speed = GPIO_SPEED_MEDIUM;
    HAL_GPIO_Init(DHT11_DATA_PORT, &GPIO_Initstrtct);
}

static void DHT11_GPIO_OUT(void)
{
    DHT11_GPIO_Direction(0);
}

static void DHT11_GPIO_IN(void)
{
    DHT11_GPIO_Direction(1);
}

static void DHT11_Start(void)
{
    DHT11_GPIO_OUT();
    dht11_low;
    delay_ms(20);
    dht11_high;
    delay_us(30);
    DHT11_GPIO_IN();
}

static uint8_t DHT11_Check(void)
{
    uint8_t retry = 0;
    DHT11_GPIO_IN();
    while (dht11_ReadData == 1 && retry < 100)
    {
        retry++;
        delay_us(1);
    }
    if (retry >= 100)
        return 1;
    else
        retry = 0;
    while (dht11_ReadData == 0 && retry < 100)
    {
        retry++;
        delay_us(1);
    }
    if (retry >= 100)
        return 1;

    return 0;
}

static uint8_t DHT11_Read_Bit(void)
{
    uint8_t retry = 0;
    while (dht11_ReadData == 1 && retry < 100) // 等待低电平
    {
        retry++;
        delay_us(1);
    }
    retry = 0;
    while (dht11_ReadData == 0 && retry < 100) // 等待高电平
    {
        retry++;
        delay_us(1);
    }
    delay_us(40);
    if (dht11_ReadData == 1) // 读取数据
        return 1;
    else
        return 0;
}
static uint8_t DHT11_Read_Byte(void)
{
    uint8_t i, dat;
    dat = 0;
    for (i = 0; i < 8; i++)
    {
        dat <<= 1;
        dat |= DHT11_Read_Bit();
    }
    return dat;
}
/**
 * @brief DHT11_Init 初始化DHT11
 * @return {uint8_t} 0:成功 1:失败
 */
uint8_t DHT11_Init(void)
{
    DHT11_GPIO_Init();
    DHT11_Start();
    return DHT11_Check();
}
uint8_t DHT11_Read_Data(uint8_t *data)
{
    uint8_t i;
    DHT11_Start();
    if (DHT11_Check() == 0)
    {
        for (i = 0; i < 5; i++)
        {
            data[i] = DHT11_Read_Byte();
        }
        if (data[0] + data[1] + data[2] + data[3] == data[4])
        {
            return 0;
        }
    }
    else
    {
        return 1;
    }
    return 0;
}
