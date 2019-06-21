#include "sercoms.h"

dummy_type *sercom_handlers[6] = {0, 0, 0, 0, 0, 0};

#define register_handler(n) \
void SERCOM##n##_Handler(void) { \
    if (sercom_handlers[n] != 0) { \
        sercom_handlers[n]->fn(n); \
    } \
}

register_handler(0)
register_handler(1)
register_handler(2)
register_handler(3)
register_handler(4)
register_handler(5)
