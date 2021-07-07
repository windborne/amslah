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
#ifdef _SAMD21_
uint32_t function_pins[2] = {0, 0};

void pwm_init(uint8_t pin) {
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
	function_pins[GPIO_PORT(pin)] |= (1U << GPIO_PIN(pin));

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
}

void pwm_set(uint8_t pin, int level) {
    if (pin == 255) return;
	if (!(function_pins[GPIO_PORT(pin)] & (1U << GPIO_PIN(pin)))) {
		/* This pin is somehow not marked as a function pin! GPIO stole it from us!!! */
		pwm_init(pin);
	}
    int tc = tcs[pin];
    if (tc >= 3) {
        TcCount8 *hw = (TcCount8*)(((char*)TCC0) + 1024 * tc);
        hw->CC[pin & 1].reg = level;
    } else {
        Tcc *hw = (Tcc*)(((char*)TCC0) + 1024 * tc);
        hw->CC[pin & 1].bit.CC= level;
    }
}
#else

uint32_t function_pins[3] = {0, 0, 0};

uint16_t pwm_status[96] = {0};

const Tcc *insts[5] = TCC_INSTS;
const Tc *tc_insts[8] = TC_INSTS;

void pwm_init_with(pwmcfg_t cfg) {

	gpio_function(cfg.pin, (cfg.pin << 16) | cfg.mux);

	function_pins[GPIO_PORT(cfg.pin)] |= (1U << GPIO_PIN(cfg.pin));

	//configASSERT(cfg.timer >= 10);

    int resolution = cfg.resolution;
    if (resolution == 0) resolution = PWM_RESOLUTION;

    int prescaler = cfg.prescaler;
    if (prescaler == 0) prescaler = PWM_PRESCALER;

    pwm_status[cfg.pin] = (cfg.mux << 8) | (cfg.timer << 4) | (cfg.output);

    configASSERT(cfg.timer != 10);
	if (cfg.timer >= 10) {
		uint8_t n = cfg.timer - 10;

		const uint8_t ids[5] = {TCC0_GCLK_ID, TCC1_GCLK_ID, TCC2_GCLK_ID, TCC3_GCLK_ID, TCC4_GCLK_ID};
		critical_section_enter();
		GCLK->PCHCTRL[ids[n]].reg = 0 | (1 << GCLK_PCHCTRL_CHEN_Pos);
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
		}
		critical_section_leave();

		Tcc *hw = insts[n];
        hw->CTRLA.bit.PRESCALER = prescaler;
        hw->WAVE.bit.WAVEGEN = 2;
        hw->PER.bit.PER = (1 << resolution) - 1;

        hw->CC[cfg.output].bit.CC = 0;
        hw->CTRLA.bit.ENABLE = 1;
	} else {
        const uint8_t ids[8] = {TC0_GCLK_ID, TC1_GCLK_ID, TC2_GCLK_ID, TC3_GCLK_ID, TC4_GCLK_ID, TC5_GCLK_ID, TC6_GCLK_ID, TC7_GCLK_ID};
        GCLK->PCHCTRL[ids[cfg.timer]].reg = 0 | (1 << GCLK_PCHCTRL_CHEN_Pos);

		if (cfg.timer == 0) MCLK->APBAMASK.reg |= MCLK_APBAMASK_TC0;
		if (cfg.timer == 1) MCLK->APBAMASK.reg |= MCLK_APBAMASK_TC1;
		if (cfg.timer == 2) MCLK->APBBMASK.reg |= MCLK_APBBMASK_TC2;
		if (cfg.timer == 3) MCLK->APBBMASK.reg |= MCLK_APBBMASK_TC3;
		if (cfg.timer == 4) MCLK->APBCMASK.reg |= MCLK_APBCMASK_TC4;
		if (cfg.timer == 5) MCLK->APBCMASK.reg |= MCLK_APBCMASK_TC5;
		if (cfg.timer == 6) MCLK->APBDMASK.reg |= MCLK_APBDMASK_TC6;
		if (cfg.timer == 7) MCLK->APBDMASK.reg |= MCLK_APBDMASK_TC6;

        TcCount8* hw = tc_insts[cfg.timer];
		if (prescaler == 0) prescaler = PWM_PRESCALER;

        hw->CTRLA.bit.MODE = 1; /* 8 bit mode. */
        hw->CTRLA.bit.PRESCALER = prescaler; /* Undivided. */
        hw->WAVE.bit.WAVEGEN = 2; /* Normal PWM. */

        hw->PER.reg = (1 << resolution) - 1;

        hw->CC[cfg.output].reg = 0;

        hw->CTRLA.bit.ENABLE = 1;

    }
}

void pwm_set(uint8_t pin, int level) {
	configASSERT(pwm_status[pin] != 0); // must have been initialized
	uint8_t mux = (pwm_status[pin] >> 8);
	if (!(function_pins[GPIO_PORT(pin)] & (1U << GPIO_PIN(pin)))) {
		/* This pin is somehow not marked as a function pin! GPIO stole it from us!!! */
		gpio_function(pin, (pin << 16) | mux);
	}
	uint8_t timer = (pwm_status[pin] >> 4) & 15;
	uint8_t output = (pwm_status[pin] & 15);
	if (timer >= 10) {
		Tcc *hw = insts[timer - 10];
        hw->CC[output].bit.CC = level;
	} else {
        TcCount8* hw = tc_insts[timer];
        hw->CC[output].bit.CC = level;
    }
}

#endif
