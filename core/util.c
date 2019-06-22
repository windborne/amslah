#include "samd21.h"

void enable_sercom_clock(int n) {
    PM->APBCMASK.reg |= 1 << (n + 2);
    critical_section_enter();
    int channel = 20 + n;
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(channel)
                        | GCLK_CLKCTRL_GEN_GCLK1
                        | (1 << GCLK_CLKCTRL_CLKEN_Pos);
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);
    critical_section_leave();
}
