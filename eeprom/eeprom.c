/*----------------------------------------------------------------------------*/
/* eeprom.c                                                                   */
/* 描述:sony 变倍机型通讯程序                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* 2017/10/17 yihuagang建立 V0.1                                                  */
/* 2017/10/17   */
/* 2017/10/17 yihuagang  v0.3                                                      */
/*            */
/*  */
/*                     */
/*                               */
/*   */
/*                      */
/*                                                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/


/* include */
#include "includes.h"
#include "at24c02.h"

/* macro */
#define EEPROM_DEVICE_ADDR (AT24C02_ADDR)


/* constant */
/**********************/
/* Common constant */
/**********************/



/* variable */





/* implementation */
int eeprom_init(void)
{
    I2C_Configuration();
    
    
	return 0;
}

int eeprom_write(unsigned char addr, unsigned char* data, int len)
{
    FunctionalState state;
    int i = 0;
    #if 1
    while(i < len)
    {
        state = I2C_WriteByte(data[i], addr + i, EEPROM_DEVICE_ADDR);
        if(state != ENABLE) {
            break;
        }
        i++;
        Wait10Ms(2);
    }
    #else
    state = I2C_WriteBytes(data, len, addr, EEPROM_DEVICE_ADDR);
    #endif
    return i;
}

int eeprom_read(unsigned char addr, unsigned char* data, int len)
{
    #if 0
    FunctionalState state;
    int i = 0;
    while(i < len)
    {
        state = I2C_ReadByte(&data[i], 1, addr + i, EEPROM_DEVICE_ADDR);
        if(state != ENABLE) {
            break;
        }
        i++;
    }
    return i;
    #endif
    FunctionalState state;
    state = I2C_ReadBytes(data, len, addr, EEPROM_DEVICE_ADDR);
    if(state != ENABLE) {
        return 0;
    }
    return len;
}


