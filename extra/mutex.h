#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

extern void* pxCurrentTCB;

class Mutex {
public:
    Mutex(){
        mutex = xSemaphoreCreateMutex();
    }
    bool take(){
        xTaskHandle current_task;
        portENTER_CRITICAL();
        {
            current_task = ( xTaskHandle ) pxCurrentTCB;
        }
        portEXIT_CRITICAL();
        if(current_task == xSemaphoreGetMutexHolder(mutex)){
            print("MUTEX WARNING: task attempted to take mutex that it already held\n");
            return true;
        }
        return xSemaphoreTake(mutex, portMAX_DELAY);
    }
    void give(){
        xSemaphoreGive(mutex);
    }
    SemaphoreHandle_t mutex = NULL;
};

template<class t> class Mutexed {
public:
    // Let it be known, joan requested verbosity
    void set(const t& arg){
        if(mutex.take()) val = arg;
        mutex.give();
    }

    t get(){
        t ret;
        if(mutex.take()) ret = val;
        mutex.give();;
        return ret;
    }
    t val;    
    Mutex mutex;
};

class LockGuard {
public:
    LockGuard(Mutex& mutex) : _mutex(mutex){
        _mutex.take();
    }
    ~LockGuard(){
        _mutex.give();
    }
    Mutex& _mutex;
};