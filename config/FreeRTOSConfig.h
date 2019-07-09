#pragma once

#include <stdint.h>
#include "amslah_config.h"

#define configCPU_CLOCK_HZ (CPU_FREQUENCY)

#define configMAX_PRIORITIES (8)

#define configMINIMAL_STACK_SIZE ((uint16_t)64)

#define configTOTAL_HEAP_SIZE ((size_t)(20000))

#define configUSE_MUTEXES 1

#define configUSE_COUNTING_SEMAPHORES 1

#define FREERTOS_ADVANCED_CONFIG 1

#define configUSE_PREEMPTION 1

#define configUSE_RECURSIVE_MUTEXES 1

#define configGENERATE_RUN_TIME_STATS (USAGE_REPORT || HIGH_RESOLUTION_TIMER)

#define configUSE_16_BIT_TICKS 0

#define configCHECK_FOR_STACK_OVERFLOW 0

#define configUSE_MALLOC_FAILED_HOOK 0

#define configUSE_IDLE_HOOK 0

#define configUSE_TICK_HOOK 0

#define configUSE_TICKLESS_IDLE 1

#define configUSE_TRACE_FACILITY 1

#define configUSE_STATS_FORMATTING_FUNCTIONS 1

#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0

#define configUSE_APPLICATION_TASK_TAG 0

#define configUSE_CO_ROUTINES 0

#define configMAX_CO_ROUTINE_PRIORITIES (2)

#define configTICK_RATE_HZ ((TickType_t)1000)

#define configUSE_TIMERS 1

#define configTIMER_TASK_PRIORITY (configMAX_PRIORITIES - 1)

#define configTIMER_QUEUE_LENGTH 10

#define configTIMER_TASK_STACK_DEPTH (128)

#define configPRIO_BITS 3

#define configMAX_TASK_NAME_LEN (8)

#define configIDLE_SHOULD_YIELD 1

#define configQUEUE_REGISTRY_SIZE 0

#define configUSE_QUEUE_SETS 1

#define configENABLE_BACKWARD_COMPATIBILITY 1

#define INCLUDE_vTaskPrioritySet 0

#define INCLUDE_uxTaskPriorityGet 0

#define INCLUDE_vTaskDelete 0

#define INCLUDE_vTaskSuspend 1

#define INCLUDE_xResumeFromISR 0

#define INCLUDE_vTaskDelayUntil 0

#define INCLUDE_vTaskDelay 1

#define INCLUDE_xTaskGetCurrentTaskHandle 0

#define INCLUDE_uxTaskGetStackHighWaterMark 0

#define INCLUDE_xTaskGetIdleTaskHandle 0

#define INCLUDE_xTimerGetTimerDaemonTaskHandle 0

#define INCLUDE_eTaskGetState 0

#define INCLUDE_vTaskCleanUpResources 0

#define INCLUDE_xTimerPendFunctionCall 0

#define configASSERT(x)                                                                                                \
	if ((x) == 0) {                                                                                                    \
		taskDISABLE_INTERRUPTS();                                                                                      \
		for (;;)                                                                                                       \
			;                                                                                                          \
	}

#define vPortSVCHandler SVCall_Handler

#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

/* Used when configGENERATE_RUN_TIME_STATS is 1. */
#if configGENERATE_RUN_TIME_STATS
extern void     vConfigureTimerForRunTimeStats(void);
extern uint32_t vGetRunTimeCounterValue(void);
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() vConfigureTimerForRunTimeStats()
#define portGET_RUN_TIME_COUNTER_VALUE() vGetRunTimeCounterValue()
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 0x07

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 4

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
