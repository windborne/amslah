#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sammy.h"
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
} spi_t;


/**
 * @brief Compute BAUD register for SPI operation
 *
 * Internal function that should not be necessary to use manually.
 *
 * @param baud Baud rate in Hz.
 *
 * @return Baud register value.
 */
inline uint16_t _spi_get_baud_reg(int baud) {
    return PERIPHERAL_FREQUENCY/(2 * baud) - 1;
}


/**
 * @brief Initialize the SPI driver on a particular SERCOM.
 *
 * The interface requires a lot of configuration and will change for the better
 * in the near future.
 *
 * The baud rate is for now initialized to 333 kHz but will soon be
 * configurable.
 *
 * @param[out] spi SPI structure (spi_t) that will be filled up with the
 *                 descriptor for this bus.
 * @param sercom Which SERCOM number to use, 0-6.
 * @param dipo RXPO setting (see CTRLA register).
 * @param dopo TXPO setting (see CTRLA register).
 * @param pin_sck Which pin to use for SCK.
 * @param mux_sck Pinmux configuration for the SCK pin.
 * @param pin_mosi Which pin to use for MOSI.
 * @param mux_mosi Pinmux configuration for the MOSI pin.
 * @param pin_miso Which pin to use for MISO.
 * @param mux_miso Pinmux configuration for the MISO pin.
 */
void spi_init(spi_t *spi, int sercom, int dipo, int dopo,
				uint8_t pin_sck, uint32_t mux_sck,
				uint8_t pin_mosi, uint32_t mux_mosi,
				uint8_t pin_miso, uint32_t mux_miso);


/**
 * @brief Performs a SPI transfer on bus spi. It does not enforce ownership
 *        of the bus and therefore the use of spi_take() and spi_give() is
 *        suggested to make sure multiple threads don't use the same bus at
 *        once.
 *
 * @param spi SPI bus to do the transfer in.
 * @param tx_buf TX buffer.
 * @param rx_buf RX buffer.
 * @param size Number of bytes to send.
 *
 * @return Returns the number of bytes sent if successful, or ERR_TIMEOUT
 *         otherwise.
 *
 * @effects The function will wait (asynchronously) until the transfer is
 *          complete.
 */
int32_t spi_transfer(spi_t *spi, uint8_t *tx_buf, uint8_t *rx_buf, int size);


/**
 * @brief Takes ownership of the given SPI bus using a given chip select pin.
 *
 * @param spi SPI bus to take ownership of.
 * @param cs Chip select pin to activate (low).
 */
void spi_take(spi_t *spi, int cs);

void spi_set_baud(spi_t *spi, int baud);


/**
 * @brief Gives away a SPI bus and deactivates the chip select pin used in
 *        spi_take().
 *
 * @param spi SPI bus to give away.
 */
void spi_give(spi_t *spi);

#ifdef __cplusplus
}
#endif
