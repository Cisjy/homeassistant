#include "my_string.h"

/**
 * @brief  16进制 -> 字符串
 * @param  src: 16进制字符串
 * @param  dest: 存储转化完成后的字符串
 * @param  len: 想要转换的个数
 * @param  separate: 转换完成后的字符串之间是否要使用空格隔开，且在每个16进制前加上“0x”
 *   @arg  FALL_TO_ADD: 不添加空格以及“0x”
 *   @arg  ADD_SPACE: 只添加空格
 *   @arg  ADD_SPACE_AND_0X: 添加空格以及“0x”
 * @retval None
 */
void HexToAscii(const void *source, void *destination, int len, uint8_t separate)
{
    const uint8_t *src = source;
    char *dest = destination;
    char dh, dl; // 字符串的高位和低位
    int i;

    if (src == NULL || dest == NULL)
    {
        printf("error:%s,%d\r\n", __FILE__, __LINE__);
        printf("src or dest is NULL\n");
        return;
    }
    if (len < 1)
    {
        printf("error:%s,%d\r\n", __FILE__, __LINE__);
        printf("length is NULL\n");
        return;
    }

    for (i = 0; i < len; i++)
    {
        dh = '0' + src[i] / 16;
        dl = '0' + src[i] % 16;
        if (dh > '9')
            dh = dh - '9' - 1 + 'A';
        if (dl > '9')
            dl = dl - '9' - 1 + 'A';

        if (separate == FALL_TO_ADD)
        {
            dest[2 * i] = dh;
            dest[2 * i + 1] = dl;
        }
        else if (separate == ADD_SPACE)
        {
            dest[3 * i] = dh;
            dest[3 * i + 1] = dl;
            dest[3 * i + 2] = ' ';
        }
        else if (separate == ADD_SPACE_AND_0X)
        {
            dest[5 * i] = '0';
            dest[5 * i + 1] = 'x';
            dest[5 * i + 2] = dh;
            dest[5 * i + 3] = dl;
            dest[5 * i + 4] = ' ';
        }
    }
    if (separate == FALL_TO_ADD)
        dest[2 * i] = '\0';
    else if (separate == ADD_SPACE)
        dest[3 * i] = '\0';
    else if (separate == ADD_SPACE_AND_0X)
        dest[5 * i] = '\0';
}

/*lowercase letters transform to capital letter*/
static char lowtocap(char c)
{
    if ((c >= 'a') && (c <= 'z'))
    {
        c = c - 32; // 或者c = c + ('a' - 'A');
    }
    return c;
}

/**
 * @brief  字符串 -> 16进制
 * @param  src: 代转换的字符串
 * @param  dest: 存储转化完成后的16进制数组
 * @param  len: 想要转换的个数
 * @retval None
 */
void AsciiToHex(char *src, uint8_t *dest, int len)
{
    int dh, dl;  // 16进制的高4位和低4位
    char ch, cl; // 字符串的高位和低位
    int i;
    if (src == NULL || dest == NULL)
    {
        printf("src or dest is NULL\n");
        return;
    }
    if (len < 1)
    {
        printf("length is NULL\n");
        return;
    }
    for (i = 0; i < len; i++)
    {
        ch = src[2 * i];
        cl = src[2 * i + 1];
        dh = lowtocap(ch) - '0';
        if (dh > 9)
        {
            dh = lowtocap(ch) - 'A' + 10; // lowtocap可以用库函数toupper
        }
        dl = lowtocap(cl) - '0';
        if (dl > 9)
        {
            dl = lowtocap(cl) - 'A' + 10; // lowtocap可以用库函数toupper
        }
        dest[i] = dh * 16 + dl;
    }
    if (len % 2 > 0) // 字符串个数为奇数
    {
        dest[len / 2] = src[len - 1] - '0';
        if (dest[len / 2] > 9)
        {
            dest[len / 2] = lowtocap(src[len - 1]) - 'A' + 10;
        }
    }
}

/**
 * @brief  计算存储着数字数组（uint8_t）的长度
 * @param  source: 带计算的数字数组
 * @retval 该数组的长度
 */
int UpdateStrlen_uint8_t(const void *source)
{
    const uint8_t *src = source;
    uint8_t last = 1, cur = 0, next = 0;
    int src_len = -1;
    int i;

    for (i = 0;; i++)
    {
        if (i)
            last = cur;

        cur = *(src + i);
        next = *(src + i + 1);
        if (last == 0 && cur == 0 && next == 0)
            break;
        else
            src_len++;
    }
    return src_len;
}

/**
 * @brief  计算存储着数字数组（uint32_t）的长度
 * @param  source: 带计算的数字数组
 * @retval 该数组的长度
 */
int UpdateStrlen_uint32_t(const void *source)
{
    const uint32_t *src = source;
    uint32_t last = 1, cur = 0, next = 0;
    int src_len = -1;
    int i;

    for (i = 0;; i++)
    {
        if (i)
            last = cur;

        cur = *(src + i);
        next = *(src + i + 1);
        if (last == 0 && cur == 0 && next == 0)
            break;
        else
            src_len++;
    }
    return src_len;
}
