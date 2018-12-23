/************************************************************
*	ProjectName:	   LT86104EX
*	FileName:	       i2c_master.h
*	BuildData:	     2013-01-03
*	Version£º        V1.3.2
* Company:	       Lontium
************************************************************/
	
//#include  "include.h"

#ifndef		_I2C_MASTER_H
#define		_I2C_MASTER_H

#include "stm32f10x.h"

typedef enum
{
	lt8619,
	lt8918
}
chiptype;

extern  bool   FLAG_I2C_ERROR;
extern  u8   I2CADR;
extern  GPIO_TypeDef      *I2C_master_Port;
extern  GPIO_Pin_TypeDef  I2C_master_SDA;
extern  GPIO_Pin_TypeDef  I2C_master_SCL;

#if 0
extern void ChipI2cSelect(chiptype chip);
extern  Pin_Status GET_GPIO_InputPin(GPIO_TypeDef* GPIOx, GPIO_Pin_TypeDef GPIO_Pin);
extern void GPIO_Write_Pin(GPIO_TypeDef* GPIOx, GPIO_Pin_TypeDef PortPins,Pin_Status Pin_set_val);
extern  void Delay_ms(u16 mscount);
extern  void delay_us(u8 uscount);
extern void delay1us(u8 uscount);
#endif
extern  u8   HDMI_ReadI2C_Byte(u8 RegAddr);
extern  bool HDMI_ReadI2C_ByteN(u8 RegAddr,u8 *p_data,u8 N);
extern  bool HDMI_WriteI2C_Byte(u8 RegAddr, u8 d);
extern  bool HDMI_WriteI2C_ByteN(u8 RegAddr, u8 *d,u16 N);
void LT8668_DEVICE_SEL(void);
void LT8911_DEVICE_SEL(void);

#endif

