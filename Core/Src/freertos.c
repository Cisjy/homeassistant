/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "string.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hard_init.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void initTask(void const *argument);
void StartTask02(void const *argument);
void StartTask03(void const *argument);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize);

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize)
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
  /* USER CODE BEGIN Init */
  uint8_t result = pdFALSE;
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */

  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  result = xTaskCreate((TaskFunction_t)initTask, "initTask", 128, NULL, 1, NULL);
  if (result == pdTRUE)
  {
    printf("initTask creation successful\r\n");
  }

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */
}
#if 0
void printFreeHeapSize(void)
{
  size_t freeHeapSize = xPortGetFreeHeapSize();
  size_t minEverFreeHeapSize = xPortGetMinimumEverFreeHeapSize();

  USART1_printf("Current free heap size: %d bytes\r\n", (unsigned int)freeHeapSize);
  USART1_printf("Minimum ever free heap size: %d bytes\r\n", (unsigned int)minEverFreeHeapSize);
}

void printStackHighWaterMark(void)
{
  UBaseType_t uxHighWaterMark;
  uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL); // NULL 表示当前任务
  USART1_printf("Stack high water mark: %d words\r\n", uxHighWaterMark);
}
#endif
/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void initTask(void const *argument)
{
  uint8_t result = pdFALSE;
  taskENTER_CRITICAL();
  result = xTaskCreate((TaskFunction_t)StartTask02,
                       "mytask02",
                       64,
                       NULL,
                       1,
                       NULL);
  if (result == pdTRUE)
  {
    printf("Task02 creation successful\r\n");
  }
  result = xTaskCreate((TaskFunction_t)StartTask03,
                       "mytask03",
                       512,
                       NULL,
                       1,
                       NULL);
  if (result == pdTRUE)
  {
    printf("Task03 creation successful\r\n");
  }
  // printFreeHeapSize();
  vTaskDelete(NULL);
  taskEXIT_CRITICAL();
}

void StartTask02(void const *argument)
{

  for (;;)
  {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_14);
    osDelay(2000);
  }
}
void StartTask03(void const *argument)
{

  for (;;)
  {
    oled_display(&oled_page);

    osDelay(10);
  }
}
