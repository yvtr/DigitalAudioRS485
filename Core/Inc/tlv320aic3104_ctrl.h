/***************************************************************************//**
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
*******************************************************************************/

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
