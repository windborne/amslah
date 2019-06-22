#pragma once

#include "samd21.h"
#include "amslah_config.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "stream_buffer.h"

#include "gpio.h"
#include "sercoms.h"

typedef struct {
    sercom_handler_f fn; 
    Sercom *hw;
    uint8_t *tx_buffer;
    uint8_t *rx_buffer;

    uint16_t size;
    uint16_t cur;

	SemaphoreHandle_t bus_mutex;
	SemaphoreHandle_t call_mutex;

	int cs;
} spi_t;

/*inline uint16_t _uart_get_baud_reg(int baud) {
    return 65536 - ((65536 * 16.0f * baud) / 4000000);
}*/

void spi_init(spi_t *spi, int sercom, int dipo, int dopo,
				uint8_t pin_sck, uint32_t mux_sck,
				uint8_t pin_mosi, uint32_t mux_mosi,
				uint8_t pin_miso, uint32_t mux_miso);
//int32_t uart_write(uart_t *uart, uint8_t *buf, uint16_t len);

int32_t spi_transfer(spi_t *spi, uint8_t *tx_buf, uint8_t *rx_buf, int size);

void spi_take(spi_t *spi, int cs);
void spi_give(spi_t *spi);
