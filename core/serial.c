#include "amslah.h"

#if USE_DEBUG_UART

#include "serial.h"
#include "pwm.h"

#include "stream_buffer.h"

StreamBufferHandle_t serial_stream;
char printf_buffer[512];
uart_t debug_uart;

void raw_print(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    int nb = vsnprintf_(printf_buffer, 512, fmt, va);

    uart_write(&debug_uart, (uint8_t*)printf_buffer, nb);
}

void print(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    taskENTER_CRITICAL();
    char *tn = pcGetCurrentTaskName();
    //char *tn = "hello!";
    int ticks = xTaskGetTickCount()/1000;
    int mins = ticks/60;
    int secs = ticks%60;
    int nc = snprintf_(printf_buffer, 21, "<%6s|%2d:%02d> ", tn, mins, secs);
    int nb = vsnprintf_(printf_buffer + nc, 512 - nc, fmt, va);
    xStreamBufferSend(serial_stream, printf_buffer, nb + nc, 0);
    taskEXIT_CRITICAL();
}

void serial_task(void *params){
    uint8_t usb_buffer[64];
    vTaskDelay(500);
    while (true) {
        int recv = xStreamBufferReceive(serial_stream, usb_buffer, 64, 200);
        if (recv > 0) {
            uart_write(&debug_uart, usb_buffer, recv);
        }
    }
}

#if USAGE_REPORT

TaskStatus_t pxTaskStatusArray[16];

int base = 0;

void usage_task(void *params) {
    int n = 0;
    volatile UBaseType_t uxArraySize, x;
    unsigned long ulTotalRunTime, ulStatsAsPercentage;
    while (true) {
        if (used_tcs[USAGE_REPORT_TC]) {
            print("ERROR ERROR IMPORTANT ERROR ERROR\n");
            print("The timer counter used for the usage report is taken by a PWM pin\n");
            print("Change USAGE_REPORT_TC in the config as necessary\n");
        }
        uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, 8, &ulTotalRunTime);

        print("RAM & CPU usage report:\n");
        for( x = 0; x < uxArraySize; x++ ) {
            uint32_t num = TC4->COUNT32.COUNT.reg;
            float pc = 0;
            if (ulTotalRunTime != 0) {
                pc = pxTaskStatusArray[x].ulRunTimeCounter*100.;
                pc /= ulTotalRunTime;
            }
            print("%8s - %3d spare words- %.1f%%\n",
                pxTaskStatusArray[ x ].pcTaskName,
                pxTaskStatusArray[ x ].usStackHighWaterMark,
                pc);
        }
        print("base is %d\n", base);
        vTaskDelay(2000);
    }
}

void
#if USAGE_REPORT_TC == 3
    TC3_Handler
#elif USAGE_REPORT_TC == 4
    TC4_Handler
#elif USAGE_REPORT_TC == 5
    TC5_Handler
#elif USAGE_REPORT_TC == 6
    TC6_Handler
#elif USAGE_REPORT_TC == 7
    TC7_Handler
#else
    #error "ohp"
#endif
() {
    TcCount16 *hw = (TcCount16*)(((char*)TCC0) + 1024 * USAGE_REPORT_TC);
    base += 65536;
    hw->INTFLAG.bit.OVF = 1;
}

inline uint32_t vGetRunTimeCounterValue(void) {
    TcCount16 *hw = (TcCount16*)(((char*)TCC0) + 1024 * USAGE_REPORT_TC);
    return base + hw->COUNT.reg;
}

void vConfigureTimerForRunTimeStats(void) {
    PM->APBCMASK.reg |= (1 << (8 + USAGE_REPORT_TC));

    int channel = 0x1a + (USAGE_REPORT_TC/2);
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(channel)
                        | GCLK_CLKCTRL_GEN_GCLK1
                        | (1 << GCLK_CLKCTRL_CLKEN_Pos);
    while(GCLK->STATUS.bit.SYNCBUSY);

    TcCount16 *hw = (TcCount16*)(((char*)TCC0) + 1024 * USAGE_REPORT_TC);
    hw->CTRLA.bit.MODE = 0x0; // 16 bit
    hw->CTRLA.bit.WAVEGEN = 0x0; // normal frequency
    hw->CTRLA.bit.PRESCALER = 0x6; // div256
    hw->INTENSET.bit.OVF = 1;
    //hw->CC[0].reg = 65537;
    hw->CTRLA.bit.ENABLE = 1;
    while (hw->STATUS.bit.SYNCBUSY);
    NVIC_EnableIRQ(TCC0_IRQn + USAGE_REPORT_TC);
/*
    REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID_TC4_TC5) ;

    TcCount32* TC = (TcCount32*) TC4;
    TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT32;
    TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_NFRQ;
    TC4->COUNT32.CC[0].reg = 4294967295;
    //TC4->COUNT32.CC[0].reg = 150000;
    TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV256;
    TC->CTRLA.reg |= TC_CTRLA_ENABLE;
    while (TC4->COUNT32.STATUS.bit.SYNCBUSY);
*/

}


#endif

void init_serial() {
    //uart_init(&debug_uart, 2, 115200, PA12, PINMUX_PA12C_SERCOM2_PAD0, PA13, PINMUX_PA13C_SERCOM2_PAD1);
    uart_init(&debug_uart, DEBUG_UART_SERCOM, DEBUG_UART_BAUD,
                DEBUG_UART_TX_PIN, DEBUG_UART_TX_MUX,
                DEBUG_UART_RX_PIN, DEBUG_UART_RX_MUX);

    serial_stream = xStreamBufferCreate(512, 64);
    xTaskCreate(serial_task, "serial", 100, 0, 5, NULL);
    #if USAGE_REPORT
        xTaskCreate(usage_task, "usage", 150, 0, 5, NULL);
    #endif
}

#endif
