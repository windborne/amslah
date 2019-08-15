/**
 * \file
 *
 * \brief gcc starttup file for SAMD21
 *
 * Copyright (c) 2018 Microchip Technology Inc.
 *
 * \asf_license_start
 *
 * \page License
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the Licence at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * \asf_license_stop
 *
 */

#include "amslah_config.h"
#include "samd21.h"
#include "mtb.h"

void init_serial();

/* Initialize segments */
extern uint32_t _sfixed;
extern uint32_t _efixed;
extern uint32_t _etext;
extern uint32_t _srelocate;
extern uint32_t _erelocate;
extern uint32_t _szero;
extern uint32_t _ezero;
extern uint32_t _sstack;
extern uint32_t _estack;

#if(USE_PERSISTENT_INFO == 1)
    #include "persistent_info.h"
    extern persistent_info_t prev_info;
    extern persistent_info_t curr_info;
#endif


/** \cond DOXYGEN_SHOULD_SKIP_THIS */
int main(void);
/** \endcond */

void __libc_init_array(void);

/* Default empty handler */
void Dummy_Handler(void);

/* Cortex-M0+ core handlers */
void NonMaskableInt_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SVCall_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Dummy_Handler")));

/* Peripherals handlers */
void PM_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SYSCTRL_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void WDT_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void RTC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void EIC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void NVMCTRL_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void DMAC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
#ifdef ID_USB
void USB_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
#endif
void EVSYS_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SERCOM0_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SERCOM1_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SERCOM2_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void SERCOM3_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
#ifdef ID_SERCOM4
void SERCOM4_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
#endif
#ifdef ID_SERCOM5
void SERCOM5_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
#endif
void TCC0_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TCC1_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TCC2_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC3_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC4_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
void TC5_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
#ifdef ID_TC6
void TC6_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
#endif
#ifdef ID_TC7
void TC7_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
#endif
#ifdef ID_ADC
void ADC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
#endif
#ifdef ID_AC
void AC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
#endif
#ifdef ID_DAC
void DAC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
#endif
#ifdef ID_PTC
void PTC_Handler(void) __attribute__((weak, alias("Dummy_Handler")));
#endif
void I2S_Handler(void) __attribute__((weak, alias("Dummy_Handler")));

/* Exception Table */
__attribute__((section(".vectors"))) const DeviceVectors exception_table = {

    /* Configure Initial Stack Pointer, using linker-generated symbols */
    .pvStack = (void *)(&_estack),

    .pfnReset_Handler          = (void *)Reset_Handler,
    .pfnNonMaskableInt_Handler = (void *)NonMaskableInt_Handler,
    .pfnHardFault_Handler      = (void *)HardFault_Handler,
    .pvReservedM12             = (void *)(0UL), /* Reserved */
    .pvReservedM11             = (void *)(0UL), /* Reserved */
    .pvReservedM10             = (void *)(0UL), /* Reserved */
    .pvReservedM9              = (void *)(0UL), /* Reserved */
    .pvReservedM8              = (void *)(0UL), /* Reserved */
    .pvReservedM7              = (void *)(0UL), /* Reserved */
    .pvReservedM6              = (void *)(0UL), /* Reserved */
    .pfnSVCall_Handler         = (void *)SVCall_Handler,
    .pvReservedM4              = (void *)(0UL), /* Reserved */
    .pvReservedM3              = (void *)(0UL), /* Reserved */
    .pfnPendSV_Handler         = (void *)PendSV_Handler,
    .pfnSysTick_Handler        = (void *)SysTick_Handler,

    /* Configurable interrupts */
    .pfnPM_Handler      = (void *)PM_Handler,      /*  0 Power Manager */
    .pfnSYSCTRL_Handler = (void *)SYSCTRL_Handler, /*  1 System Control */
    .pfnWDT_Handler     = (void *)WDT_Handler,     /*  2 Watchdog Timer */
    .pfnRTC_Handler     = (void *)RTC_Handler,     /*  3 Real-Time Counter */
    .pfnEIC_Handler     = (void *)EIC_Handler,     /*  4 External Interrupt Controller */
    .pfnNVMCTRL_Handler = (void *)NVMCTRL_Handler, /*  5 Non-Volatile Memory Controller */
    .pfnDMAC_Handler    = (void *)DMAC_Handler,    /*  6 Direct Memory Access Controller */
#ifdef ID_USB
    .pfnUSB_Handler = (void *)USB_Handler, /*  7 Universal Serial Bus */
#else
    .pvReserved7  = (void *)(0UL), /*  7 Reserved */
#endif
    .pfnEVSYS_Handler   = (void *)EVSYS_Handler,   /*  8 Event System Interface */
    .pfnSERCOM0_Handler = (void *)SERCOM0_Handler, /*  9 Serial Communication Interface 0 */
    .pfnSERCOM1_Handler = (void *)SERCOM1_Handler, /* 10 Serial Communication Interface 1 */
    .pfnSERCOM2_Handler = (void *)SERCOM2_Handler, /* 11 Serial Communication Interface 2 */
    .pfnSERCOM3_Handler = (void *)SERCOM3_Handler, /* 12 Serial Communication Interface 3 */
#ifdef ID_SERCOM4
    .pfnSERCOM4_Handler = (void *)SERCOM4_Handler, /* 13 Serial Communication Interface 4 */
#else
    .pvReserved13 = (void *)(0UL), /* 13 Reserved */
#endif
#ifdef ID_SERCOM5
    .pfnSERCOM5_Handler = (void *)SERCOM5_Handler, /* 14 Serial Communication Interface 5 */
#else
    .pvReserved14 = (void *)(0UL), /* 14 Reserved */
#endif
    .pfnTCC0_Handler = (void *)TCC0_Handler, /* 15 Timer Counter Control 0 */
    .pfnTCC1_Handler = (void *)TCC1_Handler, /* 16 Timer Counter Control 1 */
    .pfnTCC2_Handler = (void *)TCC2_Handler, /* 17 Timer Counter Control 2 */
    .pfnTC3_Handler  = (void *)TC3_Handler,  /* 18 Basic Timer Counter 0 */
    .pfnTC4_Handler  = (void *)TC4_Handler,  /* 19 Basic Timer Counter 1 */
    .pfnTC5_Handler  = (void *)TC5_Handler,  /* 20 Basic Timer Counter 2 */
#ifdef ID_TC6
    .pfnTC6_Handler = (void *)TC6_Handler, /* 21 Basic Timer Counter 3 */
#else
    .pvReserved21 = (void *)(0UL), /* 21 Reserved */
#endif
#ifdef ID_TC7
    .pfnTC7_Handler = (void *)TC7_Handler, /* 22 Basic Timer Counter 4 */
#else
    .pvReserved22 = (void *)(0UL), /* 22 Reserved */
#endif
#ifdef ID_ADC
    .pfnADC_Handler = (void *)ADC_Handler, /* 23 Analog Digital Converter */
#else
    .pvReserved23 = (void *)(0UL), /* 23 Reserved */
#endif
#ifdef ID_AC
    .pfnAC_Handler = (void *)AC_Handler, /* 24 Analog Comparators */
#else
    .pvReserved24 = (void *)(0UL), /* 24 Reserved */
#endif
#ifdef ID_DAC
    .pfnDAC_Handler = (void *)DAC_Handler, /* 25 Digital Analog Converter */
#else
    .pvReserved25 = (void *)(0UL), /* 25 Reserved */
#endif
#ifdef ID_PTC
    .pfnPTC_Handler = (void *)PTC_Handler, /* 26 Peripheral Touch Controller */
#else
    .pvReserved26 = (void *)(0UL), /* 26 Reserved */
#endif
    .pfnI2S_Handler = (void *)I2S_Handler, /* 27 Inter-IC Sound Interface */
    .pvReserved28   = (void *)(0UL)        /* 28 Reserved */
};

void init_sources() {
    uint32_t calib_osc8m;
    calib_osc8m = SYSCTRL->OSC8M.reg;
    calib_osc8m = (calib_osc8m & SYSCTRL_OSC8M_CALIB_Msk) >> SYSCTRL_OSC8M_CALIB_Pos;
    critical_section_enter();
    /*SYSCTRL->OSC8M.reg = SYSCTRL_OSC8M_CALIB(calib_osc8m)
                            | SYSCTRL_OSC8M_PRESC(0)
                            | (1 << SYSCTRL_OSC8M_ENABLE_Pos);
    */
    SYSCTRL->OSC8M.bit.PRESC = 0;
    SYSCTRL->OSC8M.reg |= 1 << SYSCTRL_OSC8M_ENABLE_Pos;

    uint32_t calib_ulp;
    calib_ulp = SYSCTRL->OSCULP32K.reg;
    calib_ulp = (calib_ulp & SYSCTRL_OSCULP32K_CALIB_Msk) >> SYSCTRL_OSCULP32K_CALIB_Pos;
    SYSCTRL->OSCULP32K.reg = SYSCTRL_OSCULP32K_CALIB(calib_ulp);

    while (!((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_OSC8MRDY) >> SYSCTRL_PCLKSR_OSC8MRDY_Pos)) {};
    SYSCTRL->OSC8M.reg |= SYSCTRL_OSC8M_ONDEMAND;

    GCLK->GENDIV.reg = GCLK_GENDIV_DIV(32) | GCLK_GENDIV_ID(3);
    GCLK->GENCTRL.reg = (1 << GCLK_GENCTRL_RUNSTDBY_Pos)
                        | (1 << GCLK_GENCTRL_GENEN_Pos)
                        | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_ID(3);

    /*
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(0)
                        | GCLK_CLKCTRL_GEN(GCLK_CLKCTRL_GEN_GCLK3_Val)
                        | (1 << GCLK_CLKCTRL_CLKEN_Pos);

    SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_ENABLE;

    while (!((SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY) >> SYSCTRL_PCLKSR_DFLLRDY_Pos)) {};

    SYSCTRL->DFLLMUL.reg = SYSCTRL_DFLLMUL_CSTEP(1)
                            | SYSCTRL_DFLLMUL_FSTEP(1)
                            | SYSCTRL_DFLLMUL_MUL(0);
    uint32_t default_coarse = (FUSES_DFLL48M_COARSE_CAL_Msk
                                & (*((uint32_t *)FUSES_DFLL48M_COARSE_CAL_ADDR)))
                                >> FUSES_DFLL48M_COARSE_CAL_Pos;
    SYSCTRL->DFLLVAL.reg = SYSCTRL_DFLLVAL_COARSE(((default_coarse) == 0x3F) ?
                                                    0x1F : (default_coarse))
                            | SYSCTRL_DFLLVAL_FINE(512);

    uint16_t dfllctrl_val = (0 << SYSCTRL_DFLLCTRL_WAITLOCK_Pos)
                            | (0 << SYSCTRL_DFLLCTRL_RUNSTDBY_Pos)
                            | (0 << SYSCTRL_DFLLCTRL_USBCRM_Pos) // This fucker
                            | (0 << SYSCTRL_DFLLCTRL_MODE_Pos)
                            | (1 << SYSCTRL_DFLLCTRL_ENABLE_Pos);
    SYSCTRL->DFLLCTRL.reg = dfllctrl_val;
    SYSCTRL->DFLLCTRL.reg |= SYSCTRL_DFLLCTRL_ONDEMAND;
    */

    while ((GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY) >> GCLK_STATUS_SYNCBUSY_Pos) {};

    #define MAX_GCLK_ID 0x25

	for (int gclk_id = 1; gclk_id < MAX_GCLK_ID; gclk_id++) {
		GCLK->CLKCTRL.reg = (gclk_id << GCLK_CLKCTRL_ID_Pos) | GCLK_CLKCTRL_GEN_GCLK7;
	}

    /* Generator 0. Sourced from DFLL, divided by two. */
    /*GCLK->GENDIV.reg = GCLK_GENDIV_DIV(48) | GCLK_GENDIV_ID(0);
    GCLK->GENCTRL.reg = (0 << GCLK_GENCTRL_RUNSTDBY_Pos)
                        | (1 << GCLK_GENCTRL_GENEN_Pos)
                        | GCLK_GENCTRL_SRC_DFLL48M | GCLK_GENCTRL_ID(0);*/

    #if CPU_FREQUENCY > 8000000
        #error Frequencies above 8 MHz require the DFLL, which is currently commented out.
    #endif

    GCLK->GENDIV.reg = GCLK_GENDIV_DIV(8000000/CPU_FREQUENCY) | GCLK_GENDIV_ID(0);
    GCLK->GENCTRL.reg = (0 << GCLK_GENCTRL_RUNSTDBY_Pos)
                        | (1 << GCLK_GENCTRL_GENEN_Pos)
                        | GCLK_GENCTRL_SRC_OSC8M | GCLK_GENCTRL_ID(0);

    #if PERIPHERAL_FREQUENCY > 8000000
        #error Peripherals run off the 8 MHz oscillator. If you really want higher clocked peripherals, go bother Joan or something.
    #endif

    /* Generator 1. Peripheral clock, sourced from OSC8M. */
    GCLK->GENDIV.reg = GCLK_GENDIV_DIV(8000000/PERIPHERAL_FREQUENCY) | GCLK_GENDIV_ID(1);
    //GCLK->GENCTRL.bit.DIVSEL = 0;
    //GCLK->GENCTRL.bit.ID = 1;
    GCLK->GENCTRL.reg = GCLK_GENCTRL_SRC_OSC8M | GCLK_GENCTRL_ID(1)| GCLK_GENCTRL_GENEN;
    //GCLK->GENCTRL.bit.GENEN = 1;


	SYSCTRL->BOD33.reg = 0;

    PM->APBCMASK.reg = 0;

    USB->DEVICE.CTRLA.reg &= ~USB_CTRLA_ENABLE;
    PM->APBBMASK.reg &= ~PM_APBBMASK_USB;
    PM->APBBMASK.reg &= ~PM_APBBMASK_DMAC;
    PM->AHBMASK.bit.USB_ = 0;
    PM->AHBMASK.bit.DMAC_ = 0;

    critical_section_leave();


}


void init_chip() {
    critical_section_enter();
    PM->CPUSEL.bit.CPUDIV = 0;
    PM->APBASEL.bit.APBADIV = 0;
    PM->APBBSEL.bit.APBBDIV = 0;
    PM->APBCSEL.bit.APBCDIV = 0;
    critical_section_leave();

    init_sources();


/*
	system_ahb_clock_clear_mask(
				(PM_AHBMASK_USB \
				| PM_AHBMASK_DSU \
				| PM_AHBMASK_HPB1 \
				| PM_AHBMASK_HPB2 \
				| PM_AHBMASK_DMAC \
				| PM_AHBMASK_HPB0 \*/
				/* These clocks should remain enabled on this bus
				| PM_AHBMASK_HPB1 \
				| PM_AHBMASK_HPB2 \
				| PM_AHBMASK_HPB0 \
				| PM_AHBMASK_NVMCTRL \
				*/
			//));
}

/**
 * \brief This is the code that gets called on processor reset.
 * To initialize the device, and call the main() routine.
 */
void Reset_Handler(void)
{
    mtb_stop();
	uint32_t *pSrc, *pDest;

#if(USE_PERSISTENT_INFO == 1)
    persistent_info_t info_save;
    memcpy(&info_save,&curr_info,sizeof(persistent_info_t));
#endif

    /* Initialize the relocate segment */
    pSrc  = &_etext;
    pDest = &_srelocate;
	if (pSrc != pDest) {
		for (; pDest < &_erelocate;) {
			*pDest++ = *pSrc++;
		}
	}
	/* Clear the zero segment */
	for (pDest = &_szero; pDest < &_ezero;) {
		*pDest++ = 0;

	}
#if(USE_PERSISTENT_INFO == 1)
    if(info_save.identifier == PERSISTENT_INFO_IDENTIFIER) memcpy(&prev_info,&info_save,sizeof(persistent_info_t));
    curr_info.identifier = PERSISTENT_INFO_IDENTIFIER;
#endif
	/* Set the vector table base address */
	pSrc      = (uint32_t *)&_sfixed;
	SCB->VTOR = ((uint32_t)pSrc & SCB_VTOR_TBLOFF_Msk);

	/* Change default QOS values to have the best performance and correct USB behaviour */
	SBMATRIX->SFR[SBMATRIX_SLAVE_HMCRAMC0].reg = 2;
#if defined(ID_USB)
	USB->DEVICE.QOSCTRL.bit.CQOS = 2;
	USB->DEVICE.QOSCTRL.bit.DQOS = 2;
#endif
	DMAC->QOSCTRL.bit.DQOS   = 2;
	DMAC->QOSCTRL.bit.FQOS   = 2;
	DMAC->QOSCTRL.bit.WRBQOS = 2;

	/* Overwriting the default value of the NVMCTRL.CTRLB.MANW bit (errata reference 13134) */
	NVMCTRL->CTRLB.bit.MANW = 1;

    NVMCTRL->CTRLB.bit.RWS = 0;


    PM->APBAMASK.reg |= PM_APBAMASK_GCLK;

    NVMCTRL->CTRLB.bit.CACHEDIS = 0;
    NVMCTRL->CTRLB.bit.READMODE = 0;

	/* Initialize the C library */
	__libc_init_array();

    init_chip();

    #if USE_DEBUG_UART
        init_serial();
    #endif
	/* Branch to main function */
	main();

	/* Infinite loop */
	while (1)
		;
}

/**
 * \brief Default interrupt handler for unused IRQs.
 */
void Dummy_Handler(void)
{
    mtb_stop();
	while (1) {
	}
}

void atomic_enter_critical(hal_atomic_t volatile *atomic) {
	*atomic = __get_PRIMASK();
	__disable_irq();
	__DMB();
}

void atomic_leave_critical(hal_atomic_t volatile *atomic) {
	__DMB();
	__set_PRIMASK(*atomic);
}

void __register_exitproc(void) { }

static void delay_cycles(uint32_t cycles) {
    volatile uint32_t i = 0;
    cycles /= 9;
    for (; i<cycles; i++) {};
}

void delay_ms(uint32_t ms) {
    delay_cycles(ms * 1000);
}


#ifdef __cplusplus
extern "C"{
  int _getpid(){ return -1;}
  int _kill(int pid, int sig){ return -1; }
  int _write(){return -1;}
}
#endif

