#include "debug_uart.h"
#include "ring_buffer.h"


// DONE: Use ifdef flags and user_amslah_config so this compiles with main boards, but not with others - think this is done?

// TODO: Make this work on the right debug sercom clock & pins
// TODO: Switch it all up to use ring buffers, parse messages, and expose via semaphore
// TODO: Make a ping-pong buffer and use that kind of implementation for debug?
// TODO: Test this out with a python loop that keeps sending serial data and we keep printing it.
// TODO: Maybe use coolterm? Maybe make firmware a loopback?
// TODO: Make it so this code is included and compiles

#if USE_DEBUG_UART_V2

RingBuffer_t transmit_buffer;
RingBuffer_t receive_buffer;

void debug_uart_init(void) {
    enable_sercom_clock(DEBUG_UART_SERCOM);

    gpio_function(DEBUG_UART_TX_PIN, DEBUG_UART_TX_MUX);
    gpio_function(DEBUG_UART_RX_PIN, DEBUG_UART_RX_MUX);

    Sercom* hw = get_sercom(DEBUG_UART_SERCOM);

    hw->USART.CTRLA.bit.SWRST = 1;
    while (hw->USART.CTRLA.bit.SWRST || hw->USART.SYNCBUSY.bit.SWRST);

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
    hw->USART.CTRLA.reg |= 1 << SERCOM_USART_CTRLA_ENABLE_Pos; // Enable

    while (hw->USART.SYNCBUSY.bit.CTRLB);
    while (hw->USART.SYNCBUSY.bit.SWRST);

// SERCOM3_0_IRQn

    NVIC_SetPriority(DEBUG_UART_0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(DEBUG_UART_0_IRQn);
    NVIC_SetPriority(DEBUG_UART_1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(DEBUG_UART_1_IRQn);
    NVIC_SetPriority(DEBUG_UART_2_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(DEBUG_UART_2_IRQn);
    NVIC_SetPriority(DEBUG_UART_3_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(DEBUG_UART_3_IRQn);    
}


// Data Register Empty interrupt
void DEBUG_UART_0_Handler(void) {
    
}

// Transmit Complete (the mcu has just sent data out, and we can put a new byte from the buffer into DATA)
void DEBUG_UART_1_Handler(void) {
    if (read_from_buffer(&transmit_buffer, (void*) &DEBUG_SERCOM->USART.DATA.reg, 1) != 0) {
        DEBUG_SERCOM->USART.INTENSET.reg = SERCOM_USART_INTENSET_TXC;
    } else {
        DEBUG_SERCOM->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_TXC;
    }
}

// Receive Complete (We have just finished shifting data into)
void DEBUG_UART_2_Handler(void) {
    const char data = (char) DEBUG_SERCOM->USART.DATA.reg;
    write_to_buffer(&receive_buffer, (const char*) &data, 1);
    DEBUG_SERCOM->USART.INTENSET.reg = SERCOM_USART_INTENSET_RXC;
}

// Receive start, clear to send, receive break, error
void DEBUG_UART_3_Handler(void) {

}



void flush_to_uart(void) {
    read_from_buffer(&transmit_buffer, (void*) &DEBUG_SERCOM->USART.DATA.reg, 1);
    DEBUG_SERCOM->USART.INTENSET.reg = SERCOM_USART_INTENSET_TXC;
}


#endif // USE_DEBUG_UART_V2
