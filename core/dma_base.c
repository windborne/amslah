#include "dma_base.h"
#include "sammy.h"


#define N_DMA_CHANNELS 2

static DmacDescriptor BaseDmacDescriptors[N_DMA_CHANNELS] = {0};
static DmacDescriptor WriteBackDescriptors[N_DMA_CHANNELS] = {0};

void dma_initialize(void) {
    DMAC->BASEADDR.reg = (uint32_t) &BaseDmacDescriptors;
    DMAC->WRBADDR.reg = (uint32_t) &WriteBackDescriptors;
}




