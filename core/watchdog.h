#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "samd21.h"

void watchdog_init(int period);
void watchdog_kick();

#ifdef __cplusplus
}
#endif
