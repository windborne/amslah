#pragma once

#include "samd21.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "gpio.h"

void adc_init();
void adc_init_pin(uint8_t pin);

int adc_sample(uint8_t pin);
