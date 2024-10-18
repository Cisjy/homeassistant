#ifndef __MQTT_H__
#define __MQTT_H__
#include "mqtt_config.h"
#include "my_string.h"
/* ================================ MQTT报文类型 ================================ */
#define MQTT_TypeCONNECT 1      // 请求连接
#define MQTT_TypeCONNACK 2      // 请求应答
#define MQTT_TypePUBLISH 3      // 发布消息
#define MQTT_TypePUBACK 4       // 发布应答
#define MQTT_TypePUBREC 5       // 发布已接收，保证传递1
#define MQTT_TypePUBREL 6       // 发布释放，保证传递2
#define MQTT_TypePUBCOMP 7      // 发布完成，保证传递3
#define MQTT_TypeSUBSCRIBE 8    // 订阅请求
#define MQTT_TypeSUBACK 9       // 订阅应答
#define MQTT_TypeUNSUBSCRIBE 10 // 取消订阅
#define MQTT_TypeUNSUBACK 11    // 取消订阅应答
#define MQTT_TypePINGREQ 12     // ping请求
#define MQTT_TypePINGRESP 13    // ping响应
#define MQTT_TypeDISCONNECT 14  // 断开连接
#define MQTT_WriteMsg 15        // 等待接收订阅的消息（自定义的）

/* ================================ CONNECT报文设置 ================================ */
#define MQTT_ClientIdentifier MQTTClientID // 客户端标识符
#define MQTT_KeepAlive Set_KeepAlive       // 心跳周期
#define MQTT_StaCleanSession 1             // 清理会话
#if (Enable_Will_Topic == 1)
#define MQTT_StaWillFlag 1                  // 遗嘱标志
#define MQTT_StaWillRetain 1                // 遗嘱保留
#define MQTT_StaWillQoS Will_Topic_Qos      // 遗嘱QoS
#define MQTT_WillTopic Will_Topic_Name      // 遗嘱主题
#define MQTT_WillMessage Will_Topic_Message // 遗嘱消息

#else
#define MQTT_StaWillFlag 0
#define MQTT_StaWillQoS 0
#define MQTT_StaWillRetain 0
#endif
#if (Enable_Username_And_Password == 1)
#define MQTT_StaUserNameFlag 1 // 用户名标志
#define MQTT_StaPasswordFlag 1 // 密码标志
#define MQTT_UserName Username // 用户名
#define MQTT_Password Password // 密码

#else
#define MQTT_StaUserNameFlag 0
#define MQTT_StaPasswordFlag 0
#endif

/* ================================ CONNACK报文返回码 ================================ */
#define Connect_Accept 0x00                   // 连接已接受
#define Connect_Refuse_Version 0x01           // 连接已拒绝，不支持的协议版本
#define Connect_Refuse_ClientId 0x02          // 连接已拒绝，不合格的客户端标识符
#define Connect_Refuse_Sever_Unavailable 0x03 // 连接已拒绝，服务端不可用
#define Connect_Refuse_Acc_Or_Pass 0x04       // 连接已拒绝，无效的用户名或密码
#define Connect_Refuse_Fail_Respond 0x05      // 连接超时，服务器未响应（自定义的）

/* ===================================== 其他定义 ==================================== */
#define Get_Fixed_Header_Type(array) (array[0] >> 4)
#define Get_Fixed_Header_Qos(array) ((array[0] & 0x06) >> 1)
#define Get_Packet_Identifier(array, index) ((array[index] << 8) | (array[index + 1]))
#define Qos0 0
#define Qos1 1
#if (USE_Aliyun_MQTT == 0)
#define Qos2 2
#endif
#define Save_Msg 1
#define No_Save_Msg 0

/* ================================ 供外部使用的函数 ================================ */
#define MQTT_Connect() SendCONNECT()
#define MQTT_Disconnect() SendDISCONNECT()
#define MQTT_Subscribe_Topic(topic_name, Qos) SendSUBSCRIBE(topic_name, Qos)
#define MQTT_Publish_Topic(topic_name, msg, Qos, retain) SendPUBLISH(0, Qos, retain, topic_name, msg)
#define MQTT_alive() SendPINGREQ()
#if (MQTT_StaWillFlag == 1)
#define MQTT_Modify_Will(msg) MQTT_Publish_Topic(MQTT_WillTopic, msg, MQTT_StaWillQoS, Save_Msg)
#endif /*MQTT_alive()*/
#define MQTT_Listen_Topic()                                  \
    {                                                        \
        if (MQTT_ReceiveMsg(MQTT_WriteMsg, mqtt_rxbuf) == 0) \
            PrintRecvMsg();                                  \
    }
#if (DEBUG_1 == 1)
#define PrintRecvMsg() PrintMsg()
#else /*DEBUG_1 == 0*/
#define PrintRecvMsg()
#endif /*DEBUG_1 == 1*/

/* ======================================= 声明 ======================================= */
struct ThemeInfo
{
    char *ThemeName;   // 主题名字
    uint8_t *ThemeMsg; // 主题发出的消息
};

extern void MQTT_SendMsg(uint8_t *mqtt_txbuf, uint8_t len);
extern uint8_t mqtt_rxbuf[MAX_BUF_SIZE];

uint8_t SendFixedHead(unsigned char MesType, unsigned char DupFlag, unsigned char QosLevel, unsigned char Retain);
uint8_t SendCONNECT(void);
void SendDISCONNECT(void);
int SendPUBLISH(unsigned char dup, unsigned char qos, unsigned char retain, const char *topic, const char *msg);
int SendSUBSCRIBE(const char *topic, unsigned char RequestedQoS);
int SendPINGREQ(void);
int MQTT_ReceiveMsg(uint8_t mqtt_msg_type, uint8_t *mqtt_rxbuf);
void PrintMsg(void);

#endif
