/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <ctype.h>
#include <stdint.h>
#include <string.h>

#include "disp7seg.h"
#include "printf.h"
#include "sound_de.h"
#include "tlv320aic3104_ctrl.h"
#include "usart2_dma.h"
#include "usart3_dma.h"
#include "uart5_it.h"
#include "wavetable.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

LL_DMA_LinkNodeTypeDef Node_GPDMA2_Channel1;
LL_DMA_LinkNodeTypeDef Node_GPDMA2_Channel0;

LL_DMA_LinkNodeTypeDef Node_GPDMA1_Channel3;
LL_DMA_LinkNodeTypeDef Node_GPDMA1_Channel1;

/* USER CODE BEGIN PV */

#define I2S2_RXDMA_BUF_SAMPLE_CNT   256
#define I2S2_TXDMA_BUF_SAMPLE_CNT   256

uint32_t I2S2RxDmaBuf[I2S2_RXDMA_BUF_SAMPLE_CNT][2] = {0};
uint32_t I2S2TxDmaBuf[I2S2_TXDMA_BUF_SAMPLE_CNT][2] = {0};

uint8_t ChSel = 0;
uint8_t Vol = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_GPDMA1_Init(void);
static void MX_GPDMA2_Init(void);
static void MX_ICACHE_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_UART5_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2S2_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2S3_Init(void);
/* USER CODE BEGIN PFP */


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static inline void LD2_On()     { LL_GPIO_SetOutputPin( LD2_GPIO_Port, LD2_Pin);   }
static inline void LD2_Off()    { LL_GPIO_ResetOutputPin( LD2_GPIO_Port, LD2_Pin); }
static inline void LD2_Toggle() { LL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin); }

/***************************************************************************//**
 * @brief Check if specified time interval has elapsed
 * @param tref Pointer to time reference variable [ms]
 * @param tcycle Time interval in milliseconds
 *//****************************************************************************/
static inline uint32_t TickChk(uint32_t *tref, int_fast16_t tcycle) {
   uint32_t t = LL_TIM_GetCounter(TIM2) / 2;    // 2kHz/2=1kHz
   int32_t tdif = t - *tref;
   if (tdif >= tcycle) {
      *tref += tcycle;
      return 1;
   }
   return 0;
}

/***************************************************************************//**
* @brief Character send interface for printf function
*//****************************************************************************/
void uart_putc (void* p, char c) {
   Uart5_PutByte(c);
}


void ProcessUsart2RxData(const uint8_t* data, uint16_t len) {
#if 0
   printf("USART2 RX(%u): ", len);
   for (uint16_t i = 0; i < len; i++) {
      printf("%c", data[i]);
   }
   printf("\n");
   Delay_us(500); // simulate long processing time
#endif
}

void ProcessUsart3RxData(const uint8_t* data, uint16_t len) {
#if 0
   printf("USART3 RX(%u): ", len);
   for (uint16_t i = 0; i < len; i++) {
      printf("%c", data[i]);
   }
   printf("\n");
   Delay_us(500); // simulate long processing time
#endif
}


/***************************************************************************//**
* @brief  List specified registers of TLV320AIC3204 codec for debugging
*//****************************************************************************/
void TLV320_AIC3204_DumpRegs(void) {
   printf("TLV320_AIC3204: Dumping registers...\n");

   CODEC_SELECT    codec = CODEC_A;
   TLV_PAGE_SELECT page = TLV_PAGE_0;
   uint8_t         reg_list[] = { 0,2,3,4,5,6,7,8,9,10,11,47,51,43,37,44,64,65,38 };

   for (size_t i = 0; i < ARRAY_COUNT(reg_list); i++) {
      uint8_t reg = reg_list[i];
      int16_t val = TlvReadReg(codec, page, reg);
      printf("Reg %2u: 0x%02X (%d) \n", reg, val, val);
   }

   printf("TLV320_AIC3204: Dump done\n");
}

/***************************************************************************//**
* @brief   TLV320AIC3204 codec init with basic config for stereo playing (DAC)
*//****************************************************************************/
void TLV320_AIC3204_Init() {
   TlvWriteReg(CODEC_A, TLV_PAGE_0,   3, 0x10);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,   7, 0x0A);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,   9, 0x10);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  47, 0x80);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  51, 0x09);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  43, 0x00);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  37, 0xD0);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  44, 0x00);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  64, 0x80);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  65, 0x09);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  38, 0x0C);
   TlvWriteReg(CODEC_A, TLV_PAGE_0, 101, 0x01);
   TlvWriteReg(CODEC_A, TLV_PAGE_0, 102, 0x02);

   TlvWriteReg(CODEC_A, TLV_PAGE_0,  15, 0x40);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  16, 0x40);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  17, 0xFF);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  18, 0xF0);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  19, 0x0D);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  21, 0x80);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  22, 0xFD);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  24, 0xF8);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  25, 0x80);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  40, 0xC1);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  41, 0x02);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  42, 0x88);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  46, 0x00);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  49, 0x00);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  50, 0x00);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  53, 0x00);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  54, 0x00);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  56, 0x00);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  57, 0x00);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  58, 0x01);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  60, 0x00);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  61, 0x00);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  63, 0x00);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  67, 0x00);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  68, 0x00);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  70, 0x00);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  71, 0x00);
   TlvWriteReg(CODEC_A, TLV_PAGE_0,  72, 0x01);
}


void Fill_I2S_Buffer(uint32_t *buf, uint32_t start_sample, uint32_t sample_count) {
   for (uint32_t i = 0; i < sample_count; i++) {
      int16_t pcmsample = 0;
      switch (ChSel) {
         case 0:
         case 1:
         case 2: {
            static uint16_t Phase = 0;
            pcmsample = Wave_Sin[Phase] / 4;        // sine waveform
            uint16_t phase_inc = (ChSel == 0) ? 4 : ((ChSel == 1) ? 8 : 16); // different frequency for different channel selection
            Phase = (Phase + phase_inc) % ARRAY_COUNT(Wave_Sin);
         }break;
         case 3:
         case 4:
         case 5: {
            static uint16_t Phase = 0;
            pcmsample = Wave_Organ[Phase] / 4;        // organ waveform
            uint16_t phase_inc = (ChSel == 3) ? 4 : ((ChSel == 4) ? 8 : 16); // different frequency for different channel selection
            Phase = (Phase + phase_inc) % ARRAY_COUNT(Wave_Organ);
         }break;
         case 6: {
            static uint32_t SoundIdx = 0;
            int8_t s = Sound_DE[SoundIdx++];
            if (SoundIdx >= ARRAY_COUNT(Sound_DE)) SoundIdx = 0;
            pcmsample = s << 8;   // 8-bit PCM in MSB
         }break;
      }
      int32_t frame = pcmsample << 16; // convert to 32-bit sample with 16-bit left justified
      uint32_t pos = (start_sample + i) * 2;  // Stereo: L,R
      buf[pos + 0] = frame/4;   // Left
      buf[pos + 1] = frame/4;   // Right
   }
}


void Audio_Task(void) {
   static uint32_t old_sample_index = 0;
   // DMA pointer --> Word-Index
   uint32_t dma_ptr = 2048 - LL_DMA_GetBlkDataLength(GPDMA2, LL_DMA_CHANNEL_0);
   uint32_t sample_index = dma_ptr / 8; // Word = 4 bytes * 2 channels = 8 bytes per sample

   if (sample_index != old_sample_index) {
      uint32_t free_samples;
      if (sample_index > old_sample_index) {
         free_samples = sample_index - old_sample_index;         // normal case: DMA is in between old and new index
         Fill_I2S_Buffer(I2S2TxDmaBuf[0], old_sample_index, free_samples);
      } else {
         free_samples = I2S2_TXDMA_BUF_SAMPLE_CNT - old_sample_index;         // Wrap-around
         Fill_I2S_Buffer(I2S2TxDmaBuf[0], old_sample_index, free_samples); // first the rest until end of buffer
         if (sample_index > 0) {
            Fill_I2S_Buffer(I2S2TxDmaBuf[0], 0, sample_index);
         }
      }

      old_sample_index = sample_index;
   }
   //printf("dma_ptr:%u dma_word_index:%u sample_index:%u\n", dma_ptr, dma_word_index, sample_index);
}




/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

  /* System interrupt init*/
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

   for (uint32_t i = 0; i < I2S2_TXDMA_BUF_SAMPLE_CNT; i++) {
      int32_t a = INT32_MAX / 2 * sin(2 * 3.14159265358979323846 * i / I2S2_TXDMA_BUF_SAMPLE_CNT); // sine wave
      int32_t b = INT32_MAX / 2 * sin(4 * 3.14159265358979323846 * i / I2S2_TXDMA_BUF_SAMPLE_CNT); // sine wave at 2x frequency
      I2S2TxDmaBuf[i][0] = a; // left channel sample
      I2S2TxDmaBuf[i][1] = b; // right channel sample
   }

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_GPDMA1_Init();
  MX_GPDMA2_Init();
  MX_ICACHE_Init();
  MX_TIM5_Init();
  MX_TIM2_Init();
  MX_USART3_UART_Init();
  MX_UART5_Init();
  MX_USART2_UART_Init();
  MX_I2S2_Init();
  MX_I2C1_Init();
  MX_I2S3_Init();
  /* USER CODE BEGIN 2 */

  CODEC_RST_LO();    // Reset the audio codec via HW reset pin
  __NOP();
  __NOP();
  CODEC_RST_HI();

  init_printf(NULL, &uart_putc);

  LL_TIM_GenerateEvent_UPDATE(TIM2);
  LL_TIM_EnableCounter(TIM2);
  LL_TIM_GenerateEvent_UPDATE(TIM5);
  LL_TIM_EnableCounter(TIM5);

  LL_Init1msTick(SystemCoreClock);

  DispPutDigit(0, ' ', 0);
  DispPutDigit(1, ' ', 0);
  DispPutDigit(2, ' ', 0);
  DispPutDigit(3, ' ', 0);
  ShiftReg_Update();

  Usart2_DMA_Init(ProcessUsart2RxData);
  Usart3_DMA_Init(ProcessUsart3RxData);
  Uart5_Init();

  TLV320_AIC3204_Init();

  printf("Hello printf\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
   while (1)
   {
      static uint32_t Tick10msRef = 0;
      if (TickChk(&Tick10msRef, 10)) {  // execute every 10ms
         ShiftReg_Update();
      }

      static uint32_t Tick1secRef = 0;
      if (TickChk(&Tick1secRef, 1000)) {  // execute every 1s
         LD2_Toggle();
         static uint8_t cnt = 0;
         DispPutDigit(2, 'A'+cnt, 0);
         cnt = (cnt + 1) % 16;
         char s[256];
         sprintf(s, "%u: Hello DMA World! This is a long message to test the double buffering mechanism of USART3 Tx DMA.\n", cnt);
         Usart3_TxBufWrite(s, strlen(s), cnt&0x04); // write data and request flush
         sprintf(s, "%u: Message from USART2 DMA.\n", cnt);
         Usart2_TxBufWrite(s, strlen(s), !(cnt&0x04));
      }

      static uint32_t Tick100msRef = 0;
      if (TickChk(&Tick100msRef, 100)) {  // execute every 100ms
         static uint8_t dot = 0;
         dot ^= 1;   // toggle dot
         DispPutDigit(3, ' ', dot);
      }

      int16_t ch = Uart5_GetByte();
      if (ch != -1) {  // if data received
         char c = ch;
         if (c == 'a') {
            printf("I2S RX buf:\n");
            for (size_t i = 0; i < I2S2_RXDMA_BUF_SAMPLE_CNT; i++) {
               printf("%d;%d\n", I2S2RxDmaBuf[i][0]/65536, I2S2RxDmaBuf[i][1]/65536);
            }
         } else if (c == ' ') {
         } else if (c == 'd') {
            TLV320_AIC3204_DumpRegs();
         }
         if (isdigit(c)) {
            ChSel = c - '0';
            DispPutDigit(0, c, 0);
         }
      }

      Usart2_DMA_Task(); // handle USART2 DMA rx/tx
      Usart3_DMA_Task(); // handle USART3 DMA rx/tx
      Audio_Task();      // handle audio data feeding to I2S Tx buffer


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_3)
  {
  }

  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE3);
  while (LL_PWR_IsActiveFlag_VOS() == 0)
  {
  }
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {
  }

  LL_RCC_PLL1_SetSource(LL_RCC_PLL1SOURCE_HSE);
  LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_8_16);
  LL_RCC_PLL1_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
  LL_RCC_PLL1_SetM(1);
  LL_RCC_PLL1_SetN(20);
  LL_RCC_PLL1_SetP(4);
  LL_RCC_PLL1_SetQ(10);
  LL_RCC_PLL1_SetR(2);
  LL_RCC_PLL1P_Enable();
  LL_RCC_PLL1_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL1_IsReady() != 1)
  {
  }

  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1)
  {
  }

  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetAPB3Prescaler(LL_RCC_APB3_DIV_1);

  LL_Init1msTick(80000000);

  LL_SetSystemCoreClock(80000000);
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  LL_RCC_PLL2_SetSource(LL_RCC_PLL2SOURCE_HSE);
  LL_RCC_PLL2_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_8_16);
  LL_RCC_PLL2_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
  LL_RCC_PLL2_SetM(10);
  LL_RCC_PLL2_SetN(256);
  LL_RCC_PLL2_SetP(10);
  LL_RCC_PLL2_SetQ(10);
  LL_RCC_PLL2_SetR(2);
  LL_RCC_PLL2P_Enable();
  LL_RCC_PLL2_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL2_IsReady() != 1)
  {
  }

  LL_RCC_PLL3_SetSource(LL_RCC_PLL3SOURCE_HSE);
  LL_RCC_PLL3_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_8_16);
  LL_RCC_PLL3_SetVCOOutputRange(LL_RCC_PLLVCORANGE_WIDE);
  LL_RCC_PLL3_SetM(1);
  LL_RCC_PLL3_SetN(8);
  LL_RCC_PLL3_SetP(2);
  LL_RCC_PLL3_SetQ(4);
  LL_RCC_PLL3_SetR(4);
  LL_RCC_PLL3Q_Enable();
  LL_RCC_PLL3R_Enable();
  LL_RCC_PLL3_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL3_IsReady() != 1)
  {
  }

}

/**
  * @brief GPDMA1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPDMA1_Init(void)
{

  /* USER CODE BEGIN GPDMA1_Init 0 */

  /* USER CODE END GPDMA1_Init 0 */

  /* Peripheral clock enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPDMA1);

  /* USER CODE BEGIN GPDMA1_Init 1 */

  /* USER CODE END GPDMA1_Init 1 */
  /* USER CODE BEGIN GPDMA1_Init 2 */

  /* USER CODE END GPDMA1_Init 2 */

}

/**
  * @brief GPDMA2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPDMA2_Init(void)
{

  /* USER CODE BEGIN GPDMA2_Init 0 */

  /* USER CODE END GPDMA2_Init 0 */

  /* Peripheral clock enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPDMA2);

  /* USER CODE BEGIN GPDMA2_Init 1 */

  /* USER CODE END GPDMA2_Init 1 */
  /* USER CODE BEGIN GPDMA2_Init 2 */

  /* USER CODE END GPDMA2_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  LL_I2C_InitTypeDef I2C_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_PLL3R);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
  /**I2C1 GPIO Configuration
  PB6   ------> I2C1_SCL
  PB7   ------> I2C1_SDA
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6|LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */

  /** I2C Initialization
  */
  LL_I2C_EnableAutoEndMode(I2C1);
  LL_I2C_DisableOwnAddress2(I2C1);
  LL_I2C_DisableGeneralCall(I2C1);
  LL_I2C_EnableClockStretching(I2C1);
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.Timing = 0x00707CBB;
  I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
  I2C_InitStruct.DigitalFilter = 0;
  I2C_InitStruct.OwnAddress1 = 0;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C1, &I2C_InitStruct);
  LL_I2C_SetOwnAddress2(I2C1, 0, LL_I2C_OWNADDRESS2_NOMASK);

  /** I2C Fast mode Plus enable
  */
  LL_I2C_EnableFastModePlus(I2C1);
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2S2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S2_Init(void)
{

  /* USER CODE BEGIN I2S2_Init 0 */

   // WARNING
   // Remove the second redefinition of NodeConfig after Cube MX code generation.
   LL_DMA_InitNodeTypeDef NodeConfig = {0};



  /* USER CODE END I2S2_Init 0 */

  LL_I2S_InitTypeDef I2S_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  LL_DMA_InitLinkedListTypeDef DMA_InitLinkedListStruct = {0};

  LL_RCC_SetSPIClockSource(LL_RCC_SPI2_CLKSOURCE_PLL2P);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
  /**I2S2 GPIO Configuration
  PC2   ------> I2S2_SDI
  PB12   ------> I2S2_WS
  PB13   ------> I2S2_CK
  PB15   ------> I2S2_SDO
  PC6   ------> I2S2_MCK
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_12|LL_GPIO_PIN_13|LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* I2S2 DMA Init */

  /* GPDMA2_REQUEST_SPI2_RX Init */
  NodeConfig.SrcAddress        = (uint32_t)LL_SPI_DMA_GetRxRegAddr(SPI2);
  NodeConfig.DestAddress       = (uint32_t)&I2S2RxDmaBuf;
  NodeConfig.BlkDataLength     = sizeof(I2S2RxDmaBuf);

  NodeConfig.DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT1;
  NodeConfig.DestHWordExchange = LL_DMA_DEST_HALFWORD_PRESERVE;
  NodeConfig.DestByteExchange = LL_DMA_DEST_BYTE_PRESERVE;
  NodeConfig.DestBurstLength = 1;
  NodeConfig.DestIncMode = LL_DMA_DEST_INCREMENT;
  NodeConfig.DestDataWidth = LL_DMA_DEST_DATAWIDTH_WORD;
  NodeConfig.SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT0;
  NodeConfig.SrcByteExchange = LL_DMA_SRC_BYTE_PRESERVE;
  NodeConfig.DataAlignment = LL_DMA_DATA_ALIGN_ZEROPADD;
  NodeConfig.SrcBurstLength = 1;
  NodeConfig.SrcIncMode = LL_DMA_SRC_FIXED;
  NodeConfig.SrcDataWidth = LL_DMA_SRC_DATAWIDTH_WORD;
  NodeConfig.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
  NodeConfig.Mode = LL_DMA_NORMAL;
  NodeConfig.TriggerPolarity = LL_DMA_TRIG_POLARITY_MASKED;
  NodeConfig.BlkHWRequest = LL_DMA_HWREQUEST_SINGLEBURST;
  NodeConfig.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  NodeConfig.Request = LL_GPDMA2_REQUEST_SPI2_RX;
  NodeConfig.UpdateRegisters = (LL_DMA_UPDATE_CTR1 | LL_DMA_UPDATE_CTR2 | LL_DMA_UPDATE_CBR1 | LL_DMA_UPDATE_CSAR | LL_DMA_UPDATE_CDAR | LL_DMA_UPDATE_CTR3 | LL_DMA_UPDATE_CBR2 | LL_DMA_UPDATE_CLLR);
  NodeConfig.NodeType = LL_DMA_GPDMA_LINEAR_NODE;
  LL_DMA_CreateLinkNode(&NodeConfig, &Node_GPDMA2_Channel1);

  LL_DMA_ConnectLinkNode(&Node_GPDMA2_Channel1, LL_DMA_CLLR_OFFSET5, &Node_GPDMA2_Channel1, LL_DMA_CLLR_OFFSET5);

  /* Next function call is commented because it will not compile as is. The Node structure address has to be cast to an unsigned int (uint32_t)pNode_DMAxCHy */
  /*

  */
  LL_DMA_SetLinkedListBaseAddr(GPDMA2, LL_DMA_CHANNEL_1, (uint32_t)&Node_GPDMA2_Channel1);

  DMA_InitLinkedListStruct.Priority = LL_DMA_LOW_PRIORITY_LOW_WEIGHT;
  DMA_InitLinkedListStruct.LinkStepMode = LL_DMA_LSM_FULL_EXECUTION;
  DMA_InitLinkedListStruct.LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT1;
  DMA_InitLinkedListStruct.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
  LL_DMA_List_Init(GPDMA2, LL_DMA_CHANNEL_1, &DMA_InitLinkedListStruct);

  /* GPDMA2_REQUEST_SPI2_TX Init */
  NodeConfig.SrcAddress        = (uint32_t)&I2S2TxDmaBuf;
  NodeConfig.DestAddress       = (uint32_t)LL_SPI_DMA_GetTxRegAddr(SPI2);
  NodeConfig.BlkDataLength     = sizeof(I2S2TxDmaBuf);

  NodeConfig.DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT0;
  NodeConfig.DestHWordExchange = LL_DMA_DEST_HALFWORD_PRESERVE;
  NodeConfig.DestByteExchange = LL_DMA_DEST_BYTE_PRESERVE;
  NodeConfig.DestBurstLength = 1;
  NodeConfig.DestIncMode = LL_DMA_DEST_FIXED;
  NodeConfig.DestDataWidth = LL_DMA_DEST_DATAWIDTH_WORD;
  NodeConfig.SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT1;
  NodeConfig.SrcByteExchange = LL_DMA_SRC_BYTE_PRESERVE;
  NodeConfig.DataAlignment = LL_DMA_DATA_ALIGN_ZEROPADD;
  NodeConfig.SrcBurstLength = 1;
  NodeConfig.SrcIncMode = LL_DMA_SRC_INCREMENT;
  NodeConfig.SrcDataWidth = LL_DMA_SRC_DATAWIDTH_WORD;
  NodeConfig.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
  NodeConfig.Mode = LL_DMA_NORMAL;
  NodeConfig.TriggerPolarity = LL_DMA_TRIG_POLARITY_MASKED;
  NodeConfig.BlkHWRequest = LL_DMA_HWREQUEST_SINGLEBURST;
  NodeConfig.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  NodeConfig.Request = LL_GPDMA2_REQUEST_SPI2_TX;
  NodeConfig.UpdateRegisters = (LL_DMA_UPDATE_CTR1 | LL_DMA_UPDATE_CTR2 | LL_DMA_UPDATE_CBR1 | LL_DMA_UPDATE_CSAR | LL_DMA_UPDATE_CDAR | LL_DMA_UPDATE_CTR3 | LL_DMA_UPDATE_CBR2 | LL_DMA_UPDATE_CLLR);
  NodeConfig.NodeType = LL_DMA_GPDMA_LINEAR_NODE;
  LL_DMA_CreateLinkNode(&NodeConfig, &Node_GPDMA2_Channel0);

  LL_DMA_ConnectLinkNode(&Node_GPDMA2_Channel0, LL_DMA_CLLR_OFFSET5, &Node_GPDMA2_Channel0, LL_DMA_CLLR_OFFSET5);

  /* Next function call is commented because it will not compile as is. The Node structure address has to be cast to an unsigned int (uint32_t)pNode_DMAxCHy */
  /*

  */
  LL_DMA_SetLinkedListBaseAddr(GPDMA2, LL_DMA_CHANNEL_0, (uint32_t)&Node_GPDMA2_Channel0);

  DMA_InitLinkedListStruct.Priority = LL_DMA_LOW_PRIORITY_LOW_WEIGHT;
  DMA_InitLinkedListStruct.LinkStepMode = LL_DMA_LSM_FULL_EXECUTION;
  DMA_InitLinkedListStruct.LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT1;
  DMA_InitLinkedListStruct.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
  LL_DMA_List_Init(GPDMA2, LL_DMA_CHANNEL_0, &DMA_InitLinkedListStruct);

  /* USER CODE BEGIN I2S2_Init 1 */

  LL_DMA_ConfigLinkUpdate(GPDMA2, LL_DMA_CHANNEL_1, LL_DMA_UPDATE_CTR1 | LL_DMA_UPDATE_CTR2 |LL_DMA_UPDATE_CBR1 | LL_DMA_UPDATE_CSAR | LL_DMA_UPDATE_CDAR | LL_DMA_UPDATE_CLLR, (uint32_t)&Node_GPDMA2_Channel1);
  LL_DMA_EnableChannel(GPDMA2, LL_DMA_CHANNEL_1);
  LL_DMA_SetLinkedListAddrOffset(GPDMA2, LL_DMA_CHANNEL_1, LL_DMA_CLLR_OFFSET5);
  LL_I2S_EnableDMAReq_RX(SPI2);

  LL_DMA_ConfigLinkUpdate(GPDMA2, LL_DMA_CHANNEL_0, LL_DMA_UPDATE_CTR1 | LL_DMA_UPDATE_CTR2 |LL_DMA_UPDATE_CBR1 | LL_DMA_UPDATE_CSAR | LL_DMA_UPDATE_CDAR | LL_DMA_UPDATE_CLLR, (uint32_t)&Node_GPDMA2_Channel0);
  LL_DMA_EnableChannel(GPDMA2, LL_DMA_CHANNEL_0);
  LL_DMA_SetLinkedListAddrOffset(GPDMA2, LL_DMA_CHANNEL_0, LL_DMA_CLLR_OFFSET5);
  LL_I2S_EnableDMAReq_TX(SPI2);


  /* USER CODE END I2S2_Init 1 */
  I2S_InitStruct.Mode = LL_I2S_MODE_MASTER_FULL_DUPLEX;
  I2S_InitStruct.Standard = LL_I2S_STANDARD_PHILIPS;
  I2S_InitStruct.DataFormat = LL_I2S_DATAFORMAT_32B;
  I2S_InitStruct.MCLKOutput = LL_I2S_MCLK_OUTPUT_ENABLE;
  I2S_InitStruct.AudioFreq = 32000;
  I2S_InitStruct.ClockPolarity = LL_I2S_POLARITY_LOW;
  LL_I2S_Init(SPI2, &I2S_InitStruct);
  /* USER CODE BEGIN I2S2_Init 2 */

  LL_I2S_Enable(SPI2);
  LL_I2S_StartTransfer(SPI2);

  /* USER CODE END I2S2_Init 2 */

}

/**
  * @brief I2S3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S3_Init(void)
{

  /* USER CODE BEGIN I2S3_Init 0 */

  /* USER CODE END I2S3_Init 0 */

  LL_I2S_InitTypeDef I2S_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_RCC_SetSPIClockSource(LL_RCC_SPI3_CLKSOURCE_PLL2P);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
  /**I2S3 GPIO Configuration
  PC7   ------> I2S3_MCK
  PA15(JTDI)   ------> I2S3_WS
  PC10   ------> I2S3_CK
  PB4(NJTRST)   ------> I2S3_SDI
  PB5   ------> I2S3_SDO
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN I2S3_Init 1 */

  /* USER CODE END I2S3_Init 1 */
  I2S_InitStruct.Mode = LL_I2S_MODE_MASTER_FULL_DUPLEX;
  I2S_InitStruct.Standard = LL_I2S_STANDARD_PHILIPS;
  I2S_InitStruct.DataFormat = LL_I2S_DATAFORMAT_32B;
  I2S_InitStruct.MCLKOutput = LL_I2S_MCLK_OUTPUT_ENABLE;
  I2S_InitStruct.AudioFreq = 32000;
  I2S_InitStruct.ClockPolarity = LL_I2S_POLARITY_LOW;
  LL_I2S_Init(SPI3, &I2S_InitStruct);
  /* USER CODE BEGIN I2S3_Init 2 */

  /* USER CODE END I2S3_Init 2 */

}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{

  /* USER CODE BEGIN ICACHE_Init 0 */

  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache (default 2-ways set associative cache)
  */
  LL_ICACHE_Enable();
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  TIM_InitStruct.Prescaler = 39999;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 4294967295;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM2, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM2);
  LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM2);
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5);

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  TIM_InitStruct.Prescaler = 79;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 4294967295;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM5, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM5);
  LL_TIM_SetClockSource(TIM5, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetTriggerOutput(TIM5, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM5);
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  LL_USART_InitTypeDef UART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_RCC_SetUARTClockSource(LL_RCC_UART5_CLKSOURCE_PLL3Q);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART5);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOD);
  /**UART5 GPIO Configuration
  PC12   ------> UART5_TX
  PD2   ------> UART5_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
  LL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* UART5 interrupt Init */
  NVIC_SetPriority(UART5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(UART5_IRQn);

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
  UART_InitStruct.BaudRate = 115200;
  UART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  UART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  UART_InitStruct.Parity = LL_USART_PARITY_NONE;
  UART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  UART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  UART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(UART5, &UART_InitStruct);
  LL_USART_ConfigAsyncMode(UART5);
  LL_USART_Enable(UART5);
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

   // WARNING
   // Remove the second redefinition of NodeConfig after Cube MX code generation.
   LL_DMA_InitNodeTypeDef NodeConfig = {0};

   NodeConfig.SrcAddress        = (uint32_t)LL_USART_DMA_GetRegAddr(USART2, LL_USART_DMA_REG_DATA_RECEIVE);
   NodeConfig.DestAddress       = (uint32_t)&Usart2RxDmaBuf;
   NodeConfig.BlkDataLength     = ARRAY_COUNT(Usart2RxDmaBuf);


  /* USER CODE END USART2_Init 0 */

  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  LL_DMA_InitLinkedListTypeDef DMA_InitLinkedListStruct = {0};
  LL_DMA_InitTypeDef DMA_InitStruct = {0};

  LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  /**USART2 GPIO Configuration
  PA1   ------> USART2_DE
  PA2   ------> USART2_TX
  PA3   ------> USART2_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_1|LL_GPIO_PIN_2|LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USART2 DMA Init */

  /* GPDMA1_REQUEST_USART2_RX Init */
  NodeConfig.DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT1;
  NodeConfig.DestHWordExchange = LL_DMA_DEST_HALFWORD_PRESERVE;
  NodeConfig.DestByteExchange = LL_DMA_DEST_BYTE_PRESERVE;
  NodeConfig.DestBurstLength = 1;
  NodeConfig.DestIncMode = LL_DMA_DEST_INCREMENT;
  NodeConfig.DestDataWidth = LL_DMA_DEST_DATAWIDTH_BYTE;
  NodeConfig.SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT0;
  NodeConfig.SrcByteExchange = LL_DMA_SRC_BYTE_PRESERVE;
  NodeConfig.DataAlignment = LL_DMA_DATA_ALIGN_ZEROPADD;
  NodeConfig.SrcBurstLength = 1;
  NodeConfig.SrcIncMode = LL_DMA_SRC_FIXED;
  NodeConfig.SrcDataWidth = LL_DMA_SRC_DATAWIDTH_BYTE;
  NodeConfig.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
  NodeConfig.Mode = LL_DMA_NORMAL;
  NodeConfig.TriggerPolarity = LL_DMA_TRIG_POLARITY_MASKED;
  NodeConfig.BlkHWRequest = LL_DMA_HWREQUEST_SINGLEBURST;
  NodeConfig.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  NodeConfig.Request = LL_GPDMA1_REQUEST_USART2_RX;
  NodeConfig.UpdateRegisters = (LL_DMA_UPDATE_CTR1 | LL_DMA_UPDATE_CTR2 | LL_DMA_UPDATE_CBR1 | LL_DMA_UPDATE_CSAR | LL_DMA_UPDATE_CDAR | LL_DMA_UPDATE_CTR3 | LL_DMA_UPDATE_CBR2 | LL_DMA_UPDATE_CLLR);
  NodeConfig.NodeType = LL_DMA_GPDMA_LINEAR_NODE;
  LL_DMA_CreateLinkNode(&NodeConfig, &Node_GPDMA1_Channel3);

  LL_DMA_ConnectLinkNode(&Node_GPDMA1_Channel3, LL_DMA_CLLR_OFFSET5, &Node_GPDMA1_Channel3, LL_DMA_CLLR_OFFSET5);

  /* Next function call is commented because it will not compile as is. The Node structure address has to be cast to an unsigned int (uint32_t)pNode_DMAxCHy */
  /*

  */
  LL_DMA_SetLinkedListBaseAddr(GPDMA1, LL_DMA_CHANNEL_3, (uint32_t)&Node_GPDMA1_Channel3);

  DMA_InitLinkedListStruct.Priority = LL_DMA_LOW_PRIORITY_LOW_WEIGHT;
  DMA_InitLinkedListStruct.LinkStepMode = LL_DMA_LSM_FULL_EXECUTION;
  DMA_InitLinkedListStruct.LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT1;
  DMA_InitLinkedListStruct.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
  LL_DMA_List_Init(GPDMA1, LL_DMA_CHANNEL_3, &DMA_InitLinkedListStruct);

  /* GPDMA1_REQUEST_USART2_TX Init */
  DMA_InitStruct.SrcAddress = 0x00000000U;
  DMA_InitStruct.DestAddress = 0x00000000U;
  DMA_InitStruct.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  DMA_InitStruct.BlkHWRequest = LL_DMA_HWREQUEST_SINGLEBURST;
  DMA_InitStruct.DataAlignment = LL_DMA_DATA_ALIGN_ZEROPADD;
  DMA_InitStruct.SrcBurstLength = 1;
  DMA_InitStruct.DestBurstLength = 1;
  DMA_InitStruct.SrcDataWidth = LL_DMA_SRC_DATAWIDTH_BYTE;
  DMA_InitStruct.DestDataWidth = LL_DMA_DEST_DATAWIDTH_BYTE;
  DMA_InitStruct.SrcIncMode = LL_DMA_SRC_INCREMENT;
  DMA_InitStruct.DestIncMode = LL_DMA_DEST_FIXED;
  DMA_InitStruct.Priority = LL_DMA_LOW_PRIORITY_LOW_WEIGHT;
  DMA_InitStruct.BlkDataLength = 0x00000000U;
  DMA_InitStruct.TriggerMode = LL_DMA_TRIGM_BLK_TRANSFER;
  DMA_InitStruct.TriggerPolarity = LL_DMA_TRIG_POLARITY_MASKED;
  DMA_InitStruct.TriggerSelection = 0x00000000U;
  DMA_InitStruct.Request = LL_GPDMA1_REQUEST_USART2_TX;
  DMA_InitStruct.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
  DMA_InitStruct.Mode = LL_DMA_NORMAL;
  DMA_InitStruct.SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT1;
  DMA_InitStruct.DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT0;
  DMA_InitStruct.LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT1;
  DMA_InitStruct.LinkStepMode = LL_DMA_LSM_FULL_EXECUTION;
  DMA_InitStruct.LinkedListBaseAddr = 0x00000000U;
  DMA_InitStruct.LinkedListAddrOffset = 0x00000000U;
  LL_DMA_Init(GPDMA1, LL_DMA_CHANNEL_2, &DMA_InitStruct);

  /* USER CODE BEGIN USART2_Init 1 */

  LL_DMA_ConfigLinkUpdate(GPDMA1, LL_DMA_CHANNEL_3, LL_DMA_UPDATE_CTR1 | LL_DMA_UPDATE_CTR2 |LL_DMA_UPDATE_CBR1 | LL_DMA_UPDATE_CSAR | LL_DMA_UPDATE_CDAR | LL_DMA_UPDATE_CLLR, (uint32_t)&Node_GPDMA1_Channel3);
  LL_DMA_EnableChannel(GPDMA1, LL_DMA_CHANNEL_3);
  LL_DMA_SetLinkedListAddrOffset(GPDMA1, LL_DMA_CHANNEL_3, LL_DMA_CLLR_OFFSET5);
  LL_USART_EnableDMAReq_RX(USART2);

  LL_DMA_SetDestAddress(GPDMA1, LL_DMA_CHANNEL_2, LL_USART_DMA_GetRegAddr(USART2, LL_USART_DMA_REG_DATA_TRANSMIT));
  LL_USART_EnableDMAReq_TX(USART2);

  /* USER CODE END USART2_Init 1 */
  USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART2, &USART_InitStruct);
  LL_USART_SetTXFIFOThreshold(USART2, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_SetRXFIFOThreshold(USART2, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_EnableDEMode(USART2);
  LL_USART_SetDESignalPolarity(USART2, LL_USART_DE_POLARITY_HIGH);
  LL_USART_SetDEAssertionTime(USART2, 0);
  LL_USART_SetDEDeassertionTime(USART2, 0);
  LL_USART_DisableFIFO(USART2);
  LL_USART_ConfigAsyncMode(USART2);
  LL_USART_Enable(USART2);
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  // WARNING
  // Remove the second redefinition of NodeConfig after Cube MX code generation.
  LL_DMA_InitNodeTypeDef NodeConfig = {0};

  NodeConfig.SrcAddress        = (uint32_t)LL_USART_DMA_GetRegAddr(USART3, LL_USART_DMA_REG_DATA_RECEIVE);
  NodeConfig.DestAddress       = (uint32_t)&Usart3RxDmaBuf;
  NodeConfig.BlkDataLength     = ARRAY_COUNT(Usart3RxDmaBuf);


  /* USER CODE END USART3_Init 0 */

  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  LL_DMA_InitLinkedListTypeDef DMA_InitLinkedListStruct = {0};
  LL_DMA_InitTypeDef DMA_InitStruct = {0};

  LL_RCC_SetUSARTClockSource(LL_RCC_USART3_CLKSOURCE_PCLK1);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
  /**USART3 GPIO Configuration
  PB1   ------> USART3_RX
  PB10   ------> USART3_TX
  PB14   ------> USART3_DE
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_1|LL_GPIO_PIN_10|LL_GPIO_PIN_14;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USART3 DMA Init */

  /* GPDMA1_REQUEST_USART3_RX Init */
  NodeConfig.DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT1;
  NodeConfig.DestHWordExchange = LL_DMA_DEST_HALFWORD_PRESERVE;
  NodeConfig.DestByteExchange = LL_DMA_DEST_BYTE_PRESERVE;
  NodeConfig.DestBurstLength = 1;
  NodeConfig.DestIncMode = LL_DMA_DEST_INCREMENT;
  NodeConfig.DestDataWidth = LL_DMA_DEST_DATAWIDTH_BYTE;
  NodeConfig.SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT0;
  NodeConfig.SrcByteExchange = LL_DMA_SRC_BYTE_PRESERVE;
  NodeConfig.DataAlignment = LL_DMA_DATA_ALIGN_ZEROPADD;
  NodeConfig.SrcBurstLength = 1;
  NodeConfig.SrcIncMode = LL_DMA_SRC_FIXED;
  NodeConfig.SrcDataWidth = LL_DMA_SRC_DATAWIDTH_BYTE;
  NodeConfig.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
  NodeConfig.Mode = LL_DMA_NORMAL;
  NodeConfig.TriggerPolarity = LL_DMA_TRIG_POLARITY_MASKED;
  NodeConfig.BlkHWRequest = LL_DMA_HWREQUEST_SINGLEBURST;
  NodeConfig.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  NodeConfig.Request = LL_GPDMA1_REQUEST_USART3_RX;
  NodeConfig.UpdateRegisters = (LL_DMA_UPDATE_CTR1 | LL_DMA_UPDATE_CTR2 | LL_DMA_UPDATE_CBR1 | LL_DMA_UPDATE_CSAR | LL_DMA_UPDATE_CDAR | LL_DMA_UPDATE_CTR3 | LL_DMA_UPDATE_CBR2 | LL_DMA_UPDATE_CLLR);
  NodeConfig.NodeType = LL_DMA_GPDMA_LINEAR_NODE;
  LL_DMA_CreateLinkNode(&NodeConfig, &Node_GPDMA1_Channel1);

  LL_DMA_ConnectLinkNode(&Node_GPDMA1_Channel1, LL_DMA_CLLR_OFFSET5, &Node_GPDMA1_Channel1, LL_DMA_CLLR_OFFSET5);

  /* Next function call is commented because it will not compile as is. The Node structure address has to be cast to an unsigned int (uint32_t)pNode_DMAxCHy */
  /*

  */
  LL_DMA_SetLinkedListBaseAddr(GPDMA1, LL_DMA_CHANNEL_1, (uint32_t)&Node_GPDMA1_Channel1);

  DMA_InitLinkedListStruct.Priority = LL_DMA_LOW_PRIORITY_LOW_WEIGHT;
  DMA_InitLinkedListStruct.LinkStepMode = LL_DMA_LSM_FULL_EXECUTION;
  DMA_InitLinkedListStruct.LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT1;
  DMA_InitLinkedListStruct.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
  LL_DMA_List_Init(GPDMA1, LL_DMA_CHANNEL_1, &DMA_InitLinkedListStruct);

  /* GPDMA1_REQUEST_USART3_TX Init */
  DMA_InitStruct.SrcAddress = 0x00000000U;
  DMA_InitStruct.DestAddress = 0x00000000U;
  DMA_InitStruct.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  DMA_InitStruct.BlkHWRequest = LL_DMA_HWREQUEST_SINGLEBURST;
  DMA_InitStruct.DataAlignment = LL_DMA_DATA_ALIGN_ZEROPADD;
  DMA_InitStruct.SrcBurstLength = 1;
  DMA_InitStruct.DestBurstLength = 1;
  DMA_InitStruct.SrcDataWidth = LL_DMA_SRC_DATAWIDTH_BYTE;
  DMA_InitStruct.DestDataWidth = LL_DMA_DEST_DATAWIDTH_BYTE;
  DMA_InitStruct.SrcIncMode = LL_DMA_SRC_INCREMENT;
  DMA_InitStruct.DestIncMode = LL_DMA_DEST_FIXED;
  DMA_InitStruct.Priority = LL_DMA_LOW_PRIORITY_LOW_WEIGHT;
  DMA_InitStruct.BlkDataLength = 0x00000000U;
  DMA_InitStruct.TriggerMode = LL_DMA_TRIGM_BLK_TRANSFER;
  DMA_InitStruct.TriggerPolarity = LL_DMA_TRIG_POLARITY_MASKED;
  DMA_InitStruct.TriggerSelection = 0x00000000U;
  DMA_InitStruct.Request = LL_GPDMA1_REQUEST_USART3_TX;
  DMA_InitStruct.TransferEventMode = LL_DMA_TCEM_BLK_TRANSFER;
  DMA_InitStruct.Mode = LL_DMA_NORMAL;
  DMA_InitStruct.SrcAllocatedPort = LL_DMA_SRC_ALLOCATED_PORT1;
  DMA_InitStruct.DestAllocatedPort = LL_DMA_DEST_ALLOCATED_PORT0;
  DMA_InitStruct.LinkAllocatedPort = LL_DMA_LINK_ALLOCATED_PORT1;
  DMA_InitStruct.LinkStepMode = LL_DMA_LSM_FULL_EXECUTION;
  DMA_InitStruct.LinkedListBaseAddr = 0x00000000U;
  DMA_InitStruct.LinkedListAddrOffset = 0x00000000U;
  LL_DMA_Init(GPDMA1, LL_DMA_CHANNEL_0, &DMA_InitStruct);

  /* USER CODE BEGIN USART3_Init 1 */

  LL_DMA_ConfigLinkUpdate(GPDMA1, LL_DMA_CHANNEL_1, LL_DMA_UPDATE_CTR1 | LL_DMA_UPDATE_CTR2 |LL_DMA_UPDATE_CBR1 | LL_DMA_UPDATE_CSAR | LL_DMA_UPDATE_CDAR | LL_DMA_UPDATE_CLLR, (uint32_t)&Node_GPDMA1_Channel1);
  LL_DMA_EnableChannel(GPDMA1, LL_DMA_CHANNEL_1);
  LL_DMA_SetLinkedListAddrOffset(GPDMA1, LL_DMA_CHANNEL_1, LL_DMA_CLLR_OFFSET5);
  LL_USART_EnableDMAReq_RX(USART3);

  LL_DMA_SetDestAddress(GPDMA1, LL_DMA_CHANNEL_0, LL_USART_DMA_GetRegAddr(USART3, LL_USART_DMA_REG_DATA_TRANSMIT));
  LL_USART_EnableDMAReq_TX(USART3);

  /* USER CODE END USART3_Init 1 */
  USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART3, &USART_InitStruct);
  LL_USART_SetTXFIFOThreshold(USART3, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_SetRXFIFOThreshold(USART3, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_EnableDEMode(USART3);
  LL_USART_SetDESignalPolarity(USART3, LL_USART_DE_POLARITY_HIGH);
  LL_USART_SetDEAssertionTime(USART3, 0);
  LL_USART_SetDEDeassertionTime(USART3, 0);
  LL_USART_DisableFIFO(USART3);
  LL_USART_ConfigAsyncMode(USART3);
  LL_USART_Enable(USART3);
  /* USER CODE BEGIN USART3_Init 2 */


  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOH);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOD);

  /**/
  LL_GPIO_ResetOutputPin(GPIOC, SHR_CLK_Pin|SHR_STR_Pin);

  /**/
  LL_GPIO_ResetOutputPin(SHR_DOUT_DISP_GPIO_Port, SHR_DOUT_DISP_Pin);

  /**/
  LL_GPIO_SetOutputPin(CODEC_RST_GPIO_Port, CODEC_RST_Pin);

  /**/
  LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);

  /**/
  GPIO_InitStruct.Pin = SHR_CLK_Pin|SHR_STR_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = CODEC_RST_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(CODEC_RST_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = SHR_DOUT_DISP_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(SHR_DOUT_DISP_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
