#ifndef __KEY_H__
#define __KEY_H__
#include "stm32f1xx_hal.h"

#define KEY1_GPIO_Port GPIOB
#define KEY1_Pin GPIO_PIN_3
#define KEY1_GPIO_CLK_ENABLE()        \
    do                                \
    {                                 \
        __HAL_RCC_GPIOB_CLK_ENABLE(); \
    } while (0) /* PC口时钟使能 */

#define KEY2_GPIO_Port GPIOB
#define KEY2_Pin GPIO_PIN_4
#define KEY2_GPIO_CLK_ENABLE()        \
    do                                \
    {                                 \
        __HAL_RCC_GPIOB_CLK_ENABLE(); \
    } while (0) /* PC口时钟使能 */

#define KEY3_GPIO_Port GPIOB
#define KEY3_Pin GPIO_PIN_5
#define KEY3_GPIO_CLK_ENABLE()        \
    do                                \
    {                                 \
        __HAL_RCC_GPIOB_CLK_ENABLE(); \
    } while (0) /* PC口时钟使能 */

#define KEY4_GPIO_Port GPIOB
#define KEY4_Pin GPIO_PIN_6
#define KEY4_GPIO_CLK_ENABLE()        \
    do                                \
    {                                 \
        __HAL_RCC_GPIOB_CLK_ENABLE(); \
    } while (0) /* PC口时钟使能 */

extern TIM_HandleTypeDef htim2;

void key_init(void);
uint8_t key_scan(void);
#endif
