#include "uart.h"

uint16_t _uart_get_baud_reg(int baud) {
    return 65536 - ((65536 * 16.0f * baud) / PERIPHERAL_FREQUENCY);
}

void uart_handler(int num) {
	//configASSERT(0);
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
	BaseType_t woke = pdFALSE;
    if ( (uart->hw->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_TXC)
             && (uart->hw->USART.INTENSET.reg & SERCOM_USART_INTENSET_TXC) ) {
        uart->hw->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_TXC;
        xSemaphoreGiveFromISR(uart->call_mutex, &woke);
        //gpio_set(GPIO(GPIO_PORTB, 30), LOW);
    }
    if ( (uart->hw->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_RXC)
             && (uart->hw->USART.INTENSET.reg & SERCOM_USART_INTENSET_RXC)) {
        char byte = uart->hw->USART.DATA.reg;
        //gpio_set(GPIO(GPIO_PORTB, 30), LOW);
        //while (1) {};
        xStreamBufferSendFromISR(uart->rx_buffer, &byte, 1, &woke);
    }
	portYIELD_FROM_ISR(woke);
}

void uart_init_with(uart_t *uart, uartcfg_t cfg) {
	uart_init(uart, cfg.sercom, cfg.baud, cfg.pin_tx, (cfg.pin_tx << 16) | cfg.mux_tx, cfg.pin_rx, (cfg.pin_rx << 16) | cfg.mux_rx);
}

void uart_init(uart_t *uart, int sercom, int baud, uint8_t pin_tx, uint32_t mux_tx, uint8_t pin_rx, uint32_t mux_rx) {
    enable_sercom_clock(sercom);

    gpio_function(pin_tx, mux_tx);
    gpio_function(pin_rx, mux_rx); 

    //PORT->Group[GPIO_PORT(pin_rx)].PINCFG[GPIO_PIN(pin_rx)].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_INEN ;

	Sercom *hw = get_sercom(sercom);

#ifdef _SAMD21_
    hw->USART.CTRLA.bit.SWRST = 1;
    while (hw->USART.CTRLA.bit.SWRST || hw->USART.SYNCBUSY.bit.SWRST);
#else
    hw->USART.CTRLA.bit.SWRST = 1;
    while (hw->USART.CTRLA.bit.SWRST || hw->USART.SYNCBUSY.bit.SWRST);
#endif

    hw->USART.CTRLA.bit.MODE = 1; // Internal clock
    hw->USART.CTRLA.bit.CMODE = 0;  // Asynchronous mode
    hw->USART.CTRLA.bit.TXPO = 0;
    hw->USART.CTRLA.bit.RXPO = 1;
    hw->USART.CTRLA.bit.DORD = 1; // LSB first
    hw->USART.CTRLB.bit.CHSIZE = 0; // 8 bits
    hw->USART.CTRLA.bit.FORM = 0;  // No parity bit
    hw->USART.CTRLB.bit.SBMODE = 0; // One stop bit
    hw->USART.BAUD.reg = _uart_get_baud_reg(baud);
	hw->USART.CTRLB.bit.ENC = 0;
    hw->USART.CTRLB.bit.RXEN = 1; // Receiver
    hw->USART.CTRLB.bit.TXEN = 1; // Transmitter
    hw->USART.INTENSET.reg = SERCOM_USART_INTENSET_RXC;
    hw->USART.CTRLA.reg |= 1 << SERCOM_USART_CTRLA_ENABLE_Pos; // Enable

    while (hw->USART.SYNCBUSY.bit.CTRLB);
    while (hw->USART.SYNCBUSY.bit.SWRST);
    enable_sercom_irq(sercom);

    uart->hw = hw;
    uart->fn = uart_handler;
    uart->bus_mutex = xSemaphoreCreateBinary();
    xSemaphoreGive(uart->bus_mutex);

    uart->call_mutex = xSemaphoreCreateBinary();

    uart->rx_buffer = xStreamBufferCreate(UART_RX_BUFFER_SIZE, 1);

    sercom_handlers[sercom] = (dummy_type*)uart;
}

void uart_reinit(uart_t *uart, int sercom, int baud, uint8_t pin_tx, uint32_t mux_tx, uint8_t pin_rx, uint32_t mux_rx){
    enable_sercom_clock(sercom);

    gpio_function(pin_tx, mux_tx);
    gpio_function(pin_rx, mux_rx); 

    //PORT->Group[GPIO_PORT(pin_rx)].PINCFG[GPIO_PIN(pin_rx)].reg = PORT_PINCFG_PMUXEN | PORT_PINCFG_INEN ;

	Sercom *hw = get_sercom(sercom);

#ifdef _SAMD21_
    hw->USART.CTRLA.bit.SWRST = 1;
    while (hw->USART.CTRLA.bit.SWRST || hw->USART.SYNCBUSY.bit.SWRST);
#else
    hw->USART.CTRLA.bit.SWRST = 1;
    while (hw->USART.CTRLA.bit.SWRST || hw->USART.SYNCBUSY.bit.SWRST);
#endif

    hw->USART.CTRLA.bit.MODE = 1; // Internal clock
    hw->USART.CTRLA.bit.CMODE = 0;  // Asynchronous mode
    hw->USART.CTRLA.bit.TXPO = 0;
    hw->USART.CTRLA.bit.RXPO = 1;
    hw->USART.CTRLA.bit.DORD = 1; // LSB first
    hw->USART.CTRLB.bit.CHSIZE = 0; // 8 bits
    hw->USART.CTRLA.bit.FORM = 0;  // No parity bit
    hw->USART.CTRLB.bit.SBMODE = 0; // One stop bit
    hw->USART.BAUD.reg = _uart_get_baud_reg(baud);
	hw->USART.CTRLB.bit.ENC = 0;
    hw->USART.CTRLB.bit.RXEN = 1; // Receiver
    hw->USART.CTRLB.bit.TXEN = 1; // Transmitter
    hw->USART.INTENSET.reg = SERCOM_USART_INTENSET_RXC;
    hw->USART.CTRLA.reg |= 1 << SERCOM_USART_CTRLA_ENABLE_Pos; // Enable

    while (hw->USART.SYNCBUSY.bit.CTRLB);
    while (hw->USART.SYNCBUSY.bit.SWRST);
    enable_sercom_irq(sercom);

    uart->hw = hw;
    uart->fn = uart_handler;
    xSemaphoreGive(uart->bus_mutex);

    sercom_handlers[sercom] = (dummy_type*)uart;
}

int32_t uart_write(uart_t *uart, const uint8_t *buf, uint16_t len) {
    xSemaphoreTake(uart->bus_mutex, portMAX_DELAY);
    uart->tx_buffer = (uint8_t*)buf;
    uart->tx_len = len;
    uart->tx_cur = 0;
    uart->hw->USART.INTENSET.bit.DRE = 1;
    int32_t ret = xSemaphoreTake(uart->call_mutex, IO_MAX_DELAY) ? len : ERR_TIMEOUT;
    xSemaphoreGive(uart->bus_mutex);
    return ret;
}

char uart_readt(uart_t *uart, uint32_t timeout) {
    char byte;
    int nb = xStreamBufferReceive(uart->rx_buffer, &byte, 1, timeout);
    if (nb != 0) return byte;
	else return 0;
}

char uart_read(uart_t *uart) {
	return uart_readt(uart, 0);
}

uint32_t uart_available(uart_t *uart) {
    return xStreamBufferBytesAvailable(uart->rx_buffer);
}

void uart_start_listening(uart_t *uart) {
    uart->hw->USART.CTRLB.bit.RXEN = 1;
    uart->hw->USART.INTENSET.reg = SERCOM_USART_INTENSET_RXC;
}

void uart_stop_listening(uart_t *uart) {
    uart->hw->USART.CTRLB.bit.RXEN = 0;
    uart->hw->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_RXC;
    xStreamBufferReset(uart->rx_buffer);
}
