#include "amslah_config.h"

#include "eic.h"
#include "gpio.h"

extern void* pxCurrentTCB;

xTaskHandle eic_handles[16] = {0};

const uint8_t eic_pins[] = {0,1,2,3,4,5,6,7,-1,9,10,11,12,13,14,15, /* PA15 */
	                    0,1,2,3,4,5,6,7,12,13,-1,15,8,-1,10,11, /* PA31 */
			    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15, /* PB15 */
			    0,1,-1,-1,-1,-1,6,7,-1,-1 /*PB25*/, -1, -1, -1, -1, 14, 15 /* PB31 */};


void EIC_Handler() {
    uint32_t flag = EIC->INTFLAG.reg;
    for (int i=0; i<16; i++) {
        if (flag & (1 << i)) {
	    EIC->INTFLAG.reg = 1 << i;
            EIC->INTENCLR.reg = 1 << i;
            if (eic_handles[i] != 0) {
                xTaskNotifyFromISR(eic_handles[i], 1 << 31, eSetBits, 0);
            }
	}
    }
}

void eic_init() {

    PM->APBAMASK.reg |= PM_APBAMASK_EIC;
    EIC->CTRL.reg |= EIC_CTRL_ENABLE;
    NVIC_EnableIRQ(EIC_IRQn);

    while (EIC->STATUS.reg & EIC_STATUS_SYNCBUSY) {};
}

bool eic_wait(uint8_t pin, uint8_t level, uint32_t timeout) {
    if (eic_pins[pin] == -1) return false;
    gpio_direction(pin, GPIO_DIRECTION_IN);
    gpio_function(pin, (eic_pins[pin] << 16) | 0);
    xTaskHandle task = (xTaskHandle)pxCurrentTCB;
    uint8_t eic = eic_pins[pin];
    uint32_t reg = 0;
    if (level == HIGH) reg = 4;
    else if (level == LOW) reg = 5;

    if (eic <= 7) {
        EIC->CONFIG[0].reg &= ~(0b1111 << (4*eic));
	EIC->CONFIG[0].reg |= reg << (4*eic);
    } else if (eic <= 15) {
	eic -= 8;
        EIC->CONFIG[1].reg &= ~(0b1111 << (4*eic));
	EIC->CONFIG[1].reg |= reg << (4*eic);
	eic += 8; // joan is dumb dumb
    }

    eic_handles[eic] = task;
    EIC->INTFLAG.reg = 1 << eic; /* Clear interrupt */
    EIC->INTENSET.reg = 1 << eic; /* Enable interrupt */
    bool ret = xTaskNotifyWait(0xffff, 0xffff, 0, timeout);
    eic_handles[eic] = 0;
    return ret;
}
