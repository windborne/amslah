#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "sammy.h"

#include "gpio.h"

extern bool used_tcs[8];

typedef struct {
	uint8_t pin; //which pin
	uint8_t timer; //which TC(C) to use
	uint8_t mux; // make sure it matches the TCC, defults to 4 (E)
	uint8_t output; //not sure - something on samd51 only???
	uint8_t prescaler; //do we divide?
	uint8_t resolution; //must be 8
	uint8_t period; //when do we loop
	uint8_t start_count; // what value do we start at
	// uint8_t invert; // should we invert the pin?
} pwmcfg_t;

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

void pwm_init_with(pwmcfg_t cfg);


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

void pwm51_init(uint8_t pin, uint8_t mux, uint8_t tc);
void pwm51_set(uint8_t pin, uint8_t tc, int level);

#ifdef __cplusplus
}
#endif
