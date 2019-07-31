#include "mtb.h"
#include "FreeRTOS.h"
#include "task.h"

extern uint32_t _smtb;
extern uint32_t _emtb;

int8_t curr_buffer = -1;
uint32_t *mtb_ePtr;
uint32_t *mtb_sPtr;
void mtb_init(){
	mtb_ePtr= &_emtb;
	mtb_sPtr= &_smtb;
	uint8_t *cb_ptr = ((uint8_t*)mtb_ePtr)-1;
	if(*cb_ptr==0){
		curr_buffer = 1;
		*cb_ptr = 1;
	} else {
		curr_buffer = 0;
		*cb_ptr = 0;
	}
	for (uint32_t *pDest = mtb_sPtr+(curr_buffer)*MTB_SIZE_WORDS; pDest < mtb_sPtr+(curr_buffer+1)*MTB_SIZE_WORDS;) {
		*pDest = 0;
		pDest++;
	}
	//print("mtb_sPtr: 0x%x\n",mtb_sPtr);
	//print("mtb_ePtr: 0x%x\n",mtb_ePtr);
	//print("curr_buffer: %d\n",curr_buffer);
	MTB->MASTER.bit.EN = 0;
	MTB->POSITION.bit.POINTER = (uint32_t)(mtb_sPtr+curr_buffer*MTB_SIZE_WORDS - MTB->BASE.reg) >> 3;
	MTB->FLOW.bit.WATERMARK = (uint32_t)(mtb_sPtr+(curr_buffer+1)*MTB_SIZE_WORDS - MTB->BASE.reg) >> 3;
	MTB->MASTER.bit.MASK = 2;
}


void mtb_start(){
	configASSERT(curr_buffer != -1);
	MTB->MASTER.bit.EN = 1;
}


void mtb_stop(){
	MTB->MASTER.bit.EN = 0;
}

uint32_t * mtb_get_curr(){
	return mtb_sPtr+curr_buffer*MTB_SIZE_WORDS;
}

uint32_t * mtb_get_prev(){
	return mtb_sPtr+(1-curr_buffer)*MTB_SIZE_WORDS;
}