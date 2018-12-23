/*----------------------------------------------------------------------------*/
/* system.c                                                                   */
/* 描述:系统调度程序                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* 2014/6/7 manwjh 建立 V0.1                                                  */
/* 2014/7/12 manwjh v0.2  modem初始化完成后,只读取状态和就异常进行跳转.       */
/* 2014/7/23 manwjh v0.3                                                      */
/* 2014/8/30 manwjh v0.4  低功耗模式取消掉MCU睡眠                             */
/*           增加g_SysCfg.WAKEUP_SAVE_ENERGY_T变量,定时唤醒并发送坐标。       */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* include */
#include "includes.h"

/* external */
extern GPS_INFO_TYPE g_GpsInfo;        //gps 数据信息
extern unsigned long g_YeelinkNextTime;
extern UPLOAD_POS_BF iPos;

/* public */
SYSTEM_CFG g_SysCfg;
SYSTEM_INFO g_System;
GSENSOR_INFO *gSensor;
int SaveEnergyStep;
int SleepStep;
int gUsbLinked=0;       //wjh add, 判断是否进入U盘模式
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
        case SYS_NORMAL_TRY:                //普通模式开机
            //
            set_gsensor_ctrl(GSNESOR_WORK_MODE,GSENSOR_NORMAL);
			//gprs_taskctrl(MODEM_POWER_ON);		//test gprs
            system_task_t_mark = GetSysTick_Sec();
            g_System.mode = SYS_NORMAL_TRY;
            //
            printf("\r\nSYS-->开机模式\r\n");
            break;

        case SYS_NORMAL:                    //普通模式
            iTrack_taskctrl(TRACK_START);
            //
            gSensor->static_time = 0;
            system_task_t_mark = GetSysTick_Sec();
            g_System.mode = SYS_NORMAL;
            //
            printf("\r\nSYS-->标准模式\r\n");
            break;
            
        case SYS_ONLY_GPX:                  //只本地存储
            #if 0
			gprs_taskctrl(MODEM_POWER_OFF);
            //
            system_task_t_mark = GetSysTick_Sec();
            g_System.mode = SYS_ONLY_GPX;
            //
            printf("\r\nSYS-->GPX ONLY模式\r\n");
			#endif
            break;
            
        case SYS_SAVE_ENERGY:               //节能模式
            if( g_SysCfg.SAVE_ENERGY_ON==true ){
				gpx_file_save();
                gps_taskctrl(GPS_POWER_OFF);
                //
                printf("\r\nSYS-->准备进入低功耗模式\r\n");
                //
                set_gsensor_ctrl(GSNESOR_WORK_MODE,GSENSOR_NORMAL);
    			system_task_t_mark = GetSysTick_Sec();
                g_System.mode = SYS_SAVE_ENERGY;
                SaveEnergyStep = 0;
            }
            break;
            
        case SYS_SLEEP:                     //待机模式
            #if 0
			gprs_taskctrl(MODEM_POWER_OFF);
            gps_taskctrl(GPS_POWER_OFF);
            iTrack_taskctrl(TRACK_END);
            gpx_file_saveexit();
            //
            printf("\r\nSYS-->准备进入休眠模式\r\n");
            //
            set_gsensor_ctrl(GSNESOR_WORK_MODE,GSENSOR_NORMAL);
            system_task_t_mark = GetSysTick_Sec();
            g_System.mode = SYS_SLEEP;
            SleepStep = 0;
			#endif
            break;
            
        case USB_MODE:               //USB模式
            g_System.mode = USB_MODE;
            printf("\r\nSYS-->USB模式\r\n");
            break;
            
        case SYS_ERROR:              //系统错误
            g_System.mode = SYS_ERROR;
            printf("\r\nSYS-->系统模式\r\n");
            break;
            
        default:
            printf("\r\nSYS-->无效模式\r\n");
            break;
    }
    return g_System.mode;
}


int system_task(void)
{
    float batt_tmp;
    unsigned long sec_tick_tmp;
	int i;
    
    
    //检测是否有外部电源
    if( hw_io_ctrl(USB_DET,GET) )
        gPowerState |= CHARGING;
    else
        gPowerState &= ~CHARGING;

    //电源管理
    #if 0
    if( g_BatteryVoltage<V_PWR_LOW ){
        //如果电量低于3.3V,系统进入SLEEP模式
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
    //获取gSensor状态
    if( gSensor->new_flg ){
        gSensor->new_flg = 0;
        //printf("\r\ngSensor-->连续振动时间:%d,连续静止时间:%d,振动次数:%d",gSensor->shock_time,gSensor->static_time,gSensor->shock_cnt);
    }
    //时间卫星同步
    if( g_GpsInfo.pos_update==true ){
        sys_time( &g_GpsInfo.time );
    }
        
    
    switch(g_System.mode)
    {
        case SYS_NORMAL_TRY:
            //如果GPS没有启动,则启动GPS模块
            #if 0
            if( GetGpsState(GPS_POWRERON)==0 ){
                gps_taskctrl(GPS_POWER_ON);
                break;
            }
			
            //如果GPS定位完成,则启动或者唤醒GPRS模块
            if( GetGpsState(GPS_LOCATED)==GPS_LOCATED ){
                if( (g_BatteryVoltage>V_OPEN_GPRS)&&(GetModemState(MD_SIM_IN)==MD_SIM_IN) ){
                    gprs_taskctrl(MODEM_POWER_ON);
                    //只要找到GPS信号立即转到NORMAL状态,GPRS最终是否能正常工作则交由NORMAL进程决定
                    g_YeelinkNextTime = 0;  //随时开始上传第一个点。
                    system_taskctrl(SYS_NORMAL);
                }else{
                    system_taskctrl(SYS_ONLY_GPX);
                }
            }
            //如果GPS无法完成定位,则系统进入休眠
            if( GetGpsState(GPS_ERR_LOCATE_FAIL|GPS_ERR_HW)!=0 )
                system_taskctrl(SYS_SAVE_ENERGY);
			#endif

            break;
        
        case SYS_NORMAL:
			#if 0
            //如果GPS出现定位失败,则转到低功耗状态
            if( GetGpsState(GPS_ERR_LOCATE_FAIL|GPS_ERR_HW)!=0 ){
                system_taskctrl(SYS_SAVE_ENERGY);
            }
            //如果GPRS检测到错误,则转到GPX状态
            if( (g_BatteryVoltage<V_CLOSE_GPRS)||GetModemState(MD_ERR_ALL) ){
                system_taskctrl(SYS_ONLY_GPX);
            }
            //检测gSensor判定为静止状态
            if( (gSensor->static_time>g_SysCfg.ENTER_SAVE_ENERGY_T)&&(gSensor->shock_time==0) ){
                system_taskctrl(SYS_SAVE_ENERGY);
            }
			#endif
            break;

        //只进行本地GPX记录模式
        //进入该状态的所有原因: MODEM硬件错误(SIM卡错,MODEM电压不符合)
        //退出该状态的所有原因: MODEM硬件错误自己恢复
        case SYS_ONLY_GPX:
			#if 0
            //如果GPS出现定位失败,则转到省电模式
            if( GetGpsState(GPS_ERR_LOCATE_FAIL|GPS_ERR_HW)!=0 ){
                system_taskctrl(SYS_SAVE_ENERGY);
            }
            //检测gSensor判定为静止状态
            if( (gSensor->static_time>g_SysCfg.ENTER_SAVE_ENERGY_T)&&(gSensor->shock_time==0) ){
                system_taskctrl(SYS_SAVE_ENERGY);
            }
			//如果GPRS电压恢复正常,并且SIM插入了
			if( (g_BatteryVoltage>V_OPEN_GPRS)&&(GetModemState(MD_SIM_IN)==MD_SIM_IN) ){
				system_taskctrl(SYS_NORMAL_TRY);
			}
			#endif
            break;

        //节能模式
        //进入该状态的所有原因:GPS无信号,电量低,gSensor静止算法
        //退出该状态的所有原因:电量足够,gSensor晃动算法
        case SYS_SAVE_ENERGY:
			#if 0
            switch(SaveEnergyStep){
                case 0: //
                    if( (iPos.pos_num==0)||(GetModemState(MD_OK_ALL)!=MD_OK_ALL)||( GetSysTick_Sec()>system_task_t_mark+200 ) ){
                        gprs_taskctrl(MODEM_POWER_OFF);
                        SaveEnergyStep = 1;
                    }
                    break;
                case 1: //关闭周边
                    if( GetModemMode()==MODEM_PWROFF ){
                        led_taskctrl(LED1,OFF,0);
                        system_task_t_mark = GetSysTick_Sec();
                        SaveEnergyStep = 2;
                    }
                    break;
                case 2: //休眠监测
                    if( (gSensor->shock_time>3)||(GetSysTick_Sec()>(system_task_t_mark+g_SysCfg.WAKEUP_SAVE_ENERGY_T)) ){
                        if( g_BatteryVoltage>V_OPEN_GPRS ){
                            led_taskctrl(LED1,OFF,1);
    				        system_task_t_mark = GetSysTick_Sec();
    				        SaveEnergyStep = 3;
				        }
				    }
                    break;
                case 3: //低功耗唤醒
                    system_taskctrl(SYS_NORMAL_TRY);
                    break;
                
            }
			#endif
            break;
            
        //待机模式
        //进入该状态的所有原因:gsensor算法
        //退出该状态的所有原因:gsensor算法
        case SYS_SLEEP:
			#if 0
            switch(SleepStep)
            {
                case 0: //准备进入Sleep
                    gprs_taskctrl(MODEM_POWER_OFF);
                    gps_taskctrl(GPS_POWER_OFF);
                    SleepStep = 1;
                    break;
                case 1: //等待系统完成处理
                    if( (GetModemState(MD_PWR_OK)==0)&&(GetGpsState(GPS_POWRERON)==0) ){
                        SleepStep = 2;
                    }
                    break;
                case 2: //睡眠
                    led_taskctrl(LED1,OFF,0);
                    myPWR_EnterSleepMode();
                    //检测电压是否符合唤醒条件
                    ADC_taskctrl(1);
                    while( ADC_task()!=4 );
                    if( g_BatteryVoltage>V_OPEN_GPRS ){
                        system_task_t_mark = GetSysTick_Sec();
                        SleepStep = 3;
                    }
                    break;
                case 3: //醒来WAKEUP_SLEEP_T秒
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

        //USB模式
        //进入该状态的所有原因:USB电脑连接
        //退出该状态的所有原因:USB电脑连接断开
        case USB_MODE:               //USB模式
            break;
    
        //系统错误
        //进入该状态的所有原因:GPS通讯不成功
        //退出该状态的所有原因:只有关机重启动,再次自检
        case SYS_ERROR:              //系统错误
            break;
    }
    
    return 0;
}



/*----------------------------------------------------------------------------*/
/* GPIO控制相关                                                               */
/* 注意MODEM的POWER_ON,POWER_OFF这是虚拟的                                    */
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
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //推挽输出
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
#endif
#if 0
    //GSM PWR ON/OFF
    if( (id==GSM_PWR)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Speed_2MHz;    //推挽输出
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    //#define HWIO_LED_1    GPIOB,GPIO_Pin_8
    if( (id==LED1)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //推挽输出
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    //#define HWIO_LED_2    GPIOB,GPIO_Pin_9
    if( (id==LED2)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //推挽输出
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    //#define HWIO_LED_3    GPIOB,GPIO_Pin_13
    if( (id==LED3)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //推挽输出
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
	#endif /// no led no gsm
    //BUZZER    PB14
    #if 0
    if( (id==BUZZER)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //推挽输出
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
	#endif  ////改为了 key
    //USB DETECTED,PB12
    if( (id==USB_DET)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;       //下拉输入
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    //SIM DETECT,PB15
    if( (id==SIM_DET)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;       //上拉输入
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }
    //#define HWIO_GPS_PWR  GPIOB,GPIO_Pin_5      //PB5
    if( (id==GPS_PWR)||(id==ALL_HWIO) ){
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //推挽输出
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
            led[i].on_cnt = value;        //亮一下的时间
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
    
    //系统允许发声吗？
    if( g_SysCfg.BEEP_ON == false )
        if( beep_taskstate!=BEEP_OFF )
            return;
    
    switch(beep_taskstate){
        case BEEP_CNT:
            beep_cnt = value;    //响三次
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
/* ARM Sleep模式                                                              */
/*                                                                            */
/*----------------------------------------------------------------------------*/
static void myPWR_EnterSleepMode(void)
{
/*
通过执行WFI或WFE指令进入睡眠状态。根据Cortex.-M3系统控制寄存器中的SLEEPONEXIT
位的值，有两种选项可用于选择睡眠模式进入机制：
● SLEEP-NOW：如果SLEEPONEXIT位被清除，当WRI或WFE被执行时，微控制器立即进
入睡眠模式。
● SLEEP-ON-EXIT：如果SLEEPONEXIT位被置位，系统从最低优先级的中断处理程序中退
出时，微控制器就立即进入睡眠模式。
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
/* 电池电压检测                                                               */
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
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;       /* 独立模式 */
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;             /* 连续多通道模式 */
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;       /* 连续转换 */
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;  /* 转换不受外界决定 */
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;               /* 右对齐 */
  ADC_InitStructure.ADC_NbrOfChannel = 1;                      /* 扫描通道数 */
  ADC_Init(ADC1, &ADC_InitStructure);

 /* ADC1 regular channel8 configuration */ 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5);  /* 通道X,采用时间为55.5周期,1代表规则通道第1个 */
  //ADC_Cmd(ADC1, ENABLE);                 /* Enable ADC1 */                      
  //ADC_SoftwareStartConvCmd(ADC1,ENABLE);    /* 使能转换开始 */
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
        case 0: //等待
            if( GetSysTick_Sec()>adc_task_t_mark+10 ){
                adc_task_t_mark = GetSysTick_Sec();
                adc_task_st = 1;
            }
            break;
        case 1: //启动ADC
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_AFIO, ENABLE);
            ADC_Cmd(ADC1, ENABLE);                  /* Enable ADC1 */
            ADC_SoftwareStartConvCmd(ADC1,ENABLE);  /* 使能转换开始 */
            adc_value_sum = 0.0;
            adc_read_cnt = 0;
            adc_task_st = 2;
            break;
        case 2: //读取ADC值
            if(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)==SET){
                adc_value_sum = adc_value_sum+ADC_GetConversionValue(ADC1)*AD_Scale;
                adc_read_cnt++;
                adc_task_st = 3;
            }
            break;
        case 3: //读取次数
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
        case 4: //关闭ADC
            ADC_Cmd(ADC1, DISABLE);                  /* Enable ADC1 */
            ADC_SoftwareStartConvCmd(ADC1,DISABLE);  /* 使能转换开始 */
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

