#define IO_MAX_DELAY 5000

#define UART_RX_BUFFER_SIZE 128

#define CPU_FREQUENCY 1000000

#define PERIPHERAL_FREQUENCY 4000000

#define USE_DEBUG_UART 1

#define DEBUG_UART_SERCOM ( 2 )
#define DEBUG_UART_BAUD ( 115200 )

#define DEBUG_UART_TX_PIN ( GPIO(GPIO_PORTA, 12) )
#define DEBUG_UART_TX_MUX ( PINMUX_PA12C_SERCOM2_PAD0 )

#define DEBUG_UART_RX_PIN ( GPIO(GPIO_PORTA, 13) )
#define DEBUG_UART_RX_MUX ( PINMUX_PA13C_SERCOM2_PAD1 )

