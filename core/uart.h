#pragma once

#include "amslah.h"
#include "semphr.h"
#include "stream_buffer.h"

#include "gpio.h"
#include "sercoms.h"

typedef struct {
    sercom_handler_f fn; 
    Sercom *hw;
    uint8_t *tx_buffer;
    uint16_t tx_len;
    uint16_t tx_cur;
    StreamBufferHandle_t rx_buffer;
	SemaphoreHandle_t bus_mutex;
	SemaphoreHandle_t call_mutex;
} uart_t;

inline uint16_t _uart_get_baud_reg(int baud) {
    return 65536 - ((65536 * 16.0f * baud) / PERIPHERAL_FREQUENCY);
}

void uart_init(uart_t *uart, int sercom, int baud, uint8_t pin_tx, uint32_t mux_tx, uint8_t pin_rx, uint32_t mux_rx);
int32_t uart_write(uart_t *uart, uint8_t *buf, uint16_t len);
