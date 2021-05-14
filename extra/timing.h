#pragma once

#include "amslah.h"

#if (CPU_FREQUENCY == 8000000)
    #define LOGFAC 3
#elif (CPU_FREQUENCY == 1600000)
    #define LOGFAC 4
#elif (CPU_FREQUENCY == 4000000)
    #define LOGFAC 2
#elif (CPU_FREQUENCY == 2000000)
    #define LOGFAC 1
#elif (CPU_FREQUENCY == 1000000)
    #define LOGFAC 0
#endif

extern int hrt_base;


class elapsed {
public:
    elapsed() {
        if (!HIGH_RESOLUTION_TIMER) {
            configASSERT(0);
        }
    };

    uint32_t base;
    uint32_t count;
    
    void start() {
#ifdef _SAMD21_
        taskENTER_CRITICAL(); 
        base = hrt_base;
	#if USAGE_REPORT_TC >= 3
		TcCount16 *hw = (TcCount16*)(((char*)TCC0) + 1024 * USAGE_REPORT_TC);
	#else
		Tcc *hw = (Tcc*)(((char*)TCC0) + 1024 * USAGE_REPORT_TC);
		hw->CTRLBSET.bit.CMD = TCC_CTRLBSET_CMD_READSYNC_Val;
		while (hw->SYNCBUSY.bit.COUNT);
    	#endif
		count = hw->COUNT.reg;
        taskEXIT_CRITICAL(); 
#else
        count = DWT->CYCCNT;
#endif
    }

    uint32_t get() {
#ifdef _SAMD21_
        taskENTER_CRITICAL(); 
        uint32_t nbase = hrt_base;
		#if USAGE_REPORT_TC >= 3
			TcCount16 *hw = (TcCount16*)(((char*)TCC0) + 1024 * USAGE_REPORT_TC);
		#else
			Tcc *hw = (Tcc*)(((char*)TCC0) + 1024 * USAGE_REPORT_TC);
			hw->CTRLBSET.bit.CMD = TCC_CTRLBSET_CMD_READSYNC_Val;
			while (hw->SYNCBUSY.bit.COUNT);
    	#endif
		uint32_t ncount = hw->COUNT.reg;
        taskEXIT_CRITICAL(); 
        return (nbase << HRT_RES) + ncount - ((base << HRT_RES) + count);
#else
        uint32_t ncount = DWT->CYCCNT;
        return (ncount - count) >> LOGFAC;
#endif
    }

    void print() {
        ::print("Elapsed %d us.\n", get());
    }
};
