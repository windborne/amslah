#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if USE_DEBUG_UART

#include "uart.h"
#include "printf.h"

extern uart_t debug_uart;


/**
 * @brief Initialize the debug serial, as enabled by USE_DEBUG_UART and
 *        configured with the various DEBUG_* options in the configuration
 *        file.
 */
void init_serial();


/**
 * @brief printf-like debug interface that prints to the debug serial. It
 *        formats the time and the task that called the print function.
 *
 * @param fmt Formatting string.
 * @param ... Optional formatting arguments.
 */
void print(const char *fmt, ...);

#endif

#if USAGE_REPORT_TC >= 3
    #define HRT_RES 16
#else
    #define HRT_RES 24
#endif

#ifdef __cplusplus
}
#endif
