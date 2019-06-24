#pragma once

#include "samd21.h"

#include "gpio.h"

void pwm_init(uint8_t pin);

void pwm_set(uint8_t pin, int level);

