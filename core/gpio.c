#include "amslah_config.h"
#include "FreeRTOS.h"
#include "task.h"

#include "gpio.h"

void gpio_pull(uint8_t pin, enum gpio_pull_mode pull_mode) {
    if(pin==NOT_A_PIN) return;
#ifdef _SAMD21_
    if(pull_mode == GPIO_PULL_ON)
        PORT_IOBUS->Group[GPIO_PORT(pin)].PINCFG[GPIO_PIN(pin)].bit.PULLEN = true;
    else
        PORT_IOBUS->Group[GPIO_PORT(pin)].PINCFG[GPIO_PIN(pin)].bit.PULLEN = false;
#else
    if(pull_mode == GPIO_PULL_ON)
        PORT->Group[GPIO_PORT(pin)].PINCFG[GPIO_PIN(pin)].bit.PULLEN = true;
    else
        PORT->Group[GPIO_PORT(pin)].PINCFG[GPIO_PIN(pin)].bit.PULLEN = false;
#endif
}

void gpio_direction(uint8_t pin, enum gpio_direction direction) {
#ifdef _SAMD21_
    if(pin==NOT_A_PIN) return;
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

        uint32_t pull_mask = PORT_IOBUS->Group[GPIO_PORT(pin)].PINCFG[GPIO_PIN(pin)].bit.PULLEN ? PORT_WRCONFIG_PULLEN : 0;

        critical_section_enter();
        PORT_IOBUS->Group[GPIO_PORT(pin)].WRCONFIG.reg = PORT_WRCONFIG_WRPINCFG | pull_mask | PORT_WRCONFIG_INEN | (pinsel & 0xffff);
        PORT_IOBUS->Group[GPIO_PORT(pin)].WRCONFIG.reg =
                PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_WRPINCFG | pull_mask | PORT_WRCONFIG_INEN | ((pinsel & 0xffff0000) >> 16);
        critical_section_leave();
    } else {
        configASSERT(0);
    }
#else
    if(pin==NOT_A_PIN) return;
    if (direction == GPIO_DIRECTION_OUT) {
        uint32_t pinsel = 1U << GPIO_PIN(pin);
        PORT->Group[GPIO_PORT(pin)].DIRSET.reg = pinsel;
        critical_section_enter();
        PORT->Group[GPIO_PORT(pin)].WRCONFIG.reg = PORT_WRCONFIG_WRPINCFG | (pinsel & 0xffff);
        PORT->Group[GPIO_PORT(pin)].WRCONFIG.reg = 
            PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_WRPINCFG | ((pinsel & 0xffff0000) >> 16);
        critical_section_leave();
    } else if (direction == GPIO_DIRECTION_IN) {
        uint32_t pinsel = 1U << GPIO_PIN(pin);
        PORT->Group[GPIO_PORT(pin)].DIRCLR.reg = pinsel;

        uint32_t pull_mask = PORT->Group[GPIO_PORT(pin)].PINCFG[GPIO_PIN(pin)].bit.PULLEN ? PORT_WRCONFIG_PULLEN : 0;

        critical_section_enter();
        PORT->Group[GPIO_PORT(pin)].WRCONFIG.reg = PORT_WRCONFIG_WRPINCFG | pull_mask | PORT_WRCONFIG_INEN | (pinsel & 0xffff);
        PORT->Group[GPIO_PORT(pin)].WRCONFIG.reg =
                PORT_WRCONFIG_HWSEL | PORT_WRCONFIG_WRPINCFG | pull_mask | PORT_WRCONFIG_INEN | ((pinsel & 0xffff0000) >> 16);
        critical_section_leave();
    } else {
        configASSERT(0);
    }
#endif
}

void gpio_function(uint8_t pin, uint32_t function) {
    if(pin==NOT_A_PIN) return;
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

void digital_out_init(uint8_t pin) {
    gpio_function(pin, GPIO_FUNCTION_OFF);
    gpio_direction(pin, GPIO_DIRECTION_OUT);
    digital_set(pin, LOW);
}

void digital_in_init(uint8_t pin) {
    gpio_function(pin, GPIO_FUNCTION_OFF);
    gpio_pull(pin, GPIO_PULL_OFF);
    gpio_direction(pin, GPIO_DIRECTION_IN);
}

void digital_in_pull_init(uint8_t pin) {
    gpio_function(pin, GPIO_FUNCTION_OFF);
    gpio_pull(pin, GPIO_PULL_ON);
    gpio_direction(pin, GPIO_DIRECTION_IN);
}

uint8_t digital_get(uint8_t pin) {
    if(pin==NOT_A_PIN) return 0;
    uint32_t pinsel = 1 << GPIO_PIN(pin);
    uint8_t port = GPIO_PORT(pin);

    if ( (PORT->Group[port].IN.reg & pinsel) != 0) {
        return HIGH;
    } else {
        return LOW;
    }

}
