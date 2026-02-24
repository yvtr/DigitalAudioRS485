/***************************************************************************//**
* @file  i2c_master.c
* @brief I2C master functions for STM32H5 using LL library
*//****************************************************************************/

//------------------------------C library---------------------------------------
#include <stdint.h>
//----------------------------user includes-------------------------------------
#include "i2c_master.h"
#include "main.h"
//------------------------------------------------------------------------------


/* Private macros ------------------------------------------------------------*/
#define I2C_TIMEOUT_US     5000


// SOFTEND in the write section prevents STOP --> Restart possible
// AUTOEND in the read section generates correct STOP
// TXIS/RXNE polling is mandatory for H5
// STOP-flag-clear is necessary, otherwise the next transaction will hang.


/* Functions -----------------------------------------------------------------*/

static int I2C_Tout(I2C_TypeDef* I2Cx, int val) {
   LL_I2C_GenerateStopCondition(I2Cx);
   LL_I2C_Disable(I2Cx);
   LL_I2C_Enable(I2Cx);
   LL_I2C_ClearFlag_NACK(I2Cx);
   LL_I2C_ClearFlag_STOP(I2Cx);
   LL_I2C_ClearFlag_ADDR(I2Cx);
   return val;
}

/***************************************************************************//**
* @brief  Write data to I2C slave device
* @param  I2Cx: I2C peripheral (e.g. I2C1)
* @param  addr: 7-bit I2C slave address, left-aligned
* @param  data: pointer to data buffer to write
* @param  len:  number of bytes to write
* @return 0 on success, -1 on TXIS timeout, -2 on STOP timeout
*//****************************************************************************/
int I2C_Write(I2C_TypeDef* I2Cx, uint8_t addr, const uint8_t* data, uint16_t len) {
   LL_I2C_HandleTransfer(I2Cx, addr, LL_I2C_ADDRSLAVE_7BIT, len, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
   for (uint16_t i = 0; i < len; i++) {
      uint32_t tref = usTimerGetAbs();
      while (!LL_I2C_IsActiveFlag_TXIS(I2Cx)) {
         if (usTimerGetRel(tref) > I2C_TIMEOUT_US) {  // timeout expired
             return I2C_Tout(I2Cx, -1);               // error: TXIS timeout
         }
      }
      LL_I2C_TransmitData8(I2Cx, data[i]);         // send data
   }
   uint32_t tref = usTimerGetAbs();
   while (!LL_I2C_IsActiveFlag_STOP(I2Cx)) {
      if (usTimerGetRel(tref) > I2C_TIMEOUT_US) {  // timeout expired
         return I2C_Tout(I2Cx, -2);                // error: STOP timeout
      }
   }
   LL_I2C_ClearFlag_STOP(I2Cx);
   return 0;
}

/***************************************************************************//**
* @brief  Read data from I2C slave device
* @param  I2Cx: I2C peripheral (e.g. I2C1)
* @param  addr: 7-bit I2C slave address, left-aligned
* @param  data: pointer to data buffer to store read data
* @param  len:  number of bytes to read
* @return 0 on success, -1 on RXNE timeout, -2 on STOP timeout
*//****************************************************************************/
int I2C_Read(I2C_TypeDef* I2Cx, uint8_t addr, uint8_t* data, uint16_t len) {
   LL_I2C_HandleTransfer(I2Cx, addr, LL_I2C_ADDRSLAVE_7BIT, len, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);
   for (uint16_t i = 0; i < len; i++) {
      uint32_t tref = usTimerGetAbs();
      while (!LL_I2C_IsActiveFlag_RXNE(I2Cx)) {
         if (usTimerGetRel(tref) > I2C_TIMEOUT_US) {  // timeout expired
             return I2C_Tout(I2Cx, -1);               // error: RXNE timeout
         }
      }
      data[i] = LL_I2C_ReceiveData8(I2Cx);         // read data
   }
   uint32_t tref = usTimerGetAbs();
   while (!LL_I2C_IsActiveFlag_STOP(I2Cx)) {
      if (usTimerGetRel(tref) > I2C_TIMEOUT_US) {  // timeout expired
         return I2C_Tout(I2Cx, -2);                // error: STOP timeout
      }
   }
   LL_I2C_ClearFlag_STOP(I2Cx);
   return 0;
}

/***************************************************************************//**
* @brief  Read data from I2C slave device register
* @param  I2Cx: I2C peripheral (e.g. I2C1)
* @param  addr: 7-bit I2C slave address, left-aligned
* @param  reg: pointer to register address buffer (1-2 bytes)
* @param  reglen: number of bytes in register address (1 or 2)
* @param  data: pointer to data buffer to store read data
* @param  datalen: number of bytes to read
* @return 0 on success, -1 on TXIS timeout, -2 on RXNE timeout, -3 on STOP timeout
*//****************************************************************************/
int I2C_ReadReg(I2C_TypeDef* I2Cx, uint8_t addr, const uint8_t* reg, uint16_t reglen, uint8_t* data, uint16_t datalen) {
   LL_I2C_HandleTransfer(I2Cx, addr, LL_I2C_ADDRSLAVE_7BIT, reglen, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);
   for (uint16_t i = 0; i < reglen; i++) {
      uint32_t tref = usTimerGetAbs();
      while (!LL_I2C_IsActiveFlag_TXIS(I2Cx)) {
         if (usTimerGetRel(tref) > I2C_TIMEOUT_US) {  // timeout expired
             return I2C_Tout(I2Cx, -1);               // error: TXIS timeout
         }
      }
      LL_I2C_TransmitData8(I2Cx, reg[i]);             // send register address
   }
   LL_I2C_HandleTransfer(I2Cx, addr, LL_I2C_ADDRSLAVE_7BIT, datalen, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_RESTART_7BIT_READ);
   for (uint16_t i = 0; i < datalen; i++) {
      uint32_t tref = usTimerGetAbs();
      while (!LL_I2C_IsActiveFlag_RXNE(I2Cx)) {
         if (usTimerGetRel(tref) > I2C_TIMEOUT_US) {  // timeout expired
             return I2C_Tout(I2Cx, -2);               // error: RXNE timeout
         }
      }
      data[i] = LL_I2C_ReceiveData8(I2Cx);         // read data
   }
   uint32_t tref = usTimerGetAbs();
   while (!LL_I2C_IsActiveFlag_STOP(I2Cx)) {
      if (usTimerGetRel(tref) > I2C_TIMEOUT_US) {  // timeout expired
         return I2C_Tout(I2Cx, -3);                // error: STOP timeout
      }
   }
   LL_I2C_ClearFlag_STOP(I2Cx);
   return 0;
}

