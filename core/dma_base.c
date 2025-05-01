#include "dma_base.h"
#include "sammy.h"
#include "FreeRTOS.h"
#include "amslah.h"

static DmacDescriptor BaseDmacDescriptors[DMAC_CHANNELS_NUMBER] = {0};
static DmacDescriptor WriteBackDescriptors[DMAC_CHANNELS_NUMBER] = {0};

typedef enum
{
    DMAC_TRANSFER_EVENT_NONE = 0,
    DMAC_TRANSFER_EVENT_COMPLETE = 1,
    DMAC_TRANSFER_EVENT_ERROR = 2
} DMAC_TRANSFER_EVENT;

typedef void (*DMAC_CHANNEL_CALLBACK) (DMAC_TRANSFER_EVENT event);

/* DMAC channels object configuration structure */
typedef struct {
    bool inUse;
    DMAC_CHANNEL_CALLBACK callback;
    uintptr_t context;
    bool isBusy;
} DMAC_CH_OBJECT;

volatile static DMAC_CH_OBJECT dmacChannelObj[DMAC_CHANNELS_NUMBER];

// static void gps_tx_callback(DMAC_TRANSFER_EVENT event);
static void dma_register_gps_tx_channel(void);
static void init_dmac_channel_objects(void);
static void dma_interrupt_enable(void);

static void init_dmac_channel_objects(void) {
    volatile DMAC_CH_OBJECT *dmacChObj = &dmacChannelObj[0];
    uint32_t channel = 0;

    /* Initialize DMAC Channel objects */
    for(channel = 0U; channel < DMAC_CHANNELS_NUMBER; channel++)
    {
        dmacChObj->inUse = false;
        // dmacChObj->callback = gps_tx_callback;
        dmacChObj->context = 0U;
        dmacChObj->isBusy = false;

        /* Point to next channel object */
        dmacChObj++;
    }
}

void init_dma(void) {
    init_dmac_channel_objects();

    DMAC->BASEADDR.reg = (uint32_t) &BaseDmacDescriptors;
    DMAC->WRBADDR.reg = (uint32_t) &WriteBackDescriptors;
    DMAC->CTRL.reg |= DMAC_CTRL_LVLEN0;

    dma_register_gps_tx_channel();

    DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN0 | DMAC_CTRL_LVLEN1 | DMAC_CTRL_LVLEN2 | DMAC_CTRL_LVLEN3;

    dma_interrupt_enable();
}

// void dma_register_gps_rx_channel() {
//     uint8_t channel = 0;

//     DMAC->Channel[channel].CHCTRLA.bit.TRIGSRC = 0x10;  // Sercom 6, RX
// }




static void dma_register_gps_tx_channel(void) {
    uint8_t channel = DMAC_CHANNEL_GPS_TX;

    DMAC->Channel[channel].CHCTRLA.bit.TRIGACT = 0x0;
    DMAC->Channel[channel].CHCTRLA.bit.TRIGSRC = 0x11;  // Sercom 6, TX

    BaseDmacDescriptors[channel].BTCTRL.reg = (
        DMAC_BTCTRL_VALID
         | DMAC_BTCTRL_BLOCKACT_INT
         | DMAC_BTCTRL_BEATSIZE_BYTE
        //  | DMAC_BTCTRL_STEPSEL_SRC
         | DMAC_BTCTRL_SRCINC);

    DMAC->Channel[channel].CHPRILVL.reg = DMAC_CHPRILVL_PRILVL_LVL0;
    DMAC->Channel[channel].CHINTENSET.reg = DMAC_CHINTENSET_TERR | DMAC_CHINTENSET_TCMPL;
}

static void dma_interrupt_enable(void) {
    NVIC_SetPriority(DMAC_0_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(DMAC_0_IRQn);
    NVIC_SetPriority(DMAC_1_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(DMAC_1_IRQn);
}

bool dma_begin_gps_tx_transfer(const void* srcAddr, uint16_t block_size) {
    uint8_t channel = DMAC_CHANNEL_GPS_TX;
    // uint8_t beat_size = 0;
    bool isBusy = dmacChannelObj[channel].isBusy;

    if (isBusy && DMAC->Channel[channel].CHINTFLAG.reg == 0) {
        // print("DMA channel is busy\n");
        return false;
    }
    DMAC->Channel[channel].CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL | DMAC_CHINTFLAG_TERR;  // Clears interrupt flags
    dmacChannelObj[channel].isBusy = true;

    BaseDmacDescriptors[channel].SRCADDR.reg = (uint32_t) (srcAddr) + block_size;
    BaseDmacDescriptors[channel].DSTADDR.reg = (uint32_t) &(SERCOM6->USART.DATA.reg);
    BaseDmacDescriptors[channel].BTCNT.reg = block_size;

    DMAC->Channel[channel].CHCTRLA.bit.ENABLE = 1;

    return true;
}


bool DMAC_ChannelIsBusy(DmacChannel_t channel) {
    bool isBusy = dmacChannelObj[channel].isBusy;
    if (((DMAC->Channel[channel].CHINTFLAG.reg & (DMAC_CHINTENCLR_TCMPL | DMAC_CHINTENCLR_TERR)) == 0U) && (isBusy)) {
        return true;
    }
    return false;
}


void DMAC_interrupt_handler(DmacChannel_t channel) {
    DMAC_TRANSFER_EVENT event = DMAC_TRANSFER_EVENT_NONE;
    if (DMAC->Channel[channel].CHINTFLAG.reg == DMAC_CHINTFLAG_TCMPL) {
        event = DMAC_TRANSFER_EVENT_COMPLETE;
        DMAC->Channel[channel].CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL;
        dmacChannelObj[channel].isBusy = false;
        // print("DMA 1\n", channel);
    }
    if (DMAC->Channel[channel].CHINTFLAG.reg == DMAC_CHINTFLAG_TERR) {
        event = DMAC_TRANSFER_EVENT_ERROR;
        DMAC->Channel[channel].CHINTFLAG.reg = DMAC_CHINTFLAG_TERR;
        dmacChannelObj[channel].isBusy = false;
    }
    if (dmacChannelObj[channel].callback) {
        dmacChannelObj[channel].callback(event);
    }
}


void DMAC_0_Handler(void) {
    DMAC_interrupt_handler(0);
}

void DMAC_1_Handler(void) {
    DMAC_interrupt_handler(1);
}

// static void gps_tx_callback(DMAC_TRANSFER_EVENT event) {
//     print("gps tx callback\n");
//     if (event == DMAC_TRANSFER_EVENT_COMPLETE) {
//         print("gps tx complete\n");
//     }
// }
