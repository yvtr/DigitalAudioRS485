#ifndef __USART3_IT_CFG_H__
#define __USART3_IT_CFG_H__

#include "stm32h5xx_ll_dma.h"


#define USART3_RXDMA_BUF_SIZE    2048
#define USART3_TXDMA_BUF_SIZE    2048

#define USART3_GPDMA             GPDMA1
#define USART3_DMA_TX_CHANNEL    LL_DMA_CHANNEL_0
#define USART3_DMA_RX_CHANNEL    LL_DMA_CHANNEL_1


#endif // __USART3_CFG_H__
