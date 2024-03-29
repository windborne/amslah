#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sammy.h"
#include "FreeRTOS.h"
#include "task.h"
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

typedef struct {
	uint8_t sercom;
	uint8_t pin_tx;
	uint8_t mux_tx;
	uint8_t pin_rx;
	uint8_t mux_rx;
	uint8_t txpo;
	uint8_t rxpo;
	uint32_t baud;
	uint32_t def;
} uartcfg_t;

void uart_init_with(uart_t *uart, uartcfg_t cfg);


/**
 * @brief Compute BAUD register for UART operation
 *
 * Internal function that should not be necessary to use manually.
 *
 * @param baud Baud rate in Hz.
 *
 * @return Baud register value.
 */
uint16_t _uart_get_baud_reg(int baud);


/**
 * @brief Initialize the UART bus on a given sercom.
 *
 * @param uart UART structure (uart_t) that will be filled up with the
 *             descriptor for this bus.
 * @param sercom Which SERCOM number to use (0-6).
 * @param baud Baud rate in Hz.
 * @param pin_tx Which pin to use for TX.
 * @param mux_tx Pinmux configuration for the TX pin.
 * @param pin_rx Which pin to use for RX.
 * @param mux_rx Pinmux configuration for the TX pin.
 */
void uart_init(uart_t *uart, int sercom, int baud, uint8_t pin_tx, uint32_t mux_tx, uint8_t pin_rx, uint32_t mux_rx);


/**
 *@brief Like the above function for for reiniting uart
 */
void uart_reinit(uart_t *uart, int sercom, int baud, uint8_t pin_tx, uint32_t mux_tx, uint8_t pin_rx, uint32_t mux_rx);

/**
 * @brief Writes a buffer to a given UART port.
 *
 * @param uart UART bus to write to.
 * @param buf Buffer that holds the data to send.
 * @param len Number of bytes to send from the buffer.
 *
 * @return Returns the number of writes written, or ERR_TIMEOUT if there's an
 *         error.
 *
 * @effects This function waits asynchronously until the transfer is complete.
 */
int32_t uart_write(uart_t *uart, const uint8_t *buf, uint16_t len);


/**
 * @brief Reads a single byte from a given UART port.
 *
 * @param uart UART bus to read from.
 *
 * @return Byte read from the UART bus.
 *
 * @effects This function is NOT protected and must be only used from a single
 *          thread. It waits until a byte is available.
 */
char uart_readt(uart_t *uart, uint32_t timeout);

char uart_read(uart_t *uart);

uint32_t uart_available(uart_t *uart);

void uart_start_listening(uart_t *uart);

void uart_stop_listening(uart_t *uart);

#ifdef __cplusplus
}
#endif
