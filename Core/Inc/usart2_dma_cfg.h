#ifndef __USART2_IT_CFG_H__
#define __USART2_IT_CFG_H__

#include "stm32h5xx_ll_dma.h"


#define USART2_RXDMA_BUF_SIZE    2048
#define USART2_TXDMA_BUF_SIZE    2048

#define USART2_GPDMA             GPDMA1
#define USART2_DMA_TX_CHANNEL    LL_DMA_CHANNEL_2
#define USART2_DMA_RX_CHANNEL    LL_DMA_CHANNEL_3


#endif // __USART2_CFG_H__
