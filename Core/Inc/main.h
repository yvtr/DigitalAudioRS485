/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h5xx_ll_icache.h"
#include "stm32h5xx_ll_pwr.h"
#include "stm32h5xx_ll_crs.h"
#include "stm32h5xx_ll_rcc.h"
#include "stm32h5xx_ll_bus.h"
#include "stm32h5xx_ll_system.h"
#include "stm32h5xx_ll_exti.h"
#include "stm32h5xx_ll_cortex.h"
#include "stm32h5xx_ll_utils.h"
#include "stm32h5xx_ll_dma.h"
#include "stm32h5xx_ll_tim.h"
#include "stm32h5xx_ll_gpio.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SHR_CLK_Pin LL_GPIO_PIN_14
#define SHR_CLK_GPIO_Port GPIOC
#define SHR_STR_Pin LL_GPIO_PIN_15
#define SHR_STR_GPIO_Port GPIOC
#define LD2_Pin LL_GPIO_PIN_11
#define LD2_GPIO_Port GPIOC
#define SHR_DOUT_DISP_Pin LL_GPIO_PIN_8
#define SHR_DOUT_DISP_GPIO_Port GPIOB
#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif

/* USER CODE BEGIN Private defines */

/***************************************************************************//**
* @brief Read actual counter value of general purpose [us] timer
* @return Timer value [us]
*//****************************************************************************/
static inline uint32_t usTimerGetAbs (void) {
   return LL_TIM_GetCounter(TIM5);
}

/***************************************************************************//**
* @brief Get [us] difference between actual timer value and reference time
* @param tref  reference time
* @return time difference [us]
*//****************************************************************************/
static inline int32_t usTimerGetRel(uint32_t tref) {
   return LL_TIM_GetCounter(TIM5) - tref;
}

static inline void Delay_us(uint32_t t_us) {
   uint32_t tref = usTimerGetAbs();
   while (usTimerGetRel(tref) <= t_us);
}

// Shift register control macros
#define SHR_COUNT    4
static inline void SHRCLK_HI()      { LL_GPIO_SetOutputPin(   SHR_CLK_GPIO_Port, 		SHR_CLK_Pin); }
static inline void SHRCLK_LO()      { LL_GPIO_ResetOutputPin( SHR_CLK_GPIO_Port, 		SHR_CLK_Pin); }
static inline void SHRSTR_HI()      { LL_GPIO_SetOutputPin(   SHR_STR_GPIO_Port, 		SHR_STR_Pin); }
static inline void SHRSTR_LO()      { LL_GPIO_ResetOutputPin( SHR_STR_GPIO_Port, 		SHR_STR_Pin); }
static inline void SHRDIN_HI()      { LL_GPIO_SetOutputPin(   SHR_DOUT_DISP_GPIO_Port, SHR_DOUT_DISP_Pin); }
static inline void SHRDIN_LO()      { LL_GPIO_ResetOutputPin( SHR_DOUT_DISP_GPIO_Port, SHR_DOUT_DISP_Pin); }


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
