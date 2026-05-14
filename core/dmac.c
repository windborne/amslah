#include "dmac.h"
#include <stddef.h>
#include <string.h>

#if __has_include("dma_config.h")
  #include "dma_config.h"
#endif

#include "sammy.h"
#include "amslah_config.h"

#if (DMAC_ENABLED && (DMAC_CHANNEL_COUNT > 0))
    static DmacDescriptor BaseDmacDescriptors[N_DMAC_CHANNELS] __attribute__((aligned(16))) = {0};
    static DmacDescriptor WriteBackDescriptors[N_DMAC_CHANNELS] __attribute__((aligned(16))) = {0};
    extern volatile dmac_channel_cfg dmac_cfgs[];
#else
    static DmacDescriptor *BaseDmacDescriptors = NULL;
    static DmacDescriptor *WriteBackDescriptors = NULL;
    static dmac_channel_cfg *dmac_cfgs = NULL;
#endif

#if DMAC_ENABLED
static dma_uart_rx_t *dma_rx_states[N_DMAC_CHANNELS] = {0};
#endif

static void dma_register_channel(DmacChannel_t channel);

void init_dma(void) {
    if (!DMAC_ENABLED) return;

    #ifdef _SAMD51_
    MCLK->AHBMASK.reg |= MCLK_AHBMASK_DMAC;

    DMAC->CTRL.reg &= ~DMAC_CTRL_DMAENABLE;
    DMAC->CTRL.reg = DMAC_CTRL_SWRST;
    while (DMAC->CTRL.reg & DMAC_CTRL_SWRST);

    DMAC->BASEADDR.reg = (uint32_t)BaseDmacDescriptors;
    DMAC->WRBADDR.reg = (uint32_t)WriteBackDescriptors;

    for (uint8_t ch = 0; ch < N_DMAC_CHANNELS; ch++) {
        if (dmac_cfgs[ch].trigSrc != 0) {
            dma_register_channel(ch);
        }
    }

    DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE
                   | DMAC_CTRL_LVLEN0 | DMAC_CTRL_LVLEN1
                   | DMAC_CTRL_LVLEN2 | DMAC_CTRL_LVLEN3;
    #endif
}

static void dma_register_channel(DmacChannel_t channel) {
    #ifdef _SAMD51_
    DMAC->Channel[channel].CHCTRLA.bit.TRIGACT = 0x2; // burst
    DMAC->Channel[channel].CHCTRLA.bit.TRIGSRC = dmac_cfgs[channel].trigSrc;

    BaseDmacDescriptors[channel].BTCTRL.reg = (
        DMAC_BTCTRL_VALID
        | DMAC_BTCTRL_BLOCKACT_INT
        | DMAC_BTCTRL_BEATSIZE_BYTE
        | (dmac_cfgs[channel].direction_is_tx ? DMAC_BTCTRL_SRCINC : DMAC_BTCTRL_DSTINC));

    DMAC->Channel[channel].CHPRILVL.reg = DMAC_CHPRILVL_PRILVL_LVL0;
    DMAC->Channel[channel].CHINTENSET.reg = DMAC_CHINTENSET_TERR | DMAC_CHINTENSET_TCMPL;

    uint8_t irqn;
    if (channel < 4) {
        irqn = DMAC_0_IRQn + channel;
    } else {
        irqn = DMAC_4_IRQn;
    }
    NVIC_SetPriority(irqn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(irqn);
    #endif
}


bool dma_uart_transfer(DmacChannel_t channel, const void *bufAddr, uint16_t block_size) {
    #ifdef _SAMD51_
    if (dmac_cfgs[channel].isBusy && DMAC->Channel[channel].CHINTFLAG.reg == 0) {
        return false;
    }
    DMAC->Channel[channel].CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL | DMAC_CHINTFLAG_TERR;
    dmac_cfgs[channel].isBusy = true;

    if (dmac_cfgs[channel].direction_is_tx) {
        BaseDmacDescriptors[channel].SRCADDR.reg = (uint32_t)(bufAddr) + block_size;
        BaseDmacDescriptors[channel].DSTADDR.reg = (uint32_t)dmac_cfgs[channel].uart_data_addr;
    } else {
        BaseDmacDescriptors[channel].SRCADDR.reg = (uint32_t)dmac_cfgs[channel].uart_data_addr;
        BaseDmacDescriptors[channel].DSTADDR.reg = (uint32_t)(bufAddr) + block_size;
    }
    BaseDmacDescriptors[channel].BTCNT.reg = block_size;
    BaseDmacDescriptors[channel].DESCADDR.reg = 0;

    DMAC->Channel[channel].CHCTRLA.bit.ENABLE = 1;
    #endif
    return true;
}


bool DMAC_ChannelIsBusy(DmacChannel_t channel) {
    #ifdef _SAMD51_
    bool isBusy = dmac_cfgs[channel].isBusy;
    if (((DMAC->Channel[channel].CHINTFLAG.reg & (DMAC_CHINTENCLR_TCMPL | DMAC_CHINTENCLR_TERR)) == 0U) && isBusy) {
        return true;
    }
    #endif
    return false;
}


void DMAC_ChannelDisable(DmacChannel_t channel) {
    #ifdef _SAMD51_
    DMAC->Channel[channel].CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
    while (DMAC->Channel[channel].CHCTRLA.reg & DMAC_CHCTRLA_ENABLE);
    #endif
    dmac_cfgs[channel].isBusy = false;
}


// ---- DMA UART RX circular buffer ----

#if DMAC_ENABLED

#include "FreeRTOS.h"
#include "stream_buffer.h"

void dma_uart_rx_init(dma_uart_rx_t *rx, DmacChannel_t channel,
                      uint8_t sercom_num, StreamBufferHandle_t rx_stream) {
    #ifdef _SAMD51_
    rx->channel = channel;
    rx->sercom_num = sercom_num;
    rx->rx_stream = rx_stream;
    rx->next_half = 0;
    rx->overflow_count = 0;
    rx->error_count = 0;
    dma_rx_states[channel] = rx;

    Sercom *hw = NULL;
    switch (sercom_num) {
    case 0: hw = SERCOM0; break;
    case 1: hw = SERCOM1; break;
    case 2: hw = SERCOM2; break;
    case 3: hw = SERCOM3; break;
    case 4: hw = SERCOM4; break;
    case 5: hw = SERCOM5; break;
    #ifdef SERCOM6
    case 6: hw = SERCOM6; break;
    case 7: hw = SERCOM7; break;
    #endif
    }
    if (!hw) return;

    // Disable per-byte RXC interrupt — DMA handles it now
    hw->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_RXC;

    volatile void *data_reg = &hw->USART.DATA.reg;
    uint16_t half = DMAC_RX_HALF_SIZE;

    // Descriptor A (lives in BaseDmacDescriptors[channel]):
    //   Fills buffer[0 .. half-1], then chains to desc_b
    BaseDmacDescriptors[channel].BTCTRL.reg =
        DMAC_BTCTRL_VALID
        | DMAC_BTCTRL_BLOCKACT_INT
        | DMAC_BTCTRL_BEATSIZE_BYTE
        | DMAC_BTCTRL_DSTINC;
    BaseDmacDescriptors[channel].BTCNT.reg = half;
    BaseDmacDescriptors[channel].SRCADDR.reg = (uint32_t)data_reg;
    // DSTADDR = end of first half (SAMD51 DSTINC addresses the end)
    BaseDmacDescriptors[channel].DSTADDR.reg = (uint32_t)(rx->buffer + half);
    BaseDmacDescriptors[channel].DESCADDR.reg = (uint32_t)&rx->desc_b;

    // Descriptor B (secondary, in rx->desc_b):
    //   Fills buffer[half .. 2*half-1], then chains back to BaseDmacDescriptors[channel]
    rx->desc_b.BTCTRL.reg =
        DMAC_BTCTRL_VALID
        | DMAC_BTCTRL_BLOCKACT_INT
        | DMAC_BTCTRL_BEATSIZE_BYTE
        | DMAC_BTCTRL_DSTINC;
    rx->desc_b.BTCNT.reg = half;
    rx->desc_b.SRCADDR.reg = (uint32_t)data_reg;
    rx->desc_b.DSTADDR.reg = (uint32_t)(rx->buffer + 2 * half);
    rx->desc_b.DESCADDR.reg = (uint32_t)&BaseDmacDescriptors[channel];

    // Configure the DMAC channel
    DMAC->Channel[channel].CHCTRLA.bit.ENABLE = 0;
    while (DMAC->Channel[channel].CHCTRLA.reg & DMAC_CHCTRLA_ENABLE);

    uint8_t rx_trig = 4 + 2 * sercom_num; // SERCOMn_DMAC_ID_RX = 4 + 2*n
    DMAC->Channel[channel].CHCTRLA.bit.TRIGACT = 0x2; // burst
    DMAC->Channel[channel].CHCTRLA.bit.TRIGSRC = rx_trig;
    DMAC->Channel[channel].CHPRILVL.reg = DMAC_CHPRILVL_PRILVL_LVL0;
    DMAC->Channel[channel].CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL | DMAC_CHINTFLAG_TERR | DMAC_CHINTFLAG_SUSP;
    DMAC->Channel[channel].CHINTENSET.reg = DMAC_CHINTENSET_TCMPL | DMAC_CHINTENSET_TERR;

    uint8_t irqn = (channel < 4) ? (DMAC_0_IRQn + channel) : DMAC_4_IRQn;
    NVIC_SetPriority(irqn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(irqn);

    // Go
    DMAC->Channel[channel].CHCTRLA.bit.ENABLE = 1;
    #endif
}

void dma_uart_rx_stop(dma_uart_rx_t *rx) {
    #ifdef _SAMD51_
    DMAC_ChannelDisable(rx->channel);
    dma_rx_states[rx->channel] = NULL;
    #endif
}

static void dma_rx_handle_tcmpl(DmacChannel_t channel) {
    dma_uart_rx_t *rx = dma_rx_states[channel];
    if (!rx) return;

    uint16_t half = DMAC_RX_HALF_SIZE;
    uint8_t *src = rx->buffer + (rx->next_half * half);

    BaseType_t woke = pdFALSE;
    size_t sent = xStreamBufferSendFromISR(rx->rx_stream, src, half, &woke);
    if (sent < half) {
        rx->overflow_count++;
    }

    rx->next_half ^= 1;
    portYIELD_FROM_ISR(woke);
}

static void dma_rx_handle_error(DmacChannel_t channel) {
    dma_uart_rx_t *rx = dma_rx_states[channel];
    if (!rx) return;

    rx->error_count++;
    rx->next_half = 0;

    DMAC->Channel[channel].CHCTRLA.bit.ENABLE = 0;
    while (DMAC->Channel[channel].CHCTRLA.reg & DMAC_CHCTRLA_ENABLE);

    DMAC->Channel[channel].CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL | DMAC_CHINTFLAG_TERR | DMAC_CHINTFLAG_SUSP;
    DMAC->Channel[channel].CHCTRLA.bit.ENABLE = 1;
}

#endif // DMAC_ENABLED


// ---- DMAC interrupt handlers ----

#ifdef _SAMD51_

static void DMAC_interrupt_handler(DmacChannel_t channel) {
    uint8_t flags = DMAC->Channel[channel].CHINTFLAG.reg;

    #if DMAC_ENABLED
    if (flags & DMAC_CHINTFLAG_TERR) {
        DMAC->Channel[channel].CHINTFLAG.reg = DMAC_CHINTFLAG_TERR;
        if (dma_rx_states[channel]) {
            dma_rx_handle_error(channel);
            return;
        }
        if (dmac_cfgs) dmac_cfgs[channel].isBusy = false;
    }

    if (flags & DMAC_CHINTFLAG_TCMPL) {
        DMAC->Channel[channel].CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL;
        if (dma_rx_states[channel]) {
            dma_rx_handle_tcmpl(channel);
            return;
        }
        if (dmac_cfgs) {
            dmac_cfgs[channel].isBusy = false;
            if (dmac_cfgs[channel].callback) {
                dmac_cfgs[channel].callback(DMAC_RESULT_COMPLETE);
            }
        }
    }
    #else
    if (flags & DMAC_CHINTFLAG_TERR)
        DMAC->Channel[channel].CHINTFLAG.reg = DMAC_CHINTFLAG_TERR;
    if (flags & DMAC_CHINTFLAG_TCMPL)
        DMAC->Channel[channel].CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL;
    #endif
}

void DMAC_0_Handler(void) { DMAC_interrupt_handler(0); }
void DMAC_1_Handler(void) { DMAC_interrupt_handler(1); }
void DMAC_2_Handler(void) { DMAC_interrupt_handler(2); }
void DMAC_3_Handler(void) { DMAC_interrupt_handler(3); }
void DMAC_4_Handler(void) {
    // Channels 4+ share this IRQ — find which channel triggered
    for (uint8_t ch = 4; ch < N_DMAC_CHANNELS; ch++) {
        if (DMAC->Channel[ch].CHINTFLAG.reg & (DMAC_CHINTFLAG_TCMPL | DMAC_CHINTFLAG_TERR)) {
            DMAC_interrupt_handler(ch);
        }
    }
}

#endif // _SAMD51_
