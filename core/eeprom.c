#include "eeprom.h"
#include "serial.h"
#include "sammy.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t eeprom_mutex = 0;

// NOTE: Sevearly barebones implementation, Litterally 0 page management
// 1. Can only read/write independendly from different rows 
// 2. Can only write the page on a given row lmao
// 3. Writes will get truncated to multiples of 4 bytes
// 4. May or may not need to wait after the row earase
// 5. 0 checking of fuse bits to see if things will work
// 6. 0 checking that you aren't gunna overwrite ur program flash lol
// 7. this shouldnt' even be named eeprom it's not even emulating the interface it's legit just nvm interface


#ifdef _SAMD51_
	// Invalidate all CMCC cache entries if CMCC cache is enabled.
	static void invalidate_CMCC_cache()
	{
	if (CMCC->SR.bit.CSTS) {
		CMCC->CTRL.bit.CEN = 0;
		while (CMCC->SR.bit.CSTS) {}
		CMCC->MAINT0.bit.INVALL = 1;
		CMCC->CTRL.bit.CEN = 1;
	}
	}
#endif

#ifdef _SAMD21_
void eeprom_init(){
	if (eeprom_mutex == 0) {
		eeprom_mutex = xSemaphoreCreateBinary();
	}
	NVMCTRL->CTRLB.bit.MANW = 1;
	//uint8_t size_val = ((*((uint32_t*)NVMCTRL_FUSES_EEPROM_SIZE_ADDR)) >> NVMCTRL_FUSES_EEPROM_SIZE_Pos) & 0b111;
	xSemaphoreGive(eeprom_mutex);
}

void eeprom_write(uint8_t * buf, uint8_t size, uint8_t num){
	xSemaphoreTake(eeprom_mutex,portMAX_DELAY);
	uint32_t * row_base = FLASH_SIZE - (1+num)*NVMCTRL_ROW_SIZE;
	for(int i = 0;i<size/4;i++){
		*(row_base + i) = *((uint32_t*)buf + i);
	} 
	for(int i = size/4;i<NVMCTRL_PAGE_SIZE/4;i++){
		*(row_base + i) = 0;	
	}
	NVMCTRL->CTRLA.reg = (NVMCTRL_CTRLA_CMD_ER_Val | NVMCTRL_CTRLA_CMDEX_KEY);
	NVMCTRL->CTRLA.reg = (NVMCTRL_CTRLA_CMD_WP_Val | NVMCTRL_CTRLA_CMDEX_KEY);
	xSemaphoreGive(eeprom_mutex);
}

void eeprom_read(uint8_t * buf, uint8_t size, uint8_t num){
	xSemaphoreTake(eeprom_mutex,portMAX_DELAY);
	uint32_t * row_base = FLASH_SIZE - (1+num)*NVMCTRL_ROW_SIZE;
	for(int i = 0;i<size;i++){
		//print("%d,%x\n",i,buf[i]);
		buf[i] = *((uint8_t*)row_base + i);
	}
	xSemaphoreGive(eeprom_mutex);
}
#endif



#ifdef _SAMD51_
//lol this implementation only lets the user read/write a single word to flash
//also it takes a full flash block
__attribute__((__aligned__(8192)))
static const uint8_t flash_block[8192] = { };
void test_flash51(){
	NVMCTRL->ADDR.reg = ((uint32_t)flash_block);
  	NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_EB;
	while (!NVMCTRL->INTFLAG.bit.DONE) { }
	invalidate_CMCC_cache();
	volatile uint32_t *dst_addr = (volatile uint32_t *)flash_block;

	// Disable auto page write
	NVMCTRL->CTRLA.bit.WMODE = 0;
	while (NVMCTRL->STATUS.bit.READY == 0) { }
	// Disable NVMCTRL cache while writing, per SAMD51 errata.
	bool original_CACHEDIS0 = NVMCTRL->CTRLA.bit.CACHEDIS0;
	bool original_CACHEDIS1 = NVMCTRL->CTRLA.bit.CACHEDIS1;
	NVMCTRL->CTRLA.bit.CACHEDIS0 = true;
	NVMCTRL->CTRLA.bit.CACHEDIS1 = true;
	
	// Execute Page Buffer Clear
	NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_PBC;
	while (NVMCTRL->STATUS.bit.READY == 0) { }
    while (NVMCTRL->INTFLAG.bit.DONE == 0) { };
	NVMCTRL->INTFLAG.bit.DONE = 1;
	//print('yo\n');
	for (int i=0; i<(NVMCTRL_PAGE_SIZE/4); i++) {
      *dst_addr = 0xfcfcfcfc;
      dst_addr++;
    }
	// Execute Write Page
	NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_WP;   
	while (NVMCTRL->STATUS.bit.READY == 0) { }
	while (NVMCTRL->INTFLAG.bit.DONE == 0) {};
	NVMCTRL->INTFLAG.bit.DONE = 1;
    invalidate_CMCC_cache();
    // Restore original NVMCTRL cache settings.
    NVMCTRL->CTRLA.bit.CACHEDIS0 = original_CACHEDIS0;
    NVMCTRL->CTRLA.bit.CACHEDIS1 = original_CACHEDIS1;
}

void write_flash51(uint32_t word){	
	NVMCTRL->ADDR.reg = ((uint32_t)flash_block);
  	NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_EB;
	while (!NVMCTRL->INTFLAG.bit.DONE) { }
	invalidate_CMCC_cache();
	volatile uint32_t *dst_addr = (volatile uint32_t *)flash_block;

	// Disable auto page write
	NVMCTRL->CTRLA.bit.WMODE = 0;
	while (NVMCTRL->STATUS.bit.READY == 0) { }
	// Disable NVMCTRL cache while writing, per SAMD51 errata.
	bool original_CACHEDIS0 = NVMCTRL->CTRLA.bit.CACHEDIS0;
	bool original_CACHEDIS1 = NVMCTRL->CTRLA.bit.CACHEDIS1;
	NVMCTRL->CTRLA.bit.CACHEDIS0 = true;
	NVMCTRL->CTRLA.bit.CACHEDIS1 = true;
	
	// Execute Page Buffer Clear
	NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_PBC;
	while (NVMCTRL->STATUS.bit.READY == 0) { }
    while (NVMCTRL->INTFLAG.bit.DONE == 0) { };
	NVMCTRL->INTFLAG.bit.DONE = 1;
	//print('yo\n');
    *dst_addr = word;
	// Execute Write Page
	NVMCTRL->CTRLB.reg = NVMCTRL_CTRLB_CMDEX_KEY | NVMCTRL_CTRLB_CMD_WP;   
	while (NVMCTRL->STATUS.bit.READY == 0) { }
	while (NVMCTRL->INTFLAG.bit.DONE == 0) {};
	NVMCTRL->INTFLAG.bit.DONE = 1;
    invalidate_CMCC_cache();
    // Restore original NVMCTRL cache settings.
    NVMCTRL->CTRLA.bit.CACHEDIS0 = original_CACHEDIS0;
    NVMCTRL->CTRLA.bit.CACHEDIS1 = original_CACHEDIS1;

}

uint32_t read_flash51(){
	return *((uint32_t*)flash_block);
}


#endif