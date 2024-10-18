#ifndef __MQTT_CONFIG_H__
#define __MQTT_CONFIG_H__
#include "my_string.h"
#include "esp8266.h"

/* ============================ 代码的一些配置 =============================== */
#define DEBUG_1 1         // 为 0 屏蔽内部所有串口输出信息
#define DEBUG_2 1         // 查看接收/发送的报文
#define USE_Aliyun_MQTT 0 // 是否使用阿里云的免费 MQTT 服务器
#define MAX_BUF_SIZE 1024 // 接收/发送的数据缓冲区
#define MAX_THEME_NUM 10  // 最大存储订阅主题数目
/* =================== 用户移植时需要提供的东西（移植必改） ================== */
#define Rx_Finish_Flag strEsp8266_Fram_Record.InfBit.FramFinishFlag // 串口接收完成标志
#define Rx_Buffer strEsp8266_Fram_Record.Data_RX_BUF                // 串口接收缓冲区
#define Rx_Buffer_Len strEsp8266_Fram_Record.InfBit.FramLength      // 串口接收数据长度
#define ESP8266_USART ESP8266_Usart                                 // ESP8266 的串口
/*
    提供一个能够准确计算一长串16进制数据的长度的函数（strlen函数有些情况统计的长度要比实际长度小）
    函数格式：int UpdateStrlen_uint8_t(const void* source)，函数返回值是实际长度值（该函数在my_string.c中）
*/
#define Count_Hex_Num(source) UpdateStrlen_uint8_t(source)
/*
    提供发送报文函数，该函数要一个一个字节的接收
*/
// #define Send_Message(usart, ch) HAL_UART_Transmit(&usart, &ch, 1, 1000);

/* ================== 用户使用MQTT参数配置（需要用户填写） =================== */
#define Set_KeepAlive 60 // 设置心跳周期

#if (USE_Aliyun_MQTT == 1)             /* 使用阿里云 MQTT 服务器 */
#define Enable_Will_Topic 0            // 阿里云MQTT服务器应该是不能使用遗嘱的，置 0
#define ClientID "zyt"                 // 自定义
#define Enable_Username_And_Password 1 // 必须置 1，不能置零！！！

#else                                  /* USE_Aliyun_MQTT == 0	使用其他的 MQTT 服务器 */
#define Enable_Will_Topic 1            // 是否使用遗嘱主题
#define MQTTClientID "cisjy"           // 自定义
#define Enable_Username_And_Password 0 // 是否使用用户名密码，有些 MQTT 服务器支持匿名登录

#endif /* USE_Aliyun_MQTT */

#if (Enable_Will_Topic == 1)          /* 使用遗嘱主题 */
#define Will_Topic_Qos Qos1           // 遗嘱主题 Qos 等级
#define Will_Topic_Name "/user/will"  // 遗嘱主题名字
#define Will_Topic_Message "off_line" // 遗嘱主题的消息内容

#endif /* Enable_Will_Topic */

/* =================== MQTT服务器连接参数（需要用户填写） ====================== */
#if (USE_Aliyun_MQTT == 1) /* 阿里云 MQTT 服务器 */
#define IP "iot-000p7xp.mqtt.iothub.aliyuncs.com"
#define PORT "1883"

#define ProductKey "xxxxx"
#define DeviceName "xxxxx"
#define DeviceSecret "xxxxx"
// 下面这两行一般无需动它
#define MQTTClientID ClientID "|securemode=3,signmethod=hmacsha1|"
#define Username DeviceName "&" ProductKey
/*
    使用 https://docs-aliyun.cn-hangzhou.oss.aliyun-inc.com/assets/attach/189223/cn_zh/
    1605168543507/MQTT_Password.7z?spm=a2c4g.11186623.2.17.d8d173a8P2h3j5&file=MQTT_Password.7z
    工具生成密码
*/
#define Password "1A4F2B28FDB634792E9F13134DBF2F774"
// 拼接阿里云MQTT主题
#define Create_Ali_Custom_Topic(suffix) "/"##ProductKey##"/"##DeviceName##"/user/"##suffix
#define Ali_Event_Post_Topic "/sys/" ProductKey "/" DeviceName "/thing/event/property/post"

#else                 /*USE_Aliyun_MQTT == 0	使用其他的 MQTT 服务器 */
#define IP "test.com" // 或者其他一些公用 MQTT 服务器的网址
#define PORT "8083"
#if (Enable_Username_And_Password == 1) /* 使用账号密码登录 */
#define Username ""                     // 用户名
#define Password ""                     // 密码

#endif /* Enable_Username_And_Password */

#endif /* USE_Aliyun_MQTT */

#endif
