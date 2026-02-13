#ifndef DISP7SEG_H_INCLUDED
#define DISP7SEG_H_INCLUDED

#include <stdint.h>

extern void DispPutDigit(uint8_t pos, char chr, uint8_t dp);
extern void ShiftReg_Update(void);


#endif
