#include "i2c.h"

#ifdef _SAMD21_

#include "gpio.h"

void i2c_handler(int num) {
    //2c_t *i2c = (i2c_t*)sercom_handlers[num];

	/*
    if ( (spi->hw->SPI.INTFLAG.reg & SERCOM_SPI_INTFLAG_RXC)
             && (spi->hw->SPI.INTENSET.reg & SERCOM_SPI_INTENSET_RXC) ) {
        if (spi->rx_buffer != 0) {
            spi->rx_buffer[spi->cur] = spi->hw->SPI.DATA.reg;
        } else {
            volatile int nop = spi->hw->SPI.DATA.reg; // this guy
            (void)nop;
        }
        spi->cur++;
        if (spi->cur < spi->size) {
            if (spi->tx_buffer != 0) {
                spi->hw->SPI.DATA.reg = spi->tx_buffer[spi->cur];
            } else {
                spi->hw->SPI.DATA.reg = spi->dummy_byte;
            }
        } else {
            spi->hw->SPI.INTENCLR.reg = SERCOM_SPI_INTENCLR_RXC;
            xSemaphoreGiveFromISR(spi->call_mutex, 0);
            portYIELD_FROM_ISR(pdTRUE);
        }
    }
	*/
}


void i2c_init(i2c_t *i2c, int sercom,
                uint8_t pin_sda, uint32_t mux_sda,
                uint8_t pin_scl, uint32_t mux_scl) {
    enable_sercom_clock(sercom);

    gpio_direction(pin_sda, GPIO_DIRECTION_OUT);
    gpio_function(pin_sda, mux_sda);
    gpio_direction(pin_scl, GPIO_DIRECTION_OUT);
    gpio_function(pin_scl, mux_scl); 

    Sercom *hw = (Sercom*)((char*)SERCOM0 + 1024 * sercom);

    while (hw->I2CM.SYNCBUSY.bit.SWRST);

    hw->I2CM.CTRLA.bit.MODE = 5; // Master
    hw->I2CM.CTRLA.bit.SDAHOLD = 3;
    hw->I2CM.CTRLB.bit.SMEN = 1;
    hw->I2CM.CTRLA.bit.SPEED = 0;
    hw->I2CM.CTRLA.bit.INACTOUT = 0;
    hw->I2CM.BAUD.bit.BAUD = 0;

    hw->I2CM.CTRLA.bit.ENABLE = 1;
	while (hw->I2CM.SYNCBUSY.reg);

    NVIC_EnableIRQ(9 + sercom);

    i2c->hw = hw;
    i2c->fn = i2c_handler;

    hw->I2CM.STATUS.bit.BUSSTATE = 1;

	while (hw->I2CM.SYNCBUSY.reg);

    sercom_handlers[sercom] = (dummy_type*)i2c;

    i2c->bus_mutex = xSemaphoreCreateBinary();
    xSemaphoreGive(i2c->bus_mutex);

    i2c->call_mutex = xSemaphoreCreateBinary();
}

int i2c_write(i2c_t *i2c, uint8_t addr, uint8_t *bytes, int len) {

	i2c->hw->I2CM.ADDR.reg = (addr << 1) | 0;

	while (i2c->hw->I2CM.INTFLAG.bit.MB == 0);

	if (i2c->hw->I2CM.STATUS.bit.RXNACK) {
    	i2c->hw->I2CM.CTRLB.bit.CMD = 3;
		return 1;
	}

	for (int i=0; i<len; i++) {
		i2c->hw->I2CM.DATA.reg = bytes[i];
		while (i2c->hw->I2CM.INTFLAG.bit.MB == 0);
		if (i2c->hw->I2CM.STATUS.bit.RXNACK) {
			i2c->hw->I2CM.CTRLB.bit.CMD = 3;
			return 2;
		}
	}

	i2c->hw->I2CM.CTRLB.bit.CMD = 3;
	return 0;
}

int i2c_read(i2c_t *i2c, uint8_t addr, uint8_t *bytes, int len) {
	i2c->hw->I2CM.ADDR.reg = (addr << 1) | 1;

	while (i2c->hw->I2CM.INTFLAG.bit.SB == 0);

	if (i2c->hw->I2CM.STATUS.bit.RXNACK) {
    	i2c->hw->I2CM.CTRLB.bit.CMD = 3;
		return 1;
	}

	i2c->hw->I2CM.CTRLB.bit.ACKACT = 0;

	for (int i=0; i<(len-1); i++) {
		bytes[i] = i2c->hw->I2CM.DATA.reg;
		while (i2c->hw->I2CM.INTFLAG.bit.SB == 0);
	}

	if (len) {
		i2c->hw->I2CM.CTRLB.bit.ACKACT = 1;
    	i2c->hw->I2CM.CTRLB.bit.CMD = 3;
		bytes[len-1] = i2c->hw->I2CM.DATA.reg;
	}

	return 0;
}

#else

int i2c_read(i2c_t *i2c, uint8_t addr, uint8_t *bytes, int len) { return 1; }
int i2c_write(i2c_t *i2c, uint8_t addr, uint8_t *bytes, int len) { return 1; }

void i2c_init(i2c_t *i2c, int sercom,
                uint8_t pin_sda, uint32_t mux_sda,
                uint8_t pin_scl, uint32_t mux_scl) {}

#endif

