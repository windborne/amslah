#include "amslah.h"

#if USE_DEBUG_UART

#include "semphr.h"
#include "serial.h"
#include "pwm.h"

#include "stream_buffer.h"

SemaphoreHandle_t serial_mutex;
SemaphoreHandle_t printing_mutex;
SemaphoreHandle_t postprint_mutex;
StreamBufferHandle_t serial_stream;
char printf_buffer[512];
uart_t debug_uart;
TaskHandle_t serial_handle = 0;

char *printing_task;
const char *printing_fmt;
va_list printing_va;

void raw_print(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    int nb = vsnprintf_(printf_buffer, 512, fmt, va);

    uart_write(&debug_uart, (uint8_t*)printf_buffer, nb);
}

void print(const char *fmt, ...) {
    xSemaphoreTake(serial_mutex, portMAX_DELAY);
    printing_task = pcGetCurrentTaskName();
    printing_fmt = fmt;
    va_start(printing_va, fmt);
    
    xSemaphoreGive(printing_mutex);
    xSemaphoreTake(postprint_mutex, portMAX_DELAY);
    va_end(printing_va);
    xSemaphoreGive(serial_mutex);
}

void serial_task(void *params){
    while (true) {
        xSemaphoreTake(printing_mutex, portMAX_DELAY);
        int ticks = xTaskGetTickCount()/1000;
        int mins = ticks/60;
        int secs = ticks%60;
        int nc = snprintf_(printf_buffer, 21, "<%6s|%2d:%02d> ", printing_task, mins, secs);
        int nb = vsnprintf_(printf_buffer + nc, 512 - nc, printing_fmt, printing_va);
        uart_write(&debug_uart, (uint8_t*)printf_buffer, nc + nb);
        xSemaphoreGive(postprint_mutex);
    }
}

#if USAGE_REPORT

TaskStatus_t task_statuses[16];

int base = 0;

void usage_task(void *params) {
    volatile UBaseType_t arr_size;
    unsigned long total_runtime;
    vTaskDelay(USAGE_REPORT_INITIAL_WAIT);
    while (true) {
        if (used_tcs[USAGE_REPORT_TC]) {
            print("ERROR ERROR IMPORTANT ERROR ERROR\n");
            print("The timer counter used for the usage report is taken by a PWM pin\n");
            print("Change USAGE_REPORT_TC in the config as necessary\n");
        }
        arr_size = uxTaskGetSystemState(task_statuses, 8, &total_runtime);

        print("RAM & CPU usage report:\n");
        for (int x = 0; x<arr_size; x++) {
            float pc = 0;
            if (total_runtime != 0) {
                pc = task_statuses[x].ulRunTimeCounter*100.;
                pc /= total_runtime;
            }
            print("%8s - %3d spare words - %.1f%%\n",
                task_statuses[x].pcTaskName,
                task_statuses[x].usStackHighWaterMark,
                pc);
        }
        vTaskDelay(USAGE_REPORT_INTERVAL);
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
    hw->INTFLAG.reg |= TC_INTFLAG_OVF;
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
    hw->CTRLA.bit.ENABLE = 1;
    while (hw->STATUS.bit.SYNCBUSY);
    NVIC_EnableIRQ(TCC0_IRQn + USAGE_REPORT_TC);

}


#endif

void init_serial() {
    uart_init(&debug_uart, DEBUG_UART_SERCOM, DEBUG_UART_BAUD,
                DEBUG_UART_TX_PIN, DEBUG_UART_TX_MUX,
                DEBUG_UART_RX_PIN, DEBUG_UART_RX_MUX);

    serial_stream = xStreamBufferCreate(512, 64);
    serial_mutex = xSemaphoreCreateBinary();
    xSemaphoreGive(serial_mutex);
    printing_mutex = xSemaphoreCreateBinary();
    postprint_mutex = xSemaphoreCreateBinary();
    xTaskCreate(serial_task, "serial", 130, 0, 5, &serial_handle);
    #if USAGE_REPORT
        xTaskCreate(usage_task, "usage", 80, 0, 5, NULL);
    #endif
}

#endif
