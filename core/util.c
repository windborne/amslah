#include "samd21.h"

void enable_sercom_clock(int n) {
    PM->APBCMASK.reg |= 1 << (n + 2);
    critical_section_enter();
    int channel = 20 + n;
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(channel)
                        | GCLK_CLKCTRL_GEN(GCLK_CLKCTRL_GEN_GCLK1_Val)
                        | (1 << GCLK_CLKCTRL_CLKEN_Pos);
    critical_section_leave();
}
