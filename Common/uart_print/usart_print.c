/*
 * @Descripttion:
 * @Author: Cisjy
 * @Date: 2024-07-19 22:28:56
 * @LastEditors: Cisjy
 * @LastEditTime: 2024-09-18 13:59:53
 */
#include "usart_print.h"
#include "main.h"
#include <stdarg.h>

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

static char *itoa(int value, char *string, int radix);

void USART1_printf(char *fmt, ...)
{
    UART_HandleTypeDef huart = huart1;
    char c;
    const char *s;
    int d;
    unsigned int u;
    char buf[16];

    va_list ap;
    va_start(ap, fmt);
    while (*fmt != 0) // same as != '\0'
    {
        if (*fmt == 0x5c) //'\'
        {
            switch (*++fmt)
            {
            case 'r':
                HAL_UART_Transmit(&huart, (uint8_t *)"\r", 1, 1000);
                fmt++;
                break;

            case 'n':
                HAL_UART_Transmit(&huart, (uint8_t *)"\n", 1, 1000);
                fmt++;
                break;
            default:
                fmt++;
                break;
            }
        }
        else if (*fmt == '%')
        {
            switch (*++fmt)
            {
            case 'c':
                c = va_arg(ap, int);
                HAL_UART_Transmit(&huart, (uint8_t *)&c, 1, 1000);
                fmt++;
                break;

            case 's':
                s = va_arg(ap, const char *);
                for (; *s; s++)
                {
                    HAL_UART_Transmit(&huart, (uint8_t *)s, 1, 1000);
                }
                fmt++;
                break;

            case 'd':
                d = va_arg(ap, int);
                itoa(d, buf, 10);
                for (s = buf; *s; s++)
                {
                    HAL_UART_Transmit(&huart, (uint8_t *)s, 1, 1000);
                }
                fmt++;
                break;
            case 'x':
                u = va_arg(ap, unsigned int);
                itoa(u, buf, 16); // Convert to hex string
                if (buf[1] == '\0')
                {
                    buf[1] = buf[0];
                    buf[0] = '0';
                    buf[2] = '\0';
                }
                // Convert to uppercase
                for (s = buf; *s; s++)
                {
                    if (*s >= 'a' && *s <= 'f')
                        c = *s - 'a' + 'A';
                    else
                        c = *s;
                    HAL_UART_Transmit(&huart, (uint8_t *)&c, 1, 1000);
                }
                fmt++;
                break;
            default:
                fmt++;
            }
        }
        else
        {
            HAL_UART_Transmit(&huart, (uint8_t *)fmt++, 1, 1000);
        }
    }
    va_end(ap);
}
void USART2_printf(char *fmt, ...)
{
    UART_HandleTypeDef huart = huart2;
    char c;
    const char *s;
    int d;
    unsigned int u;
    char buf[16];

    va_list ap;
    va_start(ap, fmt);
    while (*fmt != 0) // same as != '\0'
    {
        if (*fmt == 0x5c) //'\'
        {
            switch (*++fmt)
            {
            case 'r':
                HAL_UART_Transmit(&huart, (uint8_t *)"\r", 1, 1000);
                fmt++;
                break;

            case 'n':
                HAL_UART_Transmit(&huart, (uint8_t *)"\n", 1, 1000);
                fmt++;
                break;
            default:
                fmt++;
                break;
            }
        }
        else if (*fmt == '%')
        {
            switch (*++fmt)
            {
            case 'c':
                c = va_arg(ap, int);
                HAL_UART_Transmit(&huart, (uint8_t *)&c, 1, 1000);
                fmt++;
                break;

            case 's':
                s = va_arg(ap, const char *);
                for (; *s; s++)
                {
                    HAL_UART_Transmit(&huart, (uint8_t *)s, 1, 1000);
                }
                fmt++;
                break;

            case 'd':
                d = va_arg(ap, int);
                itoa(d, buf, 10);
                for (s = buf; *s; s++)
                {
                    HAL_UART_Transmit(&huart, (uint8_t *)s, 1, 1000);
                }
                fmt++;
                break;
            case 'x':
                u = va_arg(ap, unsigned int);
                itoa(u, buf, 16); // Convert to hex string
                if (buf[1] == '\0')
                {
                    buf[1] = buf[0];
                    buf[0] = '0';
                    buf[2] = '\0';
                }
                // Convert to uppercase
                for (s = buf; *s; s++)
                {
                    if (*s >= 'a' && *s <= 'f')
                        c = *s - 'a' + 'A';
                    else
                        c = *s;
                    HAL_UART_Transmit(&huart, (uint8_t *)&c, 1, 1000);
                }
                fmt++;
                break;
            default:
                fmt++;
            }
        }
        else
        {
            HAL_UART_Transmit(&huart, (uint8_t *)fmt++, 1, 1000);
        }
    }
    va_end(ap);
}

void USART3_printf(char *fmt, ...)
{
    UART_HandleTypeDef huart = huart3;
    char c;
    const char *s;
    int d;
    char buf[16];

    va_list ap;
    va_start(ap, fmt);
    while (*fmt != 0) // same as != '\0'
    {
        if (*fmt == 0x5c) //'\'
        {
            switch (*++fmt)
            {
            case 'r':
                HAL_UART_Transmit(&huart, (uint8_t *)"\r", 1, 1000);
                fmt++;
                break;

            case 'n':
                HAL_UART_Transmit(&huart, (uint8_t *)"\n", 1, 1000);
                fmt++;
                break;
            default:
                fmt++;
                break;
            }
        }
        else if (*fmt == '%')
        {
            switch (*++fmt)
            {
            case 'c':
                c = va_arg(ap, int);
                HAL_UART_Transmit(&huart, (uint8_t *)&c, 1, 1000);
                fmt++;
                break;

            case 's':
                s = va_arg(ap, const char *);
                for (; *s; s++)
                {
                    HAL_UART_Transmit(&huart, (uint8_t *)s, 1, 1000);
                }
                fmt++;
                break;

            case 'd':
                d = va_arg(ap, int);
                itoa(d, buf, 10);
                for (s = buf; *s; s++)
                {
                    HAL_UART_Transmit(&huart, (uint8_t *)s, 1, 1000);
                }
                fmt++;
                break;
            default:
                fmt++;
            }
        }
        else
        {
            HAL_UART_Transmit(&huart, (uint8_t *)fmt++, 1, 1000);
        }
    }
    va_end(ap);
}

/*
 * 函数名：itoa
 * 描述  ：将整形数据转换成字符串
 * 输入  ：-radix =10 表示10进制，其他结果为0
 *    		 -radix =16 表示16进制
 *         -value 要转换的整形数
 *         -string 转换后的字符串
 *         -radix = 10
 * 输出  ：无
 * 返回  ：无
 * 调用  ：被USART2_printf()调用
 */
static char *itoa(int value, char *string, int radix)
{
    char *ptr = string;
    char *ptr1 = string;
    char tmp_char;
    int tmp_value;

    // Handle negative numbers only for decimal
    if (value < 0 && radix == 10)
    {
        value = -value;
        *ptr++ = '-';
    }

    // Convert the integer to the specified base
    do
    {
        tmp_value = value;
        value /= radix;
        *ptr++ = "0123456789ABCDEF"[tmp_value - value * radix];
    } while (value);

    // Null-terminate the string
    *ptr-- = '\0';

    // Reverse the string
    while (ptr1 < ptr)
    {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    return string;
}
