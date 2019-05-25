#pragma once

#include <stdbool.h>

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
