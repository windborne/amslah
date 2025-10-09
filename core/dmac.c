#include "dmac.h"
#include <stddef.h>

#if __has_include("dma_config.h")
  #include "dma_config.h"
#endif

#include "sammy.h"
#include "amslah_config.h"


#if DMAC_ENABLED && (N_DMAC_CHANNELS > 0)
    static DmacDescriptor BaseDmacDescriptors[N_DMAC_CHANNELS] = {0};
    static DmacDescriptor WriteBackDescriptors[N_DMAC_CHANNELS] = {0};
    extern volatile dmac_channel_cfg dmac_cfgs[];
#else
    static DmacDescriptor* BaseDmacDescriptors = NULL;
    static DmacDescriptor* WriteBackDescriptors = NULL;
    static dmac_channel_cfg* dmac_cfgs = NULL;
#endif

static void dma_register_channel(DmacChannel_t channel);

static void dma_interrupt_enable(void);

void init_dma(void) {
    if (!DMAC_ENABLED) {
        return;
    }
    DMAC->BASEADDR.reg = (uint32_t) &BaseDmacDescriptors;
    DMAC->WRBADDR.reg = (uint32_t) &WriteBackDescriptors;
    DMAC->CTRL.reg |= DMAC_CTRL_LVLEN0;

    for (uint8_t channel = 0; channel < N_DMAC_CHANNELS; channel++) {
        dma_register_channel(channel);
    }

    DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN0 | DMAC_CTRL_LVLEN1 | DMAC_CTRL_LVLEN2 | DMAC_CTRL_LVLEN3;

    dma_interrupt_enable();
}

void DMAC_ChannelCallbackRegister( DmacChannel_t channel, const DMAC_CHANNEL_CALLBACK callback, const uintptr_t context )
{
    dmac_cfgs[channel].callback = callback;
    dmac_cfgs[channel].callback_context = context;
}


static void dma_register_channel(DmacChannel_t channel) {
    bool direction_is_tx = dmac_cfgs[channel].direction_is_tx;
    uint8_t trigSrc = dmac_cfgs[channel].trigSrc;
    
    DMAC->Channel[channel].CHCTRLA.bit.TRIGACT = 0x2;
    DMAC->Channel[channel].CHCTRLA.bit.TRIGSRC = trigSrc;

    BaseDmacDescriptors[channel].BTCTRL.reg = (
        DMAC_BTCTRL_VALID
         | DMAC_BTCTRL_BLOCKACT_INT
         | DMAC_BTCTRL_BEATSIZE_BYTE
         | (direction_is_tx ? DMAC_BTCTRL_SRCINC : DMAC_BTCTRL_DSTINC));

    DMAC->Channel[channel].CHPRILVL.reg = DMAC_CHPRILVL_PRILVL_LVL0;
    DMAC->Channel[channel].CHINTENSET.reg = DMAC_CHINTENSET_TERR | DMAC_CHINTENSET_TCMPL;
}


static void dma_interrupt_enable(void) {
    NVIC_SetPriority(DMAC_0_IRQn, 2);
    NVIC_EnableIRQ(DMAC_0_IRQn);
    NVIC_SetPriority(DMAC_1_IRQn, 2);
    NVIC_EnableIRQ(DMAC_1_IRQn);
}

bool dma_uart_transfer(DmacChannel_t channel, const void* bufAddr, uint16_t block_size) {
    bool isBusy = dmac_cfgs[channel].isBusy;

    if (isBusy && DMAC->Channel[channel].CHINTFLAG.reg == 0) {
        return false;
    }
    DMAC->Channel[channel].CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL | DMAC_CHINTFLAG_TERR;  // Clears interrupt flags
    dmac_cfgs[channel].isBusy = true;

    if (dmac_cfgs[channel].direction_is_tx) {
        BaseDmacDescriptors[channel].SRCADDR.reg = (uint32_t) (bufAddr) + block_size;
        BaseDmacDescriptors[channel].DSTADDR.reg = (uint32_t) dmac_cfgs[channel].uart_data_addr;
    } else {
        BaseDmacDescriptors[channel].SRCADDR.reg = (uint32_t) dmac_cfgs[channel].uart_data_addr;
        BaseDmacDescriptors[channel].DSTADDR.reg = (uint32_t) (bufAddr) + block_size;
    }
    BaseDmacDescriptors[channel].BTCNT.reg = block_size;

    DMAC->Channel[channel].CHCTRLA.bit.ENABLE = 1;

    return true;
}


bool DMAC_ChannelIsBusy(DmacChannel_t channel) {
    bool isBusy = dmac_cfgs[channel].isBusy;
    if (((DMAC->Channel[channel].CHINTFLAG.reg & (DMAC_CHINTENCLR_TCMPL | DMAC_CHINTENCLR_TERR)) == 0U) && (isBusy)) {
        return true;
    }
    return false;
}


void DMAC_ChannelSuspend(DmacChannel_t channel) {
    DMAC->Channel[channel].CHCTRLB.reg = (DMAC->Channel[channel].CHCTRLB.reg & (uint8_t)(~DMAC_CHCTRLB_CMD_Msk)) | DMAC_CHCTRLB_CMD_SUSPEND;
}

void DMAC_ChannelResume (DmacChannel_t channel) {
    DMAC->Channel[channel].CHCTRLB.reg = (DMAC->Channel[channel].CHCTRLB.reg & (uint8_t)(~DMAC_CHCTRLB_CMD_Msk)) | DMAC_CHCTRLB_CMD_RESUME;
}

void DMAC_ChannelDisable (DmacChannel_t channel) {
    /* Disable the DMA channel */
    DMAC->Channel[channel].CHCTRLA.reg &= (~DMAC_CHCTRLA_ENABLE);

    while((DMAC->Channel[channel].CHCTRLA.reg & DMAC_CHCTRLA_ENABLE) != 0U) {
        /* Wait for channel to be disabled */
    }

    dmac_cfgs[channel].isBusy = false;
}

void DMAC_interrupt_handler(DmacChannel_t channel) {
    DMAC_RESULT result = DMAC_RESULT_NONE;
    if (DMAC->Channel[channel].CHINTFLAG.reg == DMAC_CHINTFLAG_TCMPL) {
        result = DMAC_RESULT_COMPLETE;
        DMAC->Channel[channel].CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL;
        dmac_cfgs[channel].isBusy = false;
    }
    if (DMAC->Channel[channel].CHINTFLAG.reg == DMAC_CHINTFLAG_TERR) {
        result = DMAC_RESULT_ERROR;
        DMAC->Channel[channel].CHINTFLAG.reg = DMAC_CHINTFLAG_TERR;
        dmac_cfgs[channel].isBusy = false;
    }
    if (dmac_cfgs[channel].callback) {
        dmac_cfgs[channel].callback(result);
    }
}


void DMAC_0_Handler(void) {
    DMAC_interrupt_handler(0);
}

void DMAC_1_Handler(void) {
    DMAC_interrupt_handler(1);
}
