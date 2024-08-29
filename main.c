/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************

-------------------------- END-OF-HEADER -----------------------------

Задание
Реализовать систему с использованием механизма взаимного исключения при доступе к общему
аппаратному ресурсу микроконтроллера. Основные условия:
1. Система имеет 2 задачи с одинаковым приоритетом (приоритет 1).
2. Задача 1 – периодическая (период 1 с), передаёт по последовательному интерфейсу связи строку «Task
1 working».
3. Задача 2 – периодическая (период выбирается случайным в диапазоне от 1 до 3 с), передаёт по
последовательному интерфейсу связи строку «Task 2 working».
4. Обе задачи используют один и тот же последовательный интерфейс для передачи строки на ПК.
5. Выполнить сборку, запуск и тестирование проекта на отладочной плате.
Критерии готовности (Definition of Done)
1. Проект собирается и загружается в отладочную плату без ошибок и предупреждений.
2. По последовательному порту передаются сообщения от двух задач. Сообщения передаются без
искажений и нарушений следования символов (обе задачи не перебивают друг друга).
3. Сообщения от задачи 1 передаются строго каждую 1 с. Сообщения от задачи 2 имеют плавающую
периодичность.
4. В логике работы задач, для реализации взаимного исключения при доступе к интерфейсу связи,
используется мьютекс FreeRTOS.
5. Вывод строки осуществляется через примитивную функцию отправки пакета байт по UART без
использования функции printf.


USING STM32F407VET6 board:

SWD interface
PA9 & PA10 - USART1
*/
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "RCC_Init.h"
#include "usart_dbg.h"
#include "rng.h"


#include <stdio.h>
#include <stdlib.h>


/************************global values********************************/
#define SEMAPHORE_TIMEOUT 500
/* Mutex handler */
SemaphoreHandle_t mutex_usart1;
/****************************func************************************/

void vTask1Sec(void * pvParameters){
  while(1){
    if(xSemaphoreTake(mutex_usart1, SEMAPHORE_TIMEOUT)==pdTRUE){
      usart1_send_str("Task 1 working\n");
      xSemaphoreGive(mutex_usart1);
    }
    vTaskDelay(1000);
  }
  vTaskDelete(NULL);
}

void vTaskRandomSec(void * pvParameters){
  while(1){
    if(xSemaphoreTake(mutex_usart1, SEMAPHORE_TIMEOUT)==pdTRUE){
      usart1_send_str("Task 2 working\n");
      xSemaphoreGive(mutex_usart1);
    }
    uint32_t time_delay = ((float)rng_random() / RNG_RANDOM_MAX)*2000 + 1000;
    vTaskDelay(time_delay);
  }
  vTaskDelete(NULL);
}
/****************************main************************************/

 int main(void) {
  RCC_Init();
  usart1_init();
  rng_init();
  mutex_usart1 = xSemaphoreCreateMutex();

  xTaskCreate(vTask1Sec, "vTask1Sec", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  xTaskCreate(vTaskRandomSec, "vTaskRandomSec", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  vTaskStartScheduler();
  while(1){}
}
/**************************Interrupt func****************************/


/*************************** End of file ****************************/
