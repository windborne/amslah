#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

extern void* pxCurrentTCB;

#define MUTEX_DOUBLETAKE_SAFETY 1

#define MUTEX_TIMEOUT_EN 1
#define MUTEX_TIMEOUT_TIME 2000

#define MUTEX_TRACKING 1
#define MUTEX_TRACKING_WARNING_TICKS 500
#define MUTEX_TRACKING_MAX_NUM_THREADS 20
//#define MUTEX_TRACKING_MAX_NUM_MUTEXES 20

class Mutex {
public:
	Mutex();
	bool take();
	void give();
	bool take_nowarnings();
	bool take_notimeout();
	void give_nowarnings();
	SemaphoreHandle_t mutex = NULL;

	#if MUTEX_TRACKING
	int mutex_id = 0;
	static int mutex_couter;
	static uint32_t tracking_array[MUTEX_TRACKING_MAX_NUM_THREADS];
	#endif
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

class LockGuardNoTimeout {
public:
	LockGuardNoTimeout(Mutex& mutex) : _mutex(mutex){
		_mutex.take_notimeout();
	}
	~LockGuardNoTimeout(){
		_mutex.give_nowarnings();
	}
	Mutex& _mutex;
};