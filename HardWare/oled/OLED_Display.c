#include "OLED_Display.h"
#include <stdarg.h>
oled_page_t mainPage;
oled_page_t subPage1;
oled_page_t subPage1_1;
oled_page_t subPage2;
oled_page_t subPage3;
oled_page_t subPage4;
oled_page_t subPage4_3;

uint8_t wifi_flag = 0;
bool is_bluetooth_debug = false;
oled_page_t *oled_page = &mainPage;
__IO uint8_t oled_comfirm_flag = 0; // 0位 0：not in action，1:in action now
                                    // 1位: 1:confirm action
                                    // 2位: 1:cancel action
extern oledHandleTypeDef oled_handle;

static int get_decimal_length(int32_t number)
{
    if (number == 0)
    {
        return 1; // 0 的长度是 1
    }

    return (int)(log10(abs(number)) + 1);
}

///*****************自定义函数 代码冗余************************ */
void oled_show_string_lines(oledHandleTypeDef *oled_handle, uint8_t line_count, ...)
{
    va_list args;
    va_start(args, line_count);
    for (uint8_t i = 0; i < line_count; i++)
    {
        char *str = va_arg(args, char *);
        oled_handle->ShowString(oled_handle, i + 1, 1, str);
    }
    va_end(args);
}

//*******************ACTION*********************** */
// subpage1
TaskHandle_t oled_sub1_opt_task_handle = NULL;
TaskHandle_t MQTT_Receve_Msg_Task_Handle = NULL;
void oled_sub1_opt_task(void const *argument);
void MQTT_Receve_Msg_Task(void const *argument);

static void oled_sub1_opt2_action(uint8_t opt)
{
    oled_handle.Clear(&oled_handle);
#if OLED_DISPLAY_DEBUG
    printf("start oled_sub1_opt_task\r\n");
#endif
    // 创建task
    if (oled_sub1_opt_task_handle == NULL)
    {
        if (xTaskCreate((TaskFunction_t)oled_sub1_opt_task,
                        "oled_sub1_opt_task",
                        256,
                        (void *)opt,
                        1,
                        &oled_sub1_opt_task_handle) != pdTRUE)
        {
            printf("create oled_sub1_opt_task failed\r\n");
        }
    }

    if (MQTT_Receve_Msg_Task_Handle == NULL)
    {
        if (xTaskCreate((TaskFunction_t)MQTT_Receve_Msg_Task,
                        "MQTT_Receve_Msg_Task",
                        256,
                        NULL,
                        1,
                        &MQTT_Receve_Msg_Task_Handle) != pdTRUE)
        {
            printf("create MQTT_Receve_Msg_Task failed\r\n");
        }
    }
}
#if OLED_DISPLAY_DEBUG
void printHeapUsage()
{
    // 获取当前剩余的堆内存大小
    size_t freeHeapSize = xPortGetFreeHeapSize();
    // 获取自系统启动以来最小的剩余堆内存大小
    size_t minEverFreeHeapSize = xPortGetMinimumEverFreeHeapSize();

    // 打印信息
    printf("Current Free Heap Size: %u bytes\n", (unsigned int)freeHeapSize);
    printf("Minimum Ever Free Heap Size: %u bytes\n", (unsigned int)minEverFreeHeapSize);
}

#endif

static void oled_sub1_opt_task(void const *argument)
{
    float pit, rol, yaw;
    uint8_t receive_data[4] = {0};
    float lightintensity = 0.0;
    int update_count = 0, count = 0;
    while (1)
    {

        osDelay(100);

        if (MPU6050_dmp_get_data(&pit, &rol, &yaw) == 1)
        {
            continue;
        }
#if OLED_DISPLAY_DEBUG
        printHeapUsage();
#endif
        bh1750_handle.Light_Intensity(&bh1750_handle, &lightintensity);
        DHT11_Read_Data(receive_data);
        float humidity = (float)receive_data[0] + (float)receive_data[1] / 100;
        float temperature = (float)receive_data[2] + (float)receive_data[3] / 100;
        cJSON *root = cJSON_CreateObject();

        cJSON_AddNumberToObject(root, "index", count++);
        cJSON_AddNumberToObject(root, "pit", (int)(pit * 100) / 100.0); // 保留两位小数
        cJSON_AddNumberToObject(root, "rol", (int)(rol * 100) / 100.0);
        cJSON_AddNumberToObject(root, "yaw", (int)(yaw * 100) / 100.0);
        cJSON_AddNumberToObject(root, "light", (int)(lightintensity * 100) / 100.0);
        cJSON_AddNumberToObject(root, "humi", (int)(humidity * 100) / 100.0);
        cJSON_AddNumberToObject(root, "temp", (int)(temperature * 100) / 100.0);
        char *json_string = cJSON_Print(root);
        if (json_string == NULL)
        {
            printf("JSON print error\r\n");
        }
        MQTT_Publish_Topic("/user/test0", json_string, Qos0, No_Save_Msg);
        oled_handle.ShowString(&oled_handle, 1, 1, "count:");
        oled_handle.ShowNum(&oled_handle, 1, strlen("count:") + 1, update_count++, 4);

        cJSON_Delete(root);
        vPortFree(json_string);
        osDelay(2000);
    }
}
static void MQTT_Receve_Msg_Task(void const *argument)
{

    while (1)
    {
        if (MQTT_ReceiveMsg(MQTT_WriteMsg, mqtt_rxbuf) == 0)
            PrintRecvMsg();

        osDelay(2000);
    }
}

static void oled_sub1_opt3_action(uint8_t opt)
{
    static uint8_t isWifiConnected = 0;
    if (isWifiConnected == 0)
    {

        uint8_t status = 0;
        oled_handle.Clear(&oled_handle);
        oled_show_string_lines(&oled_handle, 2, "start connect", "pls wait...");
        oled_comfirm_flag |= 0x01;
        status = ESP8266_StaTcpClient_Unvarnish_ConfigTest();
        oled_handle.Clear(&oled_handle);
        if (status == HAL_OK)
        {
            oled_handle.ShowString(&oled_handle, 1, 1, "wifi connected");
            MQTT_Connect();
            MQTT_Modify_Will("on_line");
            // if (MQTT_Subscribe_Topic("/user/test0", Qos0) < 0)
            //     printf("订阅0错误，请检查MQTT服务器网络！\n");
            // oled_handle.ShowString(&oled_handle, 2, 1, "subscribe test0");
            if (MQTT_Subscribe_Topic("/user/test1", Qos0) < 0)
                printf("订阅1错误，请检查MQTT服务器网络！\n");
            oled_handle.ShowString(&oled_handle, 3, 1, "subscribe test1");
            if (MQTT_Subscribe_Topic("/user/test2", Qos0) < 0)
                printf("订阅2错误，请检查MQTT服务器网络！\n");
            oled_handle.ShowString(&oled_handle, 4, 1, "subscribe test2");
            isWifiConnected = 1;
            subPage1.optins[3].text = "CLOSE WIFI";
        }
        else if (status == HAL_TIMEOUT)
        {
        }
        else if (status == HAL_BUSY)
        {
        }
    }
    else
    {
        oled_handle.Clear(&oled_handle);
        oled_show_string_lines(&oled_handle, 2, "disconnecting", "please wait...");
        MQTT_Disconnect();
        ESP8266_ExitUnvarnishSend_QuitAP();
        oled_handle.ShowString(&oled_handle, 3, 1, "success");
        subPage1.optins[3].text = "OPEN WIFI";
        isWifiConnected = 0;
    }
    osDelay(2000);
    (*oled_page).indexFlag |= 0x08;
}
static void oled_sub1_1_opt1_action(uint8_t opt)
{
    oled_handle.Clear(&oled_handle);
    if (wifi_flag == 0)
    {
        oled_show_string_lines(&oled_handle, 4, "WIFI1 SSID:", macUser_ESP8266_ApSsid, "PASSWORD:", macUser_ESP8266_ApPwd);
    }
    else
    {
        oled_show_string_lines(&oled_handle, 4, "WIFI2 SSID:", macUser_ESP8266_ApSsid2, "PASSWORD:", macUser_ESP8266_ApPwd2);
    }
}

static void oled_sub1_1_opt2_action(uint8_t opt)
{
    wifi_flag = 0;
    oled_handle.Clear(&oled_handle);
    oled_handle.ShowString(&oled_handle, 1, 1, "change to WIFI1");
    osDelay(3000);
    (*oled_page).indexFlag |= 0x08;
}
static void oled_sub1_1_opt3_action(uint8_t opt)
{
    wifi_flag = 1;
    oled_handle.Clear(&oled_handle);
    oled_handle.ShowString(&oled_handle, 1, 1, "change to WIFI2");
    osDelay(3000);
    (*oled_page).indexFlag |= 0x08;
}
// subpage2
static void oled_sub2_opt1_action(uint8_t opt)
{
    oled_handle.Clear(&oled_handle);
    oled_show_string_lines(&oled_handle, 2, "Bluetooth info:", "name: cisjy", "USE FOR DEBUG");
}
static void oled_sub2_opt2_action(uint8_t opt)
{
    oled_handle.Clear(&oled_handle);
    if (is_bluetooth_debug)
    {
        oled_show_string_lines(&oled_handle, 1, "OPENING----");
        is_bluetooth_debug = false;
        subPage2.optins[2].text = "OPEN DEBUG";
    }
    else
    {
        oled_show_string_lines(&oled_handle, 1, "CLOSEING----");
        is_bluetooth_debug = true;
        subPage2.optins[2].text = "CLOSE DEBUG";
    }
    osDelay(3000);
    (*oled_page).indexFlag |= 0x08;
}
// subpage3
TaskHandle_t oled_sub3_opt_task_handle = NULL;
void oled_sub3_opt_task(void const *argument);
static void oled_sub3_opt_action(uint8_t opt)
{
    oled_handle.Clear(&oled_handle);
#if OLED_DISPLAY_DEBUG
    printf("start oled_sub3_opt_task\r\n");
#endif
    // 创建task
    if (oled_sub3_opt_task_handle == NULL)
    {
        if (xTaskCreate((TaskFunction_t)oled_sub3_opt_task,
                        "oled_sub3_opt_task",
                        256,
                        (void *)opt,
                        1,
                        &oled_sub3_opt_task_handle) != pdTRUE)
        {
            printf("create oled_sub3_opt_task failed\r\n");
        }
    }
}
void oled_sub3_opt_task(void const *argument)
{
    uint8_t opt = (uint8_t)argument;
    float pit, rol, yaw;
    uint8_t receive_data[4] = {0};
    float lightintensity = 0.0;
    while (1)
    {
        switch (opt)
        {
        case 1:
            MPU6050_dmp_get_data(&pit, &rol, &yaw);
            printf("pitch:%.2f,roll:%.2f,yaw:%.2f\r\n", pit, rol, yaw);
            oled_show_string_lines(&oled_handle, 4, "MPU6050", "pitch:", "roll:", "yaw:");
            oled_handle.ShowSignedNum(&oled_handle, 2, strlen("pitch:") + 1, (int)pit, 2);
            oled_handle.ShowChar(&oled_handle, 2, strlen("pitch:") + 4, '.');
            oled_handle.ShowNum(&oled_handle, 2, strlen("pitch:") + 5, (int)(pit * 100) % 100, 2);
            oled_handle.ShowSignedNum(&oled_handle, 3, strlen("roll:") + 1, (int)rol, 2);
            oled_handle.ShowChar(&oled_handle, 3, strlen("roll:") + 4, '.');
            oled_handle.ShowNum(&oled_handle, 3, strlen("roll:") + 5, (int)(rol * 100) % 100, 2);
            oled_handle.ShowSignedNum(&oled_handle, 4, strlen("yaw:") + 1, (int)yaw, 2);
            oled_handle.ShowChar(&oled_handle, 4, strlen("yaw:") + 4, '.');
            oled_handle.ShowNum(&oled_handle, 4, strlen("yaw:") + 5, (int)(yaw * 100) % 100, 2);
            break;
        case 2:
            bh1750_handle.Light_Intensity(&bh1750_handle, &lightintensity);
            oled_show_string_lines(&oled_handle, 2, "BH1750:", "Light:");
            oled_handle.ShowNum(&oled_handle, 2, 7, (int)lightintensity, 4);
            oled_handle.ShowChar(&oled_handle, 2, 11, '.');
            oled_handle.ShowNum(&oled_handle, 2, 12, (int)(lightintensity * 100) % 100, 2);
            oled_handle.ShowString(&oled_handle, 2, 14, "Lux");
            break;
        case 3:
            DHT11_Read_Data(receive_data);
            oled_show_string_lines(&oled_handle, 3, "DHT11", "hum:", "temp:");
            oled_handle.ShowNum(&oled_handle, 2, 6, receive_data[0], 2);
            oled_handle.ShowChar(&oled_handle, 2, 8, '.');
            oled_handle.ShowNum(&oled_handle, 2, 9, receive_data[1], 2);
            oled_handle.ShowNum(&oled_handle, 3, 6, receive_data[2], 2);
            oled_handle.ShowChar(&oled_handle, 3, 8, '.');
            oled_handle.ShowNum(&oled_handle, 3, 9, receive_data[3], 2);
            break;
        default:
            break;
        }
#if OLED_DISPLAY_DEBUG
        printf("oled_sub3_opt_task start%d\r\n", opt);
#endif

        osDelay(100);
    }
}

// subpage4
static void oled_sub4_opt1_action(uint8_t opt)
{
    oled_handle.Clear(&oled_handle);
    oled_show_string_lines(&oled_handle, 4, "about", "version:1.0", "author:Cisjy", "email:<EMAIL>");
}

static void oled_sub4_opt2_action(uint8_t opt)
{

    oled_handle.Clear(&oled_handle);
    oled_show_string_lines(&oled_handle, 4, "help", "nothing to help", "2.teststst", "3.teststst");
}
static void oled_sub4_3_opt1_action(uint8_t opt)
{
    uint8_t *pfile_name = pvPortMalloc(40);
    uint8_t file_length[4] = {0};
    uint8_t i = 0;
    if (pfile_name == NULL)
    {
        USART1_printf("pfile_name pvPortMalloc failed\r\n");
        return;
    }

    memset(pfile_name, 0, 40);

    while (i < 39) // 留出一个位置给 '\0'
    {
        uint8_t temp = 0;

        // 读取从 W25QXX_FLASH_ADDRESS + 5 开始的字节
        W25QXX_Read(&temp, W25QXX_FLASH_ADDRESS + 5 + i, 1);
        if (temp == 0xFF)
        {
            pfile_name[i] = '\0'; // 手动将结束符 '\0' 放在字符串末尾
            break;
        }

        pfile_name[i] = temp; // 将读取到的字节存储到 pfile_name 中
        i++;
    }
    // 确保字符串以 '\0' 结尾
    pfile_name[39] = '\0';

    uint8_t *pindex = (uint8_t *)strstr((const char *)pfile_name, "_version");
    uint8_t pnew_file_name_without_version[40] = {0};
    uint8_t pnew_file_name_version[10] = {0};
    if (pindex != NULL)
    {
        uint8_t temp_len = (pindex - pfile_name) > 11 ? 11 : (pindex - pfile_name);
        memcpy(pnew_file_name_without_version, pfile_name, temp_len);
        memcpy((void *)pnew_file_name_version, pindex + strlen("_version"), strlen((const char *)pfile_name) - strlen("_version") - (pindex - pfile_name) - strlen(".bin"));
    }
    int32_t file_length_int = 0;
    W25QXX_Read(file_length, W25QXX_FLASH_ADDRESS, 4);
    file_length_int = file_length[0] + (file_length[1] << 8) + (file_length[2] << 16) + (file_length[3] << 24);
    oled_handle.Clear(&oled_handle);
    oled_handle.ShowString(&oled_handle, 1, 1, "bin info:");
    oled_handle.ShowString(&oled_handle, 2, 1, "name:");
    oled_handle.ShowString(&oled_handle, 2, 6, (char *)pnew_file_name_without_version);
    oled_handle.ShowString(&oled_handle, 3, 1, "length:");
    oled_handle.ShowNum(&oled_handle, 3, 8, file_length_int, get_decimal_length(file_length_int));
    oled_handle.ShowString(&oled_handle, 4, 1, "version:");
    oled_handle.ShowString(&oled_handle, 4, 9, (char *)pnew_file_name_version);

    USART1_printf("update_flag_change: %s\r\n", pfile_name);
    if (is_bluetooth_debug)
    {
        USART2_printf("update_flag_change: %s\r\n", pfile_name);
    }
    // 释放分配的内存

    vPortFree(pfile_name);
}
static void oled_sub4_3_opt2_action(uint8_t opt)
{
    uint8_t receive_len = 0;
    oled_handle.Clear(&oled_handle);
    oled_handle.ShowString(&oled_handle, 1, 1, "update");
    oled_handle.ShowString(&oled_handle, 2, 1, "Ymodem rec-----");
    oled_handle.ShowString(&oled_handle, 3, 1, "click send ");
    oled_comfirm_flag |= 0x01;
    receive_len = Ymodem_Receive(NULL);
    if (receive_len > 0)
    {
        oled_handle.Clear(&oled_handle);
        oled_handle.ShowString(&oled_handle, 1, 1, "update");
        oled_handle.ShowString(&oled_handle, 2, 1, "Ymodem rec ok");
        osDelay(3000);
        (*oled_page).indexFlag |= 0x08;
    }
    else
    {
        oled_handle.Clear(&oled_handle);
        oled_handle.ShowString(&oled_handle, 1, 1, "update");
        oled_handle.ShowString(&oled_handle, 2, 1, "Ymodem rec fail");
        osDelay(3000);
        (*oled_page).indexFlag |= 0x08;
    }
}
static void oled_sub4_3_opt3_action(uint8_t opt)
{
    oled_handle.Clear(&oled_handle);
    oled_handle.ShowString(&oled_handle, 1, 1, "update now?");
    oled_handle.ShowString(&oled_handle, 2, 1, "click yes or no");
    oled_comfirm_flag |= 0x01; // 置位 update 标志位
    while (1)
    {
        if (oled_comfirm_flag & 0x02)
        {
            oled_comfirm_flag &= 0x00;
            NVIC_SystemReset();
        }
        if (oled_comfirm_flag & 0x04)
        {
            oled_comfirm_flag &= 0x00;
            break;
        }
        osDelay(10);
    }
}
static void oled_sub4_opt4_action(uint8_t opt)
{
    oled_handle.Clear(&oled_handle);
    oled_handle.ShowString(&oled_handle, 1, 1, "reboot?");
    oled_handle.ShowString(&oled_handle, 2, 1, "click yes or no");
    oled_comfirm_flag |= 0x01; // 置位 update 标志位
    while (1)
    {
        if (oled_comfirm_flag & 0x02)
        {
            oled_comfirm_flag &= 0x00;
            NVIC_SystemReset();
        }
        if (oled_comfirm_flag & 0x04)
        {
            oled_comfirm_flag &= 0x00;
            break;
        }
        osDelay(10);
    }
}
//*******************SUBPAGE*********************** */
static void oled_sub_page_init()
{
    // subpage1
    subPage1.index = 1;
    subPage1.numOptions = 3;
    subPage1.indexFlag = 0;
    subPage1.parent = &mainPage;
    subPage1.optins[1].text = "WIFI CHANGE";
    subPage1.optins[1].hasSubMenu = 1;
    subPage1.optins[1].subMenu = &subPage1_1;
    subPage1.optins[1].action = NULL;

    subPage1_1.index = 1;
    subPage1_1.numOptions = 3;
    subPage1_1.indexFlag = 0;
    subPage1_1.parent = &subPage1;
    subPage1_1.optins[1].text = "WIFI INFO";
    subPage1_1.optins[1].hasSubMenu = 0;
    subPage1_1.optins[1].subMenu = NULL;
    subPage1_1.optins[1].action = oled_sub1_1_opt1_action;

    subPage1_1.optins[2].text = "WIFI 1";
    subPage1_1.optins[2].hasSubMenu = 0;
    subPage1_1.optins[2].subMenu = NULL;
    subPage1_1.optins[2].action = oled_sub1_1_opt2_action;

    subPage1_1.optins[3].text = "WIFI 2";
    subPage1_1.optins[3].hasSubMenu = 0;
    subPage1_1.optins[3].subMenu = NULL;
    subPage1_1.optins[3].action = oled_sub1_1_opt3_action;

    subPage1.optins[2].text = "WIFI UPLOAD";
    subPage1.optins[2].hasSubMenu = 0;
    subPage1.optins[2].subMenu = NULL;
    subPage1.optins[2].action = oled_sub1_opt2_action;

    subPage1.optins[3].text = "OPEN WIFI";
    subPage1.optins[3].hasSubMenu = 0;
    subPage1.optins[3].subMenu = NULL;
    subPage1.optins[3].action = oled_sub1_opt3_action;

    // subpage2

    subPage2.index = 1;
    subPage2.numOptions = 2;
    subPage2.indexFlag = 0;
    subPage2.parent = &mainPage;
    subPage2.optins[1].text = "BLUETOOTH INFO";
    subPage2.optins[1].hasSubMenu = 0;
    subPage2.optins[1].subMenu = NULL;
    subPage2.optins[1].action = oled_sub2_opt1_action;

    subPage2.optins[2].text = "OPEN DEBUG";
    subPage2.optins[2].hasSubMenu = 0;
    subPage2.optins[2].subMenu = NULL;
    subPage2.optins[2].action = oled_sub2_opt2_action;

    // subpage3
    subPage3.index = 1;
    subPage3.numOptions = 3;
    subPage3.indexFlag = 0;
    subPage3.parent = &mainPage;
    subPage3.optins[1].text = "MPU6050";
    subPage3.optins[1].hasSubMenu = 0;
    subPage3.optins[1].subMenu = NULL;
    subPage3.optins[1].action = oled_sub3_opt_action;

    subPage3.optins[2].text = "BH1750";
    subPage3.optins[2].hasSubMenu = 0;
    subPage3.optins[2].subMenu = NULL;
    subPage3.optins[2].action = oled_sub3_opt_action;

    subPage3.optins[3].text = "DHT11";
    subPage3.optins[3].hasSubMenu = 0;
    subPage3.optins[3].subMenu = NULL;
    subPage3.optins[3].action = oled_sub3_opt_action;

    // subpage4
    subPage4.index = 1;
    subPage4.numOptions = 4;
    subPage4.indexFlag = 0;
    subPage4.parent = &mainPage;

    subPage4.optins[1].text = "about";
    subPage4.optins[1].hasSubMenu = 0;
    subPage4.optins[1].subMenu = NULL;
    subPage4.optins[1].action = oled_sub4_opt1_action;

    subPage4.optins[2].text = "help";
    subPage4.optins[2].hasSubMenu = 0;
    subPage4.optins[2].subMenu = NULL;
    subPage4.optins[2].action = oled_sub4_opt2_action;

    subPage4.optins[3].text = "update";
    subPage4.optins[3].hasSubMenu = 1;
    subPage4.optins[3].subMenu = &subPage4_3;
    subPage4.optins[3].action = NULL;
    // subpage4_3
    subPage4_3.index = 1;
    subPage4_3.numOptions = 3;
    subPage4_3.indexFlag = 0;
    subPage4_3.parent = &subPage4;
    subPage4_3.optins[1].text = "info";
    subPage4_3.optins[1].hasSubMenu = 0;
    subPage4_3.optins[1].subMenu = NULL;
    subPage4_3.optins[1].action = oled_sub4_3_opt1_action;

    subPage4_3.optins[2].text = "download";
    subPage4_3.optins[2].hasSubMenu = 0;
    subPage4_3.optins[2].subMenu = NULL;
    subPage4_3.optins[2].action = oled_sub4_3_opt2_action;

    subPage4_3.optins[3].text = "update now";
    subPage4_3.optins[3].hasSubMenu = 0;
    subPage4_3.optins[3].subMenu = NULL;
    subPage4_3.optins[3].action = oled_sub4_3_opt3_action;

    subPage4.optins[4].text = "reboot";
    subPage4.optins[4].hasSubMenu = 0;
    subPage4.optins[4].subMenu = NULL;
    subPage4.optins[4].action = oled_sub4_opt4_action;
}
//*******************MAINPAGE* *******************************/
static void oled_main_page_init(oled_page_t *mainPage)
{
    oled_sub_page_init();

    mainPage->index = 1;
    mainPage->numOptions = 4;
    mainPage->parent = NULL;

    mainPage->optins[1].text = "WIFI";
    mainPage->optins[1].hasSubMenu = 1;
    mainPage->optins[1].subMenu = &subPage1;
    mainPage->optins[1].action = NULL;

    mainPage->optins[2].text = "Bluetooth";
    mainPage->optins[2].hasSubMenu = 1;
    mainPage->optins[2].subMenu = &subPage2;
    mainPage->optins[2].action = NULL;

    mainPage->optins[3].text = "Sensor";
    mainPage->optins[3].hasSubMenu = 1;
    mainPage->optins[3].subMenu = &subPage3;
    mainPage->optins[3].action = NULL;

    mainPage->optins[4].text = "System";
    mainPage->optins[4].hasSubMenu = 1;
    mainPage->optins[4].subMenu = &subPage4;
    mainPage->optins[4].action = NULL;
}

void oled_display_init(void)
{
    oled_main_page_init(&mainPage);
}
void oled_display(oled_page_t **currentPage)
{
    static uint8_t flag = 0;

    uint8_t tmpFlag = (*currentPage)->indexFlag;
    (*currentPage)->indexFlag = 0;

    // 按下了确认键
    if (tmpFlag & 0x04)
    {
        if ((*currentPage)->optins[(*currentPage)->index].hasSubMenu == 1)
        {
            (*currentPage) = (*currentPage)->optins[(*currentPage)->index].subMenu;
            oled_handle.Clear(&oled_handle);
            goto oled_display;
        }
        else if (((*currentPage)->optins[(*currentPage)->index].action != NULL) && (flag == 0))
        {
            (*currentPage)->optins[(*currentPage)->index].action((*currentPage)->index);
            flag = 1;
        }
    }
    // 按下了返回键
    else if (tmpFlag & 0x08)
    {
        if (flag == 1)
        {
            flag = 0;
            if (oled_sub3_opt_task_handle != NULL)
            {
                vTaskDelete(oled_sub3_opt_task_handle);
                oled_sub3_opt_task_handle = NULL;
            }
            if (oled_sub1_opt_task_handle != NULL)
            {
                vTaskDelete(oled_sub1_opt_task_handle);
                oled_sub1_opt_task_handle = NULL;
                vTaskDelete(MQTT_Receve_Msg_Task_Handle);
                MQTT_Receve_Msg_Task_Handle = NULL;
            }
            oled_handle.Clear(&oled_handle);
            goto oled_display;
        }
        if ((*currentPage)->parent != NULL)
        {
            (*currentPage) = (*currentPage)->parent;
            oled_handle.Clear(&oled_handle);
            goto oled_display;
        }
    }
    // 处理上下滚动
    else if ((tmpFlag & 0x01) && (flag == 0))
    {
        // 清除当前选中行
        oled_handle.ClearLine(&oled_handle, (*currentPage)->index);
        oled_handle.ShowString(&oled_handle, (*currentPage)->index, 1, (*currentPage)->optins[(*currentPage)->index].text);

        // 更新选中的行
        if (tmpFlag & 0x02)
        {
            (*currentPage)->index = (*currentPage)->index + 1 > (*currentPage)->numOptions ? 1 : (*currentPage)->index + 1;
        }
        else
        {
            (*currentPage)->index = (*currentPage)->index - 1 < 1 ? (*currentPage)->numOptions : (*currentPage)->index - 1;
        }
    }

// 显示当前页面的所有选项
oled_display:
    if (flag == 0)
    {
        for (uint8_t i = 1; i <= (*currentPage)->numOptions; i++)
        {
            if (i == (*currentPage)->index)
            {
                oled_handle.ShowChar(&oled_handle, i, 1, '>');
                oled_handle.ShowString(&oled_handle, i, 2, (*currentPage)->optins[i].text);
            }
            else
            {
                oled_handle.ShowString(&oled_handle, i, 1, (*currentPage)->optins[i].text);
            }
        }
    }
}
