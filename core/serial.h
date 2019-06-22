#pragma once

#include "amslah.h"

#if USE_DEBUG_UART

#include "uart.h"
#include "printf.h"

extern uart_t debug_uart;

void init_serial();
void print(char *fmt, ...);

#endif
