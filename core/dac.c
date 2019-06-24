#include "amslah_config.h"

#include "adc.h"
#include "gpio.h"

void dac_init() {
    gpio_function(GPIO(GPIO_PORTA, 2), (2 << 16) | 1);

    PM->APBCMASK.reg |= PM_APBCMASK_DAC;

    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_DAC
                        | GCLK_CLKCTRL_GEN_GCLK1
                        | (1 << GCLK_CLKCTRL_CLKEN_Pos);
    while(GCLK->STATUS.bit.SYNCBUSY);

    DAC->CTRLB.bit.REFSEL = 0x1; /* VDDANA reference. */
    DAC->CTRLB.bit.EOEN = 0x1; /* Enable output.*/

    while (DAC->STATUS.reg & DAC_STATUS_SYNCBUSY) {};

    DAC->CTRLA.bit.ENABLE = 1;

    while (DAC->STATUS.reg & DAC_STATUS_SYNCBUSY) {};

}

void dac_set(int level) {
    DAC->DATA.reg = level;
}
