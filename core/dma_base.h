#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>
#include <stdint.h>

void init_dma(void);
bool dma_begin_gps_tx_transfer(const void* srcAddr, uint16_t block_size);
bool dma_begin_gps_rx_transfer(const void* dstAddr, uint16_t block_size);

typedef enum {
    DMAC_CHANNEL_GPS_RX = 0,
    DMAC_CHANNEL_GPS_TX = 1,
    DMAC_CHANNELS_NUMBER = 2,
} DmacChannel_t;

bool DMAC_ChannelIsBusy(DmacChannel_t channel);

#ifdef __cplusplus
}
#endif
