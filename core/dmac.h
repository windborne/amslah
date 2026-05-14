#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "sammy.h"
#include "amslah_config.h"

#if __has_include("dma_config.h")
  #include "dma_config.h"
#else
    #define DMAC_CHANNEL_COUNT (0)
    typedef enum {
        N_DMAC_CHANNELS,
    } DmacChannel_t;
#endif

typedef enum {
    DMAC_RESULT_NONE = 0,
    DMAC_RESULT_COMPLETE = 1,
    DMAC_RESULT_ERROR = 2
} DMAC_RESULT;

typedef void (*DMAC_CHANNEL_CALLBACK)(DMAC_RESULT result);

typedef struct {
    bool isBusy;
    DMAC_CHANNEL_CALLBACK callback;
    uintptr_t callback_context;
    uint8_t trigSrc;
    bool direction_is_tx;
    volatile void *uart_data_addr;
} dmac_channel_cfg;

void init_dma(void);
bool dma_uart_transfer(DmacChannel_t channel, const void *bufAddr, uint16_t block_size);
bool DMAC_ChannelIsBusy(DmacChannel_t channel);
void DMAC_ChannelDisable(DmacChannel_t channel);

#if DMAC_ENABLED

#include "FreeRTOS.h"
#include "stream_buffer.h"

#ifndef DMAC_RX_HALF_SIZE
#define DMAC_RX_HALF_SIZE 128
#endif

typedef struct __attribute__((aligned(16))) {
    DmacDescriptor desc_b;
    uint8_t buffer[DMAC_RX_HALF_SIZE * 2];
    StreamBufferHandle_t rx_stream;
    DmacChannel_t channel;
    uint8_t sercom_num;
    volatile uint8_t next_half;
    volatile uint32_t overflow_count;
    volatile uint32_t error_count;
} dma_uart_rx_t;

void dma_uart_rx_init(dma_uart_rx_t *rx, DmacChannel_t channel,
                      uint8_t sercom_num, StreamBufferHandle_t rx_stream);
void dma_uart_rx_stop(dma_uart_rx_t *rx);

#endif

#ifdef __cplusplus
}
#endif
