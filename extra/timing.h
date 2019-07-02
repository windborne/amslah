#pragma once

#include "amslah.h"

extern int hrt_base;

class elapsed {
public:
    elapsed() {
        if (!HIGH_RESOLUTION_TIMER) {
            configASSERT(0);
        }
        start();
    };

    uint32_t base;
    uint32_t count;
    
    void start() {
        taskENTER_CRITICAL(); 
        base = hrt_base;
        count = vGetRunTimeCounterValue();
        taskEXIT_CRITICAL(); 
    }

    uint32_t get() {
        taskENTER_CRITICAL(); 
        uint32_t nbase = hrt_base;
        uint32_t ncount = vGetRunTimeCounterValue();
        taskEXIT_CRITICAL(); 
        if (base != nbase) return ((nbase - base) << HRT_RES) + ncount - count;
        else return ncount - count;
    }

    void print() {
        ::print("Elapsed %d us.\n", get());
    }
};
