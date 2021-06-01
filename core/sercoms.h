#pragma once

#include "sammy.h"

typedef void (*sercom_handler_f)(int signum);

typedef struct {
    sercom_handler_f fn;
} dummy_type;

extern dummy_type *sercom_handlers[NUM_SERCOMS];
