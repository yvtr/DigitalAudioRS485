#ifndef  __USART3_IT_H__
#define  __USART3_IT_H__

#include <stdint.h>
#include "usart3_dma_cfg.h"


extern uint8_t  Usart3RxDmaBuf[USART3_RXDMA_BUF_SIZE];

extern void     Usart3_DMA_Init(void (*)(const uint8_t* data, uint16_t len));
extern void     Usart3_DMA_Task();
extern void     Usart3_PutByte(uint8_t d);
extern void     Usart3_PutData(const void* src, uint16_t n);
extern int16_t  Usart3_GetByte(void);


#endif

