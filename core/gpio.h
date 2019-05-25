#pragma once

#include "samd21.h"

#define GPIO_PIN(n) (((n)&0x1Fu) << 0)
#define GPIO_PORT(n) ((n) >> 5)
#define GPIO(port, pin) ((((port)&0x7u) << 5) + ((pin)&0x1Fu))
#define GPIO_FUNCTION_OFF 0xffffffff

enum gpio_port { GPIO_PORTA, GPIO_PORTB, GPIO_PORTC, GPIO_PORTD, GPIO_PORTE };
enum gpio_direction { GPIO_DIRECTION_OFF, GPIO_DIRECTION_IN, GPIO_DIRECTION_OUT };
enum gpio_pull_mode { GPIO_PULL_OFF, GPIO_PULL_UP, GPIO_PULL_DOWN };

static inline void gpio_set(uint8_t pin, uint8_t level) {
	if (level) {
		PORT_IOBUS->Group[GPIO_PORT(pin)].OUTSET.reg = 1U << GPIO_PIN(pin);
	} else {
		PORT_IOBUS->Group[GPIO_PORT(pin)].OUTCLR.reg = 1U << GPIO_PIN(pin);
	}
}

void gpio_direction(uint8_t pin, enum gpio_direction direction);
void gpio_function(uint8_t pin, uint32_t function);

