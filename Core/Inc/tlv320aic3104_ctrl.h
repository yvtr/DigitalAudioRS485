#ifndef __TLV320AIC3104_CTRL_H__
#define __TLV320AIC3104_CTRL_H__

#include <stdint.h>


typedef enum CODEC_SELECT_en {
   CODEC_A = 0,
   CODEC_B = 1,
} CODEC_SELECT;

typedef enum TLV_PAGE_SELECT_en {
   TLV_PAGE_0 = 0,
   TLV_PAGE_1 = 1,
} TLV_PAGE_SELECT;

extern int16_t TlvPageSelect(CODEC_SELECT codec, TLV_PAGE_SELECT page);
extern int16_t TlvReadReg(CODEC_SELECT codec, TLV_PAGE_SELECT page, uint8_t reg);
extern int16_t TlvWriteReg(CODEC_SELECT codec, TLV_PAGE_SELECT page, uint8_t reg, uint8_t val);


#endif
