#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "samd21.h"

#include "gpio.h"

#include "task.h"

extern xTaskHandle eic_handles[16];

void eic_init();

bool eic_wait(uint8_t pin, uint8_t level, uint32_t timeout);

#ifdef __cplusplus
}
#endif
