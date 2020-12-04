#include "amslah_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "pwm.h"

#include "gpio.h"

const uint8_t tcs[] = {2, 2, -1, -1, 0, 0, 1, 1, 0, 0, 1, 1, // PA11
                       2, 2, 3, 3, 2, 2, 3, 3, 7, 7, 4, 4, // PA23
                       5, 5, -1, -1, -1, -1, 1, 1, // PA31
                       7, 7, 6, 6, -1, -1, -1, -1, 4, 4, 5, 5, // PB11
                       4, 4, 5, 5, 6, 6, -1, -1, -1, -1, 7, 7, // PB23
                       -1, -1, -1, -1, -1, -1, 0, 0}; // PB31
bool used_tcs[8] = {0, 0, 0, 0, 0, 0, 0, 0};

uint8_t tc51[8] = {0, 0, 0, 0, 0, 0, 0, 0};

Tcc *insts[5] = TCC_INSTS;

void pwm51_init(uint8_t pin, uint8_t mux, uint8_t tc) {

	uint8_t n = tc;
	uint8_t ids[8] = {TCC0_GCLK_ID, TCC1_GCLK_ID, TCC2_GCLK_ID, TCC3_GCLK_ID, TCC4_GCLK_ID, TC5_GCLK_ID, TC6_GCLK_ID, TC7_GCLK_ID};
	configASSERT(n >= 0 && n < 8);
    critical_section_enter();
	//GCLK->PCHCTRL[ids[n]].reg = 1 | (1 << GCLK_PCHCTRL_CHEN_Pos); // GEN0 is the peripheral clock
	GCLK->PCHCTRL[ids[n]].reg = 0 | (1 << GCLK_PCHCTRL_CHEN_Pos); // GEN0 is the peripheral clock
	switch (n) {
	case 0:
		MCLK->APBBMASK.reg |= MCLK_APBBMASK_TCC0; break;
	case 1:
		MCLK->APBBMASK.reg |= MCLK_APBBMASK_TCC1; break;
	case 2:
		MCLK->APBCMASK.reg |= MCLK_APBCMASK_TCC2; break;
	case 3:
		MCLK->APBCMASK.reg |= MCLK_APBCMASK_TCC3; break;
	case 4:
		MCLK->APBDMASK.reg |= MCLK_APBDMASK_TCC4; break;
	case 5:
		MCLK->APBCMASK.reg |= MCLK_APBCMASK_TC5; break;
	case 6:
		MCLK->APBDMASK.reg |= MCLK_APBDMASK_TC6; break;
	case 7:
		MCLK->APBDMASK.reg |= MCLK_APBDMASK_TC7; break;
	}
	critical_section_leave();


    gpio_function(pin, (pin << 16) | mux);

		Tcc *hw = insts[tc];
        hw->CTRLA.bit.PRESCALER = 2;
        hw->WAVE.bit.WAVEGEN = 2;
        hw->PER.bit.PER = 4095;
		int w = 2;
		if (pin == PB31) w = 1;
        hw->CC[w].bit.CC = 0;
        hw->CTRLA.bit.ENABLE = 1;
}

void pwm51_set(uint8_t pin, uint8_t tc, int level) {
		Tcc *hw = insts[tc];
		int w = 2;
		if (pin == PB31) w = 1;
        hw->CC[w].bit.CC = level;
}

void pwm_init(uint8_t pin) {
#if _SAMD21_
    if (pin == 255) return;
    int tc = tcs[pin];
    configASSERT(tc != -1);
    used_tcs[tc] = 1;

    int channel = 0x1a + (tc/2);

    PM->APBCMASK.reg |= (1 << (8 + tc));

    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(channel)
                        | GCLK_CLKCTRL_GEN_GCLK1
                        | (1 << GCLK_CLKCTRL_CLKEN_Pos);
    while(GCLK->STATUS.bit.SYNCBUSY);

    gpio_function(pin, (pin << 16) | 4);

    #if PWM_RESOLUTION > 8
        #error "PWM resolution above 8 bit unsupported"
    #endif

    if (tc >= 3) {
        TcCount8 *hw = (TcCount8*)(((char*)TCC0) + 1024 * tc);

        hw->CTRLA.bit.MODE = 1; /* 8 bit mode. */
        hw->CTRLA.bit.PRESCALER = PWM_PRESCALER; /* Undivided. */
        hw->CTRLA.bit.WAVEGEN = 2; /* Normal PWM. */

        hw->PER.reg = (1 << PWM_RESOLUTION) - 1;

        int out = pin & 1;

        hw->CC[out].reg = 0;

        hw->CTRLA.bit.ENABLE = 1;
    } else {
        Tcc *hw = (Tcc*)(((char*)TCC0) + 1024 * tc);
        hw->CTRLA.bit.PRESCALER = PWM_PRESCALER;
        hw->WAVE.bit.WAVEGEN = 2;
        hw->PER.bit.PER = (1 << PWM_RESOLUTION) - 1;
        hw->CC[pin & 1].bit.CC = 0;
        hw->CTRLA.bit.ENABLE = 1;
    }
#endif
}

void pwm_set(uint8_t pin, int level) {
    if (pin == 255) return;
    int tc = tcs[pin];
    if (tc >= 3) {
        TcCount8 *hw = (TcCount8*)(((char*)TCC0) + 1024 * tc);
        hw->CC[pin & 1].reg = level;
    } else {
        Tcc *hw = (Tcc*)(((char*)TCC0) + 1024 * tc);
        hw->CC[pin & 1].bit.CC= level;
    }
}
