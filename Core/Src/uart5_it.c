/***************************************************************************//**
* @file  uart5_it.c
* @brief UART with interrupt and ring buffer
*//****************************************************************************/

//------------------------------C library---------------------------------------
#include <stdlib.h>
//----------------------------user includes-------------------------------------
#include "uart5_it.h"
#include "uart5_it_cfg.h"
//------------------------------------------------------------------------------




/* Private typedefs ----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

//====== UART buffer defines ===================================================
#define  UART5_RXBUF_MASK    (UART5_RXBUF_SIZE - 1)
#define  UART5_TXBUF_MASK    (UART5_TXBUF_SIZE - 1)

#if ( UART5_RXBUF_SIZE & UART5_RXBUF_MASK )
#error RX buffer size is not a power of 2!
#endif

#if ( UART5_TXBUF_SIZE & UART5_TXBUF_MASK )
#error TX buffer size is not a power of 2!
#endif


/* Private variables ---------------------------------------------------------*/

//====== Variables for transmitter =============================================
static          uint8_t  Uart5TxBuf[UART5_TXBUF_SIZE];
static          uint16_t Uart5TxWrIdx = 0;
static volatile uint16_t Uart5TxRdIdx = 0;

//====== Variables for Receiver ================================================
static          uint8_t  Uart5RxBuf[UART5_RXBUF_SIZE];
static volatile uint16_t Uart5RxWrIdx = 0;
static volatile uint16_t Uart5RxRdIdx = 0;


/* Public variables ----------------------------------------------------------*/


/* Functions -----------------------------------------------------------------*/


/***************************************************************************//**
* @brief  UART5 init
*//****************************************************************************/
void Uart5_Init(void) {
   LL_USART_EnableIT_RXNE(UART5);   // Enable RX interrupt
}


/***************************************************************************//**
* @brief  UART5 interrupt handler
*//****************************************************************************/
void UART5_IRQHandler(void) {
   if (LL_USART_IsActiveFlag_RXNE_RXFNE(UART5)) {        // RX interrupt
      uint8_t d = LL_USART_ReceiveData8(UART5);          // read received byte
      uint_fast16_t wr = Uart5RxWrIdx;
      wr = (wr + 1) & UART5_RXBUF_MASK;                  // new write index
      Uart5RxBuf[wr] = d;                                // received data to buffer
      Uart5RxWrIdx = wr;                                 // Store new index
   }

   if (LL_USART_IsActiveFlag_TXE_TXFNF(UART5)) {         // TX interrupt
      uint_fast16_t rd = Uart5TxRdIdx;
      uint_fast16_t wr = Uart5TxWrIdx;
      if (wr != rd) {                                    // data in buffer
         rd = (rd + 1) & UART5_TXBUF_MASK;               // new read index
         LL_USART_TransmitData8(UART5, Uart5TxBuf[rd]);  // send data byte
         Uart5TxRdIdx = rd;                              // Store new index
      }
      if (wr == rd) {                                    // data buffer empty
         LL_USART_DisableIT_TXE(UART5);                  // INT disable
      }
   }
}


/***************************************************************************//**
* @brief  Send byte to UART
* @param  d: byte to send
*//****************************************************************************/
void Uart5_PutByte(uint8_t d) {
   LL_USART_DisableIT_TXE_TXFNF(UART5);   // Interrupt disable during buffer update
   uint_fast16_t wr = Uart5TxWrIdx;
   wr = (wr + 1) & UART5_TXBUF_MASK;      // new write index
   if (wr == Uart5TxRdIdx) {              // No free space in buffer (overflow)
      return;                             // drop remaining data (Attention! Buffer overflow not signaled! Use bigger buffer)
   }
   Uart5TxBuf[wr] = d;                    // Store data in buffer
   Uart5TxWrIdx = wr;                     // Store new index
   LL_USART_EnableIT_TXE_TXFNF(UART5);    // Interrupt enable (start send)
}


/***************************************************************************//**
* @brief  Send more data to UART
* @param  src: data to send
* @param  n:   count of data
*//****************************************************************************/
void Uart5_PutData (const void* src, uint16_t n) {
   LL_USART_DisableIT_TXE_TXFNF(UART5);   // Interrupt disable during buffer update
   const uint8_t* p = src;
   uint_fast16_t wr = Uart5TxWrIdx;
   while (n) {
      wr = (wr + 1) & UART5_TXBUF_MASK;   // new write index
      if (wr == Uart5TxRdIdx) {           // No free space in buffer (overflow)
         break;                           // drop remaining data (Attention! Buffer overflow not signaled! Use bigger buffer)
      }
      Uart5TxBuf[wr] = *p;                // Store data in buffer
      p++;
      n--;
   }
   Uart5TxWrIdx = wr;                     // Store new index
   LL_USART_EnableIT_TXE_TXFNF(UART5);    // Interrupt enable (start send)
}

/***************************************************************************//**
* @brief  Read data from UART RX buffer
* @return received character, -1: no data in RX buffer
*//****************************************************************************/
int16_t Uart5_GetByte(void) {
   uint_fast16_t rd = Uart5RxRdIdx;
   if (rd != Uart5RxWrIdx) {              // new data in buffer
      rd = (rd + 1) & UART5_RXBUF_MASK;   // new read index
      Uart5RxRdIdx = rd;                  // store new index
      return Uart5RxBuf[rd];              // return with data
   }else {
      return -1;                          // buffer empty
   }
}

