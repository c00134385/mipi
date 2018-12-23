/************************************************************
*	ProjectName:	   LT86104EX
*	FileName:	       i2c_master.c
*	BuildData:	     2013-01-03
*	Version£º        V1.3.2
* Company:	       Lontium
************************************************************/
//#include "include.h"
#include "sof_iic.h"
#include "usb_type.h"
#include "lt8918.h"

u8 HDMI_ReadI2C_Byte(u8 RegAddr)
{
	FunctionalState state;
	u8 data = 0x5e;
    
    state = I2C_ReadBytes(&data, 1, RegAddr, Lt8918LAdr);
    if(state != ENABLE) {
        printf("\r\n HDMI_ReadI2C_Byte fail.");
        //FLAG_I2C_ERROR = TRUE;
    }

    return data;
}

bool HDMI_ReadI2C_ByteN(u8 RegAddr,u8 *p_data,u8 N)
{
	bool flag = TRUE;
    FunctionalState state;

    state = I2C_ReadBytes(p_data, N, RegAddr, Lt8918LAdr);
    if(state != ENABLE) {
        printf("\r\n HDMI_ReadI2C_ByteN fail.");
        flag = FALSE;
    }
    	
	return flag;
}


bool HDMI_WriteI2C_Byte(u8 RegAddr, u8 d)
{
	bool flag = TRUE;
    FunctionalState state;

    state = I2C_WriteByte(d, RegAddr, Lt8918LAdr);
    if(state != ENABLE) {
        printf("\r\n HDMI_WriteI2C_Byte fail.");
        flag = FALSE;
    }
    	
	return flag;
}

bool HDMI_WriteI2C_ByteN(u8 RegAddr, u8 *d,u16 N)
{
	bool flag = TRUE;
    FunctionalState state;

    state = I2C_WriteBytes(d, N, RegAddr, Lt8918LAdr);
    if(state != ENABLE) {
        printf("\r\n HDMI_WriteI2C_ByteN fail.");
        flag = FALSE;
    }
    	
	return flag;
}
