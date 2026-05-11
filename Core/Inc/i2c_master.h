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

#ifndef __I2C_MASTER_H__
#define __I2C_MASTER_H__

#include <stdint.h>
#include <stm32h5xx_ll_i2c.h>


extern int I2C_Write(I2C_TypeDef* I2Cx, uint8_t addr, const uint8_t* data, uint16_t len);
extern int I2C_Read(I2C_TypeDef* I2Cx, uint8_t addr, uint8_t* data, uint16_t len);
extern int I2C_ReadReg(I2C_TypeDef* I2Cx, uint8_t addr, const uint8_t* reg, uint16_t reglen, uint8_t* data, uint16_t datalen);


#endif
