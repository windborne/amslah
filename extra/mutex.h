#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

class Mutex {
public:
	Mutex(){
		mutex = xSemaphoreCreateMutex();
	}
	bool take(){
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

class LockGaurd {
public:
	LockGaurd(Mutex& mutex) : _mutex(mutex){
		_mutex.take();
	}
	~LockGaurd(){
		_mutex.give();
	}
	Mutex& _mutex;
};
