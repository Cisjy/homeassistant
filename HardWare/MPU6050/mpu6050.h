/*
 * @Descripttion:
 * @Author: Cisjy
 * @Date: 2024-07-14 16:41:24
 * @LastEditors: Cisjy
 * @LastEditTime: 2024-10-18 11:55:52
 */
#ifndef ___MPU6050_H
#define ___MPU6050_H
#include "mpu6050_reg.h"
#include "i2c_c.h"

#define MPU6050_DEBUG 0
#define MPU6050_SCL_PORT GPIOB
#define MPU6050_SDA_PORT GPIOB
#define MPU6050_SCL_PIN GPIO_PIN_12
#define MPU6050_SDA_PIN GPIO_PIN_13
typedef enum
{
    MPU_OK = 0U,
    MPU_ERROR = 1U,
    MPU_ERROR_ID = 2U

} MPU6050_StatusTypeDef;

typedef struct _mpu6050HandleTypeDef mpu6050HandleTypeDef;

struct _mpu6050HandleTypeDef
{

    i2c_driver_interface_t *mpu6050_i2c_driver_interface;
    uint8_t (*mpu6050_write_reg)(mpu6050HandleTypeDef *mpu6050Handle,
                                 uint8_t addr,
                                 uint8_t reg,
                                 uint8_t len,
                                 uint8_t *data);
    uint8_t (*mpu6050_write_reg_byte)(mpu6050HandleTypeDef *mpu6050Handle,
                                      uint8_t addr,
                                      uint8_t reg,
                                      uint8_t data);
    uint8_t (*mpu6050_read_reg)(mpu6050HandleTypeDef *mpu6050Handle,
                                uint8_t addr,
                                uint8_t reg,
                                uint8_t len,
                                uint8_t *data);
    uint8_t (*mpu6050_read_reg_byte)(mpu6050HandleTypeDef *mpu6050Handle,
                                     uint8_t addr,
                                     uint8_t reg,
                                     uint8_t *data);
    void (*mpu6050_sw_reset)(mpu6050HandleTypeDef *mpu6050Handle);
    uint8_t (*mpu6050_set_gyro_fsr)(mpu6050HandleTypeDef *mpu6050Handle, uint8_t fsr);
    uint8_t (*mpu6050_set_accel_fsr)(mpu6050HandleTypeDef *mpu6050Handle, uint8_t fsr);
    uint8_t (*mpu6050_set_lpf)(mpu6050HandleTypeDef *mpu6050Handle, uint16_t lpf);
    uint8_t (*mpu6050_set_rate)(mpu6050HandleTypeDef *mpu6050Handle, uint16_t rate);
    uint8_t (*mpu6050_get_temperature)(mpu6050HandleTypeDef *mpu6050Handle, int16_t *temperature);
    uint8_t (*mpu6050_get_gyroscope)(mpu6050HandleTypeDef *mpu6050Handle,
                                     int16_t *gx,
                                     int16_t *gy,
                                     int16_t *gz);
    uint8_t (*mpu6050_get_accelerometer)(mpu6050HandleTypeDef *mpu6050Handle,
                                         int16_t *ax,
                                         int16_t *ay,
                                         int16_t *az);
    /**
     * @brief 验证MPU6050芯片ID 如果who am i的值是0x68 则代表MPU6050芯片正常工作
     * @param {mpu6050HandleTypeDef} *mpu6050Handle
     * @param {uint8_t} *who_am_i
     * @return {*}
     */
    uint8_t (*mpu6050_who_am_i)(mpu6050HandleTypeDef *mpu6050Handle, uint8_t *who_am_i);
};

uint8_t mpu6050_init(mpu6050HandleTypeDef *mpu6050Handle,
                     i2c_driver_interface_t *mpu6050_i2c_driver_interface);

/*below of this two function is for inv_mpu.c   but if you want to use it,you can also use it in your project*/
uint8_t mpu_write_buf(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *data);
uint8_t mpu_read_buf(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *data);

// only for test
void demo_niming_report_status(int16_t rol, int16_t pit, int16_t yaw, uint32_t alt, uint8_t fly_mode, uint8_t armed);
void demo_niming_report_senser(int16_t acc_x, int16_t acc_y, int16_t acc_z,
                               int16_t gyro_x, int16_t gyro_y, int16_t gyro_z,
                               int16_t mag_x, int16_t mag_y, int16_t mag_z);
#endif

/*
how to use ?
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

mpu6050_init(&mpu6050_handle, &mpu6050_i2c_driver);
     if (0 == mpu6050_dmp_init())         //if you want to use dmp
     {
         USART2_printf("mpu6050 dmp init success\r\n");
     }

*/
