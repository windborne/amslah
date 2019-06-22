#include "amslah.h"

#if USE_DEBUG_UART

#include "serial.h"

#include "stream_buffer.h"

StreamBufferHandle_t serial_stream;
char printf_buffer[512];
uart_t debug_uart;

void raw_print(char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    int nb = vsnprintf_(printf_buffer, 512, fmt, va);

    uart_write(&debug_uart, (uint8_t*)printf_buffer, nb);
}

void print(char *fmt, ...) {
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

void init_serial() {
    //uart_init(&debug_uart, 2, 115200, PA12, PINMUX_PA12C_SERCOM2_PAD0, PA13, PINMUX_PA13C_SERCOM2_PAD1);
    uart_init(&debug_uart, DEBUG_UART_SERCOM, DEBUG_UART_BAUD,
                DEBUG_UART_TX_PIN, DEBUG_UART_TX_MUX,
                DEBUG_UART_RX_PIN, DEBUG_UART_RX_MUX);

    serial_stream = xStreamBufferCreate(512, 64);
    xTaskCreate(serial_task, "serial", 100, 0, 5, NULL);
}

#endif
