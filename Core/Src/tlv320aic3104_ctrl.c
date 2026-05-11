/***************************************************************************//**
* @file  tlv320aic3104_ctrl.c
* @brief Control functions for TLV320AIC3104 audio codec using I2C
*
* Copyright (c) 2026 Unicod
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*//****************************************************************************/

//------------------------------C library---------------------------------------
#include <stdint.h>
//----------------------------user includes-------------------------------------
#include "tlv320aic3104_ctrl.h"
#include "i2c_master.h"
#include "main.h"
//------------------------------------------------------------------------------


/* Private macros ------------------------------------------------------------*/

// Codecs on different I2C buses
#define I2C_CODEC_A     I2C1
#define I2C_CODEC_B     I2C3

#define CODEC_I2C_ADDR  0x30     // left-aligned 7-bit slave address

/* Private variables ---------------------------------------------------------*/
static uint8_t PageSelect[2] = {0xFF, 0xFF}; // current page selected for each codec, to optimize page switching

/* Functions -----------------------------------------------------------------*/


/***************************************************************************//**
* @brief  Read register value from codec
* @param  codec: codec select (0 for CODEC_A, 1 for CODEC_B)
* @param  page: page select (0 or 1)
* @param  reg: register address (0-127)
* @return register value (0-255) on success, negative error code on failure
*//****************************************************************************/
int16_t TlvReadReg(CODEC_SELECT codec, TLV_PAGE_SELECT page, uint8_t reg) {
   if (page != PageSelect[codec]) {
      int err = TlvPageSelect(codec, page);
      if (err < 0) return err;
   }
   if (reg) reg--; // workaround: reading gives regaddr+1 positions back at (datasheet is wrong...)
   uint8_t val = 0;
   int err = I2C_ReadReg((codec == CODEC_A) ? I2C_CODEC_A : I2C_CODEC_B,
                         CODEC_I2C_ADDR,
                         &reg, 1,
                         &val, 1);
   if (err < 0) return err;
   return val;
}

/***************************************************************************//**
* @brief  Write register value to codec
* @param  codec: codec select (0 for CODEC_A, 1 for CODEC_B)
* @param  reg: register address (0-127)
* @param  val: register value to write (0-255)
* @return 0 on success, negative error code on failure
*//****************************************************************************/
int16_t TlvWriteReg(CODEC_SELECT codec, TLV_PAGE_SELECT page, uint8_t reg, uint8_t val) {
   if (page != PageSelect[codec]) {
      int err = TlvPageSelect(codec, page);
      if (err < 0) return err;
   }
   uint8_t data[2] = {reg, val};
   int err = I2C_Write((codec == CODEC_A) ? I2C_CODEC_A : I2C_CODEC_B,
                         CODEC_I2C_ADDR,
                         data, 2);
   return err;
}

/***************************************************************************//**
* @brief  Write register value to codec
* @param  codec: codec select (0 for CODEC_A, 1 for CODEC_B)
* @param  reg: register address (0-127)
* @param  val: register value to write (0-255)
* @return 0 on success, negative error code on failure
*//****************************************************************************/
int16_t TlvPageSelect(CODEC_SELECT codec, TLV_PAGE_SELECT page) {
   if (codec > CODEC_B) codec = CODEC_B;  // clamp codec select
   PageSelect[codec] = page;              // update current page for the codec
   uint8_t reg = 0;                       // page select register address is 0
   uint8_t data[2] = {reg, page};
   int err = I2C_Write((codec == CODEC_A) ? I2C_CODEC_A : I2C_CODEC_B,
                        CODEC_I2C_ADDR,
                        data, 2);
   return err;
}




