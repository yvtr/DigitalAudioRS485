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

#ifndef __USART3_IT_CFG_H__
#define __USART3_IT_CFG_H__

#include "stm32h5xx_ll_dma.h"


#define USART3_RXDMA_BUF_SIZE    2048
#define USART3_TXDMA_BUF_SIZE    2048

#define USART3_GPDMA             GPDMA1
#define USART3_DMA_TX_CHANNEL    LL_DMA_CHANNEL_0
#define USART3_DMA_RX_CHANNEL    LL_DMA_CHANNEL_1


#endif // __USART3_CFG_H__
