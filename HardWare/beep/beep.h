#include "stm32f1xx_hal.h"

#define BEEP_GPIO_PIN GPIO_PIN_7
#define BEEP_GPIO_PORT GPIOB
#define BEEP_GPIO_CLK_ENABLE()        \
    do                                \
    {                                 \
        __HAL_RCC_GPIOB_CLK_ENABLE(); \
    } while (0)

void beep_init(void);
void beep_on(void);
void beep_off(void);
