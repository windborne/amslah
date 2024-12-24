#include "sercoms.h"

dummy_type *sercom_handlers[NUM_SERCOMS] = {0};

#ifdef _SAMD21_
#define register_handler(n) \
__attribute__((weak)) void SERCOM##n##_Handler(void) { \
    if (sercom_handlers[n] != 0) { \
        sercom_handlers[n]->fn(n); \
    } \
}

#else

#define register_handler(n) \
__attribute__((weak)) void SERCOM##n##_0_Handler(void) { \
    if (sercom_handlers[n] != 0) { \
        sercom_handlers[n]->fn(n); \
    } \
} \
__attribute__((weak)) void SERCOM##n##_1_Handler(void) { \
    if (sercom_handlers[n] != 0) { \
        sercom_handlers[n]->fn(n); \
    } \
} \
__attribute__((weak)) void SERCOM##n##_2_Handler(void) { \
    if (sercom_handlers[n] != 0) { \
        sercom_handlers[n]->fn(n); \
    } \
} \
__attribute__((weak)) void SERCOM##n##_3_Handler(void) { \
    if (sercom_handlers[n] != 0) { \
        sercom_handlers[n]->fn(n); \
    } \
} \

#endif

register_handler(0)
register_handler(1)
register_handler(2)
register_handler(3)
register_handler(4)
register_handler(5)
#if NUM_SERCOMS > 6
	register_handler(6)
#endif
#if NUM_SERCOMS > 7
	register_handler(7)
#endif
