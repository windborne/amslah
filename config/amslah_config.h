#pragma once

#include <user_amslah_config.h>

#ifndef IO_MAX_DELAY
	#define IO_MAX_DELAY 5000
#endif

#ifndef UART_RX_BUFFER_SIZE
	#define UART_RX_BUFFER_SIZE 128
#endif

#ifndef CPU_FREQUENCY
	#define CPU_FREQUENCY 8000000
#endif

#ifndef PERIPHERAL_FREQUENCY
	#define PERIPHERAL_FREQUENCY 4000000
#endif

#ifndef PWM_RESOLUTION
	#define PWM_RESOLUTION 8
#endif

#ifndef USE_DEBUG_UART
	#define USE_DEBUG_UART 1

	#define DEBUG_UART_SERCOM ( 2 )
	#define DEBUG_UART_BAUD ( 115200 )

	#define DEBUG_UART_TX_PIN ( GPIO(GPIO_PORTA, 12) )
	#define DEBUG_UART_TX_MUX ( PINMUX_PA12C_SERCOM2_PAD0 )

	#define DEBUG_UART_RX_PIN ( GPIO(GPIO_PORTA, 13) )
	#define DEBUG_UART_RX_MUX ( PINMUX_PA13C_SERCOM2_PAD1 )
#endif

