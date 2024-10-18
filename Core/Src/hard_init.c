#include "hard_init.h"
#include "string.h"
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

mpu6050HandleTypeDef mpu6050_handle;
i2c_driver_interface_t mpu6050_i2c_driver = {
    .i2c_gpio_pin = &(i2c_gpio_pin_t){
        .SCL_Port = GPIOB,
        .SCL_Pin = GPIO_PIN_12,
        .SDA_Port = GPIOB,
        .SDA_Pin = GPIO_PIN_13,
    },
    .Init = I2C_Init,
    .Start = I2C_Start,
    .Stop = I2C_Stop,
    .WaitAck = I2C_WaitAck,
    .SendByte = I2C_SendByte,
    .ReadByte = I2C_ReadByte,

};

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
void update_flag_change(void)
{
    uint8_t update_flag = 0x00;
    W25QXX_Write(&update_flag, W25QXX_FLASH_ADDRESS + 4, 1);
}
uint8_t har_buf[10];
void hard_init(void)
{

    uint8_t who_ami;
    OLED_Init(&oled_handle, &i2c_driver);
    oled_display_init();
    if (DHT11_Init() == 0)
    {
        printf("dht11 init success\r\n");
    }
    mpu6050_init(&mpu6050_handle, &mpu6050_i2c_driver);

    mpu6050_handle.mpu6050_who_am_i(&mpu6050_handle, &who_ami);
    printf("mpu6050 who_am_i: %x\r\n", who_ami);

    if (0 == mpu6050_dmp_init())
    {
        printf("mpu6050 dmp init success\r\n");
    }
    BH1750_Init(&bh1750_handle, &bh1750_i2c_driver);
    W25QXX_Init();
    update_flag_change();
    key_init();
    beep_init();
    ESP8266_Init();
}
