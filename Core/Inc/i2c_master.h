#ifndef __I2C_MASTER_H__
#define __I2C_MASTER_H__

#include <stdint.h>
#include <stm32h5xx_ll_i2c.h>


extern int I2C_Write(I2C_TypeDef* I2Cx, uint8_t addr, const uint8_t* data, uint16_t len);
extern int I2C_Read(I2C_TypeDef* I2Cx, uint8_t addr, uint8_t* data, uint16_t len);
extern int I2C_ReadReg(I2C_TypeDef* I2Cx, uint8_t addr, const uint8_t* reg, uint16_t reglen, uint8_t* data, uint16_t datalen);


#endif
