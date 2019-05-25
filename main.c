#include "core/gpio.h"

#define PB30 GPIO(GPIO_PORTB, 30)

int main() {
    gpio_set(PB30, LOW);
    gpio_direction(PB30, GPIO_DIRECTION_OUT);
    gpio_function(PB30, GPIO_FUNCTION_OFF);

    uart_init(2, PA12, PINMUX_PA12C_SERCOM2_PAD0, PA13, PINMUX_PA13C_SERCOM2_PAD1);

    //gpio_set(PB30, HIGH);
    //while (1) {};
    uint32_t s = 1;
    while (1) {
        gpio_set(PB30, s);
        s = !s;
        delay_ms(500);
    }
}
