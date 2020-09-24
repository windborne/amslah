#include "eeprom.h"
#include "serial.h"
#include "samd21.h"
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

void eeprom_init(){
	if (eeprom_mutex == 0) {
        eeprom_mutex = xSemaphoreCreateBinary();
    }
	NVMCTRL->CTRLB.bit.MANW = 1;
	uint8_t size_val = ((*((uint32_t*)NVMCTRL_FUSES_EEPROM_SIZE_ADDR)) >> NVMCTRL_FUSES_EEPROM_SIZE_Pos) & 0b111;
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