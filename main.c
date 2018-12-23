/*----------------------------------------------------------------------------*/
/* main.c                                                                     */
/* ��Ŀ����:                                                                  */
/*        Ӳ��:GPS+���ٶȴ�����+GPRS                                          */
/*    �������:yeelink                                                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* 2014/6/7         	   manwjh ���� V0.1                                   */
/* 2014/8/25~2014/8/26     �޸ĺͷ���V1.0a(���԰汾)                          */
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
    
    //ARM��ʼ��
    SystemInit();
    //�ж�����
    //2Ϊ��ռʽ�ж�,3bit�ж����ȼ�
    //USB--------------0,0
    //UART3(GPRS)------0,1
    //UART2(GPS)-------1,0
    //GSENSOR----------2,0
    //SYSTICK----------2,1
    //UART1(PRINT)-----3,3
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    //ϵͳ����ʱ�ӳ�ʼ��
    SysTick_Init();
    
	//ARMӲ����ʼ��
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
	
	//������ѹ���,�����ѹ����,ֻ��������USB.
	//led_taskinit();    ///no led
	//ADC_taskinit();   ///no adc
	while(1){
	    /* ��ص������ */
        ///ADC_task();  //no adc
        /* �����ص�ѹ���ﵽ����ֵ,����USBû�����ӹ�,���˳���ѭ�� */
	    ////if( (g_BatteryVoltage>V_OPEN_GPRS)&&(gUsbLinked==0) )
		if(gUsbLinked==0 )  ////û�е��
	        break;
	    else{
            //ֻҪUSB�������豸��IC USB
            if( hw_io_ctrl(USB_DET,GET) ){
                if( usb_device_onoff==0 )
                    USB_Ctrl(1);
            }
            //USB��������ģʽ
            if( gUsbLinked ){
                //led_taskctrl(LED1,ON,0);
                usb_link_cnt++;
                if( hw_io_ctrl(USB_DET,GET)==0 ){
                    USB_Ctrl(0);    //�ر�USB
                    SoftReset();
                }
                continue;
            }
	    }
    }

	KEY_Init();  //////������ʼ��


	//TIM3_Int_Init(499,7199);//10Khz�ļ���Ƶ�ʣ�������500Ϊ50ms  
    //�����ʼ��
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
    
	//�ļ�ϵͳ����
	if( disc_mount()==0 )
	    g_System.sys_err |= FS_ERR_FORMAT;
	check_itarcker_cfg();
	load_sys_config();

	//�����ʼ��
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
	
	//���Ź���ʼ��
	if( (g_SysCfg.WDT_ON==true)&&(g_SysCfg.SAVE_ENERGY_ON==false) ){
        IWDG_Configuration();
        if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)  /* ���ϵͳ��Ϊ���Ź���λ */
    	{	
    	  printf("\r\nmain-->IWDG RESET \r\n");
    	  RCC_ClearFlag();
    	}
    }
    
	//�Լ����
	#if 0
	if( (g_System.sys_err&HW_ERR_ALL)!=0 ){
	    printf("\r\n ------ ����ʧ�� ------ \r\n");
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
        //ֻҪUSB�������豸��IC USB
        if( hw_io_ctrl(USB_DET,GET) ){
            if( usb_device_onoff==0 )
                USB_Ctrl(1);
        }
        //USB��������ģʽ
        if( gUsbLinked ){
            //led_taskctrl(LED1,ON,0);
            usb_link_cnt++;
            if( hw_io_ctrl(USB_DET,GET)==0 ){
                USB_Ctrl(0);    //�ر�USB
                SoftReset();
            }
            continue;
        }
		 Do_KEY_Down();  //////�������¼�

        /* GPSģ��ͨѶģ�� */
		gps_task(NULL);
        
        /* ϵͳ����ģ��(��ص���/LED/BUZZER...)  */
		system_task();

		/* ��Ϊʶ��ģ�� */
		iTrack_task();
		
		/* GPX�켣��¼ģ�� */
		if( (g_System.sys_err&FS_ERR_FORMAT)==0 )
		    gpx_record_task(NULL);

		/* GPRSͨѶģ�� */
		//gprs_task();

		sonylens_task();  /////sony ��о����

		/* �����ϴ�ģ�� Yeelink */
		if( (g_System.sys_err&CFG_ERR_YEELINK)==0 )
		    yeelink_task();

        /* ��ص������ */
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
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); 	/* д��0x5555,�����������Ĵ���д�빦�� */
    IWDG_SetPrescaler(IWDG_Prescaler_256);          /* �ڲ�����ʱ��256��Ƶ 40K/256=156HZ(6.4ms) */ 
    /* ���Ź���ʱ�����ʱ�� */
    IWDG_SetReload(1562);							/* ι��ʱ�� 10s/6.4ms=1562 .ע�ⲻ�ܴ���0xfff*/
    IWDG_ReloadCounter();							/* ι��*/
    IWDG_Enable(); 									/* ʹ�ܿ��Ź�*/
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Disable); 	/* д��0x5555,�����������Ĵ���д�빦�� */
}

void SoftReset(void)
{
    __set_FAULTMASK(1);     // �ر������ж�
    NVIC_SystemReset();     // ��λ
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




