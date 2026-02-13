#ifndef  __UART5_IT_H__
#define  __UART5_IT_H__

#include <stdint.h>

extern void     Uart5_Init(void);
extern void     Uart5_PutByte(uint8_t d);
extern void     Uart5_PutData(const void* src, uint16_t n);
extern int16_t  Uart5_GetByte(void);


#endif

