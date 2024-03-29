#include "amslah_config.h"

#include "adc.h"
#include "gpio.h"

#ifdef _SAMD51_
#define ADC ADC0
#define ADC_BUSY_MASK ADC_STATUS_ADCBUSY
#else
#define ADC_BUSY_MASK ADC_STATUS_SYNCBUSY
#endif

#pragma GCC diagnostic ignored "-Warray-bounds"


#ifdef _SAMD21_
const uint8_t adc_ains[] = {-1, -1, 0, 1, 4, 5, 6, 7, 16, 17, 18, 19, /* PA11 */
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* PA25 */
                            -1, -1, -1, -1, -1, -1, /* PA31 */
                            8, 9, 10, 11, 12, 13, 14, 15, /* PB07 */
                            2, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /* PB21 */
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; /* PB31 */
#else
const uint8_t adc_ains[] = {-1, -1, 0, 1, 4, 5, 6, 7, 8, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 13, 14, 15, -1, -1, -1, -1, 2, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
#endif

SemaphoreHandle_t adc_mutex = 0;
SemaphoreHandle_t adc_call_mutex = 0;
volatile int adc_result;

uint8_t runtime_adc_reference = ADC_REFERENCE;

#define ADC_Fn \
    ADC->INTENCLR.reg = ADC_INTENCLR_RESRDY; \
    adc_result = ADC->RESULT.reg; \
	BaseType_t woke = pdFALSE; \
    xSemaphoreGiveFromISR(adc_call_mutex, &woke); \
	portYIELD_FROM_ISR(woke);

void ADC_Handler() {
	ADC_Fn
}

void ADC0_0_Handler() {
	ADC_Fn
}

void ADC0_1_Handler() {
	ADC_Fn
}

void adc_deinit() {
    xSemaphoreTake(adc_mutex, portMAX_DELAY);
    ADC->CTRLA.bit.ENABLE = 0;
    while (ADC->STATUS.reg & ADC_BUSY_MASK) {};
    xSemaphoreGive(adc_mutex);
}

void adc_init() {
	if (adc_mutex == 0) {
        adc_mutex = xSemaphoreCreateBinary();
        adc_call_mutex = xSemaphoreCreateBinary();
        xSemaphoreGive(adc_mutex);
	} else {
        xSemaphoreTake(adc_mutex, portMAX_DELAY);
    }
    if (ADC->CTRLA.bit.ENABLE) {
        xSemaphoreGive(adc_mutex);
        return;
    }

#ifdef _SAMD21_
    PM->APBCMASK.reg |= PM_APBCMASK_ADC;

    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID_ADC
                        | GCLK_CLKCTRL_GEN_GCLK1
                        | (1 << GCLK_CLKCTRL_CLKEN_Pos);
    while(GCLK->STATUS.bit.SYNCBUSY);
#else
	MCLK->APBDMASK.bit.ADC0_ = 1;
	GCLK->PCHCTRL[ADC0_GCLK_ID].reg = 0 | (1 << GCLK_PCHCTRL_CHEN_Pos);
#endif

    ADC->REFCTRL.bit.REFCOMP = 1;
    ADC->REFCTRL.bit.REFSEL = runtime_adc_reference; /* 1.0 V voltage reference by default. */

    ADC->AVGCTRL.bit.SAMPLENUM = ADC_OVERSAMPLE;

    ADC->SAMPCTRL.bit.SAMPLEN = ADC_SAMPLE_TIME;

    while (ADC->STATUS.reg & ADC_BUSY_MASK) {};

#ifdef _SAMD21_
    ADC->CTRLB.bit.PRESCALER = ADC_FREQUENCY_PRESCALER;
    ADC->CTRLB.bit.DIFFMODE = 0; /* Single ended. */
#else // rly
    ADC->CTRLA.bit.PRESCALER = ADC_FREQUENCY_PRESCALER;
    ADC->INPUTCTRL.bit.DIFFMODE = 0; /* Single ended. */
#endif
    ADC->CTRLB.bit.RESSEL = ADC_AVERAGE; /* Averaging by default. */
    ADC->CTRLB.bit.CORREN = 0;
    ADC->CTRLB.bit.FREERUN = 0;

    while (ADC->STATUS.reg & ADC_BUSY_MASK) {};

#ifdef _SAMD21_
    ADC->INPUTCTRL.bit.GAIN = 0;
    ADC->INPUTCTRL.bit.MUXNEG = 0x19;
#else
    ADC->INPUTCTRL.bit.MUXNEG = 0x18;
#endif

    while (ADC->STATUS.reg & ADC_BUSY_MASK) {};

#ifdef _SAMD21_
	ADC->CALIB.reg = ADC_CALIB_BIAS_CAL((*(uint32_t *) ADC_FUSES_BIASCAL_ADDR >> ADC_FUSES_BIASCAL_Pos)) | ADC_CALIB_LINEARITY_CAL((*(uint64_t *) ADC_FUSES_LINEARITY_0_ADDR >> ADC_FUSES_LINEARITY_0_Pos));
#else
		uint32_t refbuf = (*((uint32_t*)ADC0_FUSES_BIASREFBUF_ADDR) & ADC0_FUSES_BIASREFBUF_Msk) >> ADC0_FUSES_BIASREFBUF_Pos;
		uint32_t comp = (*((uint32_t*)ADC0_FUSES_BIASCOMP_ADDR) & ADC0_FUSES_BIASCOMP_Msk) >> ADC0_FUSES_BIASCOMP_Pos;
		ADC->CALIB.reg = ADC_CALIB_BIASREFBUF(refbuf) | ADC_CALIB_BIASCOMP(comp);
#endif
	ADC->CTRLA.reg = ADC_CTRLA_ENABLE;

    while (ADC->STATUS.reg & ADC_BUSY_MASK) {};

    ADC->INTENSET.bit.RESRDY = 1;

#ifdef _SAMD21_
    NVIC_EnableIRQ(ADC_IRQn);
#else
	//NVIC_EnableIRQ(ADC0_0_IRQn);
	NVIC_SetPriority(ADC0_1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
	NVIC_EnableIRQ(ADC0_1_IRQn);
#endif
    xSemaphoreGive(adc_mutex);
}

void adc_init_pin(uint8_t pin) {
	if (pin == 255) return;
    gpio_function(pin, (pin << 16) | 1);
    configASSERT(adc_ains[pin] != -1);
}

int adc_sample(uint8_t pin) {
	if (pin == 255) return 0;
	if (!ADC->CTRLA.bit.ENABLE) return -1;
	int ain;
	if (pin == 137) ain = 0x18;
    else if (pin == 138) ain = 0x1a;
    else if (pin == 139) ain = 0x1b;
	else if (adc_ains[pin] == -1) return -1;
	else ain = adc_ains[pin];
    xSemaphoreTake(adc_mutex, portMAX_DELAY);
    ADC->INPUTCTRL.bit.MUXPOS = ain;
    ADC->SWTRIG.reg = 0b11;
    ADC->INTENSET.bit.RESRDY = 1;

    xSemaphoreTake(adc_call_mutex, portMAX_DELAY);
    int result = adc_result;
    xSemaphoreGive(adc_mutex);

	if (ADC_AVERAGE) {
		if (ADC_OVERSAMPLE < 5) {
			result = result >> ADC_OVERSAMPLE;
		} else {
			result = result >> 4;
		}
	}
    return result;
}

/* Thank you, for once, ASF, for not making me deal with Table 37-39*/

float tempR;       /* Production Room Temperature value read from NVM memory - tempR */
float tempH;	   /* Production Hot Temperature value read from NVM memory - tempH */
float INT1VR;      /* Room temp 2’s complement of the internal 1V reference value - INT1VR */
float INT1VH;	   /* Hot temp 2’s complement of the internal 1V reference value - INT1VR */
uint16_t ADCR;     /* Production Room Temperature ADC Value read from NVM memory - ADCR */
uint16_t ADCH;     /* Production Hot Temperature ADC Value read from NVM memory - ADCH */

float VADCR;	   /* Room Temperature ADC voltage - VADCR */
float VADCH;	   /* Hot Temperature ADC voltage - VADCH */


/* Compatible definition for previous driver (begin 2) */
#define NVMCTRL_FUSES_ROOM_ADC_VAL_ADDR (NVMCTRL_TEMP_LOG + 4)
#define NVMCTRL_FUSES_ROOM_ADC_VAL_Pos 8            /**< \brief (NVMCTRL_TEMP_LOG) 12-bit ADC conversion at room temperature */
#define NVMCTRL_FUSES_ROOM_ADC_VAL_Msk (0xFFFu << NVMCTRL_FUSES_ROOM_ADC_VAL_Pos)
#define NVMCTRL_FUSES_ROOM_ADC_VAL(value) ((NVMCTRL_FUSES_ROOM_ADC_VAL_Msk & ((value) << NVMCTRL_FUSES_ROOM_ADC_VAL_Pos)))

#define NVMCTRL_FUSES_ROOM_INT1V_VAL_ADDR NVMCTRL_TEMP_LOG
#define NVMCTRL_FUSES_ROOM_INT1V_VAL_Pos 24           /**< \brief (NVMCTRL_TEMP_LOG) 2's complement of the internal 1V reference drift at room temperature (versus a 1.0 centered value) */
#define NVMCTRL_FUSES_ROOM_INT1V_VAL_Msk (0xFFu << NVMCTRL_FUSES_ROOM_INT1V_VAL_Pos)
#define NVMCTRL_FUSES_ROOM_INT1V_VAL(value) ((NVMCTRL_FUSES_ROOM_INT1V_VAL_Msk & ((value) << NVMCTRL_FUSES_ROOM_INT1V_VAL_Pos)))

#define NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_ADDR NVMCTRL_TEMP_LOG
#define NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_Pos 8            /**< \brief (NVMCTRL_TEMP_LOG) Decimal part of room temperature */
#define NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_Msk (0xFu << NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_Pos)
#define NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC(value) ((NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_Msk & ((value) << NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_Pos)))

#define NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_ADDR NVMCTRL_TEMP_LOG
#define NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_Pos 0            /**< \brief (NVMCTRL_TEMP_LOG) Integer part of room temperature in oC */
#define NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_Msk (0xFFu << NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_Pos)
#define NVMCTRL_FUSES_ROOM_TEMP_VAL_INT(value) ((NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_Msk & ((value) << NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_Pos)))


/* Compatible definition for previous driver (begin 1) */
#define NVMCTRL_FUSES_HOT_ADC_VAL_ADDR (NVMCTRL_TEMP_LOG + 4)
#define NVMCTRL_FUSES_HOT_ADC_VAL_Pos 20           /**< \brief (NVMCTRL_TEMP_LOG) 12-bit ADC conversion at hot temperature */
#define NVMCTRL_FUSES_HOT_ADC_VAL_Msk (0xFFFu << NVMCTRL_FUSES_HOT_ADC_VAL_Pos)
#define NVMCTRL_FUSES_HOT_ADC_VAL(value) ((NVMCTRL_FUSES_HOT_ADC_VAL_Msk & ((value) << NVMCTRL_FUSES_HOT_ADC_VAL_Pos)))

#define NVMCTRL_FUSES_HOT_INT1V_VAL_ADDR (NVMCTRL_TEMP_LOG + 4)
#define NVMCTRL_FUSES_HOT_INT1V_VAL_Pos 0            /**< \brief (NVMCTRL_TEMP_LOG) 2's complement of the internal 1V reference drift at hot temperature (versus a 1.0 centered value) */
#define NVMCTRL_FUSES_HOT_INT1V_VAL_Msk (0xFFu << NVMCTRL_FUSES_HOT_INT1V_VAL_Pos)
#define NVMCTRL_FUSES_HOT_INT1V_VAL(value) ((NVMCTRL_FUSES_HOT_INT1V_VAL_Msk & ((value) << NVMCTRL_FUSES_HOT_INT1V_VAL_Pos)))

#define NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_ADDR NVMCTRL_TEMP_LOG
#define NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_Pos 20           /**< \brief (NVMCTRL_TEMP_LOG) Decimal part of hot temperature */
#define NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_Msk (0xFu << NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_Pos)
#define NVMCTRL_FUSES_HOT_TEMP_VAL_DEC(value) ((NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_Msk & ((value) << NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_Pos)))

#define NVMCTRL_FUSES_HOT_TEMP_VAL_INT_ADDR NVMCTRL_TEMP_LOG
#define NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Pos 12           /**< \brief (NVMCTRL_TEMP_LOG) Integer part of hot temperature in oC */
#define NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Msk (0xFFu << NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Pos)
#define NVMCTRL_FUSES_HOT_TEMP_VAL_INT(value) ((NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Msk & ((value) << NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Pos)))


float convert_dec_to_frac(uint8_t val) {
	if (val < 10) {
		return ((float)val/10.0);
	}
	
	else if (val <100 ) {
		return ((float)val/100.0);
	}
	
	else {
		return ((float)val/1000.0);
	}
}


void adc_temp_init(void) {
	volatile uint32_t val1;				/* Temperature Log Row Content first 32 bits */
	volatile uint32_t val2;				/* Temperature Log Row Content another 32 bits */
	uint8_t room_temp_val_int;			/* Integer part of room temperature in °C */
	uint8_t room_temp_val_dec;			/* Decimal part of room temperature in °C */
	uint8_t hot_temp_val_int;			/* Integer part of hot temperature in °C */
	uint8_t hot_temp_val_dec;			/* Decimal part of hot temperature in °C */
	int8_t room_int1v_val;				/* internal 1V reference drift at room temperature */
	int8_t hot_int1v_val;				/* internal 1V reference drift at hot temperature*/
	
	uint32_t *temp_log_row_ptr = (uint32_t *)NVMCTRL_TEMP_LOG;
	
	val1 = *temp_log_row_ptr;
	temp_log_row_ptr++;
	val2 = *temp_log_row_ptr;
	
	room_temp_val_int = (uint8_t)((val1 & NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_Msk) >> NVMCTRL_FUSES_ROOM_TEMP_VAL_INT_Pos);
	room_temp_val_dec = (uint8_t)((val1 & NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_Msk) >> NVMCTRL_FUSES_ROOM_TEMP_VAL_DEC_Pos);
	hot_temp_val_int = (uint8_t)((val1 & NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Msk) >> NVMCTRL_FUSES_HOT_TEMP_VAL_INT_Pos);
	hot_temp_val_dec = (uint8_t)((val1 & NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_Msk) >> NVMCTRL_FUSES_HOT_TEMP_VAL_DEC_Pos);
	room_int1v_val = (int8_t)((val1 & NVMCTRL_FUSES_ROOM_INT1V_VAL_Msk) >> NVMCTRL_FUSES_ROOM_INT1V_VAL_Pos);
	hot_int1v_val = (int8_t)((val2 & NVMCTRL_FUSES_HOT_INT1V_VAL_Msk) >> NVMCTRL_FUSES_HOT_INT1V_VAL_Pos);
	ADCR = (uint16_t)((val2 & NVMCTRL_FUSES_ROOM_ADC_VAL_Msk) >> NVMCTRL_FUSES_ROOM_ADC_VAL_Pos);
	ADCH = (uint16_t)((val2 & NVMCTRL_FUSES_HOT_ADC_VAL_Msk) >> NVMCTRL_FUSES_HOT_ADC_VAL_Pos);
	tempR = room_temp_val_int + convert_dec_to_frac(room_temp_val_dec);
	tempH = hot_temp_val_int + convert_dec_to_frac(hot_temp_val_dec);
	INT1VR = 1 - ((float)room_int1v_val/1000.);
	INT1VH = 1 - ((float)hot_int1v_val/1000.);
	VADCR = ((float)ADCR * INT1VR)/4095.;
	VADCH = ((float)ADCH * INT1VH)/4095.;
}

float adc_get_temp() {
#ifdef _SAMD21_
	SYSCTRL->VREF.bit.TSEN = 1;
	int raw = adc_sample(137);
	SYSCTRL->VREF.bit.TSEN = 0;

	float VADC = raw/4095.;
	float coarse_temp = tempR + (((tempH - tempR)/(VADCH - VADCR)) * (VADC - VADCR));
	float INT1VM = INT1VR + (((INT1VH - INT1VR) * (coarse_temp - tempR))/(tempH - tempR));
	float VADCM = ((float)raw*INT1VM)/4095.;
	float fine_temp = tempR + (((tempH - tempR)/(VADCH - VADCR)) * (VADCM - VADCR));
	return fine_temp;
#else
	return -273.15;
#endif
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
uint32_t adc_rng(int nbits) {
#ifdef _SAMD21_
    uint32_t random = 0;
    nbits = (nbits > 32) ? 32 : nbits;
    for(int i = 0; i<nbits; i++){
        SYSCTRL->VREF.bit.TSEN = 1;
        int raw = adc_sample(137);
        SYSCTRL->VREF.bit.TSEN = 0;
        bool bit;
        for(int j=0;j<12;j++){
            bit = bit^(raw>>j & 1);
        }
        raw = adc_sample(138);
        for(int j=0;j<12;j++){
            bit = bit^(raw>>j & 1);
        }
        raw = adc_sample(139);
        for(int j=0;j<12;j++){
            bit = bit^(raw>>j & 1);
        }
        random |= bit<<i;
    }
    return random;
#else
	return 4; // chosen by fair dice roll
#endif
}
#pragma GCC diagnostic pop
