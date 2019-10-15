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


uint32_t get_32bit_core_id(){
	//ohp maybe this adress is defined in a header files somewhere idk
	uint32_t* addr = 0x0080A00C;
	uint32_t id = *(addr) ^ *(addr+1) ^ *(addr+2) ^ *(addr+3);
	return id;
}