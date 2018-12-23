/************************************************************
 * Copyright (C), 2009-2011, Donjin Tech. Co., Ltd.
 * FileName:		// 文件名
 * Author:			// 作者
 * Date:			// 日期
 * Description:		// 模块描述
 * Version:			// 版本信息
 * Function List:	// 主要函数及其功能
 *     1. -------
 * History:			// 历史修改记录
 *     <author>  <time>   <version >   <desc>
 *     David    96/10/12     1.0     build this moudle
 ***********************************************************/
//#include "include.h"
#include "lt8918.h"

Resolution_Type LastVideoFormat = SignnalOff;
Resolution_Type VideoFormat = VESA_1920x1080_30;

bool Pattern_en = 0 ;

/*
LQ055T3SX02
HS=8 HBP=16 HFP=72 Hact=1080 Htotal=1176
VS=2 VBP=6  VFP=14 Vact=1920 Vtotal=1942
*/
#define Pattern_Mode		0x3f//Blue Screen

#define Pattern_Head		0xff    //
#define Pattern_de_dlyH		0x00    //MSB hs+hbp 16+45=61
#define Pattern_de_dlyL		0x3D    //LSB
#define Pattern_de_Top		0x08    // vs+vbp 4+4
#define Pattern_de_CntH		0x04    //MSB x 1200
#define Pattern_de_CntL		0xB0    //LSB
#define Pattern_de_LineH	0x07    //MSB y 1920
#define Pattern_de_LineL	0x80    //LSB
#define Pattern_HtotalH		0x05    //MSB x+hs+bfp+hbp
#define Pattern_HtotalL		0x7d    //LSB 1200+16+45+144=1405
#define Pattern_VtotalH		0x07    //MSB y+vs+vfp+vbp
#define Pattern_VtotalL		0x90    //LSB 1920+4+4+8=1936
#define Pattern_HswH		0x00    //MSB hs 16
#define Pattern_HswL		0x10    //LSB
#define Pattern_Vsw			0x04    // vs 4
#define Pattern_HfpH		0x00    //MSB hfp 144
#define Pattern_HfpL		0x90    //LSB
#define Pattern_Hbp			0x2d    // hbp 45
#define Pattern_Vfp			0x08    // vfp 8
#define Pattern_Vbp			0x04    // vbp 4

/*
#define Pattern_HfpH	0x00
#define Pattern_HfpL	0x64        // 100
#define Pattern_Hbp		0x32        // 50
#define Pattern_Vfp		0x04        // 4
#define Pattern_Vbp		0x04        // 4
*/

#define Ptr_HBlank 0x0a
//27-->dly_cnt=012e
//25-->dly_cnt=011f



/*
#define Pattern_Head		0xff
#define Pattern_de_dlyH		0x00
#define Pattern_de_dlyL		0x06    // 6+0
#define Pattern_de_Top		0x08    // 3+5
#define Pattern_de_CntH		0x04
#define Pattern_de_CntL		0x38    // 1080
#define Pattern_de_LineH	0x07
#define Pattern_de_LineL	0x80    // 1920
#define Pattern_HtotalH		0x04
#define Pattern_HtotalL		0x42    // 1090
#define Pattern_VtotalH		0x09
#define Pattern_VtotalL		0xca    // 2506
#define Pattern_HswH		0x00
#define Pattern_HswL		0x06    // 6
#define Pattern_Vsw			0x06    // 3

#define Pattern_HfpH	0x00
#define Pattern_HfpL	0x04        // 4
#define Pattern_Hbp		0x00        // 0
#define Pattern_Vfp		0xc8        // 200
#define Pattern_Vbp		0x06        // 5

#define Ptr_HBlank 0x0a
//27-->dly_cnt=012e
//25-->dly_cnt=011f
*/
const u8	VideoFormatTab[1][15] = { 
{
 Pattern_Head, Pattern_de_dlyH, Pattern_de_dlyL, Pattern_de_Top, 
 Pattern_de_CntH, Pattern_de_CntL, Pattern_de_LineH, Pattern_de_LineL, 
 Pattern_HtotalH, Pattern_HtotalL, Pattern_VtotalH, Pattern_VtotalL, 
 Pattern_HswH, Pattern_HswL, Pattern_Vsw
} 
};

const u8	LVDSCK_DLY[7] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };

unsigned long lt8918_task_t_sec = 0;  ////单位秒


//extern bool Pattern_en;

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
void SetRegisterBank( u8 adr )
{
	HDMI_WriteI2C_Byte( 0xff, adr );
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
void Lt8918_CfgI2cEnable( void )
{
	SetRegisterBank( 0x60 );
	HDMI_WriteI2C_Byte( 0xee, 0x01 );   //cfg iic enable
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
void Lt8918_GpioEnable( void )
{
	SetRegisterBank( 0x70 );
	HDMI_WriteI2C_Byte( 0x42, 0x09 );   //Digital test output enable:
	HDMI_WriteI2C_Byte( 0x44, 0xff );   //GPIO7~GPIO0 output enable:
	HDMI_WriteI2C_Byte( 0x46, 0xff );   //GPIO7~GPIO0 output driving capability:
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
void Lt8918_GpoConfig( void )
{
	SetRegisterBank( 0x80 );
	HDMI_WriteI2C_Byte( 0x70, 0x00 );
	HDMI_WriteI2C_Byte( 0x71, 0x00 );   //GPO4~7 output select 00 = Blk_test_out[3];
	HDMI_WriteI2C_Byte( 0x72, 0x00 );   //GPO0~3 output select 00 = Blk_test_out[3];
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
void Lt8918_DesscPllInit( void )        //Bypass Mode
{
	SetRegisterBank( 0x70 );
	HDMI_WriteI2C_Byte( 0x38, 0x00 );
	SetRegisterBank( 0x80 );
	HDMI_WriteI2C_Byte( 0xa1, 0x00 );
	
	//b7 PLL divide ratio source selction :1 = PLL divide ratio from DIV_RATIO_EXT.
	//b6:0 Set divider ratio of acr audio pll
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
void Lt8918_DesscFreqSet( void )
{
	SetRegisterBank( 0x80 );
	HDMI_WriteI2C_Byte( 0xa5, 0xAA );
	HDMI_WriteI2C_Byte( 0xa9, 0x97 );
	HDMI_WriteI2C_Byte( 0xaa, 0xc2 );
	HDMI_WriteI2C_Byte( 0xab, 0x8f );
	HDMI_WriteI2C_Byte( 0xac, 0x5c );
	HDMI_WriteI2C_Byte( 0xad, 0x00 );
	HDMI_WriteI2C_Byte( 0xad, 0x02 );
	/*
	HDMI_WriteI2C_Byte(0xA5,0xaa);
	HDMI_WriteI2C_Byte(0xA9,0x9a);
	HDMI_WriteI2C_Byte(0xAA,0x3d);
	HDMI_WriteI2C_Byte(0xAB,0x70);
	HDMI_WriteI2C_Byte(0xAC,0xa3);
	HDMI_WriteI2C_Byte(0xAD,0x00);
	HDMI_WriteI2C_Byte(0xAD,0x02);
	*/
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
void Lt8918_InputConfig( void )
{
	SetRegisterBank(0x80);
	HDMI_WriteI2C_Byte(0x02,0x70);// modify for uncommon PCB design. default: 0
	//b4:6 RGB channel swap select: 111 = D[23:0] = RGB.
	 
    switch(VideoFormat) {
        case VESA_1920x1080_60:
        case VESA_1920x1080_50:
        case VESA_1920x1080_30:
        case VESA_1920x1080_25:
            HDMI_WriteI2C_Byte(0x03,0x02); 
            HDMI_WriteI2C_Byte(0x04,0x07);
            HDMI_WriteI2C_Byte(0x05,0x80);
            HDMI_WriteI2C_Byte(0x06,0x00);
            HDMI_WriteI2C_Byte(0x07,0x58);
            HDMI_WriteI2C_Byte(0x08,0x00);
            HDMI_WriteI2C_Byte(0x09,0x2C);
            HDMI_WriteI2C_Byte(0x0a,0x00);
            HDMI_WriteI2C_Byte(0x0b,0x04);
            HDMI_WriteI2C_Byte(0x0c,0x00);
        	HDMI_WriteI2C_Byte(0x0d,0x05);
            break;
        case VESA_1920x1080I_60:
        case VESA_1920x1080I_50:
            HDMI_WriteI2C_Byte(0x03,0x06); 
            HDMI_WriteI2C_Byte(0x04,0x07);
            HDMI_WriteI2C_Byte(0x05,0x80);
            HDMI_WriteI2C_Byte(0x06,0x00);
            HDMI_WriteI2C_Byte(0x07,0x58);
            HDMI_WriteI2C_Byte(0x08,0x00);
            HDMI_WriteI2C_Byte(0x09,0x2C);
            HDMI_WriteI2C_Byte(0x0a,0x00);
            HDMI_WriteI2C_Byte(0x0b,0x05);
            HDMI_WriteI2C_Byte(0x0c,0x00);
	        HDMI_WriteI2C_Byte(0x0d,0x05);
            break;
        case VESA_1280x720_60:
        case VESA_1280x720_50:
        case VESA_1280x720_30:
        case VESA_1280x720_25:
            HDMI_WriteI2C_Byte(0x03,0x02); 
            HDMI_WriteI2C_Byte(0x04,0x05);
        	HDMI_WriteI2C_Byte(0x05,0x00);
        	HDMI_WriteI2C_Byte(0x06,0x00);
        	HDMI_WriteI2C_Byte(0x07,0x6e);
        	HDMI_WriteI2C_Byte(0x08,0x00);
        	HDMI_WriteI2C_Byte(0x09,0x28);
        	HDMI_WriteI2C_Byte(0x0a,0x00);
        	HDMI_WriteI2C_Byte(0x0b,0x14);
            HDMI_WriteI2C_Byte(0x0c,0x00);
	        HDMI_WriteI2C_Byte(0x0d,0x05);
            break;
    }
    
	
	
	HDMI_WriteI2C_Byte(0x0e,0x00);
	HDMI_WriteI2C_Byte(0x0f,0x00);
	HDMI_WriteI2C_Byte(0x6a,0x00);
	HDMI_WriteI2C_Byte(0x6b,0x00);
	SetRegisterBank(0x80);
	HDMI_WriteI2C_Byte(0x33,0x2C);
	HDMI_WriteI2C_Byte(0xbe,0x40);

	

}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
void Lt8918_TxPllConfig( void )
{
    #if 0 // for pattern
	SetRegisterBank( 0x70 );
	HDMI_WriteI2C_Byte( 0x30, 0x00 );
	HDMI_WriteI2C_Byte( 0x31, 0x2c );
	HDMI_WriteI2C_Byte( 0x33, 0x32 );
	HDMI_WriteI2C_Byte( 0x34, 0x02 );
	HDMI_WriteI2C_Byte( 0x24, 0x44 );	
	HDMI_WriteI2C_Byte( 0x23, 0x2f );
	HDMI_WriteI2C_Byte( 0x23, 0xaf );
	HDMI_WriteI2C_Byte( 0x23, 0x9f );
    #else

	if(Pattern_en)
	{
		SetRegisterBank( 0x70 );
		HDMI_WriteI2C_Byte( 0x30, 0x00 );
		HDMI_WriteI2C_Byte( 0x31, 0x2c );
		HDMI_WriteI2C_Byte( 0x33, 0x32 );
		HDMI_WriteI2C_Byte( 0x34, 0x02 );
		HDMI_WriteI2C_Byte( 0x24, 0x44 );	
		HDMI_WriteI2C_Byte( 0x23, 0x2f );
		HDMI_WriteI2C_Byte( 0x23, 0xaf );
		HDMI_WriteI2C_Byte( 0x23, 0x9f );
		return;
	}
	
    SetRegisterBank( 0x70 );
    HDMI_WriteI2C_Byte( 0x30, 0x02 );
	HDMI_WriteI2C_Byte( 0x31, 0x2c );

    switch(VideoFormat) {
        case VESA_1920x1080_60:
            HDMI_WriteI2C_Byte( 0x33, 0x32 );    // 1080P@60:
            HDMI_WriteI2C_Byte( 0x34, 0x02 );
			HDMI_WriteI2C_Byte( 0x24, 0x44 );	
            break;
        case VESA_1920x1080_50:
            HDMI_WriteI2C_Byte( 0x33, 0x32 );    // 1080P@50:32
            HDMI_WriteI2C_Byte( 0x34, 0x02 );
			HDMI_WriteI2C_Byte( 0x24, 0x44 );	
            break;
        case VESA_1920x1080_30:
            HDMI_WriteI2C_Byte( 0x33, 0x1b);    // 1080P@30
            HDMI_WriteI2C_Byte( 0x34, 0x02 );
			HDMI_WriteI2C_Byte( 0x24, 0x44 );	
            break;
        case VESA_1920x1080_25:
            HDMI_WriteI2C_Byte( 0x33, 0x1b );    // 1080P@25:32
            HDMI_WriteI2C_Byte( 0x34, 0x02 );
			HDMI_WriteI2C_Byte( 0x24, 0x44 );	
            break;
        case VESA_1920x1080I_60:
            HDMI_WriteI2C_Byte( 0x33, 0x32 );    // 1080P@25:32
            HDMI_WriteI2C_Byte( 0x34, 0x02 );
			HDMI_WriteI2C_Byte( 0x24, 0x44 );	
            break;
        case VESA_1920x1080I_50:
            HDMI_WriteI2C_Byte( 0x33, 0x32 );    // 1080P@25:32
            HDMI_WriteI2C_Byte( 0x34, 0x02 );
			HDMI_WriteI2C_Byte( 0x24, 0x44 );	
            break;
        case VESA_1280x720_60:
            HDMI_WriteI2C_Byte( 0x33, 0x1b );    // 720P@60:32
            HDMI_WriteI2C_Byte( 0x34, 0x02 );
			HDMI_WriteI2C_Byte( 0x24, 0x44 );	
            break;
        case VESA_1280x720_50:
            HDMI_WriteI2C_Byte( 0x33, 0x1b );    // 720P@50:32
            HDMI_WriteI2C_Byte( 0x34, 0x02 );
			HDMI_WriteI2C_Byte( 0x24, 0x44 );	
            break;
        case VESA_1280x720_30:
            HDMI_WriteI2C_Byte( 0x33, 0x1b );    // 720P@30:32
            HDMI_WriteI2C_Byte( 0x34, 0x02 );
			HDMI_WriteI2C_Byte( 0x24, 0x44 );	
            break;
        case VESA_1280x720_25:
            HDMI_WriteI2C_Byte( 0x33, 0x0c );    // 720P@25:32
            HDMI_WriteI2C_Byte( 0x34, 0x02 );
			HDMI_WriteI2C_Byte( 0x24, 0x64 );	
            break;   
        default:
            HDMI_WriteI2C_Byte( 0x33, 0x1b );    // 1080P@25:32
            HDMI_WriteI2C_Byte( 0x34, 0x02 );
			HDMI_WriteI2C_Byte( 0x24, 0x44 );	
            break;
    }

	
	HDMI_WriteI2C_Byte( 0x23, 0x2f );
	HDMI_WriteI2C_Byte( 0x23, 0xaf );
	HDMI_WriteI2C_Byte( 0x23, 0x9f );

    #endif
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
void Lt8918_OutputConfig( void )
{
	SetRegisterBank(0x70);

    //b6 Mipi/lvds/sub-lvds tx mode select: 1 = Mipi mode.
    //b5 Mipi/lvds/sub-lvds p2s mode select: 1 = Ddr mode.
	HDMI_WriteI2C_Byte(0x23,0x2f);
	HDMI_WriteI2C_Byte(0x23,0xaf);
	HDMI_WriteI2C_Byte(0x23,0x9f);
    //b7 Mipi/lvds/sub-lvds tx soft reset:
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
#if 0
Timing_Param original = {
    44,     //u16 hs;
    148,    //u16 hbp;
    1920,    //u16 hactive;
    99,     //u16 hfp;
    
    5,     //u16 vs;
    36,    //u16 vbp;
    1080,    //u16 vactive;
    5,     //u16 vfp;
};
#endif


Timing_Param p1 = {
    44,     //u16 hs;
    148,    //u16 hbp;
    1920,    //u16 hactive;
    88,     //u16 hfp;
    
    5,     //u16 vs;
    37,    //u16 vbp;
    1080,    //u16 vactive;
    3,     //u16 vfp;
};


void Lt8918_PtnDataConfig( void )
{
    #if 1
	SetRegisterBank( 0x80 );                                        //Tx pattern
	HDMI_WriteI2C_Byte( 0xbe, 0xc0 );
	HDMI_WriteI2C_Byte( 0xbf, Pattern_Mode );//01 02 04 37 3f
	HDMI_WriteI2C_Byte( 0xc0, 0xff );
	HDMI_WriteI2C_Byte( 0xc1, 0x00 );
	HDMI_WriteI2C_Byte( 0xc2, 0xc0 );     //de_dly:192
	HDMI_WriteI2C_Byte( 0xc3, 0x29 );     //de_top:41
	HDMI_WriteI2C_Byte( 0xc4, 0x07 );
	HDMI_WriteI2C_Byte( 0xc5, 0x80 );     //de_cnt:1920
	HDMI_WriteI2C_Byte( 0xc6, 0x04 );
	HDMI_WriteI2C_Byte( 0xc7, 0x38 );     //de_lin:1080
	HDMI_WriteI2C_Byte( 0xc8, 0x08 );
	HDMI_WriteI2C_Byte( 0xc9, 0x98 );     //h_total:2200
	HDMI_WriteI2C_Byte( 0xca, 0x04 );
	HDMI_WriteI2C_Byte( 0xcb, 0x65 );    //v_total:1125
	HDMI_WriteI2C_Byte( 0xcc, 0x00 );
	HDMI_WriteI2C_Byte( 0xcd, 0x2c );    //hwidth:44
	HDMI_WriteI2C_Byte( 0xce, 0x05 );    //vwidth:5
    #else

    SetRegisterBank( 0x80 );                                        //Tx pattern
	HDMI_WriteI2C_Byte( 0xbe, 0xc0 );
	HDMI_WriteI2C_Byte( 0xbf, Pattern_Mode );//01 02 04 37 3f
	HDMI_WriteI2C_Byte( 0xc0, 0xff );
	HDMI_WriteI2C_Byte( 0xc1, (p1.hs + p1.hbp)>>8);
	HDMI_WriteI2C_Byte( 0xc2, (p1.hs + p1.hbp)&0xFF );     //de_dly:192
	HDMI_WriteI2C_Byte( 0xc3, (p1.vs + p1.vbp));     //de_top:41
	HDMI_WriteI2C_Byte( 0xc4, p1.hactive >> 8);
	HDMI_WriteI2C_Byte( 0xc5, p1.hactive & 0xFF);     //de_cnt:1920
	HDMI_WriteI2C_Byte( 0xc6, p1.vactive >> 8);
	HDMI_WriteI2C_Byte( 0xc7, p1.vactive & 0xFF);     //de_lin:1080
	HDMI_WriteI2C_Byte( 0xc8, (p1.hs + p1.hbp + p1.hactive + p1.hfp) >> 8 );
	HDMI_WriteI2C_Byte( 0xc9, (p1.hs + p1.hbp + p1.hactive + p1.hfp) & 0xFF );     //h_total:2200
	HDMI_WriteI2C_Byte( 0xca, (p1.vs + p1.vbp + p1.vactive + p1.vfp) >> 8 );
	HDMI_WriteI2C_Byte( 0xcb, (p1.vs + p1.vbp + p1.vactive + p1.vfp) & 0xFF );    //v_total:1125
	HDMI_WriteI2C_Byte( 0xcc, p1.hs >> 8 );
	HDMI_WriteI2C_Byte( 0xcd, p1.hs & 0xFF );    //hwidth:44
	HDMI_WriteI2C_Byte( 0xce, p1.vs );    //vwidth:5
    #endif
}
void Lt8918_PtnDataConfig_Pattern( void )
{
	SetRegisterBank( 0x80 );                                        //Tx pattern
	HDMI_WriteI2C_Byte( 0xbe, 0xc0 );
	HDMI_WriteI2C_Byte( 0xbf, Pattern_Mode );
	HDMI_WriteI2C_Byte( 0xc0, VideoFormatTab[VideoFormat][0] );
	HDMI_WriteI2C_Byte( 0xc1, VideoFormatTab[VideoFormat][1] );
	HDMI_WriteI2C_Byte( 0xc2, VideoFormatTab[VideoFormat][2] );     //de_dly:192
	HDMI_WriteI2C_Byte( 0xc3, VideoFormatTab[VideoFormat][3] );     //de_top:41
	HDMI_WriteI2C_Byte( 0xc4, VideoFormatTab[VideoFormat][4] );
	HDMI_WriteI2C_Byte( 0xc5, VideoFormatTab[VideoFormat][5] );     //de_cnt:1920
	HDMI_WriteI2C_Byte( 0xc6, VideoFormatTab[VideoFormat][6] );
	HDMI_WriteI2C_Byte( 0xc7, VideoFormatTab[VideoFormat][7] );     //de_lin:1080
	HDMI_WriteI2C_Byte( 0xc8, VideoFormatTab[VideoFormat][8] );
	HDMI_WriteI2C_Byte( 0xc9, VideoFormatTab[VideoFormat][9] );     //h_total:2200
	HDMI_WriteI2C_Byte( 0xca, VideoFormatTab[VideoFormat][10] );
	HDMI_WriteI2C_Byte( 0xcb, VideoFormatTab[VideoFormat][11] );    //v_total:1125
	HDMI_WriteI2C_Byte( 0xcc, VideoFormatTab[VideoFormat][12] );
	HDMI_WriteI2C_Byte( 0xcd, VideoFormatTab[VideoFormat][13] );    //hwidth:44
	HDMI_WriteI2C_Byte( 0xce, VideoFormatTab[VideoFormat][14] );    //vwidth:5
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
void Lt8918_TxDPhyConfig( void )
{
	if(Pattern_en)
	{
		SetRegisterBank(0x83); //MIPI TX PHY
		HDMI_WriteI2C_Byte(0x46,0x06);
		HDMI_WriteI2C_Byte(0x47,0x08);
		HDMI_WriteI2C_Byte(0x48,0x0a);
		HDMI_WriteI2C_Byte(0x4a,0x20);
		HDMI_WriteI2C_Byte(0x40,0x00);
		return;
	}
	
	SetRegisterBank(0x83); //MIPI TX PHY
	//HDMI_WriteI2C_Byte(0x41,0x08);
	//HDMI_WriteI2C_Byte(0x42,0x08);
	//HDMI_WriteI2C_Byte(0x43,0x07);
	//HDMI_WriteI2C_Byte(0x44,0x1f);
	HDMI_WriteI2C_Byte(0x46,0x08);
	HDMI_WriteI2C_Byte(0x47,0x08);
	HDMI_WriteI2C_Byte(0x48,0x0a);
	
    switch(VideoFormat) {
        case VESA_1920x1080_60:
        case VESA_1920x1080_50:
        case VESA_1920x1080I_60:
        case VESA_1920x1080I_50:
        case VESA_1920x1080_30:
        case VESA_1920x1080_25:
            HDMI_WriteI2C_Byte(0x4a,0x20);//1a
            break;
        case VESA_1280x720_60:
        case VESA_1280x720_50:
        case VESA_1280x720_30:
        case VESA_1280x720_25:
            HDMI_WriteI2C_Byte(0x4a,0x25);
            break;
    }
	HDMI_WriteI2C_Byte(0x40,0x00);
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
void Lt8918_TxPrtclConfig( void )
{
	if(Pattern_en)
	{
		SetRegisterBank(0x83); //MIPI TX PHY
		HDMI_WriteI2C_Byte(0x12,0x01);
        HDMI_WriteI2C_Byte(0x13,0x85);
		HDMI_WriteI2C_Byte(0x14,0x28);
    	HDMI_WriteI2C_Byte(0x15,0x01);
    	HDMI_WriteI2C_Byte(0x16,0x04);
    	HDMI_WriteI2C_Byte(0x17,0x38);
    	HDMI_WriteI2C_Byte(0x18,0x00);
    	HDMI_WriteI2C_Byte(0x19,0x01);
    	HDMI_WriteI2C_Byte(0x1a,0x01);
    	HDMI_WriteI2C_Byte(0x1b,0x01);
    	HDMI_WriteI2C_Byte(0x1c,0x07);
    	HDMI_WriteI2C_Byte(0x1d,0x80);
		HDMI_WriteI2C_Byte(0x10,0x58);
		HDMI_WriteI2C_Byte(0x1f,0x04);
		HDMI_WriteI2C_Byte(0x11,0x0c); //line num
		return;
	}
	
	SetRegisterBank(0x83); //MIPI TX
	
    switch(VideoFormat) {
        case VESA_1920x1080_60:
            HDMI_WriteI2C_Byte(0x12,0x00);
            HDMI_WriteI2C_Byte(0x13,0xc0);    // 1080P@60:
            break;
        case VESA_1920x1080_50:
            HDMI_WriteI2C_Byte(0x12,0x02);
            HDMI_WriteI2C_Byte(0x13,0x78);    // 1080P@50:32
            break;
        case VESA_1920x1080_30:
            HDMI_WriteI2C_Byte(0x12,0x01);
            HDMI_WriteI2C_Byte(0x13,0x85);    // 1080P@30:32
            break;
        case VESA_1920x1080_25:
            HDMI_WriteI2C_Byte(0x12,0x02);
            HDMI_WriteI2C_Byte(0x13,0x58);    // 1080P@25:32
            break;
        case VESA_1920x1080I_60:
            HDMI_WriteI2C_Byte(0x12,0x01);
            HDMI_WriteI2C_Byte(0x13,0x85);    // 1080P@25:32
            break;
        case VESA_1920x1080I_50:
            HDMI_WriteI2C_Byte(0x12,0x02);
            HDMI_WriteI2C_Byte(0x13,0x80);    // 1080P@25:32
            break;
        case VESA_1280x720_60:
            HDMI_WriteI2C_Byte(0x12,0x01);
            HDMI_WriteI2C_Byte(0x13,0x58);    // 720P@60:32
            break;
        case VESA_1280x720_50:
            HDMI_WriteI2C_Byte(0x12,0x02);
            HDMI_WriteI2C_Byte(0x13,0x50);    // 720P@50:32
            break;
        case VESA_1280x720_30:
            HDMI_WriteI2C_Byte(0x12,0x05);
            HDMI_WriteI2C_Byte(0x13,0x78);    // 720P@30:32
            break;
        case VESA_1280x720_25:
            HDMI_WriteI2C_Byte(0x12,0x06);
            HDMI_WriteI2C_Byte(0x13,0x72);    // 720P@25:32
            break;   
        default:
            HDMI_WriteI2C_Byte(0x12,0x01);
            HDMI_WriteI2C_Byte(0x13,0x30);    // 1080P@25:32
            break;
    }

    switch(VideoFormat) {
        case VESA_1920x1080_60:
        case VESA_1920x1080_50:
        case VESA_1920x1080_30:
        case VESA_1920x1080_25:
            HDMI_WriteI2C_Byte(0x14,0x29);
        	HDMI_WriteI2C_Byte(0x15,0x01);
        	HDMI_WriteI2C_Byte(0x16,0x04);
        	HDMI_WriteI2C_Byte(0x17,0x38);
        	HDMI_WriteI2C_Byte(0x18,0x00);
        	HDMI_WriteI2C_Byte(0x19,0x01);
        	HDMI_WriteI2C_Byte(0x1a,0x01);
        	HDMI_WriteI2C_Byte(0x1b,0x01);
        	HDMI_WriteI2C_Byte(0x1c,0x07);
        	HDMI_WriteI2C_Byte(0x1d,0x80);
            break;
        case VESA_1920x1080I_60:
        case VESA_1920x1080I_50:
            HDMI_WriteI2C_Byte(0x14,0x29);
        	HDMI_WriteI2C_Byte(0x15,0x01);
        	HDMI_WriteI2C_Byte(0x16,0x04);
        	HDMI_WriteI2C_Byte(0x17,0x38);
        	HDMI_WriteI2C_Byte(0x18,0x00);
        	HDMI_WriteI2C_Byte(0x19,0x01);
        	HDMI_WriteI2C_Byte(0x1a,0x01);
        	HDMI_WriteI2C_Byte(0x1b,0x01);
        	HDMI_WriteI2C_Byte(0x1c,0x07);
        	HDMI_WriteI2C_Byte(0x1d,0x80);
            break;
        case VESA_1280x720_60:
        case VESA_1280x720_50:
        case VESA_1280x720_30:
        case VESA_1280x720_25:
            HDMI_WriteI2C_Byte(0x14,0x09);
        	HDMI_WriteI2C_Byte(0x15,0x01);
        	HDMI_WriteI2C_Byte(0x16,0x02);
        	HDMI_WriteI2C_Byte(0x17,0xd0);
        	HDMI_WriteI2C_Byte(0x18,0x00);
        	HDMI_WriteI2C_Byte(0x19,0x1e);
        	HDMI_WriteI2C_Byte(0x1a,0x1e);
        	HDMI_WriteI2C_Byte(0x1b,0x1e);
        	HDMI_WriteI2C_Byte(0x1c,0x05);
        	HDMI_WriteI2C_Byte(0x1d,0x00);
            break;
				 }
            
	
	HDMI_WriteI2C_Byte(0x10,0x58);
	HDMI_WriteI2C_Byte(0x1f,0x04);
	HDMI_WriteI2C_Byte(0x11,0x0c); //line num
	
}	
void Lt8918_TxPrtclConfigPtn( void )
{
	SetRegisterBank(0x83); //MIPI TX
	HDMI_WriteI2C_Byte(0x12,0x01);//0x01
	HDMI_WriteI2C_Byte(0x13,0x55);//0x55
	HDMI_WriteI2C_Byte(0x14,0x28);//csi=vs+vbp-1
	HDMI_WriteI2C_Byte(0x15,0x01);//csi=0x01 vbp
	HDMI_WriteI2C_Byte(0x16,0x04);
	HDMI_WriteI2C_Byte(0x17,0x38);//vact 根据具体分辨率设置
	HDMI_WriteI2C_Byte(0x18,0x01);//vfp  csi=0x01
	HDMI_WriteI2C_Byte(0x19,0x01);//Ptr_HBlank	csi=0x01
	HDMI_WriteI2C_Byte(0x1a,0x01);//Ptr_HBlank	csi=0x01
	HDMI_WriteI2C_Byte(0x1b,0x01);//Ptr_HBlank	csi=0x01
	HDMI_WriteI2C_Byte(0x1c,0x07);
	HDMI_WriteI2C_Byte(0x1d,0x80);//hact 根据具体分辨率设置
	HDMI_WriteI2C_Byte(0x10,0x58);//CSI
	HDMI_WriteI2C_Byte(0x1f,0x04);//yuv16
	HDMI_WriteI2C_Byte(0x11,0x0c);//4 line  0x0c 
	
}	

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
void Lt8918_MLRXInit( void )
{
	SetRegisterBank(0x70);//RX PHY
	HDMI_WriteI2C_Byte(0x01,0x80);//default 0x40
	HDMI_WriteI2C_Byte(0x03,0x48);//default 0x4A :Port A lvds mode clk lane output to rxpll disable
	HDMI_WriteI2C_Byte(0x04,0xa2);//default 0x02 :
	HDMI_WriteI2C_Byte(0x0c,0x80);//default 0x40
	HDMI_WriteI2C_Byte(0x13,0x80);//default 0x88
	HDMI_WriteI2C_Byte(0x18,0x50);
	HDMI_WriteI2C_Byte(0x38,0xb0);   //Pattern 需要设置为0x00, 非Patterm则写0xB0	
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
void InitLt8918State( void )
{
	delay1ms( 10 );
	Lt8918_CfgI2cEnable( );	
	if(!Pattern_en)//BT1120 YUV
	{
        printf("\r\n lvds mode");
        
		Lt8918_MLRXInit( );
		Lt8918_InputConfig( );
		Lt8918_TxPllConfig( );
		Lt8918_OutputConfig( );
		Lt8918_TxDPhyConfig( );
		Lt8918_TxPrtclConfig( );
	}
	else//pattern_mode
	{	
        printf("\r\n pattern mode");
		Lt8918_DesscPllInit( );
		Lt8918_DesscFreqSet( );
		Lt8918_TxPllConfig( );
		Lt8918_OutputConfig( );
		Lt8918_PtnDataConfig( );
		Lt8918_TxDPhyConfig( );
		Lt8918_TxPrtclConfig( );
	}
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
u32 ClkFreqIndicate( void )
{
	u32 freq;

	SetRegisterBank( 0x80 );
	freq   = HDMI_ReadI2C_Byte( 0x30 );
	freq <<= 8;
	freq  |= HDMI_ReadI2C_Byte( 0x31 );
	freq <<= 8;
	freq  |= HDMI_ReadI2C_Byte( 0x32 );
	return freq;
}

/************************************s***********************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
bool Lt8918_ClkFreqCheck( void )
{
	u32 pre_freq;
	u32 freq;
	u8	cnt;

	pre_freq = ClkFreqIndicate( );
	delay1ms( 5 );
	freq = ClkFreqIndicate( );
	if( freq >= pre_freq )
	{
		if( ( freq - pre_freq ) <= CLK_CHANGE_MARGIN )
		{
			if( freq >= CLK_ACTIVE_MIN_LIMIT )
			{
				return TRUE;
			}
		}
	}else
	{
		if( ( pre_freq - freq ) <= CLK_CHANGE_MARGIN )
		{
			if( freq >= CLK_ACTIVE_MIN_LIMIT )
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
u16 VideoParamIndicate( Parameter_Type param )
{
	u16 data;
	SetRegisterBank( 0x80 );
	switch( param )
	{
		case Vtotal:
			data   = HDMI_ReadI2C_Byte( 0x4c ) & 0x0F;
			data <<= 8;
			data  |= HDMI_ReadI2C_Byte( 0x4d );
			break;
		case Htotal:
			data   = HDMI_ReadI2C_Byte( 0x4e ) & 0x0F;
			data <<= 8;
			data  |= HDMI_ReadI2C_Byte( 0x4f );
			break;
		case Vactive:
			data   = HDMI_ReadI2C_Byte( 0x50 ) & 0x0F;
			data <<= 8;
			data  |= HDMI_ReadI2C_Byte( 0x51 );
			break;
		case Hactive:
			data   = HDMI_ReadI2C_Byte( 0x52 ) & 0x0F;
			data <<= 8;
			data  |= HDMI_ReadI2C_Byte( 0x53 );
			break;
	}
	return data;
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
bool Lt8918_VideoJudge( void )
{
	u16 pre_htotal, htotal;
	u16 pre_vtotal, vtotal;
	u16 hactive, vactive;

	SetRegisterBank( 0x80 );
	HDMI_WriteI2C_Byte( 0x38, 0x05 );

	pre_htotal = VideoParamIndicate( Htotal );
	pre_vtotal = VideoParamIndicate( Vtotal );
	delay1ms( 1 );
	htotal = VideoParamIndicate( Htotal );
	vtotal = VideoParamIndicate( Vtotal );

	if( ( ( htotal - pre_htotal ) <= FAULT_TOLERANT ) || ( ( pre_htotal - htotal ) <= FAULT_TOLERANT ) )
	{
		if( ( ( vtotal - pre_vtotal ) <= FAULT_TOLERANT ) || ( ( pre_vtotal - vtotal ) <= FAULT_TOLERANT ) )
		{
			hactive	   = VideoParamIndicate( Hactive );
			vactive	   = VideoParamIndicate( Vactive );
			if( ( hactive > HACTIVE_MIN_LIMIT ) && ( vactive > VACTIVE_MIN_LIMIT ) )
			{
				if( ( htotal - hactive ) > HBLANK_MIN_LIMIT )
				{
					if( ( vtotal - vactive ) > VBLANK_MIN_LIMIT )
					{
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
void Lt8918_VideoCheck( void )
{
	u16 pre_htotal, htotal;
	u16 pre_vtotal, vtotal;
	u16 hactive, vactive;

	SetRegisterBank( 0x80 );
	HDMI_WriteI2C_Byte( 0x38, 0x05 );

	htotal = VideoParamIndicate( Htotal );
	vtotal = VideoParamIndicate( Vtotal );

	hactive	   = VideoParamIndicate( Hactive );
	vactive	   = VideoParamIndicate( Vactive );

	if( ( vactive <= ( 0x438 + 1 ) ) && ( vactive >= ( 0x438 - 1 ) ) ) //1080p
	{
		VideoFormat = VESA_1920x1080_60;
		Pattern_en = 1 ;
	}
	else if( ( vactive <= ( 0x780 + 1 ) ) && ( vactive >= ( 0x780 - 1 ) ) ) //1920p
	{
		//VideoFormat = VESA_1080x1920_60;
		Pattern_en = 0 ;
	}
	else
	{
		VideoFormat = SignnalOff;
		Pattern_en = 1 ;
	}

	if( VideoFormat != SignnalOff )
	{
		if( LastVideoFormat != VideoFormat )
		{
			//FlagVideoChange = TRUE;
		}
	}
	LastVideoFormat = VideoFormat;
	
}

void Lt8918_checkPixClk(void){
    u32 freq;
    //读PixClk的值
    SetRegisterBank( 0x80 );
    HDMI_WriteI2C_Byte( 0x33, 0x0c );
    freq   = HDMI_ReadI2C_Byte( 0x30 );
    freq <<= 8;
    freq  |= HDMI_ReadI2C_Byte( 0x31 );
    freq <<= 8;
    freq  |= HDMI_ReadI2C_Byte( 0x32 );
    printf("\r\n PixClk:%d", freq);
}

void Lt8918_checkByteClk(void){
    u32 freq;
    //读PixClk的值
    SetRegisterBank( 0x80 );
    HDMI_WriteI2C_Byte( 0x33, 0x09 );
    freq   = HDMI_ReadI2C_Byte( 0x30 );
    freq <<= 8;
    freq  |= HDMI_ReadI2C_Byte( 0x31 );
    freq <<= 8;
    freq  |= HDMI_ReadI2C_Byte( 0x32 );
    printf("\r\n ByteClk:%d", freq);
}

void Lt8918_setVideoResolution(uint8 format) {
    switch(format) {
        case 0:
            VideoFormat = VESA_1920x1080_60;
            break;
        case 1:
            VideoFormat = VESA_1920x1080_50;
            break;
        case 2:
            VideoFormat = VESA_1920x1080_30;
            break;
        case 3:
            VideoFormat = VESA_1920x1080_25;
            break;
        case 4:
            VideoFormat = VESA_1920x1080I_60;
            break;
        case 5:
            VideoFormat = VESA_1920x1080I_50;
            break;
        case 6:
            VideoFormat = VESA_1280x720_60;
            break;
        case 7:
            VideoFormat = VESA_1280x720_50;
            break;
        case 8:
            VideoFormat = VESA_1280x720_30;
            break;
        case 9:
            VideoFormat = VESA_1280x720_25;
            break;
        default:
            VideoFormat = VESA_1920x1080_30;
            break;
            
    }

	printf("\r\n VideoFormat:%d", VideoFormat);
    
    Lt8918L_reset();

    InitLt8918State();
}

void Lt8918_VideoCheck_lvds( void )
{
	u16 pre_htotal, htotal;
	u16 pre_vtotal, vtotal;
	u16 hactive, vactive;

    u16 vs, vbp, vfp;
    u16 hs, hbp, hfp;
    int i;
    u8 data[20] = {0};
    
	SetRegisterBank( 0x80 );
    printf("\r\nReg 0x03: value:0x%x", HDMI_ReadI2C_Byte(0x03));
    HDMI_ReadI2C_ByteN(0x43, data, 18);

    printf("\r\n");
    for(i = 1; i <= 20; i++) {
        printf("%02x ", data[i-1]);
        if(i % 10 == 0) {
            printf("\r\n");
        }
    }
    vs = data[0];
    hs = (((u16)data[1]) << 8) | data[2];
    vbp = data[3];
    vfp = data[4];
    hbp = (((u16)data[5]) << 8) | data[6];
    hfp = (((u16)data[7]) << 8) | data[8];
    vtotal = (((u16)data[9]) << 8) | data[10];
    htotal = (((u16)data[11]) << 8) | data[12];
    vactive = (((u16)data[13]) << 8) | data[14];
    hactive = (((u16)data[15]) << 8) | data[16];

    printf("\r\n hs:%d", hs);
    printf("\r\n hbp:%d", hbp);
    printf("\r\n hactive:%d", hactive);
    printf("\r\n hfp:%d", hfp);
    printf("\r\n htotal:%d", htotal);

    printf("\r\n");
    
    printf("\r\n vs:%d", vs);
    printf("\r\n vbp:%d", vbp);
    printf("\r\n vactive:%d", vactive);
    printf("\r\n vfp:%d", vfp);
    printf("\r\n vtotal:%d", vtotal);
    
    printf("\r\n\r\n");

}


/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
void Lt8918_LvdsClkPhaseAdjust( u8 value )
{
	SetRegisterBank( 0x70 );
	HDMI_WriteI2C_Byte( 0x07, 0xa8 | value );
	//HDMI_WriteI2C_Byte(0x11,0x00);
	HDMI_WriteI2C_Byte( 0x12, 0xa8 | value );
}

/***********************************************************
* Function:       // 函数名称
* Description:    // 函数功能、性能等的描述
* Input:          // 1.输入参数1，说明，包括每个参数的作用、取值说明及参数间关系
* Input:          // 2.输入参数2，说明，包括每个参数的作用、取值说明及参数间关系
* Output:         // 1.输出参数1，说明
* Return:         // 函数返回值的说明
* Others:         // 其它说明
***********************************************************/
void lt8918l_Process( void )
{
	static bool Process_Once1 = 0;
	static bool Process_Once2 = 1;
	//ChipI2cSelect( lt8918 );	
	
	if( !Pattern_en )//Pattern_en disable
	{
		if(VideoFormat != LastVideoFormat) {
            LastVideoFormat = VideoFormat;
            printf("\r\n L:%d bt1120 mode. ", __LINE__);
			Lt8918_MLRXInit( );
    		Lt8918_InputConfig( );
    		Lt8918_TxPllConfig( );
    		Lt8918_OutputConfig( );
    		Lt8918_TxDPhyConfig( );
    		Lt8918_TxPrtclConfig( );

			//IIC_disable();
		}
	}
	else //Pattern_en enable
	{
		if(VideoFormat != LastVideoFormat) {
            LastVideoFormat = VideoFormat;
			//IIC_disable();
		}
        #if 0
		if(VideoFormat != LastVideoFormat)
		{	
            LastVideoFormat = VideoFormat;
			printf("\r\n pattern mode");
    		Lt8918_DesscPllInit( );
    		Lt8918_DesscFreqSet( );
    		Lt8918_TxPllConfig( );
    		Lt8918_OutputConfig( );
    		Lt8918_PtnDataConfig( );
    		Lt8918_TxDPhyConfig( );
    		Lt8918_TxPrtclConfig( );
		}
        #endif
	}

    if( GetSysTick_Sec() > (lt8918_task_t_sec + 2) )
    {
        lt8918_task_t_sec = GetSysTick_Sec();
		
        Lt8918_VideoCheck_lvds();
        Lt8918_checkPixClk();
        Lt8918_checkByteClk();
    }
}

void Lt8918L_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    u32 deviceId;

    printf("\r\n %s() is called.", __FUNCTION__);
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);	

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;			
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
    GPIO_Init(GPIOA, &GPIO_InitStructure);				
    GPIO_SetBits(GPIOA, GPIO_Pin_7);

    Lt8918L_reset();

    deviceId = Lt8918L_deviceId();

    printf("\r\n Lt8918L device id:0x%x.", deviceId);

    InitLt8918State();

    printf("\r\n freq:0x%x", ClkFreqIndicate());

    lt8918_task_t_sec = GetSysTick_Sec();
}

void Lt8918L_reset(void)
{
    printf("\r\n %s() is called.", __FUNCTION__);
    LT8918L_RST = 0;
    Wait10Ms(15);
    LT8918L_RST = 1;
    Wait10Ms(5);
}

u32 Lt8918L_deviceId(void)
{
    u32 id;
    printf("\r\n %s() is called.", __FUNCTION__);
    
    Lt8918_CfgI2cEnable();
    HDMI_ReadI2C_ByteN(0x00,(u8*)(&id), sizeof(u32));

    return id;
}


/************************************** The End Of File **************************************/
