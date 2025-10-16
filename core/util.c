#include "sammy.h"
#include "FreeRTOS.h"
#include "task.h"

void enable_sercom_irq(int sercom) {
	#ifdef _SAMD21_
		NVIC_EnableIRQ(9 + sercom);
	#else
		for (int i=0; i<4; i++) {
			int nn = 46 + 4 * sercom + i;
			NVIC_SetPriority(nn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
			NVIC_EnableIRQ(nn);
		}
	#endif
}

void disable_uart_tx_interrupts(int sercom) {
    #ifdef _SAMD21_
        NVIC_DisableIRQ(9 + sercom);
    #else
        for (int i=0; i<2; i++) {
            int nn = 46 + 4 * sercom + i;
            NVIC_DisableIRQ(nn);
        }
    #endif
}

void enable_sercom_clock(int n) {
#ifdef _SAMD21_
    PM->APBCMASK.reg |= 1 << (n + 2);
    critical_section_enter();
    int channel = 20 + n;
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(channel)
                        | GCLK_CLKCTRL_GEN_GCLK1
                        | (1 << GCLK_CLKCTRL_CLKEN_Pos);
    while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);
    critical_section_leave();
#else
	uint8_t ids[8] = {SERCOM0_GCLK_ID_CORE, SERCOM1_GCLK_ID_CORE, SERCOM2_GCLK_ID_CORE, SERCOM3_GCLK_ID_CORE, SERCOM4_GCLK_ID_CORE, SERCOM5_GCLK_ID_CORE, SERCOM6_GCLK_ID_CORE, SERCOM7_GCLK_ID_CORE};
	configASSERT(n >= 0 && n < 8);
    critical_section_enter();
	//GCLK->PCHCTRL[ids[n]].reg = 1 | (1 << GCLK_PCHCTRL_CHEN_Pos); // GEN0 is the peripheral clock
	GCLK->PCHCTRL[ids[n]].reg = 0 | (1 << GCLK_PCHCTRL_CHEN_Pos); // GEN0 is the peripheral clock
	switch (n) {
	case 0:
		MCLK->APBAMASK.reg |= MCLK_APBAMASK_SERCOM0; break;
	case 1:
		MCLK->APBAMASK.reg |= MCLK_APBAMASK_SERCOM1; break;
	case 2:
		MCLK->APBBMASK.reg |= MCLK_APBBMASK_SERCOM2; break;
	case 3:
		MCLK->APBBMASK.reg |= MCLK_APBBMASK_SERCOM3; break;
	case 4:
		MCLK->APBDMASK.reg |= MCLK_APBDMASK_SERCOM4; break;
	case 5:
		MCLK->APBDMASK.reg |= MCLK_APBDMASK_SERCOM5; break;
	case 6:
		MCLK->APBDMASK.reg |= MCLK_APBDMASK_SERCOM6; break;
	case 7:
		MCLK->APBDMASK.reg |= MCLK_APBDMASK_SERCOM7; break;
	}
	critical_section_leave();
#endif
}


uint32_t get_32bit_core_id(){
	//ohp maybe this adress is defined in a header files somewhere idk
#ifdef _SAMD21_
	uint32_t* addr = (uint32_t*)0x0080A00C;
	uint32_t id = *(addr) ^ *(addr+1) ^ *(addr+2) ^ *(addr+3);
#else
	uint32_t* addr = (uint32_t*)0x008061FC;
	uint32_t* addr2 = (uint32_t*)0x00806010;
	uint32_t id = (*addr) ^ (*(addr2)) ^ (*(addr2+1)) ^ (*(addr2+2));
#endif
	return id;
}

Sercom *get_sercom(int sercom) {
#ifdef _SAMD21_
    Sercom *hw = (Sercom*)((char*)SERCOM0 + 1024 * sercom);
#else
	Sercom *hw = 0;
	switch (sercom) {
	case 0:
		hw = SERCOM0; break;
	case 1:
		hw = SERCOM1; break;
	case 2:
		hw = SERCOM2; break;
	case 3:
		hw = SERCOM3; break;
	case 4:
		hw = SERCOM4; break;
	case 5:
		hw = SERCOM5; break;
	#ifdef SERCOM6
	case 6:
		hw = SERCOM6; break;
	case 7:
		hw = SERCOM7; break;
	#endif
	}
#endif
	return hw;
}
