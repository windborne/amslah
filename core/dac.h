#pragma once

#include "samd21.h"

#include "gpio.h"

void dac_init();

int dac_set(int level);
