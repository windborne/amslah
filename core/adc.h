#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "samd21.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "gpio.h"


/**
 * @brief Initializes the ADC driver.
 *
 * Configuration is determined by ADC_OVERSAMPLE, ADC_FREQUENCY_PRESCALER, and
 * ADC_SAMPLE_TIME in the configuration file. This will change for the better
 * in the near future.
 *
 * @effects The function waits for synchronization, which should not pose a
 *          problem unless something is extremely wrong with the MCU to begin
 *          with.
 */
void adc_init();

void adc_deinit();

/**
 * @brief Initializes a pin for use with the ADC.
 *
 * @param pin Pin to initialize for analog reading.
 *
 * @effects If the pin cannot be used with the ADC it will halt the processor.
 */
void adc_init_pin(uint8_t pin);


/**
 * @brief Trigger a conversion from the ADC on a particular pin
 *
 * @param Pin to read from, which must have been previously initialized with
 *        adc_init_pin(pin);
 *
 * @return ADC counts, in 12-bit resolution (i.e. 0-4095 range).
 *
 * @effects The function might wait as it tries to take ownership of the ADC
 *          while some other thread is using it. Once it has ownership of the
 *          ADC it will non-busily wait for the conversion to be finished by
 *          using the ADC interrupt event.
 */
int adc_sample(uint8_t pin);

void adc_temp_init();
float adc_get_temp();

uint32_t adc_rng(int nbits);

#ifdef __cplusplus
}
#endif
