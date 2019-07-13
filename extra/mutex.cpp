#include "amslah.h"
#include "mutex.h"
#include "task.h"


Mutex::Mutex(){
	#if MUTEX_TRACKING
		mutex_id = mutex_couter;
		mutex_couter++;
	#endif
	mutex = xSemaphoreCreateMutex();
}

bool Mutex::take(){
	#if (MUTEX_DOUBLETAKE_SAFETY || MUTEX_TRACKING) 
		xTaskHandle current_task;
		portENTER_CRITICAL();
		{
			current_task = ( xTaskHandle ) pxCurrentTCB;
		}
		portEXIT_CRITICAL();
		uint8_t task_num  = pxGetCurrentTaskNumber();
		#if MUTEX_TRACKING
			if(tracking_array[task_num]){
				print("MUTEX WARNING: task holds multiple mutexes\n");
			}
			tracking_array[task_num] = xTaskGetTickCount();
		#endif
		#if MUTEX_DOUBLETAKE_SAFETY
			if(current_task == xSemaphoreGetMutexHolder(mutex)){
				print("MUTEX WARNING: task attempted to take mutex that it already held\n");
				return true;
			} 
		#endif
	#endif
	if(xSemaphoreTake(mutex, MUTEX_TIMEOUT_TIME)){
		return true;
	} 
	#if MUTEX_TIMEOUT_EN
		else {
			print("MUTEX WARNING: mutex take timed out\n");
			return false;
		}
	#endif
}

void Mutex::give(){
#if (MUTEX_TRACKING) 
	xTaskHandle current_task;
	uint8_t task_num  = pxGetCurrentTaskNumber();
	int time_held = xTaskGetTickCount() - tracking_array[task_num] ;
	if(time_held > MUTEX_TRACKING_WARNING_TICKS){
		print("MUTEX_WARNING: task held mutex for %d ticks (or it took another mutex in this time)\n",time_held);
	}
	tracking_array[task_num] = 0;
#endif
xSemaphoreGive(mutex);
}

bool Mutex::take_nowarnings(){
	if(xSemaphoreTake(mutex, MUTEX_TIMEOUT_TIME)){
		return true;
	} 
	return false;
}
void Mutex::give_nowarnings(){
	xSemaphoreGive(mutex);
}

int Mutex::mutex_couter = 0;
uint32_t Mutex::tracking_array[MUTEX_TRACKING_MAX_NUM_THREADS] = {0};
