#include "spi.h"

void spi_handler(int num) {
    spi_t *spi = (spi_t*)sercom_handlers[num];

    /*if ( (spi->hw->SPI.INTFLAG.reg & SERCOM_SPI_INTFLAG_TXC)
             && (spi->hw->SPI.INTENSET.reg & SERCOM_SPI_INTENSET_TXC) ) {
        spi->hw->SPI.INTENCLR.reg = SERCOM_SPI_INTENCLR_TXC;
    } else */
    if ( (spi->hw->SPI.INTFLAG.reg & SERCOM_SPI_INTFLAG_RXC)
             && (spi->hw->SPI.INTENSET.reg & SERCOM_SPI_INTENSET_RXC) ) {
        spi->rx_buffer[spi->cur++] = spi->hw->SPI.DATA.reg;
        if (spi->cur < spi->size) {
            spi->hw->SPI.DATA.reg = spi->tx_buffer[spi->cur];
        } else {
            spi->hw->SPI.INTENCLR.reg = SERCOM_SPI_INTENCLR_RXC;
            xSemaphoreGiveFromISR(spi->call_mutex, 0);
            portYIELD_FROM_ISR(pdTRUE);
        }
    }
    /* else if ( (spi->hw->SPI.INTFLAG.reg & SERCOM_SPI_INTFLAG_DRE)_
             && (spi->hw->SPI.INTENSET.reg & SERCOM_SPI_INTENSET_DRE) ) {
        spi->hw->SPI.INTENCLR.reg = SERCOM_SPI_INTENCLR_DRE;
    }*/
/*
    uart_t *uart = (uart_t*)sercom_handlers[num];
    if ( (uart->hw->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_DRE)
            && (uart->hw->USART.INTENSET.reg & SERCOM_USART_INTENSET_DRE) ) {
        if (uart->tx_cur != uart->tx_len) {
            uart->hw->USART.DATA.reg = uart->tx_buffer[uart->tx_cur];
            uart->tx_cur++;
        } else {
            uart->hw->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
            uart->hw->USART.INTENSET.bit.TXC = 1;
        }
    }
    if ( (uart->hw->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_TXC)
             && (uart->hw->USART.INTENSET.reg & SERCOM_USART_INTENSET_TXC) ) {
        uart->hw->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_TXC;
        xSemaphoreGiveFromISR(uart->call_mutex, 0);
        //gpio_set(GPIO(GPIO_PORTB, 30), LOW);
    }
    if ( (uart->hw->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_RXC)
             && (uart->hw->USART.INTENSET.reg & SERCOM_USART_INTENSET_RXC)) {
        char byte = uart->hw->USART.DATA.reg;
        //gpio_set(GPIO(GPIO_PORTB, 30), LOW);
        //while (1) {};
        xStreamBufferSendFromISR(uart->rx_buffer, &byte, 1, 0);
    }
*/
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

    Sercom *hw = (Sercom*)((char*)SERCOM0 + 1024 * sercom);

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

    hw->SPI.BAUD.reg = 30;

    hw->SPI.INTENSET.reg = SERCOM_SPI_INTENSET_RXC;
    //hw->SPI.INTENSET.reg = SERCOM_SPI_INTENSET_TXC;
    //hw->SPI.INTENSET.reg = SERCOM_SPI_INTENSET_DRE;
    

    hw->SPI.CTRLA.bit.ENABLE = 1;
    while (hw->SPI.SYNCBUSY.bit.CTRLB);

/*
    hw->USART.CTRLA.bit.MODE = 1; // Internal clock
    hw->USART.CTRLA.bit.CMODE = 0;  // Asynchronous mode
    hw->USART.CTRLA.bit.TXPO = 0;
    hw->USART.CTRLA.bit.RXPO = 1;
    hw->USART.CTRLA.bit.DORD = 1; // LSB first
    hw->USART.CTRLB.bit.CHSIZE = 0; // 8 bits
    hw->USART.CTRLA.bit.FORM = 0;  // No parity bit
    hw->USART.CTRLB.bit.SBMODE = 0; // One stop bit
    hw->USART.BAUD.reg = _uart_get_baud_reg(115200);
	hw->USART.CTRLB.bit.ENC = 0;
    hw->USART.CTRLB.bit.RXEN = 1; // Receiver
    hw->USART.CTRLB.bit.TXEN = 1; // Transmitter
    hw->USART.INTENSET.reg = SERCOM_USART_INTENSET_RXC;
    hw->USART.INTENSET.reg = SERCOM_USART_INTENSET_ERROR;
    hw->USART.CTRLA.reg |= 1 << SERCOM_USART_CTRLA_ENABLE_Pos; // Enable

    while (hw->SPI.SYNCBUSY.bit.CTRLB);
*/

    NVIC_EnableIRQ(9 + sercom);



    spi->hw = hw;
    spi->fn = spi_handler;
    spi->bus_mutex = xSemaphoreCreateBinary();
    xSemaphoreGive(spi->bus_mutex);

    spi->call_mutex = xSemaphoreCreateBinary();

    sercom_handlers[sercom] = (dummy_type*)spi;
}

int32_t spi_transfer(spi_t *spi, uint8_t *tx_buf, uint8_t *rx_buf, int size) {
    xSemaphoreTake(spi->bus_mutex, portMAX_DELAY);
    spi->tx_buffer = tx_buf;
    spi->rx_buffer = rx_buf;
    spi->size = size;
    spi->cur = 0;

    spi->hw->SPI.INTENSET.reg = SERCOM_SPI_INTENSET_RXC;
    spi->hw->SPI.DATA.reg = tx_buf[0];

    int32_t ret = xSemaphoreTake(spi->call_mutex, IO_MAX_DELAY) ? size : ERR_TIMEOUT;
    xSemaphoreGive(spi->bus_mutex);
    return ret;
}

/*
int32_t uart_write(uart_t *uart, uint8_t *buf, uint16_t len) {
    xSemaphoreTake(uart->bus_mutex, portMAX_DELAY);
    uart->tx_buffer = buf;
    uart->tx_len = len;
    uart->tx_cur = 0;
    uart->hw->USART.INTENSET.bit.DRE = 1;
    int32_t ret = xSemaphoreTake(uart->call_mutex, IO_MAX_DELAY) ? len : ERR_TIMEOUT;
    xSemaphoreGive(uart->bus_mutex);
    return ret;
}
*/
