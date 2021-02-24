#include "spi.h"

#include "gpio.h"

void spi_handler(int num) {
    spi_t *spi = (spi_t*)sercom_handlers[num];

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
}


uint8_t Read = 255;
int nb = 0;

void spi_handler_slave(int num) {
    spi_slave_t *spi = (spi_slave_t*)sercom_handlers[num];

	uint32_t flag = spi->hw->SPI.INTFLAG.reg;
	uint32_t set = spi->hw->SPI.INTENSET.reg;

    if ( (flag & SERCOM_SPI_INTFLAG_SSL)
             && (set & SERCOM_SPI_INTENSET_SSL) ) {
		spi->hw->SPI.INTFLAG.bit.SSL = 1;
		Read = 255;
		nb = 0;
	}

    if ( (flag & SERCOM_SPI_INTFLAG_RXC)
             && (set & SERCOM_SPI_INTENSET_RXC) ) {
		Read = spi->hw->SPI.DATA.reg;
		nb++;
		if (nb == 1) {
			spi->write = (Read & 128) == 128;
			Read = Read & 127;
			if (Read >= spi->nreg) Read = 0;
			spi->data = spi->regs[Read].ptr;
			spi->max = spi->regs[Read].len;
			spi->cur = -1;
		} else if (nb == 2) {
			spi->cur = Read;
		} else if (spi->write && nb >= 3) {
			if (spi->cur < spi->max && spi->cur >= 0) {
				spi->data[spi->cur++] = Read;
			}
		}
		//spi->hw->SPI.DATA.reg = 42;
	}


    if ( (flag & SERCOM_SPI_INTFLAG_DRE)
             && (set & SERCOM_SPI_INTENSET_DRE) ) {
		uint8_t out = 0;
		if ((!spi->write) && spi->cur < spi->max && spi->cur >= 0) {
			out = spi->data[spi->cur++];	
		}
		spi->hw->SPI.DATA.reg = out;
	}

    if ( (flag & SERCOM_SPI_INTFLAG_TXC)
             && (set & SERCOM_SPI_INTENSET_TXC) ) {
		spi->xfer_t = xTaskGetTickCount();
		spi->hw->SPI.INTFLAG.bit.TXC = 1;
	}

	/*
    if ( (spi->hw->SPI.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC)
             && (spi->hw->SPI.INTENSET.reg & SERCOM_SPI_INTENSET_TXC) ) {
		spi->hw->SPI.INTFLAG.bit.TXC = 1;
		Read = 0;
		//spi->hw->SPI.DATA.reg = 0;
	}
	*/

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


void spi_init_slave(spi_slave_t *spi, int sercom, int dipo, int dopo,
				uint8_t pin_ss, uint32_t mux_ss,
                uint8_t pin_sck, uint32_t mux_sck,
                uint8_t pin_mosi, uint32_t mux_mosi,
                uint8_t pin_miso, uint32_t mux_miso) {
    enable_sercom_clock(sercom);

    gpio_direction(pin_ss, GPIO_DIRECTION_IN);
    gpio_function(pin_ss, mux_ss); 

    gpio_direction(pin_sck, GPIO_DIRECTION_OUT);
    gpio_function(pin_sck, mux_sck);

    gpio_direction(pin_mosi, GPIO_DIRECTION_OUT);
    gpio_function(pin_mosi, mux_mosi); 

    gpio_direction(pin_miso, GPIO_DIRECTION_IN);
    gpio_function(pin_miso, mux_miso); 

    Sercom *hw = (Sercom*)((char*)SERCOM0 + 1024 * sercom);

    while (hw->SPI.SYNCBUSY.bit.SWRST);

    hw->SPI.CTRLA.bit.MODE = 2; // Slave
    hw->SPI.CTRLB.bit.MSSEN = 0;
    hw->SPI.CTRLA.bit.DORD = 0;
    hw->SPI.CTRLA.bit.CPOL = 0;
    hw->SPI.CTRLA.bit.CPHA = 0;
    hw->SPI.CTRLA.bit.DIPO = dipo;
    hw->SPI.CTRLA.bit.DOPO = dopo;
    hw->SPI.CTRLA.bit.FORM = 0;

    hw->SPI.CTRLB.bit.RXEN = 1; // Enable RX
    hw->SPI.CTRLB.bit.CHSIZE = 0; // Single byte
    hw->SPI.CTRLB.bit.SSDE = 1; // slave low detect
    //hw->SPI.CTRLB.bit.PLOADEN = 1; // preload

	
	//hw->SPI.DATA.reg = 0;
    hw->SPI.INTENSET.reg = SERCOM_SPI_INTENSET_RXC | SERCOM_SPI_INTENSET_DRE | SERCOM_SPI_INTENSET_SSL | SERCOM_SPI_INTENSET_TXC;
    

    hw->SPI.CTRLA.bit.ENABLE = 1;
    while (hw->SPI.SYNCBUSY.bit.CTRLB);

    NVIC_EnableIRQ(9 + sercom);

    spi->hw = hw;
    spi->fn = spi_handler_slave;

    sercom_handlers[sercom] = (dummy_type*)spi;
}

void spi_init(spi_t *spi, int sercom, int dipo, int dopo,
                uint8_t pin_sck, uint32_t mux_sck,
                uint8_t pin_mosi, uint32_t mux_mosi,
                uint8_t pin_miso, uint32_t mux_miso) {
    enable_sercom_clock(sercom);

    gpio_direction(pin_sck, GPIO_DIRECTION_OUT);
    gpio_function(pin_sck, mux_sck);

    gpio_direction(pin_mosi, GPIO_DIRECTION_OUT);
    gpio_function(pin_mosi, mux_mosi); 

    gpio_direction(pin_miso, GPIO_DIRECTION_IN);
    gpio_function(pin_miso, mux_miso); 

#ifdef _SAMD21_
    Sercom *hw = (Sercom*)((char*)SERCOM0 + 1024 * sercom);
#else
	Sercom *hw;
	switch (sercom) {
	case 0:
		hw = SERCOM0; break;
	case 1:
		hw = SERCOM1; break;
	case 2:
		hw = SERCOM2; break;
	case 3:
		hw = SERCOM3; break;
	case 4:
		hw = SERCOM4; break;
	case 5:
		hw = SERCOM5; break;
	case 6:
		hw = SERCOM6; break;
	case 7:
		hw = SERCOM7; break;
	}
#endif

    while (hw->SPI.SYNCBUSY.bit.SWRST);

    hw->SPI.CTRLA.bit.MODE = 3; // Master
    hw->SPI.CTRLB.bit.MSSEN = 0;
    hw->SPI.CTRLA.bit.DORD = 0;
    hw->SPI.CTRLA.bit.CPOL = 0;
    hw->SPI.CTRLA.bit.CPHA = 0;
    hw->SPI.CTRLA.bit.DIPO = dipo;
    hw->SPI.CTRLA.bit.DOPO = dopo;
    hw->SPI.CTRLA.bit.FORM = 0;

    hw->SPI.CTRLB.bit.RXEN = 1; // Enable RX
    hw->SPI.CTRLB.bit.CHSIZE = 0; // Single byte

    hw->SPI.BAUD.reg = 5;

    hw->SPI.INTENSET.reg = SERCOM_SPI_INTENSET_RXC;
    

    hw->SPI.CTRLA.bit.ENABLE = 1;
    while (hw->SPI.SYNCBUSY.bit.CTRLB);

	enable_sercom_irq(sercom);

    spi->hw = hw;
    spi->fn = spi_handler;
    spi->dummy_byte = 0x0;
    spi->bus_mutex = xSemaphoreCreateBinary();
    xSemaphoreGive(spi->bus_mutex);

    spi->call_mutex = xSemaphoreCreateBinary();

    sercom_handlers[sercom] = (dummy_type*)spi;
}

void spi_set_baud(spi_t *spi, int baud) {
    xSemaphoreTake(spi->bus_mutex, portMAX_DELAY);
    spi->hw->SPI.CTRLA.bit.ENABLE = 0;
    while (spi->hw->SPI.SYNCBUSY.bit.ENABLE);
    spi->hw->SPI.BAUD.reg = baud;
    spi->hw->SPI.CTRLA.bit.ENABLE = 1;
    while (spi->hw->SPI.SYNCBUSY.bit.ENABLE);
    xSemaphoreGive(spi->bus_mutex);
}

int32_t spi_transfer(spi_t *spi, uint8_t *tx_buf, uint8_t *rx_buf, int size) {
    spi->tx_buffer = tx_buf;
    spi->rx_buffer = rx_buf;
    spi->size = size;
    spi->cur = 0;

    spi->hw->SPI.INTENSET.reg = SERCOM_SPI_INTENSET_RXC;
    if (tx_buf != 0) {
        spi->hw->SPI.DATA.reg = tx_buf[0];
    } else {
        spi->hw->SPI.DATA.reg = spi->dummy_byte;
    }

    int32_t ret = xSemaphoreTake(spi->call_mutex, IO_MAX_DELAY) ? size : ERR_TIMEOUT;
    return ret;
}

void spi_take(spi_t *spi, int cs) {
    xSemaphoreTake(spi->bus_mutex, portMAX_DELAY);
    spi->cs = cs;
    digital_set(cs, LOW);
}

void spi_give(spi_t *spi) {
    digital_set(spi->cs, HIGH);
    xSemaphoreGive(spi->bus_mutex);
}

