#include "dma_uart_rx.h"
#include "sammy.h"

#ifdef _SAMD51_

#include <string.h>

// SAMD51 DMAC requires two globally-aligned descriptor arrays:
//   base[ch]  — loaded by hardware when channel ch is enabled
//   wb[ch]    — hardware writes back transfer state on block completion
// Channels 0-3 each have a dedicated IRQ (DMAC_0_IRQn .. DMAC_3_IRQn).

#define MAX_CH 4

static DmacDescriptor _base[MAX_CH] __attribute__((aligned(16)));
static DmacDescriptor _wb[MAX_CH]   __attribute__((aligned(16)));

typedef struct {
    DmacDescriptor link __attribute__((aligned(16)));
    uint8_t buf[DMA_RX_BUF_SIZE];
    volatile uint32_t written;
    uint32_t          read;
    uint8_t  channel;
    uint8_t  sercom_num;
    volatile uint32_t overflows;
    volatile uint32_t errors;
    volatile uint32_t blocks;
    bool     active;
} rx_state_t;

static rx_state_t _st[MAX_CH];
static bool _hw_ready;

static Sercom *sercom_ptr(uint8_t n) {
    switch (n) {
    case 0: return SERCOM0;
    case 1: return SERCOM1;
    case 2: return SERCOM2;
    case 3: return SERCOM3;
    case 4: return SERCOM4;
    case 5: return SERCOM5;
#ifdef SERCOM6
    case 6: return SERCOM6;
    case 7: return SERCOM7;
#endif
    default: return NULL;
    }
}

static void hw_init(void) {
    if (_hw_ready) return;

    MCLK->AHBMASK.reg |= MCLK_AHBMASK_DMAC;

    DMAC->CTRL.reg &= ~DMAC_CTRL_DMAENABLE;
    DMAC->CTRL.reg = DMAC_CTRL_SWRST;
    while (DMAC->CTRL.reg & DMAC_CTRL_SWRST) {}

    DMAC->BASEADDR.reg = (uint32_t)_base;
    DMAC->WRBADDR.reg  = (uint32_t)_wb;

    DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE
                   | DMAC_CTRL_LVLEN0 | DMAC_CTRL_LVLEN1
                   | DMAC_CTRL_LVLEN2 | DMAC_CTRL_LVLEN3;

    _hw_ready = true;
}

static void setup_descriptors(rx_state_t *s) {
    uint8_t ch = s->channel;
    Sercom *hw = sercom_ptr(s->sercom_num);
    volatile void *data = &hw->USART.DATA.reg;
    uint16_t half = DMA_RX_HALF_SIZE;

    // Descriptor A: fills buf[0 .. half-1], chains to link (descriptor B)
    // DSTADDR points to the END of the destination region (SAMD51 DSTINC quirk)
    _base[ch].BTCTRL.reg = DMAC_BTCTRL_VALID
                         | DMAC_BTCTRL_BLOCKACT_INT
                         | DMAC_BTCTRL_BEATSIZE_BYTE
                         | DMAC_BTCTRL_DSTINC;
    _base[ch].BTCNT.reg    = half;
    _base[ch].SRCADDR.reg  = (uint32_t)data;
    _base[ch].DSTADDR.reg  = (uint32_t)(s->buf + half);
    _base[ch].DESCADDR.reg = (uint32_t)&s->link;

    // Descriptor B: fills buf[half .. 2*half-1], chains back to A
    s->link.BTCTRL.reg = DMAC_BTCTRL_VALID
                       | DMAC_BTCTRL_BLOCKACT_INT
                       | DMAC_BTCTRL_BEATSIZE_BYTE
                       | DMAC_BTCTRL_DSTINC;
    s->link.BTCNT.reg    = half;
    s->link.SRCADDR.reg  = (uint32_t)data;
    s->link.DSTADDR.reg  = (uint32_t)(s->buf + 2 * half);
    s->link.DESCADDR.reg = (uint32_t)&_base[ch];
}

static void enable_channel(uint8_t ch, uint8_t sercom_num) {
    // SERCOM n RX trigger = 4 + 2*n (SAMD51 Table 22-2)
    DMAC->Channel[ch].CHCTRLA.bit.ENABLE = 0;
    while (DMAC->Channel[ch].CHCTRLA.reg & DMAC_CHCTRLA_ENABLE) {}

    DMAC->Channel[ch].CHCTRLA.bit.TRIGACT = 0x2; // burst (one beat per trigger)
    DMAC->Channel[ch].CHCTRLA.bit.TRIGSRC = 4 + 2 * sercom_num;
    DMAC->Channel[ch].CHPRILVL.reg = DMAC_CHPRILVL_PRILVL_LVL0;

    DMAC->Channel[ch].CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL
                                    | DMAC_CHINTFLAG_TERR
                                    | DMAC_CHINTFLAG_SUSP;
    DMAC->Channel[ch].CHINTENSET.reg = DMAC_CHINTENSET_TCMPL
                                     | DMAC_CHINTENSET_TERR;

    IRQn_Type irqn = (IRQn_Type)(DMAC_0_IRQn + ch);
    NVIC_SetPriority(irqn, DMA_RX_IRQ_PRIORITY);
    NVIC_EnableIRQ(irqn);

    DMAC->Channel[ch].CHCTRLA.bit.ENABLE = 1;
}

// ---- Public API ----

void dma_rx_init(uint8_t channel, uint8_t sercom_num) {
    if (channel >= MAX_CH) return;
    Sercom *hw = sercom_ptr(sercom_num);
    if (!hw) return;

    hw_init();

    rx_state_t *s = &_st[channel];
    memset(s, 0, sizeof(*s));
    s->channel    = channel;
    s->sercom_num = sercom_num;
    s->active     = true;

    hw->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_RXC;

    setup_descriptors(s);
    enable_channel(channel, sercom_num);
}

uint16_t dma_rx_count(uint8_t channel) {
    if (channel >= MAX_CH || !_st[channel].active) return 0;
    rx_state_t *s = &_st[channel];

    uint32_t avail = s->written - s->read;
    if (avail > DMA_RX_BUF_SIZE) {
        s->read = s->written - DMA_RX_BUF_SIZE;
        avail = DMA_RX_BUF_SIZE;
    }
    return (uint16_t)avail;
}

uint16_t dma_rx_read(uint8_t channel, uint8_t *dst, uint16_t max_len) {
    uint16_t avail = dma_rx_count(channel);
    if (avail == 0 || max_len == 0) return 0;

    rx_state_t *s = &_st[channel];
    uint16_t n   = (max_len < avail) ? max_len : avail;
    uint16_t pos = s->read & (DMA_RX_BUF_SIZE - 1);
    uint16_t first = DMA_RX_BUF_SIZE - pos;

    if (first >= n) {
        memcpy(dst, &s->buf[pos], n);
    } else {
        memcpy(dst, &s->buf[pos], first);
        memcpy(dst + first, s->buf, n - first);
    }

    s->read += n;
    return n;
}

int16_t dma_rx_read_byte(uint8_t channel) {
    if (dma_rx_count(channel) == 0) return -1;
    rx_state_t *s = &_st[channel];
    uint8_t b = s->buf[s->read & (DMA_RX_BUF_SIZE - 1)];
    s->read++;
    return b;
}

void dma_rx_stop(uint8_t channel) {
    if (channel >= MAX_CH) return;
    DMAC->Channel[channel].CHCTRLA.bit.ENABLE = 0;
    while (DMAC->Channel[channel].CHCTRLA.reg & DMAC_CHCTRLA_ENABLE) {}
    _st[channel].active = false;
}

void dma_rx_restart(uint8_t channel) {
    if (channel >= MAX_CH) return;
    rx_state_t *s = &_st[channel];

    Sercom *hw = sercom_ptr(s->sercom_num);
    if (!hw) return;

    DMAC->Channel[channel].CHCTRLA.bit.ENABLE = 0;
    while (DMAC->Channel[channel].CHCTRLA.reg & DMAC_CHCTRLA_ENABLE) {}

    hw->USART.INTENCLR.reg = SERCOM_USART_INTENCLR_RXC;

    s->written = 0;
    s->read    = 0;

    setup_descriptors(s);
    enable_channel(channel, s->sercom_num);
    s->active = true;
}

uint32_t dma_rx_overflows(uint8_t channel) {
    return (channel < MAX_CH) ? _st[channel].overflows : 0;
}

uint32_t dma_rx_errors(uint8_t channel) {
    return (channel < MAX_CH) ? _st[channel].errors : 0;
}

uint32_t dma_rx_total_bytes(uint8_t channel) {
    return (channel < MAX_CH) ? _st[channel].written : 0;
}

uint32_t dma_rx_blocks(uint8_t channel) {
    return (channel < MAX_CH) ? _st[channel].blocks : 0;
}

// ---- ISR ----

static void handle_irq(uint8_t ch) {
    uint8_t flags = DMAC->Channel[ch].CHINTFLAG.reg;

    if (flags & DMAC_CHINTFLAG_TERR) {
        DMAC->Channel[ch].CHINTFLAG.reg = DMAC_CHINTFLAG_TERR;
        rx_state_t *s = &_st[ch];
        if (!s->active) return;
        s->errors++;
        DMAC->Channel[ch].CHCTRLA.bit.ENABLE = 0;
        while (DMAC->Channel[ch].CHCTRLA.reg & DMAC_CHCTRLA_ENABLE) {}
        DMAC->Channel[ch].CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL
                                        | DMAC_CHINTFLAG_TERR
                                        | DMAC_CHINTFLAG_SUSP;
        setup_descriptors(s);
        DMAC->Channel[ch].CHCTRLA.bit.ENABLE = 1;
        return;
    }

    if (flags & DMAC_CHINTFLAG_TCMPL) {
        DMAC->Channel[ch].CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL;
        rx_state_t *s = &_st[ch];
        if (!s->active) return;

        s->written += DMA_RX_HALF_SIZE;
        s->blocks++;

        if ((s->written - s->read) > DMA_RX_BUF_SIZE) {
            s->overflows++;
        }
    }
}

void DMAC_0_Handler(void) { handle_irq(0); }
void DMAC_1_Handler(void) { handle_irq(1); }
void DMAC_2_Handler(void) { handle_irq(2); }
void DMAC_3_Handler(void) { handle_irq(3); }

#endif // _SAMD51_
