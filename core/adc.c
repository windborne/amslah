#include "amslah_config.h"

#include "adc.h"
#include "gpio.h"

const uint8_t adc_ains[] = {-1, -1, 0, 1, 4, 5, 6, 7, 16, 17, 18, 19, /* PA11 */
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* PA25 */
                            -1, -1, -1, -1, -1, -1, /* PA31 */
                            8, 9, 10, 11, 12, 13, 14, 16, /* PB07 */
                            2, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* PB21 */
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; /* PB31 */

SemaphoreHandle_t adc_mutex;
SemaphoreHandle_t adc_call_mutex;
volatile int adc_result;

void ADC_Handler() {
    ADC->INTENCLR.reg = ADC_INTENCLR_RESRDY; 
    adc_result = ADC->RESULT.reg;
    xSemaphoreGiveFromISR(adc_call_mutex, 0);
}

void adc_init() {
    PM->APBCMASK.reg |= PM_APBCMASK_ADC;

    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_ADC
                        | GCLK_CLKCTRL_GEN_GCLK1
                        | (1 << GCLK_CLKCTRL_CLKEN_Pos);
    while(GCLK->STATUS.bit.SYNCBUSY);

    ADC->REFCTRL.bit.REFCOMP = 1;
    ADC->REFCTRL.bit.REFSEL = 0x0; /* 1.0 V voltage reference. */

    ADC->AVGCTRL.bit.SAMPLENUM = ADC_OVERSAMPLE;

    ADC->SAMPCTRL.bit.SAMPLEN = ADC_SAMPLE_TIME;

    while (ADC->STATUS.reg & ADC_STATUS_SYNCBUSY) {};

    ADC->CTRLB.bit.PRESCALER = ADC_FREQUENCY_PRESCALER;
    ADC->CTRLB.bit.RESSEL = 0x1; /* Averaging. */
    ADC->CTRLB.bit.CORREN = 0;
    ADC->CTRLB.bit.FREERUN = 0;
    ADC->CTRLB.bit.DIFFMODE = 0; /* Single ended. */

    while (ADC->STATUS.reg & ADC_STATUS_SYNCBUSY) {};

    ADC->INPUTCTRL.bit.GAIN = 0;
    ADC->INPUTCTRL.bit.MUXNEG = 0x19;

    while (ADC->STATUS.reg & ADC_STATUS_SYNCBUSY) {};

	ADC->CALIB.reg = ADC_CALIB_BIAS_CAL((*(uint32_t *) ADC_FUSES_BIASCAL_ADDR >> ADC_FUSES_BIASCAL_Pos)) | ADC_CALIB_LINEARITY_CAL((*(uint64_t *) ADC_FUSES_LINEARITY_0_ADDR >> ADC_FUSES_LINEARITY_0_Pos));

	ADC->CTRLA.reg = ADC_CTRLA_ENABLE;

    while (ADC->STATUS.reg & ADC_STATUS_SYNCBUSY) {};

    ADC->INTENSET.bit.RESRDY = 1;

    NVIC_EnableIRQ(ADC_IRQn);

    adc_mutex = xSemaphoreCreateBinary();
    xSemaphoreGive(adc_mutex);

    adc_call_mutex = xSemaphoreCreateBinary();

}

void adc_init_pin(uint8_t pin) {
    gpio_function(pin, (pin << 16) | 1);
    if (adc_ains[pin] == -1) {
        __asm("BKPT #0");
    }
}

int adc_sample(uint8_t pin) {
    xSemaphoreTake(adc_mutex, portMAX_DELAY);
    ADC->INPUTCTRL.bit.MUXPOS = adc_ains[pin];
    ADC->SWTRIG.reg = 0b11;
    ADC->INTENSET.bit.RESRDY = 1;

    xSemaphoreTake(adc_call_mutex, portMAX_DELAY);
    int result = adc_result;
    xSemaphoreGive(adc_mutex);
    if (ADC_OVERSAMPLE < 5) {
        result = result >> ADC_OVERSAMPLE;
    } else {
        result = result >> 4;
    }
    return result;
}
