#ifndef  __USART2_IT_H__
#define  __USART2_IT_H__

#include <stdint.h>
#include "usart2_dma_cfg.h"


extern uint8_t  Usart2RxDmaBuf[USART2_RXDMA_BUF_SIZE];

extern void     Usart2_DMA_Init(void (*)(const uint8_t* data, uint16_t len));
extern void     Usart2_DMA_Task();
extern uint8_t  Usart2_TxBufWrite(const void* src, size_t n, uint8_t flush);


#endif

