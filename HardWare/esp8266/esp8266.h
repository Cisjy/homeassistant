#ifndef __ESP8266_H__
#define __ESP8266_H__
#include "stm32f1xx_hal.h"
#include "usart_print.h"
#include "stdbool.h"
#pragma anon_unions
#define ESP8266_Usart(fmt, ...) USART3_printf(fmt, ##__VA_ARGS__)
#define Usart_printf(fmt, ...) USART1_printf(fmt, ##__VA_ARGS__)
#define RX_BUF_MAX_LEN 1024 // max length of receive buffer
// /******************************* ESP8266 structure *************************/
typedef struct
{
    char Data_RX_BUF[RX_BUF_MAX_LEN];
    union
    {
        __IO uint16_t InfAll;
        struct
        {
            __IO uint16_t FramLength : 15;
            __IO uint16_t FramFinishFlag : 1;
        } InfBit;
    };

} STRUCT_USARTx_Fram;

typedef enum
{
    STA,
    AP,
    STA_AP
} ENUM_Net_ModeTypeDef;
typedef enum
{
    enumTCP,
    enumUDP,
} ENUM_NetPro_TypeDef;
typedef enum
{
    Multiple_ID_0 = 0,
    Multiple_ID_1 = 1,
    Multiple_ID_2 = 2,
    Multiple_ID_3 = 3,
    Multiple_ID_4 = 4,
    Single_ID_0 = 5,
} ENUM_ID_NO_TypeDef;
typedef enum
{
    OPEN = 0,
    WEP = 1,
    WPA_PSK = 2,
    WPA2_PSK = 3,
    WPA_WPA2_PSK = 4,
} ENUM_AP_PsdMode_TypeDef;
// /******************************* ESP8266 extern variable ***************************/
extern uint8_t g_usart3_rx_tmp_buffer[1];
extern uint8_t g_usart3_rx_flag;
extern uint32_t g_usart3_rx_cnt;
extern uint8_t g_usart3_rx_buffer[128];

extern UART_HandleTypeDef huart3;

extern STRUCT_USARTx_Fram strEsp8266_Fram_Record;
extern STRUCT_USARTx_Fram strUSART_Fram_Record;

void ESP8266_Init(void);
bool ESP8266_Cmd(char *cmd, char *reply1, char *reply2, uint32_t waittime);
bool ESP8266_AT_Test(void);
bool ESP8266_Net_Mode_Choose(ENUM_Net_ModeTypeDef enumMode);
bool ESP8266_JoinAP(char *pSSID, char *pPassWord);
bool ESP8266_QuitAP(void);
bool ESP8266_BuildAP(char *pSSID, char *pPassWord, ENUM_AP_PsdMode_TypeDef enunPsdMode);
bool ESP8266_Enable_MultipleId(FunctionalState enumEnUnvarnishTx);
bool ESP8266_Link_Server(ENUM_NetPro_TypeDef enumE, char *ip, char *ComNum, ENUM_ID_NO_TypeDef id);
bool ESP8266_StartOrShutServer(FunctionalState enumMode, char *pPortNum, char *pTimeOver);
uint8_t ESP8266_Get_LinkStatus(void);
uint8_t ESP8266_Get_IdLinkStatus(void);
uint8_t ESP8266_Inquire_ApIp(char *pApIp, uint8_t ucArrayLength);
bool ESP8266_UnvarnishSend(void);
void ESP8266_ExitUnvarnishSend(void);
bool ESP8266_SendString(FunctionalState enumEnUnvarnishTx, char *pStr, uint32_t ulStrLength, ENUM_ID_NO_TypeDef ucId);
char *ESP8266_ReceiveString(FunctionalState enumEnUnvarnishTx);
#endif
