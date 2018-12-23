/*----------------------------------------------------------------------------*/
/* main.c                                                                     */
/* 项目描述:                                                                  */
/*        硬件:GPS+加速度传感器+GPRS                                          */
/*    网络服务:yeelink                                                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* 2014/6/7         	   manwjh 建立 V0.1                                   */
/* 2014/8/25~2014/8/26     修改和发布V1.0a(测试版本)                          */
/*                                                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/* version */
const char version_msg[200]={
"\r\n"
"**********************************************\r\n"
"[LVDS2MIPI Project]\r\n"
"V1.0c\r\n"
"2018/3/29\r\n"
"Server:cjl\r\n"
"MODEM:STM32/Lontium\r\n"
"**********************************************\r\n"
};

/* include */
#include "includes.h"
#include "lt8918.h"

/* private */
void IWDG_Configuration(void);
void SoftReset(void);

/* external */
extern SYSTEM_CFG g_SysCfg;
extern int gUsbLinked;
extern SYSTEM_INFO g_System;




/* public */
static char usb_device_onoff;
void USB_Ctrl(char onoff);

static u8 Lock_Code[4]; // 
const u32 Lock_Code_Holder = 0x98B4CE33;
BOOL checkLockCode(void)
{
	u32 CpuID[3];
	u32 code;
	BOOL ret = false;
	//get cpu unique id
	CpuID[0]=*(vu32*)(0x1ffff7e8);
	CpuID[1]=*(vu32*)(0x1ffff7ec);
	CpuID[2]=*(vu32*)(0x1ffff7f0);
	//encrypt algy
	code=(CpuID[0]>>1)+(CpuID[1]>>2)+(CpuID[2]>>3);
	code = ~code;
	code *=3;
	//printf("\r\n 0x%08x 0x%08x 0x%08x 0x%08x", CpuID[0], CpuID[1], CpuID[2], code);
	Lock_Code[0] = code >> 24;
	Lock_Code[1] = code >> 16;
	Lock_Code[2] = code >> 8;
	Lock_Code[3] = code;

	//STMFLASH_Read((u32)&Lock_Code_Holder,(u16*)code,4);
	if(code == Lock_Code_Holder)
	{
		//printf("\r\n L:%d write flash...", __LINE__);
		//STMFLASH_Write((u32)&Lock_Code_Holder,(u16*)Lock_Code,4);
		//printf("\r\n L:%d write flash finished!", __LINE__);
		ret = true;
	}
	else
	{
		//printf("\r\n L:%d Lock_Code_bak:%02x %02x %02x %02x", __LINE__, code[0], code[1], code[2], code[3]);
		ret = false;
	}
	return ret;
}



/*----------------------------------------------------------------------------*/
/* main()                                                                     */
/*                                                                            */
/*----------------------------------------------------------------------------*/
int main(void)
{
    int usb_link_cnt;
    BOOL check = true;
    
    //ARM初始化
    SystemInit();
    //中断配置
    //2为抢占式中断,3bit中断优先级
    //USB--------------0,0
    //UART3(GPRS)------0,1
    //UART2(GPS)-------1,0
    //GSENSOR----------2,0
    //SYSTICK----------2,1
    //UART1(PRINT)-----3,3
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    //系统基础时钟初始化
    SysTick_Init();
    
	//ARM硬件初始化
	hw_io_init(ALL_HWIO,0);
	UART1_Configuration();
	UART2_Configuration();
	UART3_Configuration();


    //check = checkLockCode();
    if(!check)
    {
        printf("%c", 64);
        while(1)
        {
            printf(version_msg);
        }
    }
    
	printf(version_msg);
	
	//开机电压监测,如果电压过低,只允许运行USB.
	//led_taskinit();    ///no led
	//ADC_taskinit();   ///no adc
	while(1){
	    /* 电池电量检测 */
        ///ADC_task();  //no adc
        /* 如果电池电压充电达到启动值,并且USB没有联接过,则退出此循环 */
	    ////if( (g_BatteryVoltage>V_OPEN_GPRS)&&(gUsbLinked==0) )
		if(gUsbLinked==0 )  ////没有电池
	        break;
	    else{
            //只要USB连结了设备打开IC USB
            if( hw_io_ctrl(USB_DET,GET) ){
                if( usb_device_onoff==0 )
                    USB_Ctrl(1);
            }
            //USB数据连结模式
            if( gUsbLinked ){
                //led_taskctrl(LED1,ON,0);
                usb_link_cnt++;
                if( hw_io_ctrl(USB_DET,GET)==0 ){
                    USB_Ctrl(0);    //关闭USB
                    SoftReset();
                }
                continue;
            }
	    }
    }

	KEY_Init();  //////按键初始化


	//TIM3_Int_Init(499,7199);//10Khz的计数频率，计数到500为50ms  
    //外设初始化
    g_System.sys_err = 0;
    //
    if( w25xx_init() )
        printf("\r\nmain-->spi flash: OK!");
    else{
        printf("\r\nmain-->spi flash: FAIL!");
        g_System.sys_err |= HW_ERR_FLASH;
    }
    //
    #if 0
    if( MBA220init() )
        printf("\r\nmain-->gSensor: OK!");
    else{
        printf("\r\nmain-->gSensor: FAIL!");
        g_System.sys_err |= HW_ERR_GSENSOR;
    }
    #endif

    IIC_Init();

    Lt8918L_init();

    /*
    while(1) {
        Lt8918_VideoCheck_lvds();
        Lt8918_checkPixClk();
        Lt8918_checkByteClk();
        Wait10Ms(200);
    }
    */

		AT24CXX_Init();
    

    //Lt8918L_SelfCheck();

    //Lt8918L_pattern();
    
	//文件系统加载
	if( disc_mount()==0 )
	    g_System.sys_err |= FS_ERR_FORMAT;
	check_itarcker_cfg();
	load_sys_config();

	//任务初始化
	gps_taskinit();
	gpx_record_taskinit();
	iTrack_taskinit();
	//gprs_taskinit();  ///uart 3
	sonylens_init();
    
	yeelink_taskinit();
	//led_taskinit();  ///no led
	//beep_taskinit();   //// no beep
	gSensor_taskinit();
	//
	system_taskinit();
	//ADC_taskinit();
	
	//看门狗初始化
	if( (g_SysCfg.WDT_ON==true)&&(g_SysCfg.SAVE_ENERGY_ON==false) ){
        IWDG_Configuration();
        if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)  /* 如果系统以为看门狗复位 */
    	{	
    	  printf("\r\nmain-->IWDG RESET \r\n");
    	  RCC_ClearFlag();
    	}
    }
    
	//自检完成
	#if 0
	if( (g_System.sys_err&HW_ERR_ALL)!=0 ){
	    printf("\r\n ------ 启动失败 ------ \r\n");
	    beep_taskctrl(BEEP_CNT,5);
	    while(1);
	}else{
	    printf("\r\n ------ Let's go ------ \r\n");
	    beep_taskctrl(BEEP_ON_T,50);
	}
	#endif ////no beep
    //
    while(1){
        /* WDT */
        if( (g_SysCfg.WDT_ON==true)&&(g_SysCfg.SAVE_ENERGY_ON==false) )
            IWDG_ReloadCounter();

        /* USB DET */
        //只要USB连结了设备打开IC USB
        if( hw_io_ctrl(USB_DET,GET) ){
            if( usb_device_onoff==0 )
                USB_Ctrl(1);
        }
        //USB数据连结模式
        if( gUsbLinked ){
            //led_taskctrl(LED1,ON,0);
            usb_link_cnt++;
            if( hw_io_ctrl(USB_DET,GET)==0 ){
                USB_Ctrl(0);    //关闭USB
                SoftReset();
            }
            continue;
        }
		 Do_KEY_Down();  //////处理按键事件

        /* GPS模块通讯模块 */
		gps_task(NULL);
        
        /* 系统任务模块(电池电量/LED/BUZZER...)  */
		system_task();

		/* 行为识别模块 */
		iTrack_task();
		
		/* GPX轨迹记录模块 */
		if( (g_System.sys_err&FS_ERR_FORMAT)==0 )
		    gpx_record_task(NULL);

		/* GPRS通讯模块 */
		//gprs_task();

		sonylens_task();  /////sony 机芯任务

		/* 网络上传模块 Yeelink */
		if( (g_System.sys_err&CFG_ERR_YEELINK)==0 )
		    yeelink_task();

        /* 电池电量检测 */
        //ADC_task();  ///no adc
	}
}



/*******************************************************************************
* Function Name  : WWDG_Configuration
* Description    : Configures WWDG.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void IWDG_Configuration(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); 	/* 写入0x5555,用于允许狗狗寄存器写入功能 */
    IWDG_SetPrescaler(IWDG_Prescaler_256);          /* 内部低速时钟256分频 40K/256=156HZ(6.4ms) */ 
    /* 看门狗定时器溢出时间 */
    IWDG_SetReload(1562);							/* 喂狗时间 10s/6.4ms=1562 .注意不能大于0xfff*/
    IWDG_ReloadCounter();							/* 喂狗*/
    IWDG_Enable(); 									/* 使能看门狗*/
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable); 	/* 写入0x5555,用于允许狗狗寄存器写入功能 */
}

void SoftReset(void)
{
    __set_FAULTMASK(1);     // 关闭所有中端
    NVIC_SystemReset();     // 复位
}



static char usb_device_onoff;

void USB_Ctrl(char onoff)
{
    #if 0
    if( onoff ){
        Set_USBClock();
        USB_Interrupts_Config();
        USB_Init();
        USB_Disconnect_Config();
        MAL_Config();
        //USB_Configured_LED();
        usb_device_onoff = 1;
    }else{
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, DISABLE);
        USB_NotConfigured_LED();
        usb_device_onoff = 0;
    }
    #endif
}




