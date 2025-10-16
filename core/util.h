#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define ERR_NONE 0
#define ERR_INVALID_DATA -1
#define ERR_NO_CHANGE -2
#define ERR_ABORTED -3
#define ERR_BUSY -4
#define ERR_SUSPEND -5
#define ERR_IO -6
#define ERR_REQ_FLUSHED -7
#define ERR_TIMEOUT -8
#define ERR_BAD_DATA -9
#define ERR_NOT_FOUND -10
#define ERR_UNSUPPORTED_DEV -11
#define ERR_NO_MEMORY -12
#define ERR_INVALID_ARG -13
#define ERR_BAD_ADDRESS -14
#define ERR_BAD_FORMAT -15
#define ERR_BAD_FRQ -16
#define ERR_DENIED -17
#define ERR_ALREADY_INITIALIZED -18
#define ERR_OVERFLOW -19
#define ERR_NOT_INITIALIZED -20
#define ERR_SAMPLERATE_UNAVAILABLE -21
#define ERR_RESOLUTION_UNAVAILABLE -22
#define ERR_BAUDRATE_UNAVAILABLE -23
#define ERR_PACKET_COLLISION -24
#define ERR_PROTOCOL -25
#define ERR_PIN_MUX_INVALID -26
#define ERR_UNSUPPORTED_OP -27
#define ERR_NO_RESOURCE -28
#define ERR_NOT_READY -29
#define ERR_FAILURE -30
#define ERR_WRONG_LENGTH -31

typedef uint32_t hal_atomic_t;

void atomic_enter_critical(hal_atomic_t volatile *atomic);
void atomic_leave_critical(hal_atomic_t volatile *atomic);

#define critical_section_enter() \
	{ \
		volatile hal_atomic_t __atomic; \
		atomic_enter_critical(&__atomic); \

#define critical_section_leave() \
		atomic_leave_critical(&__atomic); \
	}

#define LOW 0
#define HIGH 1

void delay_ms(uint32_t ms);

void enable_sercom_clock(int n);

void enable_sercom_irq(int sercom);

void disable_uart_tx_interrupts(int sercom);

uint32_t get_32bit_core_id();

Sercom *get_sercom(int sercom);

#ifdef __cplusplus
}
#endif
