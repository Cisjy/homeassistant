#include "mpu6050.h"
#include "i2c_c.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
/**
 * @brief : wirte mpu6050 data to reg
 * @param {mpu6050HandleTypeDef} *mpu6050Handle
 * @param {uint8_t} addr
 * @param {uint8_t} reg
 * @param {uint8_t} len
 * @param {uint8_t} *data
 * @return {0:success,1:fail}
 */
static uint8_t mpu6050_write_reg(mpu6050HandleTypeDef *mpu6050Handle,
                                 uint8_t addr,
                                 uint8_t reg,
                                 uint8_t len,
                                 uint8_t *data)
{
    uint8_t i;
    mpu6050Handle->mpu6050_i2c_driver_interface->Start(
        mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin);
    mpu6050Handle->mpu6050_i2c_driver_interface->SendByte(
        mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin, addr << 1 | I2C_WRITE);
    if (I2C_NACK == mpu6050Handle->mpu6050_i2c_driver_interface->WaitAck(
                        mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin))
    {
        mpu6050Handle->mpu6050_i2c_driver_interface->Stop(
            mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin);
        return MPU_ERROR;
    }
    mpu6050Handle->mpu6050_i2c_driver_interface->SendByte(
        mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin, reg);
    if (I2C_NACK == mpu6050Handle->mpu6050_i2c_driver_interface->WaitAck(
                        mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin))
    {
        mpu6050Handle->mpu6050_i2c_driver_interface->Stop(
            mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin);
        return MPU_ERROR;
    }
    for (i = 0; i < len; i++)
    {

        mpu6050Handle->mpu6050_i2c_driver_interface->SendByte(
            mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin, data[i]);
        if (I2C_NACK == mpu6050Handle->mpu6050_i2c_driver_interface->WaitAck(
                            mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin))
        {
            mpu6050Handle->mpu6050_i2c_driver_interface->Stop(
                mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin);
            return MPU_ERROR;
        }
    }
    mpu6050Handle->mpu6050_i2c_driver_interface->Stop(
        mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin);
    return MPU_OK;
}
/**
 * @brief : write mpu6050 one byte data to reg
 * @param {mpu6050HandleTypeDef} *mpu6050Handle
 * @param {uint8_t} addr
 * @param {uint8_t} reg
 * @param {uint8_t} data
 * @return {0:success,1:fail}
 */
static uint8_t mpu6050_write_reg_byte(mpu6050HandleTypeDef *mpu6050Handle, uint8_t addr, uint8_t reg, uint8_t data)
{
    return mpu6050_write_reg(mpu6050Handle, addr, reg, 1, &data);
}

/**
 * @brief :
 * @param {mpu6050HandleTypeDef} *mpu6050Handle
 * @param {uint8_t} reg
 * @param {uint8_t} len
 * @param {uint8_t} *data
 * @return {0:success,1:fail}
 */
static uint8_t mpu6050_read_reg(mpu6050HandleTypeDef *mpu6050Handle,
                                uint8_t addr,
                                uint8_t reg,
                                uint8_t len,
                                uint8_t *data)
{

    mpu6050Handle->mpu6050_i2c_driver_interface->Start(
        mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin);
    mpu6050Handle->mpu6050_i2c_driver_interface->SendByte(
        mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin, addr << 1 | I2C_WRITE);
    if (I2C_NACK == mpu6050Handle->mpu6050_i2c_driver_interface->WaitAck(
                        mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin))
    {
        mpu6050Handle->mpu6050_i2c_driver_interface->Stop(
            mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin);
        return MPU_ERROR;
    }
    mpu6050Handle->mpu6050_i2c_driver_interface->SendByte(
        mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin, reg);
    if (I2C_NACK == mpu6050Handle->mpu6050_i2c_driver_interface->WaitAck(
                        mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin))
    {
        mpu6050Handle->mpu6050_i2c_driver_interface->Stop(
            mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin);
        return MPU_ERROR;
    }
    mpu6050Handle->mpu6050_i2c_driver_interface->Start(
        mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin);
    mpu6050Handle->mpu6050_i2c_driver_interface->SendByte(
        mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin, addr << 1 | I2C_READ);
    if (I2C_NACK == mpu6050Handle->mpu6050_i2c_driver_interface->WaitAck(
                        mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin))
    {
        mpu6050Handle->mpu6050_i2c_driver_interface->Stop(
            mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin);
        return MPU_ERROR;
    }
    while (len)
    {
        *data = mpu6050Handle->mpu6050_i2c_driver_interface->ReadByte(
            mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin, (len > 1) ? I2C_ACK : I2C_NACK);
        data++;
        len--;
    }
    mpu6050Handle->mpu6050_i2c_driver_interface->Stop(
        mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin);
    return MPU_OK;
}

/**
 * @brief :
 * @param {mpu6050HandleTypeDef} *mpu6050Handle
 * @param {uint8_t} reg
 * @param {uint8_t} *data
 * @return {success:0,fail:1}
 */
static uint8_t mpu6050_read_reg_byte(mpu6050HandleTypeDef *mpu6050Handle, uint8_t addr, uint8_t reg, uint8_t *data)
{
    return mpu6050_read_reg(mpu6050Handle, addr, reg, 1, data);
}

/**
 * @brief :
 * @param {mpu6050HandleTypeDef} *mpu6050Handle
 * @return {*}
 */
static void mpu6050_sw_reset(mpu6050HandleTypeDef *mpu6050Handle)
{
    mpu6050_write_reg_byte(mpu6050Handle, MPU6050_ADDRESS, MPU_PWR_MGMT1_REG, 0x80);
    delay_ms(100);
    mpu6050_write_reg_byte(mpu6050Handle, MPU6050_ADDRESS, MPU_PWR_MGMT1_REG, 0x00);
}

/**
 * @brief : set mpu6050 gyroscope range
 * @param {mpu6050HandleTypeDef} *mpu6050Handle
 * @param {uint8_t} fsr       {0:250dps,1:500dps,2:1000dps,3:2000dps}
 * @return {0:success,1:fail}
 */
static uint8_t mpu6050_set_gyro_fsr(mpu6050HandleTypeDef *mpu6050Handle, uint8_t fsr)
{
    return mpu6050_write_reg_byte(mpu6050Handle, MPU6050_ADDRESS, MPU_GYRO_CFG_REG, fsr << 3);
}

/**
 * @brief : set mpu6050 accelerometer range
 * @param {mpu6050HandleTypeDef} *mpu6050Handle
 * @param {uint8_t} fsr       {0:2g,1:4g,2:8g,3:16g}
 * @return {0:success,1:fail}
 */
static uint8_t mpu6050_set_accel_fsr(mpu6050HandleTypeDef *mpu6050Handle, uint8_t fsr)
{
    return mpu6050_write_reg_byte(mpu6050Handle, MPU6050_ADDRESS, MPU_ACCEL_CFG_REG, fsr << 3);
}

/**
 * @brief : 设置数字低通滤波器频率
 * @param {mpu6050HandleTypeDef} *mpu6050Handle
 * @param {uint16_t} lpf       {188,98,42,20,10,5}
 * @return {0:success,1:fail}
 */
static uint8_t mpu6050_set_lpf(mpu6050HandleTypeDef *mpu6050Handle, uint16_t lpf)
{
    uint8_t data;
    if (lpf == 188)
    {
        data = 0x01;
    }
    else if (lpf == 98)
    {
        data = 0x02;
    }
    else if (lpf == 42)
    {
        data = 0x03;
    }
    else if (lpf == 20)
    {
        data = 0x04;
    }
    else if (lpf == 10)
    {
        data = 0x05;
    }
    else if (lpf == 5)
    {
        data = 0x06;
    }
    else
    {
        return 1;
    }
    return mpu6050_write_reg_byte(mpu6050Handle, MPU6050_ADDRESS, MPU_CFG_REG, data);
}
/**
 * @brief :set mpu6050 sample rate
 * @param {mpu6050HandleTypeDef} *mpu6050Handle
 * @param {uint16_t} rate  4-1000
 * @return {0:success,1:fail}
 */
static uint8_t mpu6050_set_rate(mpu6050HandleTypeDef *mpu6050Handle, uint16_t rate)
{
    uint8_t data;
    if (rate > 1000)
    {
        rate = 1000;
    }
    if (rate < 4)
    {

        rate = 4;
    }
    data = 1000 / rate - 1;

    if (MPU_ERROR == mpu6050_write_reg_byte(mpu6050Handle, MPU6050_ADDRESS, MPU_SAMPLE_RATE_REG, data))
    {
        return MPU_ERROR;
    }

    return MPU_OK;
}

/**
 * @brief :
 * @param {mpu6050HandleTypeDef} *mpu6050Handle
 * @param {int16_t} *temperature
 * The true temperature is the result divided by 100
 * @return {0:success,1:fail}
 */
static uint8_t mpu6050_get_temperature(mpu6050HandleTypeDef *mpu6050Handle, int16_t *temperature)
{

    uint8_t data[2];
    int16_t raw = 0;

    if (MPU_OK == mpu6050_read_reg(mpu6050Handle, MPU6050_ADDRESS, MPU_TEMP_OUTH_REG, 2, data))
    {
        raw = (uint16_t)data[0] << 8 | data[1];
        *temperature = (int16_t)((36.53f + (float)raw / 340.0f) * 100);
        return MPU_OK;
    }
    return MPU_ERROR;
}

/**
 * @brief :get mpu6050 accelerometer data
 * @param {mpu6050HandleTypeDef} *mpu6050Handle
 * @param {int16_t} *gx
 * @param {int16_t} *gy
 * @param {int16_t} *gz
 * @return {MPU_OK:success,MPU_ERROR:fail}
 */
static uint8_t mpu6050_get_gyroscope(mpu6050HandleTypeDef *mpu6050Handle,
                                     int16_t *gx,
                                     int16_t *gy,
                                     int16_t *gz)
{

    /*#define MPU_GYRO_XOUTH_REG 0X43 // 陀螺仪值,X轴高8位寄存器
    #define MPU_GYRO_XOUTL_REG 0X44 // 陀螺仪值,X轴低8位寄存器
    #define MPU_GYRO_YOUTH_REG 0X45 // 陀螺仪值,Y轴高8位寄存器
    #define MPU_GYRO_YOUTL_REG 0X46 // 陀螺仪值,Y轴低8位寄存器
    #define MPU_GYRO_ZOUTH_REG 0X47 // 陀螺仪值,Z轴高8位寄存器
    #define MPU_GYRO_ZOUTL_REG 0X48 // 陀螺仪值,Z轴低8位寄存器*/

    uint8_t data[6] = {0};

    if (MPU_OK == mpu6050_read_reg(mpu6050Handle, MPU6050_ADDRESS, MPU_GYRO_XOUTH_REG, 6, data))
    {
        *gx = ((uint16_t)data[0] << 8) | data[1];
        *gy = ((uint16_t)data[2] << 8) | data[3];
        *gz = ((uint16_t)data[4] << 8) | data[5];
        return MPU_OK;
    }

    return MPU_ERROR;
}
/**
 * @brief :
 * @param {mpu6050HandleTypeDef} *mpu6050Handle
 * @param {int16_t} *ax
 * @param {int16_t} *ay
 * @param {int16_t} *az
 * @return {MPU_OK:success,MPU_ERROR:fail}
 */
static uint8_t mpu6050_get_accelerometer(mpu6050HandleTypeDef *mpu6050Handle,
                                         int16_t *ax,
                                         int16_t *ay,
                                         int16_t *az)
{
#if 0
#define MPU_ACCEL_XOUTH_REG 0X3B // 加速度值,X轴高8位寄存器
#define MPU_ACCEL_XOUTL_REG 0X3C // 加速度值,X轴低8位寄存器
#define MPU_ACCEL_YOUTH_REG 0X3D // 加速度值,Y轴高8位寄存器
#define MPU_ACCEL_YOUTL_REG 0X3E // 加速度值,Y轴低8位寄存器
#define MPU_ACCEL_ZOUTH_REG 0X3F // 加速度值,Z轴高8位寄存器
#define MPU_ACCEL_ZOUTL_REG 0X40 // 加速度值,Z轴低8位寄存器
#endif
    uint8_t data[6] = {0};

    if (MPU_OK == mpu6050_read_reg(mpu6050Handle, MPU6050_ADDRESS, MPU_ACCEL_XOUTH_REG, 6, data))
    {
        *ax = ((uint16_t)data[0] << 8) | data[1];
        *ay = ((uint16_t)data[2] << 8) | data[3];
        *az = ((uint16_t)data[4] << 8) | data[5];
        return MPU_OK;
    }

    return MPU_ERROR;
}

static uint8_t mpu6050_who_am_i(mpu6050HandleTypeDef *mpu6050Handle, uint8_t *who_am_i)
{

    return mpu6050_read_reg_byte(mpu6050Handle, MPU6050_ADDRESS, MPU_DEVICE_ID_REG, who_am_i);
}

uint8_t mpu6050_init(mpu6050HandleTypeDef *mpu6050Handle,
                     i2c_driver_interface_t *mpu6050_i2c_driver_interface)
{
    uint8_t id;
    mpu6050Handle->mpu6050_i2c_driver_interface = mpu6050_i2c_driver_interface;
    mpu6050Handle->mpu6050_write_reg = mpu6050_write_reg;
    mpu6050Handle->mpu6050_write_reg_byte = mpu6050_write_reg_byte;
    mpu6050Handle->mpu6050_read_reg = mpu6050_read_reg;
    mpu6050Handle->mpu6050_read_reg_byte = mpu6050_read_reg_byte;
    mpu6050Handle->mpu6050_sw_reset = mpu6050_sw_reset;
    mpu6050Handle->mpu6050_set_gyro_fsr = mpu6050_set_gyro_fsr;
    mpu6050Handle->mpu6050_set_accel_fsr = mpu6050_set_accel_fsr;
    mpu6050Handle->mpu6050_set_lpf = mpu6050_set_lpf;
    mpu6050Handle->mpu6050_set_rate = mpu6050_set_rate;
    mpu6050Handle->mpu6050_get_temperature = mpu6050_get_temperature;
    mpu6050Handle->mpu6050_get_gyroscope = mpu6050_get_gyroscope;
    mpu6050Handle->mpu6050_get_accelerometer = mpu6050_get_accelerometer;
    mpu6050Handle->mpu6050_who_am_i = mpu6050_who_am_i;

    // init
    mpu6050Handle->mpu6050_i2c_driver_interface->Init(mpu6050Handle->mpu6050_i2c_driver_interface->i2c_gpio_pin);
    mpu6050_sw_reset(mpu6050Handle);

    mpu6050_write_reg_byte(mpu6050Handle, MPU6050_ADDRESS, MPU_PWR_MGMT1_REG, 0x01); // set clock source to gyroscope
    mpu6050_write_reg_byte(mpu6050Handle, MPU6050_ADDRESS, MPU_PWR_MGMT2_REG, 0x00); // set wakeup frequency to 1Hz

    mpu6050_set_rate(mpu6050Handle, 100);    // 10分频
                                             // mpu6050_set_lpf(mpu6050Handle, 5);       // set low pass filter
    mpu6050_set_gyro_fsr(mpu6050Handle, 3);  // set gyroscope range to 2000dps
    mpu6050_set_accel_fsr(mpu6050Handle, 0); // set accelerometer range to 16g

    // mpu6050_write_reg_byte(mpu6050Handle, MPU6050_ADDRESS, MPU_INT_EN_REG, 0x00);    // disable interrupt
    // mpu6050_write_reg_byte(mpu6050Handle, MPU6050_ADDRESS, MPU_USER_CTRL_REG, 0x00); // disable I2C master mode
    // mpu6050_write_reg_byte(mpu6050Handle, MPU6050_ADDRESS, MPU_FIFO_EN_REG, 0x00);   // disable FIFO
    // mpu6050_write_reg_byte(mpu6050Handle, MPU6050_ADDRESS, MPU_INTBP_CFG_REG, 0x80); // set INT ping low to push interrupt

    return MPU_OK;
}

/**
 * @brief : below of this two function is for inv_mpu.c
 *          but if you want to use it,you can also use it in your project
 * @param {uint8_t} addr
 * @param {uint8_t} reg
 * @param {uint8_t} len
 * @param {uint8_t} *data
 * @return {*}
 */
uint8_t mpu_write_buf(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *data)
{
    i2c_gpio_pin_t mpu6050_i2c_gpio_pin = {
        .SCL_Port = MPU6050_SCL_PORT,
        .SCL_Pin = MPU6050_SCL_PIN,
        .SDA_Port = MPU6050_SDA_PORT,
        .SDA_Pin = MPU6050_SDA_PIN};

    uint8_t i;
    I2C_Start(&mpu6050_i2c_gpio_pin);
    I2C_SendByte(&mpu6050_i2c_gpio_pin, addr << 1 | I2C_WRITE);
    if (I2C_NACK == I2C_WaitAck(&mpu6050_i2c_gpio_pin))
    {
        I2C_Stop(&mpu6050_i2c_gpio_pin);
        return MPU_ERROR;
    }
    I2C_SendByte(&mpu6050_i2c_gpio_pin, reg);
    if (I2C_NACK == I2C_WaitAck(&mpu6050_i2c_gpio_pin))
    {
        I2C_Stop(&mpu6050_i2c_gpio_pin);
        return MPU_ERROR;
    }
    for (i = 0; i < len; i++)
    {

        I2C_SendByte(&mpu6050_i2c_gpio_pin, data[i]);
        if (I2C_NACK == I2C_WaitAck(&mpu6050_i2c_gpio_pin))
        {
            I2C_Stop(&mpu6050_i2c_gpio_pin);
            return MPU_ERROR;
        }
    }
    I2C_Stop(&mpu6050_i2c_gpio_pin);
    return MPU_OK;
}
/**
 * @brief : below of this two function is for inv_mpu.c
 *          but if you want to use it,you can also use it in your project
 * @param {uint8_t} addr
 * @param {uint8_t} reg
 * @param {uint8_t} len
 * @param {uint8_t} *data
 * @return {*}
 */
uint8_t mpu_read_buf(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *data)
{

    i2c_gpio_pin_t mpu6050_i2c_gpio_pin = {
        .SCL_Port = MPU6050_SCL_PORT,
        .SCL_Pin = MPU6050_SCL_PIN,
        .SDA_Port = MPU6050_SDA_PORT,
        .SDA_Pin = MPU6050_SDA_PIN};

    I2C_Start(&mpu6050_i2c_gpio_pin);
    I2C_SendByte(&mpu6050_i2c_gpio_pin, addr << 1 | I2C_WRITE);
    if (I2C_NACK == I2C_WaitAck(&mpu6050_i2c_gpio_pin))
    {
        I2C_Stop(&mpu6050_i2c_gpio_pin);
        return MPU_ERROR;
    }
    I2C_SendByte(&mpu6050_i2c_gpio_pin, reg);
    if (I2C_NACK == I2C_WaitAck(&mpu6050_i2c_gpio_pin))
    {
        I2C_Stop(&mpu6050_i2c_gpio_pin);
        return MPU_ERROR;
    }
    I2C_Start(&mpu6050_i2c_gpio_pin);
    I2C_SendByte(&mpu6050_i2c_gpio_pin, addr << 1 | I2C_READ);
    if (I2C_NACK == I2C_WaitAck(&mpu6050_i2c_gpio_pin))
    {
        I2C_Stop(&mpu6050_i2c_gpio_pin);
        return MPU_ERROR;
    }
    while (len)
    {
        *data = I2C_ReadByte(&mpu6050_i2c_gpio_pin, (len > 1) ? I2C_ACK : I2C_NACK);
        data++;
        len--;
    }
    I2C_Stop(&mpu6050_i2c_gpio_pin);
    return MPU_OK;
}

/**
 * @brief       通过串口1发送数据至匿名地面站V4
 * @param       fun: 功能字
 *              dat: 待发送的数据（最多28字节）
 *              len: dat数据的有效位数
 * @retval      无
 */
static void demo_usart1_niming_report(uint8_t fun, uint8_t *dat, uint8_t len)
{
    uint8_t send_buf[32];
    uint8_t i;

    if (len > 28)
    {
        return;
    }

    send_buf[len + 4] = 0;    /* 校验位清零 */
    send_buf[0] = 0xAA;       /* 帧头为0xAAAA */
    send_buf[1] = 0xAA;       /* 帧头为0xAAAA */
    send_buf[2] = fun;        /* 功能字 */
    send_buf[3] = len;        /* 数据长度 */
    for (i = 0; i < len; i++) /* 复制数据 */
    {
        send_buf[4 + i] = dat[i];
    }
    for (i = 0; i < (len + 4); i++) /* 计算校验和 */
    {
        send_buf[len + 4] += send_buf[i];
    }
    extern UART_HandleTypeDef huart1;
    /* 发送数据 */
    HAL_UART_Transmit(&huart1, send_buf, len + 5, HAL_MAX_DELAY);
}

/**
 * @brief       发送状态帧至匿名地面站V4
 * @param       rol     : 横滚角
 *              pit     : 俯仰角
 *              yaw     : 航向角
 *              alt     : 飞行高度，单位：cm
 *              fly_mode: 飞行模式
 *              armed   : 锁定状态，0xA0：加锁 0xA1：解锁
 * @retval      无
 */
void demo_niming_report_status(int16_t rol, int16_t pit, int16_t yaw, uint32_t alt, uint8_t fly_mode, uint8_t armed)
{
    uint8_t send_buf[12];

    /* 横滚角 */
    send_buf[0] = (rol >> 8) & 0xFF;
    send_buf[1] = rol & 0xFF;
    /* 俯仰角 */
    send_buf[2] = (pit >> 8) & 0xFF;
    send_buf[3] = pit & 0xFF;
    /* 航向角 */
    send_buf[4] = (yaw >> 8) & 0xFF;
    send_buf[5] = yaw & 0xFF;
    /* 飞行高度 */
    send_buf[6] = (alt >> 24) & 0xFF;
    send_buf[7] = (alt >> 16) & 0xFF;
    send_buf[8] = (alt >> 8) & 0xFF;
    send_buf[9] = alt & 0xFF;
    /* 飞行模式 */
    send_buf[10] = fly_mode;
    /* 锁定状态 */
    send_buf[11] = armed;

    /* 状态帧的功能字为0x01 */
    demo_usart1_niming_report(0x01, send_buf, 12);
}

/**
 * @brief       发送传感器帧至匿名地面站V4
 * @param       acc_x : x轴上的加速度值
 *              acc_y : y轴上的加速度值
 *              acc_z : z轴上的加速度值
 *              gyro_x: x轴上的陀螺仪值
 *              gyro_y: y轴上的陀螺仪值
 *              gyro_z: z轴上的陀螺仪值
 *              mag_x : x轴上的磁力计值（ATK-MS6050不支持）
 *              mag_y : y轴上的磁力计值（ATK-MS6050不支持）
 *              mag_z : z轴上的磁力计值（ATK-MS6050不支持）
 * @retval      无
 */
void demo_niming_report_senser(int16_t acc_x, int16_t acc_y, int16_t acc_z,
                               int16_t gyro_x, int16_t gyro_y, int16_t gyro_z,
                               int16_t mag_x, int16_t mag_y, int16_t mag_z)
{
    uint8_t send_buf[18];

    /* x轴上的加速度值 */
    send_buf[0] = (acc_x >> 8) & 0xFF;
    send_buf[1] = acc_x & 0xFF;
    /* y轴上的加速度值 */
    send_buf[2] = (acc_y >> 8) & 0xFF;
    send_buf[3] = acc_y & 0xFF;
    /* z轴上的加速度值 */
    send_buf[4] = (acc_z >> 8) & 0xFF;
    send_buf[5] = acc_z & 0xFF;
    /* x轴上的陀螺仪值 */
    send_buf[6] = (gyro_x >> 8) & 0xFF;
    send_buf[7] = gyro_x & 0xFF;
    /* y轴上的陀螺仪值 */
    send_buf[8] = (gyro_y >> 8) & 0xFF;
    send_buf[9] = gyro_y & 0xFF;
    /* z轴上的陀螺仪值 */
    send_buf[10] = (gyro_z >> 8) & 0xFF;
    send_buf[11] = gyro_z & 0xFF;
    /* x轴上的磁力计值 */
    send_buf[12] = (mag_x >> 8) & 0xFF;
    send_buf[13] = mag_x & 0xFF;
    /* y轴上的磁力计值 */
    send_buf[14] = (mag_y >> 8) & 0xFF;
    send_buf[15] = mag_y & 0xFF;
    /* z轴上的磁力计值 */
    send_buf[16] = (mag_z >> 8) & 0xFF;
    send_buf[17] = mag_z & 0xFF;

    /* 传感器的功能字为0x02 */
    demo_usart1_niming_report(0x02, send_buf, 18);
}