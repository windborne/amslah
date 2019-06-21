#include <FreeRTOS.h>
#include <task.h>

#include <string.h>

#include "core/gpio.h"
#include "core/uart.h"
#include "core/spi.h"

#include "printf.h"

#include "bmp3.h"

/* Definitions of SD commands */
#define CMD0    (0x40+0)        /* GO_IDLE_STATE            */
#define CMD1    (0x40+1)        /* SEND_OP_COND (MMC)       */
#define ACMD41  (0xC0+41)       /* SEND_OP_COND (SDC)       */
#define CMD8    (0x40+8)        /* SEND_IF_COND             */
#define CMD9    (0x40+9)        /* SEND_CSD                 */
#define CMD16   (0x40+16)       /* SET_BLOCKLEN             */
#define CMD17   (0x40+17)       /* READ_SINGLE_BLOCK        */
#define CMD24   (0x40+24)       /* WRITE_SINGLE_BLOCK       */
#define CMD32   (0x40+32)
#define CMD33   (0x40+33)
#define CMD38   (0x40+38)
#define CMD42   (0x40+42)       /* LOCK_UNLOCK              */
#define CMD55   (0x40+55)       /* APP_CMD                  */
#define CMD58   (0x40+58)       /* READ_OCR                 */
#define CMD59   (0x40+59)       /* CRC_ON_OFF               */

#define PB30 GPIO(GPIO_PORTB, 30)
#define PA12 GPIO(GPIO_PORTA, 12)
#define PA13 GPIO(GPIO_PORTA, 13)
#define PA04 GPIO(GPIO_PORTA, 4)
#define PA05 GPIO(GPIO_PORTA, 5)
#define PA18 GPIO(GPIO_PORTA, 18)
#define PA16 GPIO(GPIO_PORTA, 16)
#define PA21 GPIO(GPIO_PORTA, 21)

#define PB06 GPIO(GPIO_PORTB, 6)
#define PB08 GPIO(GPIO_PORTB, 8)
#define PB09 GPIO(GPIO_PORTB, 9)
#define PB10 GPIO(GPIO_PORTB, 10)

#define NRF_CSN GPIO(GPIO_PORTA, 19)
#define NRF_CE GPIO(GPIO_PORTA, 17)

#define BMP380_CS GPIO(GPIO_PORTA, 22)

uart_t uart;
spi_t spi;

uint8_t SPI_RW(uint8_t d) {
    uint8_t recv = 42;
    spi_transfer(&spi, &d, &recv, 1);
    return recv;
}

char printf_buffer[512];

void print(char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    int nb = vsnprintf_(printf_buffer, 512, fmt, va);

    uart_write(&uart, (uint8_t*)printf_buffer, nb);
}

uint8_t sdcmd(uint8_t cmd, uint32_t arg) {

    gpio_set(PB06, HIGH);
    SPI_RW(0xFF);

    gpio_set(PB06, LOW);
    SPI_RW(0xFF);

    SPI_RW(cmd);
    SPI_RW((uint8_t)(arg >> 24));
    SPI_RW((uint8_t)(arg >> 16));
    SPI_RW((uint8_t)(arg >> 8));
    SPI_RW((uint8_t)(arg >> 0));

    uint8_t crc = 0x01;
    if (cmd == CMD0) crc = 0x95;
    else if (cmd == CMD8) crc = 0x87;
    SPI_RW(crc);

    uint8_t res;
    do {
        res = SPI_RW(0xFF);
        print("got %d!\n", res);
    } while ((res & 0x80));
    //uint8_t res = SPI_RW(0xFF);

    return res;
}

uint8_t readregister(uint8_t reg) {
    gpio_set(NRF_CSN, LOW);
    uint8_t recv = 42;
    uint8_t cmd = reg & 0x1f;
    spi_transfer(&spi, &cmd, &recv, 1);
    cmd = 0;
    spi_transfer(&spi, &cmd, &recv, 1);
    gpio_set(NRF_CSN, HIGH);
    return recv;
}

uint8_t readstatus() {
    gpio_set(NRF_CSN, LOW);

    int nop = 255;
    spi_transfer(&spi, &nop, &nop, 1);

    gpio_set(NRF_CSN, HIGH);
    return nop;
}

void dummy2() {

    gpio_direction(PB06, GPIO_DIRECTION_OUT);
    gpio_function(PB06, GPIO_FUNCTION_OFF);
    gpio_set(PB06, HIGH);

    gpio_direction(NRF_CSN, GPIO_DIRECTION_OUT);
    gpio_function(NRF_CSN, GPIO_FUNCTION_OFF);
    gpio_set(NRF_CSN, HIGH);

    gpio_direction(NRF_CE, GPIO_DIRECTION_OUT);
    gpio_function(NRF_CE, GPIO_FUNCTION_OFF);
    gpio_set(NRF_CE, LOW);
    
    while (1) {
        //uint8_t reg = readstatus();
        uint8_t reg = readregister(0x07);
        print("got register! %d\n", reg);
        vTaskDelay(500);
    }
    
    /*for(int idx = 0; idx != 10; idx++) SPI_RW(0xFF);

    vTaskDelay(500);

    print("Starting SD...\n");

    gpio_set(PB06, LOW);


    while (1) {
        uint8_t ret = sdcmd(CMD0, 0);
        print("got %d\n", ret);
        print("Hey\n");
        gpio_set(PB30, HIGH);
        vTaskDelay(500);
        gpio_set(PB30, LOW);
        vTaskDelay(500);
    }
    */

    /*
    int i = 0;
    while (1) {
        char buffer[64];
        char str[64];
        memcpy(str, "got YY bytes 'XXXX'\n", 20);
        int num = 0;
        if (xStreamBufferBytesAvailable(uart.rx_buffer) != 0) {
            num = xStreamBufferReceive(uart.rx_buffer, buffer, 64, portMAX_DELAY);
        }
        //uart_write(&uart, (uint8_t*)"hello world!", 12);
        str[4] = '0' + (num/10);
        str[5] = '0' + (num%10);
        str[14] = buffer[0];
        str[15] = buffer[1];
        str[16] = buffer[2];
        str[17] = buffer[3];
        uart_write(&uart, (uint8_t*)str, 20);
        gpio_set(PB30, HIGH);
        vTaskDelay(500);
        gpio_set(PB30, LOW);
        vTaskDelay(500);
        if (i++ == 5) {
            //SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
            //__DSB();
            //__WFI();
        }
    }*/
}

int8_t user_spi_read_2(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    uint8_t send[24];
    uint8_t recv[24];
    (void)send;
    (void)recv;
    memset(send, 0, len + 1);
    memset(recv, 0, len + 1);
    send[0] = reg_addr | 0x80;
    gpio_set(BMP380_CS, false);
    spi_transfer(&spi, send, recv, len+1);
    gpio_set(BMP380_CS, true);
    memcpy(data, recv + 1, len);
    return 0;
}

int8_t user_spi_write_2(uint8_t dev_id, uint8_t reg_addr, uint8_t *data, uint16_t len) {
    uint8_t send[24];
    uint8_t recv[24];
    (void)send;
    (void)recv;
    memset(send, 0, len+2);
    memcpy(send+1, data, len);
    send[0] = reg_addr;

    gpio_set(BMP380_CS, false);
    for(int i=0;i<len+1;i++){
        spi_transfer(&spi, send+i, recv+i, 1);
    }
    gpio_set(BMP380_CS, true);
    //printf("recv: %d %d %d %d\n", recv[0], recv[1], recv[2], recv[3]);

    return 0;
}

static void user_delay_ms(uint32_t period) {
    vTaskDelay(period / portTICK_PERIOD_MS);
}


void bmp_task(void *params) {

    gpio_direction(BMP380_CS, GPIO_DIRECTION_OUT);
    gpio_function(BMP380_CS, GPIO_FUNCTION_OFF);
    gpio_set(BMP380_CS, LOW);

    int result;
    int8_t rslt = BMP3_OK;
    int8_t mode;

    /* Sensor_0 interface over SPI with native chip select line */
    struct bmp3_dev dev;
    dev.dev_id = BMP380_CS;
    dev.intf = BMP3_SPI_INTF;
    dev.read = user_spi_read_2;
    dev.write = user_spi_write_2;
    dev.delay_ms = user_delay_ms;

    rslt = bmp3_init(&dev);
    /*while (true) {
        print("init result was %d\n", rslt);
        vTaskDelay(1000);
    }*/

    {
    vTaskDelay(10);
    // Actual soft reboot. 
    uint8_t set[] = {0xb6};
    gpio_set(BMP380_CS, false);
    user_spi_write_2(BMP380_CS, 126, set, 1);
    gpio_set(BMP380_CS, true);
    vTaskDelay(5);

    uint8_t recv[5];
    uint8_t odr[5] = {0x1d, 4};
    gpio_set(BMP380_CS, false); 
    spi_transfer(&spi, odr, recv, 2);
    gpio_set(BMP380_CS, true);

    uint8_t send3[3] = {0x1c, 13};
    gpio_set(BMP380_CS, false);
    spi_transfer(&spi, send3, recv, 2);
    gpio_set(BMP380_CS, true);

    uint8_t send[5] = {0x1b, 51};
    gpio_set(BMP380_CS, false); 
    spi_transfer(&spi, send, recv, 2);
    gpio_set(BMP380_CS, true);

    /*
    uint8_t recv[5];
    // 32x oversampling now works! the order of the following lines matters 
    uint8_t send2[5] = {0x1d, 0, 4};
    gpio_set(BMP380_CS, false);
    spi_transfer(&spi, send2, recv, 3);
    gpio_set(BMP380_CS, true);
    vTaskDelay(5);
    uint8_t send3[3] = {0x1c, 0, 13};
    gpio_set(BMP380_CS, false);
    spi_transfer(&spi, send3, recv, 3);
    gpio_set(BMP380_CS, true);
    vTaskDelay(5);
    uint8_t send[5] = {0x1b, 0, 51};
    gpio_set(BMP380_CS, false); 
    spi_transfer(&spi, send, recv, 3);
    gpio_set(BMP380_CS, true);
    */
    }
    while(1){
        uint8_t reg_addr[3];
        reg_addr[0] = 27 | 128;
        uint8_t hmm = 0;
        bmp3_get_regs(28, &hmm, 1, &dev);
        struct bmp3_data data;
        uint8_t sensor_comp = BMP3_PRESS | BMP3_TEMP;
        // Temperature and Pressure data are read and stored in the bmp3_data instance 
        rslt = bmp3_get_sensor_data(sensor_comp, &data, &dev);
        print("response: %d, %d, %f, %f\n", rslt, mode, data.temperature, data.pressure);

    uint8_t recv[6];
    uint8_t send[6] = {0x1b | 0x80, 0, 0, 0, 0, 0};
    gpio_set(BMP380_CS, false); 
    spi_transfer(&spi, send, recv, 6);
    gpio_set(BMP380_CS, true);
    print("got regs %d %d %d %d %d\n", recv[0], recv[1], recv[2], recv[3], recv[4], recv[5]);


    }
}

int main() {
    gpio_set(PB30, HIGH);
    gpio_direction(PB30, GPIO_DIRECTION_OUT);
    gpio_function(PB30, GPIO_FUNCTION_OFF);
    //PM->AHBMASK.reg = 0;
    //PM->AHBMASK.bit.USB = 0;

    uart_init(&uart, 2, 115200, PA12, PINMUX_PA12C_SERCOM2_PAD0, PA13, PINMUX_PA13C_SERCOM2_PAD1);
    //uart_init(&uart, 0, 115200, PA04, PINMUX_PA04D_SERCOM0_PAD0, PA05, PINMUX_PA05D_SERCOM0_PAD1);

    /*
    spi_init(&spi, 4, 2, 0,
                PB09, PINMUX_PB09D_SERCOM4_PAD1,
                PB08, PINMUX_PB08D_SERCOM4_PAD0,
                PB10, PINMUX_PB10D_SERCOM4_PAD2);
    */
    spi_init(&spi, 3, 2, 3,
                PA21, PINMUX_PA21D_SERCOM3_PAD3,
                PA16, PINMUX_PA16D_SERCOM3_PAD0,
                PA18, PINMUX_PA18D_SERCOM3_PAD2);

    //xTaskCreate(dummy2, "Doot!", 150, 0, 3, NULL);

    xTaskCreate(bmp_task, "bmp", 250, 0, 2, NULL);

    vTaskStartScheduler();

/*
    gpio_set(PB30, 0);
    vTaskDelay(1000);
    gpio_set(PB30, 1);
    vTaskDelay(1000);
    uart_write(&uart, (uint8_t*)"hello this is a longer text! woohoo! it works!!", 47);
    vTaskDelay(1000);
    for (int i=0; i<uart.tx_cur; i++) {
        gpio_set(PB30, 0);
        vTaskDelay(250);
        gpio_set(PB30, 1);
        vTaskDelay(250);
    }
    while (1) {};
    //gpio_set(PB30, HIGH);
    //while (1) {};
    uint32_t s = 1;
    while (1) {
        gpio_set(PB30, s);
        s = !s;
        vTaskDelay(500);
    }
*/
}
