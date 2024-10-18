#include "key.h"
#include "delay.h"
#include "usart_print.h"
TIM_HandleTypeDef htim2;
__IO uint8_t g_pressed_key = 0;
void TIM2_Init(void);
void key_init(void)
{
    KEY1_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pin = KEY1_Pin;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(KEY1_GPIO_Port, &GPIO_InitStruct);
    HAL_NVIC_SetPriority(EXTI3_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);

    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pin = KEY2_Pin;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(KEY2_GPIO_Port, &GPIO_InitStruct);
    HAL_NVIC_SetPriority(EXTI4_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);

    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pin = KEY3_Pin;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(KEY3_GPIO_Port, &GPIO_InitStruct);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pin = KEY4_Pin;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(KEY4_GPIO_Port, &GPIO_InitStruct);
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

    TIM2_Init();
}
void TIM2_Init(void)
{
    // TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    // TIM_MasterConfigTypeDef sMasterConfig = {0};
    // TIM_OC_InitTypeDef sConfigOC = {0};
    // htim2.Instance = TIM2;
    // htim2.Init.Prescaler = 7200 - 1;
    // htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    // htim2.Init.Period = 400 - 1; // 40ms
    // htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    // htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    // if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    // {
    //     Error_Handler();
    // }
    // sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    // if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    // {
    //     Error_Handler();
    // }
    // if (HAL_TIM_OC_Init(&htim2) != HAL_OK)
    // {
    //     Error_Handler();
    // }
    // sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    // sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    // if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    // {
    //     Error_Handler();
    // }
    // sConfigOC.OCMode = TIM_OCMODE_TIMING;
    // sConfigOC.Pulse = 0;
    // sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    // sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    // if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    // {
    //     Error_Handler();
    // }
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 7200 - 1; // 10kHz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 400 - 1; // 40ms
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
}
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *tim_baseHandle)
{

    if (tim_baseHandle->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_ENABLE();
        HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(TIM2_IRQn);
    }
}

void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
    HAL_TIM_Base_Stop_IT(&htim2);
    if (g_pressed_key > 0)
    {
        g_pressed_key = 0;
    }
}

void EXTI3_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(KEY1_Pin);
}

void EXTI4_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(KEY2_Pin);
}

void EXTI9_5_IRQHandler(void)
{
    if (__HAL_GPIO_EXTI_GET_IT(KEY3_Pin) != RESET)
    {
        HAL_GPIO_EXTI_IRQHandler(KEY3_Pin);
    }

    if (__HAL_GPIO_EXTI_GET_IT(KEY4_Pin) != RESET)
    {
        HAL_GPIO_EXTI_IRQHandler(KEY4_Pin);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
    case KEY1_Pin:
        g_pressed_key = 1;
        break;
    case KEY2_Pin:
        g_pressed_key = 2;
        break;
    case KEY3_Pin:
        g_pressed_key = 3;
        break;
    case KEY4_Pin:
        g_pressed_key = 4;
        break;
    default:
        g_pressed_key = 0;
        break;
    }
    HAL_TIM_Base_Start_IT(&htim2);
}