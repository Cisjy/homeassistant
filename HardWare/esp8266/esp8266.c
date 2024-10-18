#include <stdio.h>
#include <string.h>
#include "esp8266.h"
#include "delay.h"
#include "cmsis_os.h"
uint8_t g_usart3_rx_tmp_buffer[1];
uint8_t g_usart3_rx_flag = 0;
uint32_t g_usart3_rx_cnt = 0;
uint8_t g_usart3_rx_buffer[128];
UART_HandleTypeDef huart3;
STRUCT_USARTx_Fram strEsp8266_Fram_Record = {0};
STRUCT_USARTx_Fram strUSART_Fram_Record = {0};
static void ESP8266_GPIO_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
   */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

static void ESP8266_UART_Init(void)
{

    __HAL_RCC_USART3_CLK_ENABLE();

    huart3.Instance = USART3;
    huart3.Init.BaudRate = 115200;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart3);
    HAL_NVIC_SetPriority(USART3_IRQn, 4, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
    HAL_UART_Receive_IT(&huart3, (uint8_t *)g_usart3_rx_tmp_buffer, 1);
}

void ESP8266_Restart(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
}
void ESP8266_Init(void)
{
    ESP8266_GPIO_Init();
    ESP8266_UART_Init();
    ESP8266_Restart();
}

bool ESP8266_Cmd(char *cmd, char *reply1, char *reply2, uint32_t waittime)
{
    strEsp8266_Fram_Record.InfBit.FramLength = 0;
    ESP8266_Usart("%s\r\n", cmd);
    if ((0 == reply1) && (0 == reply2))
    {
        return true;
    }

    delay_ms(waittime);

    strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';
    Usart_printf("%s", strEsp8266_Fram_Record.Data_RX_BUF);
    strEsp8266_Fram_Record.InfBit.FramLength = 0;
    strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;
    if ((0 != reply1) && (0 != reply2))
    {
        return (bool)((NULL != strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply1)) ||
                      (NULL != strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply2)));
    }
    else if (0 != reply1)
    {
        return (bool)(NULL != strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply1));
    }
    else
    {

        return (bool)(NULL != strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply2));
    }
}

bool ESP8266_AT_Test(void)
{
    char count = 0;
    Usart_printf("ATteststest\r\n");

    delay_ms(500);

    while (count < 10)
    {
        Usart_printf("\r\nATtest %d......\r\n", count);
        if (ESP8266_Cmd("AT", "OK", NULL, 500))
        {
            Usart_printf("ATtest OKstart achive%d\r\n", count);
            return 1;
        }
        count++;
    }
    return 0;
}

/**
 * @brief :esp8266 net mode switch
 * @param {ENUM_Net_ModeTypeDef} enumMode
 *          STA: Station mode
            AP: Access point mode
            STA_AP: Station + Access point mode
 * @return true  false
 */
bool ESP8266_Net_Mode_Choose(ENUM_Net_ModeTypeDef enumMode)
{
    switch (enumMode)
    {
    case STA:
        return ESP8266_Cmd("AT+CWMODE=1", "OK", "no change", 1000);

    case AP:
        return ESP8266_Cmd("AT+CWMODE=2", "OK", "no change", 1000);

    case STA_AP:
        return ESP8266_Cmd("AT+CWMODE=3", "OK", "no change", 1000);

    default:
        return false;
    }
}

/**
 * @brief ESP8266 join AP
 * @param {char} *pSSID  your wifi name
 * @param {char} *pPassWord your wifi password
 * @return true  false
 */
bool ESP8266_JoinAP(char *pSSID, char *pPassWord)
{
    char cCmd[120];
    sprintf(cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord);
    return ESP8266_Cmd(cCmd, "OK", NULL, 2500);
}

/**
 * @brief Esp8266 Quit AP
 * @return true  false
 */
bool ESP8266_QuitAP(void)
{
    char cCmd[120];
    sprintf(cCmd, "AT+CWQAP");
    return ESP8266_Cmd(cCmd, "OK", NULL, 2500);
}
/**
 * @brief :esp8266 build AP
 * @param {char} *pSSID  wifi name
 * @param {char} *pPassWord  wifi password
 * @param {ENUM_AP_PsdMode_TypeDef} enunPsdMode  wifi password mode
 *              OPEN = 0,
                WEP = 1,
                WPA_PSK = 2,
                WPA2_PSK = 3,
                WPA_WPA2_PSK = 4,
 * @return true  false
 */
bool ESP8266_BuildAP(char *pSSID, char *pPassWord, ENUM_AP_PsdMode_TypeDef enunPsdMode)
{
    char cCmd[120];

    sprintf(cCmd, "AT+CWSAP=\"%s\",\"%s\",1,%d", pSSID, pPassWord, enunPsdMode);

    return ESP8266_Cmd(cCmd, "OK", 0, 1000);
}

/**
 * @brief :
 * @param {FunctionalState} enumEnUnvarnishTx
 *   DISABLE = 0,
     ENABLE =  1
 * @return true  false
 */
bool ESP8266_Enable_MultipleId(FunctionalState enumEnUnvarnishTx)
{
    char cStr[20];

    sprintf(cStr, "AT+CIPMUX=%d", (enumEnUnvarnishTx ? 1 : 0));

    return ESP8266_Cmd(cStr, "OK", 0, 500);
}

/**
 * @brief :
 * @param {ENUM_NetPro_TypeDef} enumE
 *           TCP  UDP
 * @param {char} *ip
 * @param {char} *ComNum
 * @param {ENUM_ID_NO_TypeDef} id
 * @return true  false
 */
bool ESP8266_Link_Server(ENUM_NetPro_TypeDef enumE, char *ip, char *ComNum, ENUM_ID_NO_TypeDef id)
{
    char cStr[100] = {0}, cCmd[120];
    switch (enumE)
    {
    case enumTCP:
        sprintf(cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum);
        break;
    case enumUDP:
        sprintf(cStr, "\"%s\",\"%s\",%s", "UDP", ip, ComNum);
        break;
    default:
        break;
    }
    if (id < 5)
    {
        sprintf(cCmd, "AT+CIPSTART=%d,%s", id, cStr);
    }
    else
    {
        sprintf(cCmd, "AT+CIPSTART=%s", cStr);
    }
    return ESP8266_Cmd(cCmd, "OK", "ALREAY CONNECT", 1000);
}

/**
 * @brief :wifi esp8266 start or shut server
 * @param {FunctionalState} enumMode
 * @param {char} *pPortNum
 * @param {char} *pTimeOver
 * @return {*}
 */
bool ESP8266_StartOrShutServer(FunctionalState enumMode, char *pPortNum, char *pTimeOver)
{
    char cCmd1[120], cCmd2[120];
    if (enumMode)
    {
        sprintf(cCmd1, "AT+CIPSERVER=%d,%s", 1, pPortNum);

        sprintf(cCmd2, "AT+CIPSTO=%s", pTimeOver); // 设置服务器超时时间

        return (ESP8266_Cmd(cCmd1, "OK", 0, 500) &&
                ESP8266_Cmd(cCmd2, "OK", 0, 500));
    }
    else
    {
        sprintf(cCmd1, "AT+CIPSERVER=%d,%s", 0, pPortNum);

        return ESP8266_Cmd(cCmd1, "OK", 0, 500);
    }
}
/**
 * @brief :
 * @return 2 3 4
 */
uint8_t ESP8266_Get_LinkStatus(void)
{
    if (ESP8266_Cmd("AT+CIPSTATUS", "OK", 0, 500))
    {
        if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "STATUS:2\r\n"))
            return 2;

        else if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "STATUS:3\r\n"))
            return 3;

        else if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "STATUS:4\r\n"))
            return 4;
    }

    return 0;
}

/**
 * @brief :获取 WF-ESP8266 的端口（Id）连接状态，较适合多端口时使用
 * @return 端口（Id）的连接状态，低5位为有效位，分别对应Id5~0，某位若置1表该Id建立了连接，若被清0表该Id未建立连接
 */
uint8_t ESP8266_Get_IdLinkStatus(void)
{
    uint8_t ucIdLinkStatus = 0x00;

    if (ESP8266_Cmd("AT+CIPSTATUS", "OK", 0, 500))
    {
        if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+CIPSTATUS:0,"))
            ucIdLinkStatus |= 0x01;
        else
            ucIdLinkStatus &= ~0x01;

        if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+CIPSTATUS:1,"))
            ucIdLinkStatus |= 0x02;
        else
            ucIdLinkStatus &= ~0x02;

        if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+CIPSTATUS:2,"))
            ucIdLinkStatus |= 0x04;
        else
            ucIdLinkStatus &= ~0x04;

        if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+CIPSTATUS:3,"))
            ucIdLinkStatus |= 0x08;
        else
            ucIdLinkStatus &= ~0x08;

        if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+CIPSTATUS:4,"))
            ucIdLinkStatus |= 0x10;
        else
            ucIdLinkStatus &= ~0x10;
    }

    return ucIdLinkStatus;
}

/**
 * @brief : 获取 WIFI-ESP8266 的 AP IP
 * @param {char} *pApIp
 * @param {uint8_t} ucArrayLength
 * @return 1，获取成功
 *         0，获取失败
 */
uint8_t ESP8266_Inquire_ApIp(char *pApIp, uint8_t ucArrayLength)
{
    char uc;

    char *pCh;

    ESP8266_Cmd("AT+CIFSR", "OK", 0, 500);

    pCh = strstr(strEsp8266_Fram_Record.Data_RX_BUF, "APIP,\"");

    if (pCh)
        pCh += 6;

    else
        return 0;

    for (uc = 0; uc < ucArrayLength; uc++)
    {
        pApIp[uc] = *(pCh + uc);

        if (pApIp[uc] == '\"')
        {
            pApIp[uc] = '\0';
            break;
        }
    }

    return 1;
}

/**
 * @brief : 配置WF-ESP8266模块进入透传发送
 * @return 1，配置成功
 *         0，配置失败
 */
bool ESP8266_UnvarnishSend(void)
{
    if (!ESP8266_Cmd("AT+CIPMODE=1", "OK", 0, 500))
        return false;

    return ESP8266_Cmd("AT+CIPSEND", "OK", ">", 500);
}

/**
 * @brief : 配置WF-ESP8266模块退出透传模式
 * @return {*}
 */
void ESP8266_ExitUnvarnishSend(void)
{

    delay_ms(1000);

    ESP8266_Usart("+++");

    delay_ms(500);
}

/**
 * @brief :WF-ESP8266模块发送字符串
 * @param {FunctionalState} enumEnUnvarnishTx 声明是否已使能了透传模式
 * @param {char} *pStr 要发送的字符串
 * @param {uint32_t} ulStrLength 要发送的字符串的字节数
 * @param {ENUM_ID_NO_TypeDef} ucId 哪个ID发送的字符串
 * @return 1，发送成功
 *         0，发送失败
 */
bool ESP8266_SendString(FunctionalState enumEnUnvarnishTx, char *pStr, uint32_t ulStrLength, ENUM_ID_NO_TypeDef ucId)
{
    char cStr[20];
    bool bRet = false;

    if (enumEnUnvarnishTx)
    {
        ESP8266_Usart("%s", pStr);
        bRet = true;
    }

    else
    {
        if (ucId < 5)
            sprintf(cStr, "AT+CIPSEND=%d,%d", ucId, ulStrLength + 2);

        else
            sprintf(cStr, "AT+CIPSEND=%d", ulStrLength + 2);

        ESP8266_Cmd(cStr, "> ", 0, 100);

        bRet = ESP8266_Cmd(pStr, "SEND OK", 0, 500);
    }

    return bRet;
}

/**
 * @brief : WF-ESP8266模块接收字符串
 * @param {FunctionalState} enumEnUnvarnishTx 声明是否已使能了透传模式
 * @return 接收到的字符串首地址
 */
char *ESP8266_ReceiveString(FunctionalState enumEnUnvarnishTx)
{
    char *pRecStr = 0;

    strEsp8266_Fram_Record.InfBit.FramLength = 0;
    strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;

    while (!strEsp8266_Fram_Record.InfBit.FramFinishFlag)
        ;
    strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';

    if (enumEnUnvarnishTx)
        pRecStr = strEsp8266_Fram_Record.Data_RX_BUF;

    else
    {
        if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+IPD"))
            pRecStr = strEsp8266_Fram_Record.Data_RX_BUF;
    }

    return pRecStr;
}
