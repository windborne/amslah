#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#ifndef DMA_RX_HALF_SIZE
#define DMA_RX_HALF_SIZE 128
#endif

#define DMA_RX_BUF_SIZE (DMA_RX_HALF_SIZE * 2)

#ifndef DMA_RX_IRQ_PRIORITY
#define DMA_RX_IRQ_PRIORITY 4
#endif

void dma_rx_init(uint8_t channel, uint8_t sercom_num);

uint16_t dma_rx_count(uint8_t channel);

uint16_t dma_rx_read(uint8_t channel, uint8_t *dst, uint16_t max_len);

int16_t dma_rx_read_byte(uint8_t channel);

void dma_rx_stop(uint8_t channel);

void dma_rx_restart(uint8_t channel);

uint32_t dma_rx_overflows(uint8_t channel);
uint32_t dma_rx_errors(uint8_t channel);
uint32_t dma_rx_total_bytes(uint8_t channel);

#ifdef __cplusplus
}
#endif
