#pragma once
#include "sammy.h"

#ifdef __cplusplus
extern "C" {
#endif

void eeprom_init();

void eeprom_write(uint8_t * buf, uint8_t size, uint8_t row_num);
void eeprom_read(uint8_t * buf, uint8_t size, uint8_t row_num);

#ifdef __cplusplus
}
#endif
