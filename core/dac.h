#pragma once

#include "samd21.h"

#include "gpio.h"


/**
 * @brief Initializes the DAC driver.
 * 
 * It uses the VDDANA reference, usually 3.3 V. The only pin that can be used
 * as a DAC output is PA02, which gets set up as such by this function.
 *
 * @effects The function waits for synchronization, which should not pose a
 *          problem unless something is extremely wrong with the MCU to begin
 *          with.
 */
void dac_init();


/**
 * @brief Sets the number of counts for the DAC, which is always the PA02 pin.
 *
 * @param Number of counts to set the DAC output, in 10-bit resolution (i.e.
 *        range of 0 to 1023).
 */
void dac_set(int level);
