/***************************************************************************//**
* @file disp7seg.c
* @brief 7-segment display handling
*******************************************************************************/

#include "disp7seg.h"
#include "main.h"


static uint8_t ShrBuf[SHR_COUNT] = {0};   // buffer for ext. shift register
static uint8_t ShrDirty = 1;              // update request

/***************************************************************************//**
* @brief Write data to external shift register on changes
*******************************************************************************/
void ShiftReg_Update() {
   if (ShrDirty == 0) return;   // register is up to date
   ShrDirty = 0;

   uint_fast8_t shrsel = SHR_COUNT;
   while (shrsel--) {
      uint8_t reg = ShrBuf[shrsel];
      uint_fast8_t clkcnt;
      for (clkcnt = 0; clkcnt < 8; clkcnt++) {
         SHRCLK_LO();
         if (reg & 0x80) { SHRDIN_HI(); }
         else            { SHRDIN_LO(); }
         reg <<= 1;     // shift left
         Delay_us(1);
         SHRCLK_HI();   // clock pulse
         Delay_us(1);
      }
   }
   SHRSTR_HI();      // strobe pulse
   SHRCLK_LO();
   SHRDIN_LO();
   Delay_us(1);
   SHRSTR_LO();
}

/***************************************************************************//**
/brief   Segment table for 7 segment display

For ASCII characters range 0x20...0x60

Segment <--> bit allocation
/code

    0000
   5    1
   5    1
    6666
   4    2
   4    2
    3333  77
*******************************************************************************/
static const uint8_t LcdAscii7segTable[] = {
   0x00, 0x82, 0xA2, 0x5C, 0x6D, 0x00, 0x7F, 0x02,                // space ... apostrof
   0x39, 0x0F, 0x00, 0x00, 0x04, 0x40, 0x00, 0x52,                // ( ... slash
   0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F,    // 0 ... 9
   0x48, 0x48, 0x58, 0x48, 0x4C, 0x53, 0x7B,                      // : ... @

   0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x3D, 0x76,                // ABCDEFGH
   0x06, 0x1E, 0x7A, 0x38, 0x37, 0x54, 0x3F, 0x73,                // IJKLMNOP
   0x67, 0x50, 0x6D, 0x78, 0x3E, 0x3E, 0x3E, 0x76,                // QRSTUVWX
   0x6E, 0x5B, 0x39, 0x64, 0x0F, 0x23, 0x08, 0x60,                // YZ[\]^_`

   0x5F, 0x7C, 0x58, 0x5E, 0x79, 0x71, 0x6F, 0x74,                // abcdefgh
   0x04, 0x1E, 0x7A, 0x06, 0x37, 0x54, 0x5C, 0x73,                // ijklmnop
   0x67, 0x50, 0x6D, 0x78, 0x1C, 0x1C, 0x1C, 0x76,                // qrstuvwx
   0x6E, 0x5B, 0x39, 0x30, 0x0F, 0x62                             // yz{|}~
};

/***************************************************************************//**
* \brief Print characters to 7 segment display
*******************************************************************************/
void DispPutDigit(uint8_t pos, char chr, uint8_t dp) {
   if (pos >= SHR_COUNT) return;

   uint8_t* disp_buf = ShrBuf;
   uint8_t  segments;

   uint8_t c = chr;
   if (c >= ' ' && c < 127) segments = LcdAscii7segTable[c-' '];
   //else if (c == '°') segments = 0x63;
   else segments = 0;
   if (dp) segments |= 0x80;           // decimal point
   uint8_t prevseg = disp_buf[pos];
   disp_buf[pos] = ~segments;          // write new digit

   if (prevseg != disp_buf[pos]) {     // buffer changed
      ShrDirty = 1;                    // update request
   }
}
