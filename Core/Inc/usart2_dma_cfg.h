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

#ifndef __USART2_IT_CFG_H__
#define __USART2_IT_CFG_H__

#include "stm32h5xx_ll_dma.h"


#define USART2_RXDMA_BUF_SIZE    2048
#define USART2_TXDMA_BUF_SIZE    4096

#define USART2_GPDMA             GPDMA1
#define USART2_DMA_TX_CHANNEL    LL_DMA_CHANNEL_2
#define USART2_DMA_RX_CHANNEL    LL_DMA_CHANNEL_3


#endif // __USART2_CFG_H__
