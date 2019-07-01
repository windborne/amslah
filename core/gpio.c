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
    } else if (direction == GPIO_DIRECTION_IN) {
        uint32_t pinsel = 1U << GPIO_PIN(pin);
        PORT_IOBUS->Group[GPIO_PORT(pin)].DIRCLR.reg = pinsel;

        critical_section_enter();
        PORT_IOBUS->Group[GPIO_PORT(pin)].WRCONFIG.reg = PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_INEN | (pinsel & 0xffff);
        PORT_IOBUS->Group[GPIO_PORT(pin)].WRCONFIG.reg =
                PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_WRPINCFG | PORT_WRCONFIG_INEN | ((pinsel & 0xffff0000) >> 16);
        critical_section_leave();
    } 

else {
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

    if (function != GPIO_FUNCTION_OFF) {
        if (GPIO_PIN(pin) & 1) {
            PORT->Group[GPIO_PORT(pin)].PMUX[GPIO_PIN(pin) >> 1].bit.PMUXO = function & 0xffff;
        } else {
            PORT->Group[GPIO_PORT(pin)].PMUX[GPIO_PIN(pin) >> 1].bit.PMUXE = function & 0xffff;
        }
    }
    critical_section_leave();
}

void gpio_init(uint8_t pin) {
    gpio_function(pin, GPIO_FUNCTION_OFF);
    gpio_direction(pin, GPIO_DIRECTION_OUT);
    gpio_set(pin, LOW);
}

void gpio_in_init(uint8_t pin) {
    gpio_function(pin, GPIO_FUNCTION_OFF);
    gpio_direction(pin, GPIO_DIRECTION_IN);
}

uint8_t gpio_get(uint8_t pin) {
    uint32_t pinsel = 1 << GPIO_PIN(pin);
    uint8_t port = GPIO_PORT(pin);

    if ( (PORT->Group[port].IN.reg & pinsel) != 0) {
        return HIGH;
    } else {
        return LOW;
    }

}
