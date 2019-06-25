#pragma once

#include "samd21.h"

#include "gpio.h"


/**
 * @brief Initialize a pin so that it can be pulse width modulated.
 * 
 * Not all pins are supported; they need to be usable with a timer-counter or a
 * timer-counter for control applications, as in the Table 7-1 of the datasheet.
 * 
 * The default resolution is 8 bits. It is not possible to have independent PWM
 * between 9 and 15 bits, but it is if using exactly 16 bits (not yet
 * implemented). The resolution can be changed with the PWM_RESOLUTION
 * configuration variable.
 *
 * @param pin Pin to set up for PWM.
 *
 * @effects If the pin cannot be used with PWM it will halt the processor. The
 *          function also waits for synchronization, which should not pose a
 *          problem unless something is extremely wrong with the MCU to begin
 *          with.
 *
 *          The function also possibly starts one of the TC or TCCs.
 */
void pwm_init(uint8_t pin);


/**
 * @brief Sets the PWM level of of a pin to a given number of counts, with 100%
 *        corresponding to (2^PWM_RESOLUTION - 1).
 *
 * @param pin Pin to change the level of, which must have been previously
 *            initialized.
 * @param level Integer count of the PWM level.
 *
 * @effects The PWM level is set directly and the function does not wait.
 *          Optionally see Figure 30-7 in the datasheet to see how changing the
 *          timer value happens.
 */
void pwm_set(uint8_t pin, int level);

