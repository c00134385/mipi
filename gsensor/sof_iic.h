/****************************************Copyright (c)**************************************************                         
**
**                                 http://www.powermcu.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			24C02.h
** Descriptions:		24C02 ���������� 
**
**------------------------------------------------------------------------------------------------------
** Created by:			AVRman
** Created date:		2010-10-29
** Version:				1.0
** Descriptions:		The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:			
** Modified date:	
** Version:
** Descriptions:		
********************************************************************************************************/
#ifndef __24C02_H
#define __24C02_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Private define ------------------------------------------------------------*/
#define SCL_H         GPIOB->BSRR = GPIO_Pin_6 /* GPIO_SetBits(GPIOB , GPIO_Pin_10)   */
#define SCL_L         GPIOB->BRR  = GPIO_Pin_6  /* GPIO_ResetBits(GPIOB , GPIO_Pin_10) */
   
#define SDA_H         GPIOB->BSRR = GPIO_Pin_7  /* GPIO_SetBits(GPIOB , GPIO_Pin_11)   */
#define SDA_L         GPIOB->BRR  = GPIO_Pin_7   /* GPIO_ResetBits(GPIOB , GPIO_Pin_11) */

#define SCL_read      GPIOB->IDR  & GPIO_Pin_6  /* GPIO_ReadInputDataBit(GPIOB , GPIO_Pin_10) */
#define SDA_read      GPIOB->IDR  & GPIO_Pin_7  /* GPIO_ReadInputDataBit(GPIOB , GPIO_Pin_11) */

#define I2C_PageSize  8  /* 24C02ÿҳ8�ֽ� */

#define ADDR_24LC02		0xA0

/* Private function prototypes -----------------------------------------------*/
void I2C_Configuration(void);
FunctionalState I2C_WriteByte(uint8_t SendByte, uint16_t WriteAddress, uint8_t DeviceAddress);
FunctionalState I2C_WriteBytes(uint8_t* pBuffer, uint16_t length, uint16_t WriteAddress,  uint8_t DeviceAddress);         
FunctionalState I2C_ReadByte(uint8_t* ReadByte, uint16_t ReadAddress, uint8_t DeviceAddress);
FunctionalState I2C_ReadBytes(uint8_t* pBuffer,   uint16_t length,   uint16_t ReadAddress,  uint8_t DeviceAddress);

#endif 
/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
