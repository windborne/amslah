#include "amslah_config.h"

#include "pwm.h"

#include "gpio.h"

const uint8_t tcs[] = {2, 2, -1, -1, 0, 0, 1, 1, 0, 0, 1, 1, // PA11
                       2, 2, 3, 3, 2, 2, 3, 3, 7, 7, 4, 4, // PA23
                       5, 5, -1, -1, -1, -1, 1, 1, // PA31
                       7, 7, 6, 6, -1, -1, -1, -1, 4, 4, 5, 5, // PB11
                       4, 4, 5, 5, 6, 6, -1, -1, -1, -1, 7, 7, // PB23
                       -1, -1, -1, -1, -1, -1, 0, 0}; // PB31

void pwm_init(uint8_t pin) {
    int tc = tcs[pin];
    if (tc == -1) {
        __asm("BKPT #0");
    }

    int channel = 0x1a + (tc/2);

    PM->APBCMASK.reg |= (1 << (8 + tc));

    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(channel)
                        | GCLK_CLKCTRL_GEN_GCLK1
                        | (1 << GCLK_CLKCTRL_CLKEN_Pos);
    while(GCLK->STATUS.bit.SYNCBUSY);

    gpio_function(pin, (pin << 16) | 4);

    print("tc %d for pin %d\n", tc, pin);

    if (tc >= 3) {
        TcCount8 *hw = (TcCount8*)(((char*)TCC0) + 1024 * tc);

        hw->CTRLA.bit.MODE = 1; /* 8 bit mode. */
        hw->CTRLA.bit.PRESCALER = 0; /* Undivided. */
        hw->CTRLA.bit.WAVEGEN = 2; /* Normal PWM. */

        hw->PER.reg = (1 << PWM_RESOLUTION) - 1;

        int out = pin & 1;

        hw->CC[out].reg = 0;

        hw->CTRLA.bit.ENABLE = 1;
    } else {
        Tcc *hw = (Tcc*)(((char*)TCC0) + 1024 * tc);
        hw->CTRLA.bit.PRESCALER = 0;
        hw->WAVE.bit.WAVEGEN = 2;
        hw->PER.bit.PER = 0xff;
        hw->CC[pin & 1].bit.CC = 0;
        hw->CTRLA.bit.ENABLE = 1;
    }

}

void pwm_set(uint8_t pin, int level) {
    int tc = tcs[pin];
    if (tc >= 3) {
        TcCount8 *hw = (TcCount8*)(((char*)TCC0) + 1024 * tc);
        hw->CC[pin & 1].reg = level;
    } else {
        Tcc *hw = (Tcc*)(((char*)TCC0) + 1024 * tc);
        hw->CC[pin & 1].bit.CC= level;
    }
}
