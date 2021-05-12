#include "amslah_config.h"

#include "dac.h"
#include "gpio.h"


bool base_init = false;

void dac_init_with(daccfg_t cfg) {

	gpio_function(cfg.pin, (cfg.pin << 16) | 1);

	if (!base_init) {
		#ifdef _SAMD21_

			PM->APBCMASK.reg |= PM_APBCMASK_DAC;

			GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_DAC
								| GCLK_CLKCTRL_GEN_GCLK1
								| (1 << GCLK_CLKCTRL_CLKEN_Pos);
			while(GCLK->STATUS.bit.SYNCBUSY);

			DAC->CTRLB.bit.REFSEL = cfg.ref; /* VDDANA reference. */
			DAC->CTRLB.bit.EOEN = 0x1; /* Enable output.*/

			while (DAC->STATUS.reg & DAC_STATUS_SYNCBUSY) {};

			DAC->CTRLA.bit.ENABLE = 1;

			while (DAC->STATUS.reg & DAC_STATUS_SYNCBUSY) {};
		#else
			MCLK->APBDMASK.bit.DAC_ = 1;
			GCLK->PCHCTRL[DAC_GCLK_ID].reg = 0 | (1 << GCLK_PCHCTRL_CHEN_Pos);

			DAC->CTRLB.bit.REFSEL = cfg.ref;
		#endif

			base_init = true;
	}

#ifdef _SAMD51_

	DAC->CTRLA.bit.ENABLE = 0;
	while (DAC->SYNCBUSY.bit.ENABLE) {};
	
	int which = (cfg.pin == PA02) ? 0 : 1;
	DAC->DACCTRL[which].bit.ENABLE = 1;
	DAC->DACCTRL[which].bit.REFRESH = 2; // needed for static voltages (!!)

	DAC->CTRLA.bit.ENABLE = 1;
	while (DAC->SYNCBUSY.bit.ENABLE) {};
#endif
}

void dac_init() {
	daccfg_t cfg;
	cfg.pin = PA02;
	cfg.ref = DAC_REF_VDDANA;
	dac_init_with(cfg);
}

void dac_set_pin(uint8_t pin, int level) {
#if _SAMD21_
    DAC->DATA.reg = level;
#else
	int which = (pin == PA02) ? 0 : 1;
    DAC->DATA[which].reg = level << 2;
#endif
}

void dac_set(int level) {
	dac_set_pin(PA02, level);
}
