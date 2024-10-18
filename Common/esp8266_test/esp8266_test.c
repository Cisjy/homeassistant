#include "esp8266_test.h"
#include "my_string.h"
volatile uint8_t ucTcpClosedFlag = 0;
extern __IO uint8_t oled_comfirm_flag;

/**
 * @brief display the time out message on the oled screen
 * @param {char} *str
 * @param {uint8_t} times
 * @return {uint8_t} 1: cancel 2:timeout
 */
HAL_StatusTypeDef ESP8266_time_out_oled_display(char *str, uint8_t times)
{
    if (times == 1)
        oled_handle.ClearLine(&oled_handle, 3);
    oled_handle.ShowString(&oled_handle, 3, 1, str);
    oled_handle.ShowNum(&oled_handle, 3, strlen(str) + 1, times, 2);
    if (oled_comfirm_flag & 0x04)
    {
        oled_handle.ClearLine(&oled_handle, 3);
        oled_handle.ShowString(&oled_handle, 3, 1, "cancel");
        osDelay(3000);
        oled_comfirm_flag = 0;
        return HAL_BUSY;
    }
    if (times > 100)
    {

        oled_handle.ClearLine(&oled_handle, 3);
        oled_handle.ShowString(&oled_handle, 3, 1, "config timeout");
        osDelay(3000);
        return HAL_TIMEOUT;
    }
    return HAL_OK;
}

uint8_t ESP8266_StaTcpClient_Unvarnish_ConfigTest(void)
{
    uint8_t count, status;
    printf("正在配置 ESP8266 ......\r\n");
    if (is_bluetooth_debug)
    {
        USART2_printf("正在配置 ESP8266 ......\r\n");
    }
    count = 0;
    while (!ESP8266_AT_Test())
    {
        count++;
        status = ESP8266_time_out_oled_display("Cig ATtest:", count);
        if (status == HAL_BUSY)
        {
            return HAL_BUSY;
        }
        else if (status == HAL_TIMEOUT)
        {
            return HAL_TIMEOUT;
        }
    };
    count = 0;
    printf("正在配置工作模式 STA ......\r\n");
    if (is_bluetooth_debug)
    {
        USART2_printf("正在配置工作模式 STA ......\r\n");
    }
    while (!ESP8266_Net_Mode_Choose(STA_AP))
    {
        count++;
        status = ESP8266_time_out_oled_display("Cig NetMode:", count);
        if (status == HAL_BUSY)
        {
            return HAL_BUSY;
        }
        else if (status == HAL_TIMEOUT)
        {
            return HAL_TIMEOUT;
        }
    }
    count = 0;

    printf("正在连接 WiFi ......\r\n");
    if (is_bluetooth_debug)
    {
        USART2_printf("正在连接 WiFi ......\r\n");
    }
    while (!(wifi_flag ? ESP8266_JoinAP(macUser_ESP8266_ApSsid2, macUser_ESP8266_ApPwd2) : ESP8266_JoinAP(macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd)))
    {
        count++;
        status = ESP8266_time_out_oled_display("Cig JoinAP:", count);
        if (status == HAL_BUSY)
        {
            return HAL_BUSY;
        }
        else if (status == HAL_TIMEOUT)
        {
            return HAL_TIMEOUT;
        }
    }

    count = 0;
    printf("禁止多连接 ......\r\n");
    if (is_bluetooth_debug)
    {
        USART2_printf("禁止多连接 ......\r\n");
    }
    while (!ESP8266_Enable_MultipleId(DISABLE))
    {
        count++;
        status = ESP8266_time_out_oled_display("Cig MulId:", count);
        if (status == HAL_BUSY)
        {
            return HAL_BUSY;
        }
        else if (status == HAL_TIMEOUT)
        {
            return HAL_TIMEOUT;
        }
    }
    count = 0;
    printf("正在连接 Server ......\r\n");
    if (is_bluetooth_debug)
    {
        USART2_printf("正在连接 Server ......\r\n");
    }
    while (!ESP8266_Link_Server(enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0))
    {
        count++;
        status = ESP8266_time_out_oled_display("Cig LinkServer:", count);
        if (status == HAL_BUSY)
        {
            return HAL_BUSY;
        }
        else if (status == HAL_TIMEOUT)
        {
            return HAL_TIMEOUT;
        }
    }
    count = 0;
    printf("进入透传发送模式 ......\r\n");
    if (is_bluetooth_debug)
    {
        USART2_printf("进入透传发送模式 ......\r\n");
    }
    while (!ESP8266_UnvarnishSend())
    {
        count++;
        status = ESP8266_time_out_oled_display("Cig Unvarnish:", count);
        if (status == HAL_BUSY)
        {
            return HAL_BUSY;
        }
        else if (status == HAL_TIMEOUT)
        {
            return HAL_TIMEOUT;
        }
    }

    printf("配置 ESP8266 完毕\r\n");
    printf("开始透传......\r\n");
    if (is_bluetooth_debug)
    {
        USART2_printf("配置 ESP8266 完毕\r\n");
        USART2_printf("开始透传......\r\n");
    }
    return HAL_OK;
}

void ESP8266_ExitUnvarnishSend_QuitAP(void)
{
    ESP8266_ExitUnvarnishSend(); // 退出透传模式
    ESP8266_QuitAP();            // 退出AP模式
}

/**
 * @brief  ESP8266 检查是否接收到了数据，检查连接和掉线重连
 * @param  无
 * @retval 无
 */
void ESP8266_CheckRecvDataTest(void)
{
    uint8_t ucStatus;
    uint16_t i;
    char dest[512];

    /* 如果接收到了串口调试助手的数据 */
    if (strUSART_Fram_Record.InfBit.FramFinishFlag == 1)
    {
        for (i = 0; i < strUSART_Fram_Record.InfBit.FramLength; i++)
        {
            // USART_SendData(macESP8266_USARTx, strUSART_Fram_Record.Data_RX_BUF[i]); // 转发给ESP82636
            HAL_UART_Transmit(&huart2, (uint8_t *)strUSART_Fram_Record.Data_RX_BUF[i], 1, 1000);
            // while (USART_GetFlagStatus(macESP8266_USARTx, USART_FLAG_TC) == RESET)
            // {
            // } // 等待发送完成
        }
        strUSART_Fram_Record.InfBit.FramLength = 0;     // 接收数据长度置零
        strUSART_Fram_Record.InfBit.FramFinishFlag = 0; // 接收标志置零
    }
    memset(dest, 0, 512);
    /* 如果接收到了ESP8266的数据 */
    if (strEsp8266_Fram_Record.InfBit.FramFinishFlag)
    {
        HexToAscii((uint8_t *)strEsp8266_Fram_Record.Data_RX_BUF, dest, 30, ADD_SPACE_AND_0X);
        printf("%s\n", dest);
        //		printf("%s\n", strEsp8266_Fram_Record .Data_RX_BUF);
        strEsp8266_Fram_Record.InfBit.FramLength = 0;     // 接收数据长度置零
        strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0; // 接收标志置零
    }

    if (ucTcpClosedFlag) // 检测是否失去连接
    {
        ESP8266_ExitUnvarnishSend(); // 退出透传模式

        do
            ucStatus = ESP8266_Get_LinkStatus(); // 获取连接状态
        while (!ucStatus);

        if (ucStatus == 4) // 确认失去连接后重连
        {
            printf("\r\n正在重连热点和服务器 ......\r\n");

            while (!ESP8266_JoinAP(macUser_ESP8266_ApSsid, macUser_ESP8266_ApPwd))
                ;

            while (!ESP8266_Link_Server(enumTCP, macUser_ESP8266_TcpServer_IP, macUser_ESP8266_TcpServer_Port, Single_ID_0))
                ;

            printf("\r\n重连热点和服务器成功\r\n");
        }

        while (!ESP8266_UnvarnishSend())
            ;
    }
}
