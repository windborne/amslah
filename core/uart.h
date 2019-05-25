#include "samd21a.h"

typedef struct {
    int yeet;
} uart;

void uart_init(int sercom, uint8_t pin_tx, uint32_t mux_tx, uint8_t pin_rx, uint32_t mux_rx) {
    enable_sercom_clock(sercom);


    gpio_function(pin_tx, mux_tx);
    gpio_function(pin_rx, mux_rx); 
}
