
#ifndef __MY_STRING_H__
#define __MY_STRING_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "usart.h"
#include "usart_print.h"

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;

// 自定义警告信息
#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "("__STR1__(__LINE__) ")"
#define _warning(iCode, msgStr) message(__LOC__ " : warning C" __STR1__(iCode) ": " msgStr)
// 使用举例  #pragma _warning(1234, "Need to do 3D collision testing")

#define FALL_TO_ADD 0
#define ADD_SPACE 1
#define ADD_SPACE_AND_0X 2

void HexToAscii(const void *source, void *destination, int len, uint8_t separate);
void AsciiToHex(char *src, uint8_t *dest, int len);
int UpdateStrlen_uint8_t(const void *source);
int UpdateStrlen_uint32_t(const void *source);

#endif // !__MY_STRING_
