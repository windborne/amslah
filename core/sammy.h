#pragma once

#if defined(__SAMD21J18A__) || defined(__SAMD21E18A__)
#include "samd21.h"
#define NUM_SERCOMS 6
#else
#include "samd51.h"
#include "samd51defs.h"
#define NUM_SERCOMS 8
#endif

