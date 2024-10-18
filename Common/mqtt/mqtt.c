#include "mqtt.h"

// 待发送的报文存储区
static uint8_t mqtt_txbuf[MAX_BUF_SIZE];

// 接收到的报文存储区
uint8_t mqtt_rxbuf[MAX_BUF_SIZE];

// 存储字符串格式的报文
static uint8_t buf[MAX_BUF_SIZE];

static uint16_t new_pid = 0; // 报文标识符
static int ThemeNum = 0;     // 当前订阅的主题数目
struct ThemeInfo themes[MAX_THEME_NUM];
extern void MQTT_SendMsg(uint8_t *mqtt_txbuf, uint8_t len);
/**
 * @brief  往发送的报文中插入剩余长度
 *
 * @param  mqtt_txbuf: 要插入剩余长度的报文缓存区
 * @param  cps_len: 补偿长度（使用这个位通常是报文最后面是0，要额外加上去）//经过测试，这个参数只能为0
 *
 * @retval 返回报文总长度
 */
static int AddRemainingLength(void *mqtt_txbuf, uint8_t cps_len)
{
    uint8_t *txbuf = mqtt_txbuf;
    uint8_t last = 1, cur = 0, next_1 = 0, next_2 = 0;
    int src_len = -1;
    int i;
    int remain_len, remain_num;
    for (i = 0;; i++)
    {
        if (i)
            last = cur;
        cur = *(txbuf + i);
        next_1 = *(txbuf + i + 1);
        next_2 = *(txbuf + i + 2);
        if (last == 0 && cur == 0 && next_1 == 0 && next_2 == 0)
            break;
        else
            src_len++;
    }
    remain_len = src_len + cps_len - 1;
    // 将剩余长度值插入进发送数组中
    if (remain_len <= 127)
        remain_num = 1;
    else if (remain_len <= 128 && remain_len >= 16383)
        remain_num = 2;

    for (i = src_len; i > 0; i--)
        txbuf[i + remain_num] = txbuf[i];
    switch (remain_num)
    {
    case 1:
        txbuf[1] = remain_len;
        break;
    case 2:
        txbuf[1] = 0x80 | (remain_len / 128);
        txbuf[2] = remain_len % 128;
        break;
    }
    return (remain_len + remain_num + 1);
}
/**
 * @brief  生成固定报头
 *
 * @param  MesType: mqtt报文类型（详见mqtt.h）
 * @param	DupFlag: 重发标志
 *		@arg	0: 客户端或服务端第一次请求发送这个报文
 *		@arg	1: 可能是一个早前报文请求的重发
 * @param	QosLevel: Qos等级
 * @param	Retain: 保留标志（设置后在订阅了该主题后马上接收到一条该主题的信息）
 *		@arg	0: 不发布保留消息
 *		@arg	1: 发布保留消息
 *
 * @retval 返回固定报头（8bit/1位）
 */
uint8_t SendFixedHead(unsigned char MesType, unsigned char DupFlag, unsigned char QosLevel, unsigned char Retain)
{
    // 7-4位报文类型,3-0位
    uint8_t data = 0;
    data = (MesType & 0x0f) << 4;
    data |= (DupFlag & 0x01) << 3;
    data |= (QosLevel & 0x03) << 1;
    data |= (Retain & 0x01);

    return data;
}
/**
 * @brief  获取连接的数据包
 *
 * @param  None
 *
 * @retval 返回连接返回码
 *		@arg	Connect_Accept: 					连接已接受
 *		@arg	Connect_Refuse_Version:				连接已拒绝，不支持的协议版本
 *		@arg	Connect_Refuse_ClientId:			连接已拒绝，不合格的客户端标识符
 *		@arg	Connect_Refuse_Sever_Unavailable:	连接已拒绝，服务端不可用
 *		@arg	Connect_Refuse_Acc_Or_Pass:			连接已拒绝，无效的用户名或密码
 *		@arg	Connect_Refuse_Fail_Respond:		连接已拒绝，服务器未响应
 */
uint8_t SendCONNECT(void)
{
    uint32_t i, len, lennum = 0;
    uint8_t *msg;
    memset(Rx_Buffer, 0, Rx_Buffer_Len);
    /*1、固定报头 */
    mqtt_txbuf[0] = SendFixedHead(MQTT_TypeCONNECT, 0, 0, 0);
    // 固定报头共2个字节，一个字节占8位。第一个字节前4位是用于指定控制报文类型的标志位，后4位是控制报文类型；第二个字节是剩余长度。
    // 第二个字节的剩余长度最后再添加
    /*2、可变报头  */
    // （1）协议名
    mqtt_txbuf[1] = 0x00; // 报文标识符MSB
    mqtt_txbuf[2] = 0x04; // 报文标识符LSB
    mqtt_txbuf[3] = 'M';
    mqtt_txbuf[4] = 'Q';
    mqtt_txbuf[5] = 'T';
    mqtt_txbuf[6] = 'T';
    // （2）协议级别
    mqtt_txbuf[7] = 0x04;
    // （3）连接标志
    mqtt_txbuf[8] = 0 | (MQTT_StaCleanSession << 1) | (MQTT_StaWillFlag << 2) | (MQTT_StaWillQoS << 3) | (MQTT_StaWillRetain << 5) | (MQTT_StaPasswordFlag << 6) | (MQTT_StaUserNameFlag << 7);
    // （4）保持连接
    mqtt_txbuf[9] = MQTT_KeepAlive >> 8;
    mqtt_txbuf[10] = MQTT_KeepAlive;
    /* 3、有效载荷 */
    // 3.1客户端标识符ID
    len = strlen(MQTT_ClientIdentifier);
    mqtt_txbuf[11] = len >> 8;
    mqtt_txbuf[12] = len;
    msg = (uint8_t *)MQTT_ClientIdentifier;
    for (i = 0; i < len; i++)
    {
        mqtt_txbuf[13 + i] = msg[i];
    }
    lennum += len;
    // 3.2 遗嘱主题
#if (MQTT_StaWillFlag == 1)
    char *will_topic = MQTT_WillTopic;
    len = strlen(will_topic);
    mqtt_txbuf[12 + lennum + 1] = len >> 8;
    mqtt_txbuf[12 + lennum + 2] = len;
    lennum += 2;
    msg = (uint8_t *)will_topic;
    for (i = 0; i < len; i++)
    {
        mqtt_txbuf[13 + lennum + i] = msg[i];
    }
    lennum += len;
    // 3.3 遗嘱消息
    char *will_message = MQTT_WillMessage;
    len = strlen(will_message);
    mqtt_txbuf[12 + lennum + 1] = len >> 8;
    mqtt_txbuf[12 + lennum + 2] = len;
    lennum += 2;
    msg = (uint8_t *)will_message;
    for (i = 0; i < len; i++)
    {
        mqtt_txbuf[13 + lennum + i] = msg[i];
    }
    lennum += len;
#endif
    // 3.4 用户名
#if (MQTT_StaUserNameFlag == 1)
    len = strlen(MQTT_UserName);
    mqtt_txbuf[12 + lennum + 1] = len >> 8;
    mqtt_txbuf[12 + lennum + 2] = len;
    lennum += 2;
    msg = (uint8_t *)MQTT_UserName;
    for (i = 0; i < len; i++)
    {
        mqtt_txbuf[13 + lennum + i] = msg[i];
    }
    lennum += len;
#endif
    // 3.5 密码
#if (MQTT_StaPasswordFlag == 1)
    len = strlen(MQTT_Password);
    mqtt_txbuf[12 + lennum + 1] = len >> 8;
    mqtt_txbuf[12 + lennum + 2] = len;
    lennum += 2;
    msg = (uint8_t *)MQTT_Password;
    for (i = 0; i < len; i++)
    {
        mqtt_txbuf[13 + lennum + i] = msg[i];
    }
    lennum += len;
#endif
    /* 1、固定报头（补剩余长度值） */
    len = AddRemainingLength(mqtt_txbuf, 0);
    /* 将 CONNECT 报文发送 */
    MQTT_SendMsg(mqtt_txbuf, len);

    int ret = MQTT_ReceiveMsg(MQTT_TypeCONNECT, mqtt_rxbuf);
    if (ret < 0)
    {
        ret = Connect_Refuse_Fail_Respond;
    }
    else
    {
        ret = mqtt_rxbuf[3];
    }
#if (DEBUG_1 == 1)
    switch (ret)
    {
    case Connect_Accept:
        printf("\"%s\" 连接成功!\n", MQTT_ClientIdentifier);
        break;
    case Connect_Refuse_Version:
        printf("\"%s\" 连接已拒绝，不支持的协议版本\n", MQTT_ClientIdentifier);
        break;
    case Connect_Refuse_ClientId:
        printf("\"%s\" 连接已拒绝，不合格的客户端标识符\n", MQTT_ClientIdentifier);
        break;
    case Connect_Refuse_Sever_Unavailable:
        printf("\"%s\" 连接已拒绝，服务端不可用\n", MQTT_ClientIdentifier);
        break;
    case Connect_Refuse_Acc_Or_Pass:
        printf("\"%s\" 连接已拒绝，无效的用户名或密码\n", MQTT_ClientIdentifier);
        break;
    case Connect_Refuse_Fail_Respond:
        printf("\"%s\" 连接超时，服务器未响应\n", MQTT_ClientIdentifier);
        break;
    }
#endif
    return ret;
}

/**
 * @brief 发送订阅消息的 SUBSCRIBE 报文
 * @param  topic: 想要订阅的主题
 * @param  RequestedQoS: 服务质量等级 Qos
 * @retval >=0: 订阅成功，具体返回订阅的 Qos值 	-1：订阅失败
 *
 */
int SendSUBSCRIBE(const char *topic, unsigned char RequestedQoS)
{
    unsigned int i, len = 0, lennum = 0;
    uint8_t cps_len = 0;

    /* 1、固定报头 */
    // 高四位值必须为8,第3,2,1,0位是保留位，必须分别设置为0,0,1,0
    mqtt_txbuf[0] = 0x82;

    /* 2、可变报头 */
    // 2.1 报文标识符（如果一个客户端要重发这个特殊的控制报文，在随后重发那个报文时，它必须使用相同的标识符）
    // 校验输入的报文标识符是否符合要求
    if (RequestedQoS == 0)
        cps_len = 1;
    else if (RequestedQoS > 0)
        new_pid++;
    mqtt_txbuf[1] = new_pid >> 8;
    mqtt_txbuf[2] = new_pid;

    /* 3、有效载荷 */
    // 3.1 想要订阅的主题（符合主题过滤器的要求，即主题中含有'/'的分级符）
    len = strlen(topic);
    mqtt_txbuf[3] = len >> 8;
    mqtt_txbuf[4] = len;
    for (i = 0; i < len; i++)
        mqtt_txbuf[5 + i] = topic[i];
    lennum = len;
    // 3.2 服务质量要求（Requested QoS）
    mqtt_txbuf[5 + lennum] = RequestedQoS;

    /* 1、固定报头（补剩余长度值） */
    len = AddRemainingLength(mqtt_txbuf, cps_len);

    /* 发送 SUBSCRIBE 报文 */
    MQTT_SendMsg(mqtt_txbuf, len);

    /* 接收发送回来的报文，判断是否订阅成功 */
    MQTT_ReceiveMsg(MQTT_TypeSUBSCRIBE, mqtt_rxbuf);
    if (Get_Fixed_Header_Type(mqtt_rxbuf) == MQTT_TypeSUBACK && mqtt_rxbuf[4] != 0x80)
    {
        themes[ThemeNum].ThemeName = (char *)topic;
        themes[ThemeNum].ThemeMsg = NULL;
        ThemeNum++;
#if (DEBUG_1 == 1)
        printf("\"%s\" 主题订阅成功!\n", themes[ThemeNum - 1].ThemeName);

#endif
        return RequestedQoS;
    }
    else
        return -1;
}

/**
 * @brief  接收到 Qos = 2 的 PUBLISH 报文，需要回复 PUBREC 报文，
 *		  并且要接收对方发送的 PUBREL 报文
 *
 * @param  pid：报文标识符，要与 Qos = 2 的 PUBLISH 报文标识符一致
 *
 * @retval 0：对方成功接收	-1：对方接收失败
 */
static int SendPUBREC(uint16_t pid)
{
    /* 1、固定报头 */
    mqtt_txbuf[0] = SendFixedHead(MQTT_TypePUBREC, 0, 0, 0);
    mqtt_txbuf[1] = 2;

    /* 2、可变报头 */
    // 2.1 报文标识符
    mqtt_txbuf[2] = pid >> 8;
    mqtt_txbuf[3] = pid;

    /* 发送 PUBREC 报文 */
    MQTT_SendMsg(mqtt_txbuf, 2 + mqtt_txbuf[1]);
    /* 接收发送回来的 PUBREL 报文，判断是否发布成功 */
    MQTT_ReceiveMsg(MQTT_TypePUBREC, mqtt_rxbuf);
    if ((mqtt_rxbuf[0] >> 4) == MQTT_TypePUBREL) // 处理报文标识符
        return 0;
    else
        return -1;
}

/**
 * @brief  接收到 Qos = 2 的 PUBREC 报文，需要回复 PUBREL 报文，
 *		  并且要接收对方发送的 PUBCOMP 报文
 *
 * @param  pid：报文标识符，要与 Qos = 2 的 PUBLISH 报文标识符一致
 *
 * @retval 0：对方成功接收	-1：对方接收失败
 */
static int SendPUBREL(uint16_t pid)
{
    /* 1、固定报头 */
    mqtt_txbuf[0] = SendFixedHead(MQTT_TypePUBREL, 0, 1, 0);
    mqtt_txbuf[1] = 2;

    /* 2、可变报头 */
    // 2.1 报文标识符
    mqtt_txbuf[2] = pid >> 8;
    mqtt_txbuf[3] = pid;

    /* 发送 PUBREL 报文 */
    MQTT_SendMsg(mqtt_txbuf, 2 + mqtt_txbuf[1]);
    /* 接收发送回来的报文，判断是否发布成功 */
    MQTT_ReceiveMsg(MQTT_TypePUBREL, mqtt_rxbuf);
    if ((mqtt_rxbuf[0] >> 4) == MQTT_TypePUBCOMP) // 处理报文标识符
        return 0;
    else
        return -1;
}

/**
 * @brief  接收到 Qos = 2 的 PUBREL 报文，需要回复 PUBCOMP 报文
 *
 * @param  pid：报文标识符，要与 Qos = 2 的 PUBLISH 报文标识符一致
 *
 * @retval none
 */
static void SendPUBCOMP(uint16_t pid)
{
    /* 1、固定报头 */
    mqtt_txbuf[0] = SendFixedHead(MQTT_TypePUBCOMP, 0, 0, 0);
    mqtt_txbuf[1] = 2;

    /* 2、可变报头 */
    // 2.1 报文标识符
    mqtt_txbuf[2] = pid >> 8;
    mqtt_txbuf[3] = pid;

    /* 发送 PUBCOMP 报文 */
    MQTT_SendMsg(mqtt_txbuf, 2 + mqtt_txbuf[1]);
}

/**
 * @brief  接收到 Qos = 1 的 PUBLISH 报文，需要回复 PUBACK 报文
 *
 * @param  pid：报文标识符，要与 Qos = 1 的 PUBLISH 报文标识符一致
 *
 * @retval none
 */
static void SendPUBACK(uint16_t pid)
{
    /* 1、固定报头 */
    mqtt_txbuf[0] = SendFixedHead(MQTT_TypePUBACK, 0, 0, 0);
    mqtt_txbuf[1] = 2;

    /* 2、可变报头 */
    // 2.1 报文标识符
    mqtt_txbuf[2] = pid >> 8;
    mqtt_txbuf[3] = pid;

    /* 发送 PUBACK 报文 */
    MQTT_SendMsg(mqtt_txbuf, 2 + mqtt_txbuf[1]);
}

/**
 * @brief  获取发布消息的数据包
 *
 * @param	dup: 重发标志
 *		@arg	0: 客户端或服务端第一次请求发送这个报文
 *		@arg	1: 可能是一个早前报文请求的重发
 * @param	qos: Qos等级
 * @param	retain: 保留标志（设置后在订阅了该主题后马上接收到一条该主题的信息）
 *		@arg	0: 不发布保留消息
 *		@arg	1: 发布保留消息
 * @param  topic: 主题名
 * @param  msg: 待发布的消息
 *
 * @retval 0: 订阅成功	-1：订阅失败
 */
int SendPUBLISH(unsigned char dup, unsigned char qos, unsigned char retain, const char *topic, const char *msg)
{
    unsigned int i, len = 0, lennum = 0;
    uint8_t cps_len = 0;

    /* 1、固定报头 */
    mqtt_txbuf[0] = SendFixedHead(MQTT_TypePUBLISH, dup, qos, retain);

    /* 2、可变报头 */
    // 2.1 主题名
    len = strlen(topic);
    mqtt_txbuf[1] = len >> 8;
    mqtt_txbuf[2] = len;
    for (i = 0; i < len; i++)
        mqtt_txbuf[3 + i] = topic[i];
    lennum = len;

    // 2.2 报文标识符
    // 校验输入的报文标识符是否符合要求

    if (qos > 0)
    {
        new_pid++;
        mqtt_txbuf[2 + lennum + 1] = new_pid >> 8;
        mqtt_txbuf[2 + lennum + 2] = new_pid;
        lennum += 2;
    }

    /* 3、有效载荷 */
    // 3.1 消息
    len = strlen(msg);
    for (i = 0; i < len; i++)
        mqtt_txbuf[3 + i + lennum] = msg[i];
    lennum += len;
    /* 1、固定报头（补剩余长度值） */
    len = AddRemainingLength(mqtt_txbuf, cps_len);
    /* 发送 PUBLISH 报文 */
    MQTT_SendMsg(mqtt_txbuf, len);

    if (qos == 0)
    {
        // printf("qos == 0\r\n");
        return 0;
    }
    else
    {
        /* 接收发送回来的 PUBACK/PUBREC 报文，判断是否发布成功（Qos > 0 才会有返回报文） */
        MQTT_ReceiveMsg(MQTT_TypePUBLISH, mqtt_rxbuf);
        if (mqtt_rxbuf[2] == 0 && mqtt_rxbuf[3] == new_pid) // 处理报文标识符
        {
            // 接收到的是 PUBREC 报文
            if (Get_Fixed_Header_Type(mqtt_rxbuf) == MQTT_TypePUBREC)
            {
                if (SendPUBREL(new_pid) < 0)
                    return -1;
                else
                    return 0;
            }
            // 接收到的是 PUBACK 报文
            else if (Get_Fixed_Header_Type(mqtt_rxbuf) == MQTT_TypePUBACK)
                return 0;
            else
                return -1;
        }
        else
            return -1;
    }
}

/**
 * @brief  发送断开连接的数据包
 *
 * @param  mqtt_txbuf: 存储待发送报文的数组
 *
 * @retval None
 */
void SendDISCONNECT(void)
{
    mqtt_txbuf[0] = 0xe0;
    mqtt_txbuf[1] = 0x00;

    /* 发送 DISCONNECT 报文 */
    MQTT_SendMsg(mqtt_txbuf, 2);
}

/**
 * @brief  发送心跳请求
 *
 * @param  None
 *
 * @retval 0：与MQTT服务器通讯正常	 	-1：通讯可能断开，可以多发几次确认一下
 */
int SendPINGREQ(void)
{
    mqtt_txbuf[0] = 0xc0;
    mqtt_txbuf[1] = 0x00;

    /* 发送 PINGREQ 报文 */
    MQTT_SendMsg(mqtt_txbuf, 2);
    /* 接收心跳响应报文 PINGRESP */
    MQTT_ReceiveMsg(MQTT_TypePINGREQ, mqtt_rxbuf);
    if (Get_Fixed_Header_Type(mqtt_rxbuf) == MQTT_TypePINGRESP)
        return 0;
    else
        return -1;
}

/**
 * @brief  通过串口发送给esp8266，然后由esp8266将报文发送给MQTT服务器
 *
 * @param  mqtt_txbuf: 存储待发送报文的数组
 *
 * @retval None
 */
void MQTT_SendMsg(uint8_t *mqtt_txbuf, uint8_t len)
{
    // 将报文通过8266传输至mqtt服务器
    for (int i = 0; i < len; i++)
        // Send_Message(ESP8266_USART, mqtt_txbuf[i]);
        HAL_UART_Transmit(&huart3, &mqtt_txbuf[i], 1, 1000);

    // #if ((DEBUG_1 == 1) && (DEBUG_2 == 1))
    //     // 将发送的报文显示在串口调试助手上
    //     HexToAscii(mqtt_txbuf, buf, len, ADD_SPACE_AND_0X);
    //     printf("send post: %s\n", buf);
    // #endif

    // 发送完成后顺便会将 mqtt_txbuf 数组清空
    memset(mqtt_txbuf, 0, MAX_BUF_SIZE);
}

/**
 * @brief  保存接收到的 PUBLISH 报文中的消息
 *
 * @param  len：PUBLISH 报文长度
 *
 * @retval 返回报文标识符的索引
 */
static int SaveReceiveBuf(uint8_t len)
{
    uint8_t i, j = 0, k;
    uint8_t themeName[128] = {0};
    uint8_t *themeMsg = NULL;
    char themeNameFlag = 0, themeTagIndex = 0;
    int packet_identifier = 0;

    for (i = 4; i < len; i++)
    {
        // 剔除主题名字
        if (themeNameFlag == 0)
        {
            // 主题名字在报文的第五个字节开始
            themeName[j++] = mqtt_rxbuf[i];

            for (k = 0; k < ThemeNum; k++)
            {
                // 逐步提取出全部的主题名字
                if (strcmp((char *)themeName, themes[k].ThemeName) == 0)
                {
                    themeNameFlag = 1; // 找到主题名称
                    themeTagIndex = k;

                    // 仅当尚未分配主题消息内存时才进行分配
                    if (themes[themeTagIndex].ThemeMsg == NULL)
                    {
                        // printf("Before malloc, Free Heap Size: %d bytes\n", xPortGetFreeHeapSize());
                        themeMsg = pvPortMalloc(sizeof(uint8_t) * (len - 4 - j + 1 + 4));
                        if (themeMsg == NULL)
                        {
                            // 处理内存分配失败的情况
                            printf("Memory allocation failed!\n");
                            return -1;
                        }
                        // printf("After malloc, Free Heap Size: %d bytes\n", xPortGetFreeHeapSize());
                    }
                    packet_identifier = 4 + j;
                    j = 0;
                    break; // 找到后退出
                }
            }
        }
        else
        {
            themeMsg[j++] = mqtt_rxbuf[i]; // 填充消息
        }
    }

    // 只有在主题消息已分配时才进行字符串终止
    if (themeMsg != NULL)
    {
        themeMsg[j] = '\0'; // 添加字符串结束符

        // 将剔除主题名字后剩余的消息存储起来
        themes[themeTagIndex].ThemeMsg = themeMsg;
    }

    return packet_identifier;
}

// static int SaveReceiveBuf(uint8_t len)
// {
//     uint8_t i, j = 0, k;
//     uint8_t themeName[128] = {0};
//     uint8_t *themeMsg = NULL;
//     char themeNameFlag = 0, themeTagIndex = 0;
//     int packet_identifier = 0;

//     for (i = 4, j = 0; i < len; i++)
//     {
//         // 剔除主题名字
//         if (themeNameFlag == 0)
//         {
//             // 主题名字在报文的第五个字节开始
//             themeName[j++] = mqtt_rxbuf[i];

//             for (k = 0; k < ThemeNum; k++)
//             {
//                 // 逐步提取出全部的主题名字
//                 if (strcmp((char *)themeName, themes[k].ThemeName) == 0)
//                 {
//                     themeNameFlag = 1;
//                     themeTagIndex = k;

//                     printf("Before malloc, Free Heap Size: %d bytes\n", xPortGetFreeHeapSize());
//                     themeMsg = pvPortMalloc(sizeof(uint8_t) * (len - 4 - j + 1 + 4));
//                     printf("After malloc, Free Heap Size: %d bytes\n", xPortGetFreeHeapSize());
//                     printf("ThemeNum:%d i:%d K:%d j:%d\r\n", ThemeNum, i, k, j);
//                     // themeMsg = malloc(sizeof(uint8_t) * (len - 4 - j + 1 + 4));
//                     packet_identifier = 4 + j;
//                     j = 0;
//                     break;
//                 }
//             }
//         }
//         else
//         {
//             themeMsg[j] = mqtt_rxbuf[i];
//             j++;
//         }
//     }
//     themeMsg[strlen((char *)themeMsg)] = '\0';

//     // 将剔除主题名字后剩余的消息存储起来
//     themes[themeTagIndex].ThemeMsg = themeMsg;

//     return packet_identifier;
// }

/**
 * @brief  从esp8266获取到mqtt服务器返回的报文
 *
 * @param  mqtt_msg_type: 在MQTT_SendMsg()函数中发送报文的类型（类型详见mqtt.h）
 * @param  mqtt_rxbuf: 存储从与esp8266连接的串口中暂存的报文
 *
 * @retval 0：有接收到数据			-1：无数据
 */
int MQTT_ReceiveMsg(uint8_t mqtt_msg_type, uint8_t *mqtt_rxbuf)
{
    uint8_t len = 0;
    uint8_t qos;
    uint16_t PBULISH_pid;

    delay_ms(500);

    /* 如果接收到了ESP8266的数据 */
    if (Rx_Finish_Flag)
    {
        // 根据发送的报文类型得到接收到的报文长度
        switch (mqtt_msg_type)
        {
        case MQTT_TypeCONNECT:     // 返回报文类型是 CONNACK
        case MQTT_TypePUBLISH:     // 返回报文类型是 PUBACK/PUBREC
        case MQTT_TypePUBREC:      // 返回报文类型是 PUBREL
        case MQTT_TypePUBREL:      // 返回报文类型是 PUBCOMP
        case MQTT_TypeUNSUBSCRIBE: // 返回报文类型是 UNSUBACK
            len = 4;
            break;
        case MQTT_TypeSUBSCRIBE: // 返回报文类型是 SUBACK
            len = 5;
            break;
        case MQTT_TypePINGREQ: // 返回报文类型是 PINGRESP
            len = 2;
            break;
        case MQTT_WriteMsg: // 等待接收订阅的消息
            len = Count_Hex_Num(Rx_Buffer);
            break;
        default:
            break;
        }

        memset(mqtt_rxbuf, 0, MAX_BUF_SIZE);

        // #if ((DEBUG_1 == 1) && (DEBUG_2 == 1))
        //         // 将接收到的报文显示在串口调试助手上
        //         memset(buf, 0, MAX_BUF_SIZE);
        //         HexToAscii(Rx_Buffer, buf, len, ADD_SPACE_AND_0X);
        //         printf("接收报文: %s\n", buf);
        //         printf("接收报文长度: %d\n", len);
        // #endif

        // 将指定长度的报文保存至mqtt_rxbuf中
        memcpy(mqtt_rxbuf, Rx_Buffer, len);

        // 函数接收到了订阅的主题发送过来的消息报文
        if (mqtt_msg_type == MQTT_WriteMsg)
        {
            // 判断接收到的报文是什么类型的
            switch (Get_Fixed_Header_Type(mqtt_rxbuf))
            {
            // 接收到 PUBLISH 报文
            case MQTT_TypePUBLISH:
                // 先保存发送过来的消息内容
                PBULISH_pid = Get_Packet_Identifier(mqtt_rxbuf, SaveReceiveBuf(len));
                // 提取固定报头中的 qos 等级
                qos = Get_Fixed_Header_Qos(mqtt_rxbuf);
                // qos = 1：PUBLISH -> PUBACK
                if (qos == 1)
                    SendPUBACK(PBULISH_pid);
                // qos = 2：PUBLISH -> PUBREC PUBREL -> PUBCOMP
                else if (qos == 2)
                {
                    /* 我也不知道自己那个服务器怎么回事，
                    第一次发送 PUBREC 报文返回的好像还是 PUBLISH 报文，
                    所以只能再发一次就好了 */
                    //
                    // SendPUBREC(PBULISH_pid);

                    // delay_ms(100);

                    SendPUBREC(PBULISH_pid);

                    delay_ms(100);

                    SendPUBCOMP(PBULISH_pid);
                }
                break;
            default:
                break;
            }
        }
        Rx_Buffer_Len = 0;                    // 接收数据长度置零
        Rx_Finish_Flag = 0;                   // 接收标志置零
        memset(Rx_Buffer, 0, RX_BUF_MAX_LEN); // 清空数据缓冲区
        return 0;
    }
    else
        return -1;
}

#if (DEBUG_1 == 1)
void PrintMsg(void)
{
    // 打印接收到的消息
    for (int i = 0; i < ThemeNum; i++)
    {
        if (themes[i].ThemeMsg != NULL)
        {
            printf("\"%s\" 主题发送：", themes[i].ThemeName);
            printf("%s\n", themes[i].ThemeMsg);
            if (is_bluetooth_debug)
            {
                USART2_printf("\"%s\" 主题发送：", themes[i].ThemeName);
                USART2_printf("%s\n", themes[i].ThemeMsg);
            }
            cJSON *json = cJSON_Parse(themes[i].ThemeMsg);
            if (json == NULL)
            {
                printf("JSON 解析失败！\n");
                return;
            }
            cJSON *led_item = cJSON_GetObjectItem(json, "led");
            cJSON *beep_item = cJSON_GetObjectItem(json, "beep");
            if (cJSON_IsNumber(led_item) || cJSON_IsNumber(beep_item))
            {
                switch (led_item->valueint)
                {
                case 0:
                    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
                    break;
                case 1:
                    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
                    break;
                default:
                    break;
                }
                switch (beep_item->valueint)
                {
                case 0:
                    beep_off();
                    break;
                case 1:
                    beep_on();
                    break;
                default:
                    break;
                }
            }
            else
            {
                printf("没有收到 led 或 beep 指令！\n");
                if (is_bluetooth_debug)
                {
                    USART2_printf("没有收到 led 或 beep 指令！\n");
                }
            }

            cJSON_Delete(json);
            // for (int j = 0; j < Count_Hex_Num(themes[i].ThemeMsg);)
            // {
            //     if (themes[i].ThemeMsg[j] >= 0xA1 && themes[i].ThemeMsg[j + 1] >= 0xA1)
            //     {
            //         printf("%c%c", themes[i].ThemeMsg[j], themes[i].ThemeMsg[j + 1]);
            //         j += 2;
            //     }
            //     else
            //     {
            //         printf("%c", themes[i].ThemeMsg[j]);
            //         j++;
            //     }
            // }
            // printf("\n");

            // printf("Before vPortFree, Free Heap Size: %d bytes\n", xPortGetFreeHeapSize());
            // vPortFree(themes[i].ThemeMsg);
            // printf("After vPortFree, Free Heap Size: %d bytes\n", xPortGetFreeHeapSize());

            // free(themes[i].ThemeMsg);
            themes[i].ThemeMsg = NULL;
        }
    }
}
#endif
