#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include <stdbool.h>
#include <stdint.h>

#include "dma_config.h"  // Provides DmacChannel_t

typedef enum
{
    DMAC_RESULT_NONE = 0,
    DMAC_RESULT_COMPLETE = 1,
    DMAC_RESULT_ERROR = 2
} DMAC_RESULT;

typedef void (*DMAC_CHANNEL_CALLBACK) (DMAC_RESULT result);

typedef struct {
    bool isBusy;
    DMAC_CHANNEL_CALLBACK callback;
    uintptr_t callback_context;
    uint8_t trigSrc;
    bool direction_is_tx;
    volatile void* uart_data_addr;
} dmac_channel_cfg;

void init_dma(void);
bool dma_uart_transfer(DmacChannel_t channel, const void* bufAddr, uint16_t block_size);

void DMAC_ChannelCallbackRegister (DmacChannel_t channel, const DMAC_CHANNEL_CALLBACK callback, const uintptr_t context);
bool DMAC_ChannelIsBusy(DmacChannel_t channel);
void DMAC_ChannelSuspend(DmacChannel_t channel);
void DMAC_ChannelResume(DmacChannel_t channel);
void DMAC_ChannelDisable(DmacChannel_t channel);



#ifdef __cplusplus
}
#endif
