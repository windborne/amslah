#pragma once

#include "amslah.h"

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
    }

    uint32_t get() {
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
    }

    void print() {
        ::print("Elapsed %d us.\n", get());
    }
};
