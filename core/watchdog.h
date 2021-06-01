#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sammy.h"

void watchdog_init(int period);
void watchdog_kick();

#ifdef __cplusplus
}
#endif
