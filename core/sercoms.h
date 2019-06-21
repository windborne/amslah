#pragma once

#include "samd21.h"

typedef void (*sercom_handler_f)(int signum);

typedef struct {
    sercom_handler_f fn;
} dummy_type;

extern dummy_type *sercom_handlers[6];
