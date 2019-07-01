#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "samd21.h"

#define GPIO_PIN(n) (((n)&0x1Fu) << 0)
#define GPIO_PORT(n) ((n) >> 5)
#define GPIO(port, pin) ((((port)&0x7u) << 5) + ((pin)&0x1Fu))
#define GPIO_FUNCTION_OFF 0xffffffff

enum gpio_port { GPIO_PORTA, GPIO_PORTB, GPIO_PORTC, GPIO_PORTD, GPIO_PORTE };
enum gpio_direction { GPIO_DIRECTION_OFF, GPIO_DIRECTION_IN, GPIO_DIRECTION_OUT };
enum gpio_pull_mode { GPIO_PULL_OFF, GPIO_PULL_UP, GPIO_PULL_DOWN };


/**
 * @brief Set the digital output of a GPIO pin. Must have been initialized
 *        with gpio_init(pin) first.
 *
 * @param pin Pin to set.
 * @param level Digital level to set it (LOW and HIGH can be used).
 */
static inline void gpio_set(uint8_t pin, uint8_t level) {
	if (level) {
		PORT_IOBUS->Group[GPIO_PORT(pin)].OUTSET.reg = 1U << GPIO_PIN(pin);
	} else {
		PORT_IOBUS->Group[GPIO_PORT(pin)].OUTCLR.reg = 1U << GPIO_PIN(pin);
	}
}


/**
 * @brief Sets the pin direction.
 *
 * @param pin Pin to set the direction of.
 * @param direction One of enum gpio_direction (off, in, or out).
 */
void gpio_direction(uint8_t pin, enum gpio_direction direction);


/**
 * @brief Sets the pin function. The function will be something other than OFF
 *        if it's being used as part of some peripheral (e.g. a SERCOM or an
 *        ADC input.
 *
 * @param pin Pin to set the function of.
 * @param function Pin function, or GPIO_FUNCTION_OFF if none.
 */
void gpio_function(uint8_t pin, uint32_t function);


/**
 * @brief Initialize a digital output pin. It sets it as an output and defaults
 *        to being LOW.
 *
 * @param pin Pin to initialize as a digital output.
 */
void gpio_init(uint8_t pin);

void gpio_in_init(uint8_t pin);

uint8_t gpio_get(uint8_t pin);

#ifdef __cplusplus
}
#endif
