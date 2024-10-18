
#ifndef __ESP8266_TEST_H__
#define __ESP8266_TEST_H__
#include "main.h"
/********************************** 用户需要设置的参数**********************************/
#define TCP_CLIENT_TEST 0

#if (1 == TCP_CLIENT_TEST)
#define macUser_ESP8266_TcpServer_IP "192.168.0.102"
#define macUser_ESP8266_TcpServer_Port "6666"
#else
#define macUser_ESP8266_TcpServer_IP "test.com" // 要连接的服务器的 IP
#define macUser_ESP8266_TcpServer_Port "1883"   // 要连接的服务器的端口

#endif
extern uint8_t wifi_flag;

#define macUser_ESP8266_ApSsid "mFi_116D14" //"mFi_116D14" // 要连接的热点的名称
#define macUser_ESP8266_ApPwd "xx123456"    // 要连接的热点的密钥

#define macUser_ESP8266_ApSsid2 "项ㅋㅋ的HUAWEI P60 Pro" //"  " // 要连接的热点的名称
#define macUser_ESP8266_ApPwd2 "xx123456"                // 要连接的热点的密钥

extern volatile uint8_t ucTcpClosedFlag;
uint8_t ESP8266_StaTcpClient_Unvarnish_ConfigTest(void);
void ESP8266_ExitUnvarnishSend_QuitAP(void);
#endif
