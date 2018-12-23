//#include "include.h"
#include "DCS.h"

u8 Buf[0x08] = {0x00,0x00,0x00,0x00,0x08,0x0F,0x0F,0x01};

void DcsLongPktWrite_1(void)
{
	SetRegisterBank(0x83);
	HDMI_WriteI2C_Byte(0x40,0x04);

	HDMI_WriteI2C_Byte(0x32,0x04);
	HDMI_WriteI2C_Byte(0x33,0x08);
	HDMI_WriteI2C_Byte(0x34,Buf[0]);
	HDMI_WriteI2C_Byte(0x34,Buf[1]);
	HDMI_WriteI2C_Byte(0x34,Buf[2]);
	HDMI_WriteI2C_Byte(0x34,Buf[3]);
	HDMI_WriteI2C_Byte(0x34,Buf[4]);
	HDMI_WriteI2C_Byte(0x34,Buf[5]);
	HDMI_WriteI2C_Byte(0x34,Buf[6]);
	HDMI_WriteI2C_Byte(0x34,Buf[7]);
	HDMI_WriteI2C_Byte(0x32,0x00);
}

void DcsLongPktWrite(u8 DataID,u8 Len)
{
	u8 i;
	SetRegisterBank(0x83);
	HDMI_WriteI2C_Byte(0x40,0x04);

	HDMI_WriteI2C_Byte(0x32,0x0E);
	HDMI_WriteI2C_Byte(0x33,(Len+6));
	HDMI_WriteI2C_Byte(0x34,DataID);
	HDMI_WriteI2C_Byte(0x34,Len);
	HDMI_WriteI2C_Byte(0x34,0x00);

	for(i=0;i<Len;i++)
	{
	HDMI_WriteI2C_Byte(0x34,Buf[i]);
	}
	HDMI_WriteI2C_Byte(0x32,0x00);
}

void DcsShortPktWrite(u8 DataID,u8 Data0,u8 Data1)
{   
	SetRegisterBank(0x83);
	HDMI_WriteI2C_Byte(0x40,0x04);

	HDMI_WriteI2C_Byte(0x32,0x0C);
	HDMI_WriteI2C_Byte(0x33,0x04);
	HDMI_WriteI2C_Byte(0x34,DataID);
	HDMI_WriteI2C_Byte(0x34,Data0);
	HDMI_WriteI2C_Byte(0x34,Data1);
	HDMI_WriteI2C_Byte(0x32,0x00);
}

void InitialCode(void)
{
	SetRegisterBank(0x60);
	HDMI_WriteI2C_Byte(0xee,0x01);
	SetRegisterBank(0x70);//  40  ff 70
	HDMI_WriteI2C_Byte(0x23,0xa0); //40 23 a0
	HDMI_WriteI2C_Byte(0x29,0x81);
	HDMI_WriteI2C_Byte(0x38,0x00);
	HDMI_WriteI2C_Byte(0x34,0x49);
	HDMI_WriteI2C_Byte(0x35,0x80);
	SetRegisterBank(0x83);
	HDMI_WriteI2C_Byte(0x11,0x00);
/*
	//ta_array[0] = 0x00010500;  //soft reset						 
    DcsShortPktWrite(0x05,0x01,0x00);//i_set_cmdq(data_array, 1, 1); 

    delay1ms(20);//ELAY(5);
	//data_array[0] = 0x00110500;  //exit sleep mode						 
    DcsShortPktWrite(0x05,0x11,0x00);//i_set_cmdq(data_array, 1, 1); 

    delay1ms(120); //MDELAY(120);   
    
	//data_array[0] = 0x04B02300;  //MCAP						 
    DcsShortPktWrite(0x23,0xb0,0x04);//dsi_set_cmdq(data_array, 1, 1); 

	//data_array[0] = 0x01D62300;  //				 
    DcsShortPktWrite(0x23,0xd6,0x01);//dsi_set_cmdq(data_array, 1, 1); 

    //data_array[0] = 0x00062902;  //interface setting	
    //data_array[1] = 0x000814B3;  //5 paras  04-->14
	//data_array[2] = 0x00000022;  	
    //dsi_set_cmdq(data_array, 3, 1); 
	Buf[0] = 0xb3;Buf[1] = 0x14;Buf[2] = 0x08;Buf[3] = 0x00;
	Buf[4] = 0x22;;Buf[5] = 0x00;
	DcsLongPktWrite(0x29,6);

    //data_array[0] = 0x00022902;  //interface ID setting
    //data_array[1] = 0x00000CB4;  
    //dsi_set_cmdq(data_array, 2, 1); 
	Buf[0] = 0xb4;Buf[1] = 0x0c;
	DcsLongPktWrite(0x29,2);

	//data_array[0] = 0x00032902;  //DSI control
    //data_array[1] = 0x00D33AB6;     //D3
    Buf[0] = 0xb6;Buf[1] = 0x3a;Buf[2] = 0x33;
	DcsLongPktWrite(0x29,3);//dsi_set_cmdq(data_array, 2, 1); 

    //data_array[0] = 0x773A1500;  //set pixel format  
    DcsShortPktWrite(0x15,0x3a,0x77);//dsi_set_cmdq(data_array, 1, 1); 
   
	
    //data_array[0] = 0x00351500;  //					 
    DcsShortPktWrite(0x15,0x35,0x00);//dsi_set_cmdq(data_array, 1, 1); 

    //data_array[0] = 0x00290500;  //set display on					 
    DcsShortPktWrite(0x05,0x29,0x00);//dsi_set_cmdq(data_array, 1, 1); 
    */
    DcsShortPktWrite(0x05,0x11,0x00);//0X13
	DcsShortPktWrite(0x05,0x29,0x00);
	
}

