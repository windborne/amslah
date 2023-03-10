#pragma once

#include <user_amslah_config.h>

#ifndef SAMD51_WAIT_STATES
	#define SAMD51_WAIT_STATES 0
#endif

#ifndef SAMD51_CACHE_ENABLE
	#define SAMD51_CACHE_ENABLE 1
#endif

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

#ifndef USAGE_REPORT
    #define USAGE_REPORT 1
#endif

#ifndef USAGE_REPORT_TC
    #define USAGE_REPORT_TC 0
#endif

#ifndef USAGE_REPORT_INTERVAL
    #define USAGE_REPORT_INTERVAL 20000
#endif

#ifndef USAGE_REPORT_INITIAL_WAIT
    #define USAGE_REPORT_INITIAL_WAIT 5000
#endif

#ifndef HIGH_RESOLUTION_TIMER
    #define HIGH_RESOLUTION_TIMER 1
#endif

#ifndef PWM_RESOLUTION
	#define PWM_RESOLUTION 8
#endif

#ifndef PWM_PRESCALER
	#define PWM_PRESCALER 0
#endif

#ifndef ADC_OVERSAMPLE
	/* See Table 33-2:
	 *   0x0: 1x
	 *   0x1: 2x
	 *   0x2: 4x
	 *   0x3: 8x
	 *   0x4: 16x
	 *   0x5: 32x
	 *   0x6: 64x
	 *   0x7: 128x
	 *   0x8: 256x
	 *   0x9: 512x
	 *   0xA: 1024x
	 */
	#define ADC_OVERSAMPLE 0x4
#endif

#ifndef ADC_FREQUENCY_PRESCALER
	/* See Sec. 33.8.5, bits 10:8:
	 *   0x0: DIV4
	 *   0x1: DIV8
	 *   0x2: DIV16
	 *   0x3: DIV32
	 *   0x4: DIV64
	 *   0x5: DIV128
	 *   0x6: DIV256
	 *   0x7: DIV512
	 * Editor's note: you probably want to divide by a decent number
	 * to have time to fill up the capacitors! This can also be done
	 * with ADC_SAMPLE_TIME.
	 */
	#define ADC_FREQUENCY_PRESCALER 0
#endif

#ifndef ADC_SAMPLE_TIME
	/* See Sec. 33.8.4, bits 5:0. Units are half ADC clocks. */
	#define ADC_SAMPLE_TIME 63
#endif

#ifndef ADC_AVERAGE
	/* Average the oversampling down to 12 bits (0--4095). */
	#define ADC_AVERAGE 1
#endif

#ifndef ADC_REFERENCE
	/* ADC reference source. */
	#define ADC_REFERENCE 0
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

#ifndef SERIAL_TASK
	#define SERIAL_TASK USE_DEBUG_UART
#endif

#ifndef USE_PERSISTENT_INFO
	#define USE_PERSISTENT_INFO 0
#else 
	#ifndef PERSISTENT_INFO_IDENTIFIER
		#define PERSISTENT_INFO_IDENTIFIER 0xaabbccdd
	#endif
#endif

#ifndef AMSLAH_IDLE_HOOK
	#define AMSLAH_IDLE_HOOK 0
#endif
