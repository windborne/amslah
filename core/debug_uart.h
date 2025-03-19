#pragma once

#include "amslah.h"
#include "ring_buffer.h"
#include "user_amslah_config.h"


#if USE_DEBUG_UART_V2

extern RingBuffer_t debug_uart_tx;
extern RingBuffer_t debug_uart_rx;


/// Claude was here
#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)
#define DEBUG_SERCOM CONCAT(SERCOM, DEBUG_UART_SERCOM)
// If you're getting a compile error, try removing the parentheses in this line in user_amslah_config.h:
// Wrong: #define DEBUG_UART_SERCOM (1)
// Right: #define DEBUG_UART_SERCOM 1


#define SERCOMn_0_Handler(n) SERCOM##n##_0_Handler
#define SERCOMn_1_Handler(n) SERCOM##n##_1_Handler
#define SERCOMn_2_Handler(n) SERCOM##n##_2_Handler
#define SERCOMn_3_Handler(n) SERCOM##n##_3_Handler

#define DEBUG_UART_0_Handler SERCOMn_0_Handler(DEBUG_UART_SERCOM)
#define DEBUG_UART_1_Handler SERCOMn_1_Handler(DEBUG_UART_SERCOM)
#define DEBUG_UART_2_Handler SERCOMn_2_Handler(DEBUG_UART_SERCOM)
#define DEBUG_UART_3_Handler SERCOMn_3_Handler(DEBUG_UART_SERCOM)


#define _SERCOM_IRQn(num, irqnum) SERCOM##num##_##irqnum##_IRQn
#define SERCOM_IRQn(num, irqnum) _SERCOM_IRQn(num, irqnum)

#define SERCOMn_0_IRQn(n) SERCOM_IRQn(n, 0)
#define SERCOMn_1_IRQn(n) SERCOM_IRQn(n, 1)
#define SERCOMn_2_IRQn(n) SERCOM_IRQn(n, 2)
#define SERCOMn_3_IRQn(n) SERCOM_IRQn(n, 3)

#define DEBUG_UART_0_IRQn SERCOMn_0_IRQn(DEBUG_UART_SERCOM)
#define DEBUG_UART_1_IRQn SERCOMn_1_IRQn(DEBUG_UART_SERCOM)
#define DEBUG_UART_2_IRQn SERCOMn_2_IRQn(DEBUG_UART_SERCOM)
#define DEBUG_UART_3_IRQn SERCOMn_3_IRQn(DEBUG_UART_SERCOM)


void debug_uart_init(void);
void flush_to_uart(void);
void debug_tx_send(const char* message, uint32_t length);
uint32_t wait_until_string(const char* response, uint32_t len, uint32_t timeout);
bool wait_for_bytes(uint32_t n_bytes);

// Interrupt handlers
void DEBUG_UART_0_Handler(void);  // Data Register Empty interrupt
void DEBUG_UART_1_Handler(void);  // Transmit Complete interrupt
void DEBUG_UART_2_Handler(void);  // Receive Complete interrupt
void DEBUG_UART_3_Handler(void);  // Receive start, clear to send, receive break, error


#endif // USE_DEBUG_UART_V2
