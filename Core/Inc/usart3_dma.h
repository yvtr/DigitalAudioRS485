#ifndef  __USART3_IT_H__
#define  __USART3_IT_H__

#include <stdint.h>
#include "usart3_dma_cfg.h"


extern uint8_t  Usart3RxDmaBuf[USART3_RXDMA_BUF_SIZE];

extern void     Usart3_DMA_Init(void (*)(const uint8_t* data, uint16_t len));
extern void     Usart3_DMA_Task();
extern uint8_t  Usart3_TxBufWrite(const void* src, size_t n, uint8_t flush);


#endif

