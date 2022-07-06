#pragma once
#include "sammy.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _SAMD21_
void eeprom_init();
void eeprom_write(uint8_t * buf, uint8_t size, uint8_t row_num);
void eeprom_read(uint8_t * buf, uint8_t size, uint8_t row_num);
#endif 

#ifdef _SAMD51_
void test_flash51();
uint32_t read_flash51();
void write_flash51(uint32_t);
void write_flash51_array(uint32_t* words, int n);

volatile uint32_t* get_raw_flash();
#endif

#ifdef __cplusplus
}
#endif
