#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sammy.h"

// NOTE: if the size is changed here, the size _and_ alignment in the linker must be manually changed
#define MTB_MAGNITUDE 6
#define MTB_SIZE (1<<MTB_MAGNITUDE)
#define MTB_SIZE_WORDS (1<<(MTB_MAGNITUDE-2))


/**
 * @brief Initializes the Micro Trace Buffer.
 * 
 * This function uses a dedicated place in memory to store the buffer (.mbt), which will not
 * get overwritten on reset. It stores enough space for 2 buffers that it switches between 
 * on init each time, so that after a reset the program check the program counter history after a 
 * reset.
 */
void mtb_init();


/**
 * @brief Starts Micro Trace Buffer capture.
 */
void mtb_start();


/**
 * @brief Stops Micro Trace Buffer capture. 
 *
 * This should be called before any while loops
 * that will halt the processor, so that after a reset one can check what caused the program
 * to enter such loop.
 */
void mtb_stop();

/**
 * @brief Gets pointer to the current buffer, i.e. the buffer that is curently configured to
 * be written to. 
 *
 * @return pointer to the start of the current trace buffer
 */
uint32_t * mtb_get_curr();

/**
 * @brief Gets pointer to the previous buffer
 *
 * @return pointer to the start of the previous trace buffer
 */
uint32_t * mtb_get_prev();

#ifdef __cplusplus
}
#endif
