/*----------------------------------------------------------------------------*/
/* system.c                                                                   */
/* ����:ϵͳ���ȳ���                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* 2014/6/7 manwjh ���� V0.1                                                  */
/* 2014/7/12 manwjh v0.2  modem��ʼ����ɺ�,ֻ��ȡ״̬�;��쳣������ת.       */
/* 2014/7/23 manwjh v0.3                                                      */
/* 2014/8/30 manwjh v0.4  �͹���ģʽȡ����MCU˯��                             */
/*           ����g_SysCfg.WAKEUP_SAVE_ENERGY_T����,��ʱ���Ѳ��������ꡣ       */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* include */
#include "includes.h"

/* external */
extern GPS_INFO_TYPE g_GpsInfo;        //gps ������Ϣ
extern unsigned long g_YeelinkNextTime;
extern UPLOAD_POS_BF iPos;

/* public */
SYSTEM_CFG g_SysCfg;
SYSTEM_INFO g_System;
GSENSOR_INFO *gSensor;
int SaveEnergyStep;
int SleepStep;
int gUsbLinked=0;       //wjh add, �ж��Ƿ����U��ģʽ
float g_BatteryVoltage;
float g_pre_BatteryVoltage = 0;
int gPowerState;

/* private */
static void myPWR_EnterSleepMode(void);
static unsigned long system_task_t_mark=0;
static int system_taskstate;

/* define */


/*----------------------------------------------------------------------------*/
/* system_task                                                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
void syscfg_init(void)
{
    g_SysCfg.BEEP_ON = true;
    g_SysCfg.SAVE_ENERGY_ON = true;
    g_SysCfg.WDT_ON = false;
    g_SysCfg.LET_US_GO_MAX_T = 5*60;
    g_SysCfg.NET_SEND_T_MIN = 5;
    g_SysCfg.ENTER_SAVE_ENERGY_T = 2*60;
    g_SysCfg.WAKEUP_SAVE_ENERGY_T = 5*60;
    g_SysCfg.WAKEUP_SLEEP_T = 10;
    g_SysCfg.GPRS_RETRY_T = 15*60;
    g_SysCfg.GPS_SEARCH_T = 6*60;
}


void system_taskinit(void)
{
    system_taskctrl(SYS_NORMAL_TRY);
    //
    gUsbLinked = 0;
    //
    gPowerState = 0;
    //
    gSensor = get_gsensor_state();
    set_gsensor_ctrl(GSNESOR_WORK_MODE,GSENSOR_NORMAL);
}

int system_taskctrl(int cmd)
{
    switch(cmd){
        case SYS_NORMAL_TRY:                //��ͨģʽ����
            //
            set_gsensor_ctrl(GSNESOR_WORK_MODE,GSENSOR_NORMAL);
			//gprs_taskctrl(MODEM_POWER_ON);		//test gprs
            system_task_t_mark = GetSysTick_Sec();
            g_System.mode = SYS_NORMAL_TRY;
            //
            printf("\r\nSYS-->����ģʽ\r\n");
            break;

        case SYS_NORMAL:                    //��ͨģʽ
            iTrack_taskctrl(TRACK_START);
            //
            gSensor->static_time = 0;
            system_task_t_mark = GetSysTick_Sec();
            g_System.mode = SYS_NORMAL;
            //
            printf("\r\nSYS-->��׼ģʽ\r\n");
            break;
            
        case SYS_ONLY_GPX:                  //ֻ���ش洢
            #if 0
			gprs_taskctrl(MODEM_POWER_OFF);
            //
            system_task_t_mark = GetSysTick_Sec();
            g_System.mode = SYS_ONLY_GPX;
            //
            printf("\r\nSYS-->GPX ONLYģʽ\r\n");
			#endif
            break;
            
        case SYS_SAVE_ENERGY:               //����ģʽ
            if( g_SysCfg.SAVE_ENERGY_ON==true ){
				gpx_file_save();
                gps_taskctrl(GPS_POWER_OFF);
                //
                printf("\r\nSYS-->׼������͹���ģʽ\r\n");
                //
                set_gsensor_ctrl(GSNESOR_WORK_MODE,GSENSOR_NORMAL);
    			system_task_t_mark = GetSysTick_Sec();
                g_System.mode = SYS_SAVE_ENERGY;
                SaveEnergyStep = 0;
            }
            break;
            
        case SYS_SLEEP:                     //����ģʽ
            #if 0
			gprs_taskctrl(MODEM_POWER_OFF);
            gps_taskctrl(GPS_POWER_OFF);
            iTrack_taskctrl(TRACK_END);
            gpx_file_saveexit();
            //
            printf("\r\nSYS-->׼����������ģʽ\r\n");
            //
            set_gsensor_ctrl(GSNESOR_WORK_MODE,GSENSOR_NORMAL);
            system_task_t_mark = GetSysTick_Sec();
            g_System.mode = SYS_SLEEP;
            SleepStep = 0;
			#endif
            break;
            
        case USB_MODE:               //USBģʽ
            g_System.mode = USB_MODE;
            printf("\r\nSYS-->USBģʽ\r\n");
            break;
            
        case SYS_ERROR:              //ϵͳ����
            g_System.mode = SYS_ERROR;
            printf("\r\nSYS-->ϵͳģʽ\r\n");
            break;
            
        default:
            printf("\r\nSYS-->��Чģʽ\r\n");
            break;
    }
    return g_System.mode;
}


int system_task(void)
{
    float batt_tmp;
    unsigned long sec_tick_tmp;
	int i;
    
    
    //����Ƿ����ⲿ��Դ
    if( hw_io_ctrl(USB_DET,GET) )
        gPowerState |= CHARGING;
    else
        gPowerState &= ~CHARGING;

    //��Դ����
    #if 0
    if( g_BatteryVoltage<V_PWR_LOW ){
        //�����������3.3V,ϵͳ����SLEEPģʽ
        gPowerState |= UNDER_VOLTAGE;
        if( g_System.mode!=SYS_SLEEP ){
            beep_taskctrl(BEEP_CNT,3);
            system_taskctrl(SYS_SLEEP);
        }
    }else if( g_BatteryVoltage>V_PWR_HIGH ){
        gPowerState |= OVER_VOLTAGE;
    }else{
        gPowerState &= ~(OVER_VOLTAGE|UNDER_VOLTAGE);
        gPowerState |= NORMAL_BATT;
    }
	#endif //// no bat
    //��ȡgSensor״̬
    if( gSensor->new_flg ){
        gSensor->new_flg = 0;
        //printf("\r\ngSensor-->������ʱ��:%d,������ֹʱ��:%d,�񶯴���:%d",gSensor->shock_time,gSensor->static_time,gSensor->shock_cnt);
    }
    //ʱ������ͬ��
    if( g_GpsInfo.pos_update==true ){
        sys_time( &g_GpsInfo.time );
    }
        
    
    switch(g_System.mode)
    {
        case SYS_NORMAL_TRY:
            //���GPSû������,������GPSģ��
            #if 0
            if( GetGpsState(GPS_POWRERON)==0 ){
                gps_taskctrl(GPS_POWER_ON);
                break;
            }
			
            //���GPS��λ���,���������߻���GPRSģ��
            if( GetGpsState(GPS_LOCATED)==GPS_LOCATED ){
                if( (g_BatteryVoltage>V_OPEN_GPRS)&&(GetModemState(MD_SIM_IN)==MD_SIM_IN) ){
                    gprs_taskctrl(MODEM_POWER_ON);
                    //ֻҪ�ҵ�GPS�ź�����ת��NORMAL״̬,GPRS�����Ƿ���������������NORMAL���̾���
                    g_YeelinkNextTime = 0;  //��ʱ��ʼ�ϴ���һ���㡣
                    system_taskctrl(SYS_NORMAL);
                }else{
                    system_taskctrl(SYS_ONLY_GPX);
                }
            }
            //���GPS�޷���ɶ�λ,��ϵͳ��������
            if( GetGpsState(GPS_ERR_LOCATE_FAIL|GPS_ERR_HW)!=0 )
                system_taskctrl(SYS_SAVE_ENERGY);
			#endif

            break;
        
        case SYS_NORMAL:
			#if 0
            //���GPS���ֶ�λʧ��,��ת���͹���״̬
            if( GetGpsState(GPS_ERR_LOCATE_FAIL|GPS_ERR_HW)!=0 ){
                system_taskctrl(SYS_SAVE_ENERGY);
            }
            //���GPRS��⵽����,��ת��GPX״̬
            if( (g_BatteryVoltage<V_CLOSE_GPRS)||GetModemState(MD_ERR_ALL) ){
                system_taskctrl(SYS_ONLY_GPX);
            }
            //���gSensor�ж�Ϊ��ֹ״̬
            if( (gSensor->static_time>g_SysCfg.ENTER_SAVE_ENERGY_T)&&(gSensor->shock_time==0) ){
                system_taskctrl(SYS_SAVE_ENERGY);
            }
			#endif
            break;

        //ֻ���б���GPX��¼ģʽ
        //�����״̬������ԭ��: MODEMӲ������(SIM����,MODEM��ѹ������)
        //�˳���״̬������ԭ��: MODEMӲ�������Լ��ָ�
        case SYS_ONLY_GPX:
			#if 0
            //���GPS���ֶ�λʧ��,��ת��ʡ��ģʽ
            if( GetGpsState(GPS_ERR_LOCATE_FAIL|GPS_ERR_HW)!=0 ){
                system_taskctrl(SYS_SAVE_ENERGY);
            }
            //���gSensor�ж�Ϊ��ֹ״̬
            if( (gSensor->static_time>g_SysCfg.ENTER_SAVE_ENERGY_T)&&(gSensor->shock_time==0) ){
                system_taskctrl(SYS_SAVE_ENERGY);
            }
			//���GPRS��ѹ�ָ�����,����SIM������
			if( (g_BatteryVoltage>V_OPEN_GPRS)&&(GetModemState(MD_SIM_IN)==MD_SIM_IN) ){
				system_taskctrl(SYS_NORMAL_TRY);
			}
			#endif
            break;

        //����ģʽ
        //�����״̬������ԭ��:GPS���ź�,������,gSensor��ֹ�㷨
        //�˳���״̬������ԭ��:�����㹻,gSensor�ζ��㷨
        case SYS_SAVE_ENERGY:
			#if 0
            switch(SaveEnergyStep){
                case 0: //
                    if( (iPos.pos_num==0)||(GetModemState(MD_OK_ALL)!=MD_OK_ALL)||( GetSysTick_Sec()>system_task_t_mark+200 ) ){
                        gprs_taskctrl(MODEM_POWER_OFF);
                        SaveEnergyStep = 1;
                    }
                    break;
                case 1: //�ر��ܱ�
                    if( GetModemMode()==MODEM_PWROFF ){
                        led_taskctrl(LED1,OFF,0);
                        system_task_t_mark = GetSysTick_Sec();
                        SaveEnergyStep = 2;
                    }
                    break;
                case 2: //���߼��
                    if( (gSensor->shock_time>3)||(GetSysTick_Sec()>(system_task_t_mark+g_SysCfg.WAKEUP_SAVE_ENERGY_T)) ){
                        if( g_BatteryVoltage>V_OPEN_GPRS ){
                            led_taskctrl(LED1,OFF,1);
    				        system_task_t_mark = GetSysTick_Sec();
    				        SaveEnergyStep = 3;
				        }
				    }
                    break;
                case 3: //�͹��Ļ���
                    system_taskctrl(SYS_NORMAL_TRY);
                    break;
                
            }
			#endif
            break;
            
        //����ģʽ
        //�����״̬������ԭ��:gsensor�㷨
        //�˳���״̬������ԭ��:gsensor�㷨
        case SYS_SLEEP:
			#if 0
            switch(SleepStep)
            {
                case 0: //׼������Sleep
                    gprs_taskctrl(MODEM_POWER_OFF);
                    gps_taskctrl(GPS_POWER_OFF);
                    SleepStep = 1;
                    break;
                case 1: //�ȴ�ϵͳ��ɴ���
                    if( (GetModemState(MD_PWR_OK)==0)&&(GetGpsState(GPS_POWRERON)==0) ){
                        SleepStep = 2;
                    }
                    break;
                case 2: //˯��
                    led_taskctrl(LED1,OFF,0);
                    myPWR_EnterSleepMode();
                    //����ѹ�Ƿ���ϻ�������
                    ADC_taskctrl(1);
                    while( ADC_task()!=4 );
                    if( g_BatteryVoltage>V_OPEN_GPRS ){
                        system_task_t_mark = GetSysTick_Sec();
                        SleepStep = 3;
                    }
                    break;
                case 3: //����WAKEUP_SLEEP_T��
                    led_taskctrl(LED1,ON,0);
                    if( GetSysTick_Sec()<(system_task_t_mark+g_SysCfg.WAKEUP_SLEEP_T) ){
                        if( gSensor->shock_time>g_SysCfg.WAKEUP_SLEEP_T/2 ){
                            //beep_taskctrl(BEEP_ON_T,50);
                            //system_taskctrl(SYS_NORMAL_TRY);
                            SoftReset();
                        }
                    }else
                        SleepStep = 2;
                    break;
            }
			#endif
            break;

        //USBģʽ
        //�����״̬������ԭ��:USB��������
        //�˳���״̬������ԭ��:USB�������ӶϿ�
        case USB_MODE:               //USBģʽ
            break;
    
        //ϵͳ����
        //�����״̬������ԭ��:GPSͨѶ���ɹ�
        //�˳���״̬������ԭ��:ֻ�йػ�������,�ٴ��Լ�
        case SYS_ERROR:              //ϵͳ����
            break;
    }
    
    return 0;
}



/*----------------------------------------------------------------------------*/
/* GPIO�������                                                               */
/* ע��MODEM��POWER_ON,POWER_OFF���������                                    */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*
    PB13->GSM_RST
*/

#define HWIO_GSM_RST  GPIOB,GPIO_Pin_13
#define HWIO_LED_1    GPIOB,GPIO_Pin_8
#define HWIO_LED_2    GPIOB,GPIO_Pin_9
#define HWIO_LED_3    GPIOB,GPIO_Pin_13
#define HWIO_BUZZER   GPIOB,GPIO_Pin_14
#define HWIO_GSM_PWR  GPIOB,GPIO_Pin_1
#define HWIO_GPS_PWR  GPIOB,GPIO_Pin_5      //PB5

#define HWIO_USB_DET  GPIOB,GPIO_Pin_12
#define HWIO_SIM_DET  GPIOB,GPIO_Pin_15


void hw_io_init(int id,int mode)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB , ENABLE);
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE);

    //GSM RST
#if 0
    if( (id==GSM_RST)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //�������
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
#endif
#if 0
    //GSM PWR ON/OFF
    if( (id==GSM_PWR)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Speed_2MHz;    //�������
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    //#define HWIO_LED_1    GPIOB,GPIO_Pin_8
    if( (id==LED1)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //�������
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    //#define HWIO_LED_2    GPIOB,GPIO_Pin_9
    if( (id==LED2)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //�������
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    //#define HWIO_LED_3    GPIOB,GPIO_Pin_13
    if( (id==LED3)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //�������
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
	#endif /// no led no gsm
    //BUZZER    PB14
    #if 0
    if( (id==BUZZER)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //�������
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
	#endif  ////��Ϊ�� key
    //USB DETECTED,PB12
    if( (id==USB_DET)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;       //��������
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    //SIM DETECT,PB15
    if( (id==SIM_DET)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;       //��������
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    //#define HWIO_GPS_PWR  GPIOB,GPIO_Pin_5      //PB5
    if( (id==GPS_PWR)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //�������
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
}


unsigned int hw_io_ctrl(int id,int mode)
{
    unsigned int rslt=1;
    
    if( mode==GET )
    {
        switch(id)
        {
            case USB_DET:
                rslt = (int)GPIO_ReadInputDataBit(HWIO_USB_DET);
                break;
            case SIM_DET:
                rslt = (int)GPIO_ReadInputDataBit(HWIO_SIM_DET);
                break;
            default:
                break;
        }
        
        return rslt;
    }
    /* GSM RST */
#if 0
    if( id==GSM_RST ){
        switch(mode){
            case HW_RST:
                GPIO_SetBits(HWIO_GSM_RST);
                Wait10Ms(2);
                GPIO_ResetBits(HWIO_GSM_RST);
                break;
        }
    }
#endif
    /* GSM PWR */
#if 0
    if( id==GSM_PWR ){
        switch(mode){
            case POWER_OFF:
            case POWER_ON:
                GPIO_SetBits(HWIO_GSM_PWR);
                Wait10Ms(150);
                GPIO_ResetBits(HWIO_GSM_PWR);
                break;
        }
    }
	#endif 
	#if 0
    /* LED1 */
    if( id==LED1 ){
        switch(mode){
            case ON:
                GPIO_SetBits(HWIO_LED_1);
                break;
            case OFF:
                GPIO_ResetBits(HWIO_LED_1);
                break;
        }
    }
    /* LED2 */
    if( id==LED2 ){
        switch(mode){
            case ON:
                GPIO_SetBits(HWIO_LED_2);
                break;
            case OFF:
                GPIO_ResetBits(HWIO_LED_2);
                break;
        }
    }
    /* BUZZER */
    if( id==BUZZER ){
        switch(mode){
            case ON:
                GPIO_SetBits(HWIO_BUZZER);
                break;
            case OFF:
                GPIO_ResetBits(HWIO_BUZZER);
                break;
        }
    }
	#endif  ///no led
    /* GPS POWER */
    if( id==GPS_PWR ){
        switch(mode){
            case ON:
                GPIO_SetBits(HWIO_GPS_PWR);
                break;
            case OFF:
                GPIO_ResetBits(HWIO_GPS_PWR);
                break;
        }
    }
    
    return rslt;
}

/*----------------------------------------------------------------------------*/
/* led_task                                                                   */
/*                                                                            */
/*----------------------------------------------------------------------------*/

//type LED_1HZ, LED_2HZ, LED_ON, LED_OFF, LED_ON_T
#define LED_NUM 2

struct LED_CTRL{
    int name;
    int taskstate;
    int on_cnt;
    int off_cnt;
}led[LED_NUM];


void led_taskinit(void)
{
    led[0].name = LED1;
    led[0].taskstate = LED_IDLE;
    
    led[1].name = LED2;
    led[1].taskstate = LED_IDLE;
    
}


void led_task(void)
{
    int i;
    
    for( i=0; i<LED_NUM; i++){
        switch( led[i].taskstate ){
            case FLASH_5HZ:
                if( led[i].on_cnt>0 ){
                    led[i].on_cnt--;
                    if( led[i].off_cnt==0 )
                        hw_io_ctrl(led[i].name,OFF);
                        led[i].off_cnt = 5;
                }else{
                    if( led[i].off_cnt>0 ){
                        led[i].off_cnt--;
                    }else{
                        led[i].on_cnt = 15;
                        hw_io_ctrl(led[i].name,ON);
                    }
                }
                break;
            case FLASH_2HZ:
                if( led[i].on_cnt>0 ){
                    led[i].on_cnt--;
                    if( led[i].off_cnt==0 )
                        hw_io_ctrl(led[i].name,OFF);
                        led[i].off_cnt = 20;
                }else{
                    if( led[i].off_cnt>0 ){
                        led[i].off_cnt--;
                    }else{
                        led[i].on_cnt = 30;
                        hw_io_ctrl(led[i].name,ON);
                    }
                }
                break;


            case FLASH_1HZ:
                if( led[i].on_cnt>0 ){
                    led[i].on_cnt--;
                    if( led[i].off_cnt==0 )
                        hw_io_ctrl(led[i].name,OFF);
                        led[i].off_cnt = 20;
                }else{
                    if( led[i].off_cnt>0 ){
                        led[i].off_cnt--;
                    }else{
                        led[i].on_cnt = 80;
                        hw_io_ctrl(led[i].name,ON);
                    }
                }
                break;

            case FLASH_SLOW:
                if( led[i].on_cnt>0 ){
                    led[i].on_cnt--;
                    if( led[i].off_cnt==0 )
                        hw_io_ctrl(led[i].name,OFF);
                        led[i].off_cnt = 100;
                }else{
                    if( led[i].off_cnt>0 ){
                        led[i].off_cnt--;
                    }else{
                        led[i].on_cnt = 100;
                        hw_io_ctrl(led[i].name,ON);
                    }
                }
                break;

            case ON_T:
                if(led[i].on_cnt==0){
                    hw_io_ctrl(led[i].name,OFF);
                    led[i].taskstate = LED_IDLE;
                }else
                	led[i].on_cnt--;
                break;
    
            default:
            case LED_IDLE:
                break;
        }
    }
}


void led_taskctrl(int led_name, unsigned char type, int value)
{
    int i;
    
    if( led_name==LED1 )
        i = 0;
    else if(led_name==LED2 )
        i = 1;
    else if(led_name==LED3 )
        i = 2;

    if( i>LED_NUM )
        return;
    if( led[i].taskstate == type )
        return;
        
    led[i].taskstate = type;
    
    switch(type){
        case FLASH_5HZ:
            hw_io_ctrl(led_name,ON);
            led[i].off_cnt = 5;
            led[i].on_cnt = 15;
            break;
            
        case FLASH_2HZ:
            hw_io_ctrl(led_name,ON);
            led[i].off_cnt = 20;
            led[i].on_cnt = 30;
            break;
        
        case FLASH_1HZ:
            hw_io_ctrl(led_name,ON);
            led[i].off_cnt = 50;
            led[i].on_cnt = 50;
            break;
        case FLASH_SLOW:
            hw_io_ctrl(led_name,ON);
            led[i].off_cnt = 120;
            led[i].on_cnt = 80;
            break;
        case ON:
            hw_io_ctrl(led_name,ON);
            break;
        case OFF:
            hw_io_ctrl(led_name,OFF);
            break;
        case ON_T:
            hw_io_ctrl(led_name,ON);
            led[i].on_cnt = value;        //��һ�µ�ʱ��
            break;
    }
}

/*----------------------------------------------------------------------------*/
/* beep_task                                                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/
unsigned char beep_taskstate;
unsigned char beep_on_cnt,beep_off_cnt;
unsigned int beep_cnt;


void beep_taskinit(void)
{
    hw_io_ctrl(BUZZER,OFF);
    beep_taskstate = BEEP_IDLE;
}

void beep_task(void)
{
    switch(beep_taskstate){
        case BEEP_ON_T:
            if(beep_on_cnt>0){
                beep_on_cnt--;
                if(beep_on_cnt==0){
                    hw_io_ctrl(BUZZER,OFF);
                    beep_taskstate = BEEP_IDLE;
                }
            }
            break;
            
        case BEEP_CNT:
            if(beep_cnt>0){
                if(beep_on_cnt){
                    beep_on_cnt--;
                    if(beep_on_cnt==0){
                        beep_off_cnt = 15;
                        hw_io_ctrl(BUZZER,OFF);
                    }
                }else{
                    if(beep_off_cnt){
                        beep_off_cnt--;
                        if( beep_off_cnt==0 ){
                            beep_on_cnt = 15;
                            beep_cnt--;
                            if( beep_cnt > 0 )
                                hw_io_ctrl(BUZZER,ON);
                        }
                    }
                }
            }else
                beep_taskstate = BEEP_IDLE;
            break;
        
        case BEEP_IDLE:
        default:
            break;
    }
}

void beep_taskctrl(unsigned char type,int value)
{
    beep_taskstate = type;
    
    //ϵͳ��������
    if( g_SysCfg.BEEP_ON == false )
        if( beep_taskstate!=BEEP_OFF )
            return;
    
    switch(beep_taskstate){
        case BEEP_CNT:
            beep_cnt = value;    //������
            beep_off_cnt = 15;
            beep_on_cnt = 15;
            hw_io_ctrl(BUZZER,ON);
            break;

        case BEEP_ON_T:
            beep_on_cnt = value;
            hw_io_ctrl(BUZZER,ON);
            break;

        case BEEP_OFF:
            hw_io_ctrl(BUZZER,OFF);
            break;
            
        case BEEP_ON:
            hw_io_ctrl(BUZZER,ON);
            break;
    }
}

/*----------------------------------------------------------------------------*/
/* ARM Sleepģʽ                                                              */
/*                                                                            */
/*----------------------------------------------------------------------------*/
static void myPWR_EnterSleepMode(void)
{
/*
ͨ��ִ��WFI��WFEָ�����˯��״̬������Cortex.-M3ϵͳ���ƼĴ����е�SLEEPONEXIT
λ��ֵ��������ѡ�������ѡ��˯��ģʽ������ƣ�
�� SLEEP-NOW�����SLEEPONEXITλ���������WRI��WFE��ִ��ʱ��΢������������
��˯��ģʽ��
�� SLEEP-ON-EXIT�����SLEEPONEXITλ����λ��ϵͳ��������ȼ����жϴ����������
��ʱ��΢����������������˯��ģʽ��
*/
    //GPIO_InitTypeDef GPIO_InitStructure;
    //SPI_InitTypeDef SPI_InitStructure;
    
    USART_Cmd(USART1, DISABLE);
    USART_Cmd(USART2, DISABLE);
    USART_Cmd(USART3, DISABLE);
    SysTick_Ctrl(DISABLE);
    //
    PWR_EnterSTOPMode(PWR_Regulator_LowPower,PWR_STOPEntry_WFI);        //PWR_Regulator_ON,PWR_Regulator_LowPower
    //
    SysTick_Ctrl(ENABLE);
    USART_Cmd(USART1, ENABLE);
    USART_Cmd(USART2, ENABLE);
    USART_Cmd(USART3, ENABLE);
    SysTick_Ctrl(ENABLE);
    SystemInit();
}


/*----------------------------------------------------------------------------*/
/* ��ص�ѹ���                                                               */
/*                                                                            */
/*----------------------------------------------------------------------------*/
//#define AD_Scale (float)3.3*2.023/4096
#define AD_Scale (float)3.3*2.06/4096

unsigned long adc_task_t_mark;
char adc_task_st,adc_read_cnt;
static float adc_value_sum;


void ADC_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  ADC_InitTypeDef ADC_InitStructure;
  
  /* GPIO CONFIG */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* -------------------------------------------------------------------------*/
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_AFIO, ENABLE);
    
  /* ADC1 configuration ------------------------------------------------------*/
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;       /* ����ģʽ */
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;             /* ������ͨ��ģʽ */
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;       /* ����ת�� */
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;  /* ת������������ */
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;               /* �Ҷ��� */
  ADC_InitStructure.ADC_NbrOfChannel = 1;                      /* ɨ��ͨ���� */
  ADC_Init(ADC1, &ADC_InitStructure);

 /* ADC1 regular channel8 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5);  /* ͨ��X,����ʱ��Ϊ55.5����,1�������ͨ����1�� */
  //ADC_Cmd(ADC1, ENABLE);                 /* Enable ADC1 */                      
  //ADC_SoftwareStartConvCmd(ADC1,ENABLE);    /* ʹ��ת����ʼ */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_AFIO, DISABLE);
}


char ADC_taskctrl(char cmd)
{
	adc_task_st = cmd;
    
    return adc_task_st;
}

char ADC_task(void)
{
    
    switch(adc_task_st)
    {
        case 0: //�ȴ�
            if( GetSysTick_Sec()>adc_task_t_mark+10 ){
                adc_task_t_mark = GetSysTick_Sec();
                adc_task_st = 1;
            }
            break;
        case 1: //����ADC
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_AFIO, ENABLE);
            ADC_Cmd(ADC1, ENABLE);                  /* Enable ADC1 */
            ADC_SoftwareStartConvCmd(ADC1,ENABLE);  /* ʹ��ת����ʼ */
            adc_value_sum = 0.0;
            adc_read_cnt = 0;
            adc_task_st = 2;
            break;
        case 2: //��ȡADCֵ
            if(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)==SET){
                adc_value_sum = adc_value_sum+ADC_GetConversionValue(ADC1)*AD_Scale;
                adc_read_cnt++;
                adc_task_st = 3;
            }
            break;
        case 3: //��ȡ����
            if( adc_read_cnt<5 )
                adc_task_st = 2;
            else{
                g_BatteryVoltage = adc_value_sum/5;
                if((g_BatteryVoltage+0.1) < g_pre_BatteryVoltage)
                    g_BatteryVoltage = g_pre_BatteryVoltage;
                g_pre_BatteryVoltage = adc_value_sum/5;
                //g_BatteryVoltage = (g_BatteryVoltage+adc_value_sum/5)/2;
                adc_task_st = 4;
                printf("\r\n*-----BatteryVol:%4.2fV---------*\r\n", g_BatteryVoltage);
                if( hw_io_ctrl(USB_DET,GET) ){
                    if( g_BatteryVoltage<V_PWR_LOW )
                        led_taskctrl(LED1,FLASH_5HZ,0);
                    else
                        led_taskctrl(LED1,ON,0);
                }else if( g_BatteryVoltage>V_PWR_ALARM )
                    led_taskctrl(LED1,ON,0);
                else if( g_BatteryVoltage<V_PWR_LOW )
                    led_taskctrl(LED1,FLASH_5HZ,0);
                else
                    led_taskctrl(LED1,FLASH_1HZ,0);
            }
            break;
        case 4: //�ر�ADC
            ADC_Cmd(ADC1, DISABLE);                  /* Enable ADC1 */
            ADC_SoftwareStartConvCmd(ADC1,DISABLE);  /* ʹ��ת����ʼ */
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_AFIO, DISABLE);
            adc_task_st = 0;
            break;
    }
    return adc_task_st;
}

void ADC_taskinit(void)
{
    adc_task_t_mark = 0;
    //
    ADC_Configuration();
    //
	adc_task_st = 1;
    while( ADC_task()<4 );
}

