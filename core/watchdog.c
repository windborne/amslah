#include "watchdog.h"

void watchdog_init(int period) {
	GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_WDT |
						GCLK_CLKCTRL_CLKEN |
						GCLK_CLKCTRL_GEN_GCLK3;
	NVIC_EnableIRQ(WDT_IRQn);

	WDT->CTRL.reg = 0;
	while (WDT->STATUS.bit.SYNCBUSY);

	WDT->CONFIG.bit.PER = period;
	WDT->CTRL.bit.WEN = 0;							
	WDT->INTENSET.bit.EW = 1;							//enable warning interrupt
	WDT->EWCTRL.bit.EWOFFSET = period-1;  //set period to half that of the reset interval
	while (WDT->STATUS.bit.SYNCBUSY);

	WDT->CTRL.bit.ENABLE = 1;
	while (WDT->STATUS.bit.SYNCBUSY);
}

void watchdog_kick() {
	if (!WDT->STATUS.bit.SYNCBUSY)
		WDT->CLEAR.reg = 0xa5;
	//while (WDT->STATUS.bit.SYNCBUSY);
}
