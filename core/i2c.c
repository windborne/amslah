#include "i2c.h"

#if 1

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
	i2c->pin_sda = pin_sda;
	i2c->pin_scl = pin_scl;
	if (i2c->bitbang) {
		return;
	}
    enable_sercom_clock(sercom);

    gpio_direction(pin_sda, GPIO_DIRECTION_OUT);
    gpio_function(pin_sda, mux_sda);
    gpio_direction(pin_scl, GPIO_DIRECTION_OUT);
    gpio_function(pin_scl, mux_scl); 

    Sercom *hw = get_sercom(sercom);

    while (hw->I2CM.SYNCBUSY.bit.SWRST);

    hw->I2CM.CTRLA.bit.MODE = 5; // Master
    hw->I2CM.CTRLA.bit.SDAHOLD = 3;
    hw->I2CM.CTRLB.bit.SMEN = 1;
    hw->I2CM.CTRLA.bit.SPEED = 0;
    hw->I2CM.CTRLA.bit.INACTOUT = 0;
	if (i2c->baud != 0) {
    	hw->I2CM.BAUD.bit.BAUD = i2c->baud;
	} else {
    	hw->I2CM.BAUD.bit.BAUD = i2c->slow ? 20 : 0;
	}

    hw->I2CM.CTRLA.bit.ENABLE = 1;
	while (hw->I2CM.SYNCBUSY.reg);

    enable_sercom_irq(sercom);

    i2c->hw = hw;
    i2c->fn = i2c_handler;

    hw->I2CM.STATUS.bit.BUSSTATE = 1;

	while (hw->I2CM.SYNCBUSY.reg);

	//print("hey wtf sercom is %d\n", sercom);

    //sercom_handlers[sercom] = (dummy_type*)i2c;

    i2c->bus_mutex = xSemaphoreCreateBinary();

    xSemaphoreGive(i2c->bus_mutex);

    i2c->call_mutex = xSemaphoreCreateBinary();
}

#ifndef _SAMD21_
#define PORT_IOBUS PORT
#endif

static inline void gpio_direction_fast(uint8_t pin, enum gpio_direction direction) {
    if (direction == GPIO_DIRECTION_OUT) {
        uint32_t pinsel = 1U << GPIO_PIN(pin);
        PORT_IOBUS->Group[GPIO_PORT(pin)].DIRSET.reg = pinsel;
        PORT_IOBUS->Group[GPIO_PORT(pin)].WRCONFIG.reg = PORT_WRCONFIG_WRPINCFG | (pinsel & 0xffff);
        PORT_IOBUS->Group[GPIO_PORT(pin)].WRCONFIG.reg =
            PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_WRPINCFG | ((pinsel & 0xffff0000) >> 16);
    } else if (direction == GPIO_DIRECTION_IN) {
        uint32_t pinsel = 1U << GPIO_PIN(pin);
        PORT_IOBUS->Group[GPIO_PORT(pin)].DIRCLR.reg = pinsel;
        PORT_IOBUS->Group[GPIO_PORT(pin)].WRCONFIG.reg = PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_INEN | (pinsel & 0xffff);
        PORT_IOBUS->Group[GPIO_PORT(pin)].WRCONFIG.reg =
                PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_INEN | ((pinsel & 0xffff0000) >> 16);
    } 
}

static inline void digital_set_fast(uint8_t pin, uint8_t level) {
    if (level) {
        PORT_IOBUS->Group[GPIO_PORT(pin)].OUTSET.reg = 1U << GPIO_PIN(pin);
    } else {
        PORT_IOBUS->Group[GPIO_PORT(pin)].OUTCLR.reg = 1U << GPIO_PIN(pin);
    }
}


/*
#define data_high() digital_in_init(i2c->pin_sda);

#define clk_high() digital_in_init(i2c->pin_scl);

#define data_low() digital_out_init(i2c->pin_sda); digital_set(i2c->pin_sda, LOW);

#define clk_low() digital_out_init(i2c->pin_scl); digital_set(i2c->pin_scl, LOW);
*/

#define data_high() gpio_direction_fast(i2c->pin_sda, GPIO_DIRECTION_IN);

#define clk_high() gpio_direction_fast(i2c->pin_scl, GPIO_DIRECTION_IN);

#define data_low() gpio_direction_fast(i2c->pin_sda, GPIO_DIRECTION_OUT); digital_set_fast(i2c->pin_sda, LOW);

#define clk_low()  gpio_direction_fast(i2c->pin_scl, GPIO_DIRECTION_OUT); digital_set_fast(i2c->pin_scl, LOW);


#define _bitbang_delay() 

//for (volatile int ii=0; ii<1; ii++) { asm("nop"); } 

// if (i2c->slow) { asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); asm("nop"); }

//#define _bitbang_delay() asm("nop"); // vTaskDelay(1);

inline static void _bitbang_start(i2c_t *i2c) {
	clk_high();
	data_high();
	_bitbang_delay();
	data_low();
	_bitbang_delay();
	//uint32_t t0 = xTaskGetTickCount();
    //while((!digital_get(i2c->pin_scl)) && (xTaskGetTickCount() - t0) < 5) {};
	clk_low();
}

inline static uint8_t _bitbang_write(i2c_t *i2c, uint8_t byte) {
	uint8_t ret = 0;
	//uint8_t inbit;
	
	for (int i=0; i<8; i++) {
		if (byte & 0x80) {
			data_high();
		} else {
			data_low();
		}
		byte <<= 1;
		_bitbang_delay();
		clk_high();
		_bitbang_delay();
		clk_low();
	}
	data_high();
	_bitbang_delay();
	clk_high();
	_bitbang_delay();
	//uint32_t t0 = xTaskGetTickCount();
    //while((!digital_get(i2c->pin_scl)) && (xTaskGetTickCount() - t0) < 5) {};
	for (volatile int ii=0; ii<4; ii++) { asm("nop"); }

	if (digital_get(i2c->pin_sda)) ret = 1;
	/* read data here */
	clk_low();
	return ret;
}

#define ACK 0
#define NACK 1

inline static uint8_t _bitbang_read(i2c_t *i2c, uint8_t a) {
	data_high();
	uint8_t c = 0;
	for (int i=0; i<8; i++) {
		c <<= 1;
		if (digital_get(i2c->pin_sda)) c |= 1;
		clk_high();
		_bitbang_delay();
		clk_low();
	}
	if (a == ACK) {
		data_low();
	}
	clk_high();
	_bitbang_delay();
	clk_low();
	return c;
}

inline static void _bitbang_stop(i2c_t *i2c) {
	data_low();
	clk_high();
	_bitbang_delay();
	data_high();
}

int i2c_write(i2c_t *i2c, uint8_t addr, uint8_t *bytes, int len) {
if (!i2c->bitbang) {
	i2c->hw->I2CM.ADDR.reg = (addr << 1) | 0;

	uint32_t t0 = xTaskGetTickCount();
	while (i2c->hw->I2CM.INTFLAG.bit.MB == 0 && (xTaskGetTickCount() - t0 < 20)) {;}
	//print("took %d ms\n", xTaskGetTickCount()-t0);
	/*
		if (i++ > 10) print("waiting 1\n");

		vTaskDelay(10);
	}
*/

	if (i2c->hw->I2CM.STATUS.bit.RXNACK || i2c->hw->I2CM.INTFLAG.bit.MB == 0) {
    	i2c->hw->I2CM.CTRLB.bit.CMD = 3;
		return 1;
	}

	for (int i=0; i<len; i++) {
		i2c->hw->I2CM.DATA.reg = bytes[i];
		//int j = 0;
		//micros.start();
		t0 = xTaskGetTickCount();
		while (i2c->hw->I2CM.INTFLAG.bit.MB == 0 && (xTaskGetTickCount() - t0 < 20)) {/*
				if (j++ > 10) print("waiting 2\n");

				vTaskDelay(10);*/
		}
		//print("took %d ms\n", xTaskGetTickCount()-t0);
		if (i2c->hw->I2CM.STATUS.bit.RXNACK || i2c->hw->I2CM.INTFLAG.bit.MB == 0) {
			i2c->hw->I2CM.CTRLB.bit.CMD = 3;
			return 2;
		}
	}

	if (!i2c->nostop) i2c->hw->I2CM.CTRLB.bit.CMD = 3;
	return 0;
} else {
portENTER_CRITICAL();
	_bitbang_start(i2c);
	_bitbang_write(i2c, (addr << 1) | 0);
	for (int i=0; i<len; i++) {
		_bitbang_write(i2c, bytes[i]);
	}
	if (!i2c->nostop) _bitbang_stop(i2c);
portEXIT_CRITICAL();
	return 0;
}
}

int i2c_read(i2c_t *i2c, uint8_t addr, uint8_t *bytes, int len) {
if (!i2c->bitbang) {
	i2c->hw->I2CM.ADDR.reg = (addr << 1) | 1;

	uint32_t t0 = xTaskGetTickCount();
	while (i2c->hw->I2CM.INTFLAG.bit.SB == 0 && (xTaskGetTickCount()-t0 < 20));

	if (i2c->hw->I2CM.STATUS.bit.RXNACK || i2c->hw->I2CM.INTFLAG.bit.SB == 0) {
    	i2c->hw->I2CM.CTRLB.bit.CMD = 3;
		return 1;
	}

	i2c->hw->I2CM.CTRLB.bit.ACKACT = 0;

	for (int i=0; i<(len-1); i++) {
		bytes[i] = i2c->hw->I2CM.DATA.reg;
		t0 = xTaskGetTickCount();
		while (i2c->hw->I2CM.INTFLAG.bit.SB == 0 && (xTaskGetTickCount()-t0 < 20));
	}

	if (len) {
		i2c->hw->I2CM.CTRLB.bit.ACKACT = 1;
    	i2c->hw->I2CM.CTRLB.bit.CMD = 3;
		bytes[len-1] = i2c->hw->I2CM.DATA.reg;
	}

	return 0;
} else {
portENTER_CRITICAL();
		_bitbang_start(i2c);
		_bitbang_write(i2c, (addr << 1) | 1);
	for (int i=0; i<len; i++) {
		bytes[i] = _bitbang_read(i2c, (i == (len-1)) ? NACK : ACK);
	}
	_bitbang_stop(i2c);
portEXIT_CRITICAL();
	return 0;
}
}

#else

int i2c_read(i2c_t *i2c, uint8_t addr, uint8_t *bytes, int len) { return 1; }
int i2c_write(i2c_t *i2c, uint8_t addr, uint8_t *bytes, int len) { return 1; }

void i2c_init(i2c_t *i2c, int sercom,
                uint8_t pin_sda, uint32_t mux_sda,
                uint8_t pin_scl, uint32_t mux_scl) {}

#endif
