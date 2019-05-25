#include "gpio.h"

void gpio_direction(uint8_t pin, enum gpio_direction direction) {
	if (direction == GPIO_DIRECTION_OUT) {
		uint32_t pinsel = 1U << GPIO_PIN(pin);
		PORT_IOBUS->Group[GPIO_PORT(pin)].DIRSET.reg = pinsel;
		critical_section_enter();
		PORT_IOBUS->Group[GPIO_PORT(pin)].WRCONFIG.reg = PORT_WRCONFIG_WRPINCFG | (pinsel & 0xffff);
		PORT_IOBUS->Group[GPIO_PORT(pin)].WRCONFIG.reg = 
			PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_WRPINCFG | ((pinsel & 0xffff0000) >> 16);
		critical_section_leave();
	} else {
		__asm("BKPT #0");
	}
}

void gpio_function(uint8_t pin, uint32_t function) {
	uint8_t tmp;
	critical_section_enter();
    tmp = PORT->Group[GPIO_PORT(pin)].PINCFG[GPIO_PIN(pin)].reg;
    tmp &= ~PORT_PINCFG_PMUXEN;
    tmp |= (function != GPIO_FUNCTION_OFF) << PORT_PINCFG_PMUXEN_Pos;
    PORT->Group[GPIO_PORT(pin)].PINCFG[GPIO_PIN(pin)].reg = tmp;
	critical_section_leave();

	if (function != GPIO_FUNCTION_OFF) {
		__asm("BKPT #0");
	}
}

