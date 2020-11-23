#pragma once

#ifdef __cplusplus
extern "C" {
#endif

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
    uint8_t dummy_byte;

    uint16_t size;
    uint16_t cur;

	SemaphoreHandle_t bus_mutex;
	SemaphoreHandle_t call_mutex;

	int cs;
} i2c_t;

void i2c_init(i2c_t *i2c, int sercom,
				uint8_t pin_sda, uint32_t mux_sda,
				uint8_t pin_scl, uint32_t mux_scl);

int i2c_write(i2c_t *i2c, uint8_t addr, uint8_t *bytes, int len);

int i2c_read(i2c_t *i2c, uint8_t addr, uint8_t *bytes, int len);

#ifdef __cplusplus
}
#endif
