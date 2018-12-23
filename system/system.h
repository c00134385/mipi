#ifndef __SYSTEM_H_
#define __SYSTEM_H_


enum{
    BEEP_IDLE,
    BEEP_ON_T,
    BEEP_ON,
    BEEP_OFF,
    BEEP_CNT,
};

/* 系统状态 */
enum{
    SYS_NORMAL_TRY=0,          //普通模式开机
    SYS_NORMAL=10,             //普通模式
    SYS_ONLY_GPX=20,           //只本地存储
    SYS_SAVE_ENERGY=30,        //节能模式
    SYS_SLEEP=40,              //待机模式
    USB_MODE=50,               //USB模式
    SYS_ERROR=60               //系统错误
};

/* hw control */
enum{
    HW_RST,
    POWER_OFF,
    POWER_ON,
    ON,
    OFF,
    GET,
};

enum{
    ALL_HWIO,
    GSM_RST,
    GSM_PWR,
    LED1,
    LED2,
    LED3,
    BUZZER,
    USB_DET,
    SIM_DET,
    GPS_PWR,
    //FOR LED DEFINE
    LED_IDLE,
    FLASH_SLOW,
    FLASH_1HZ,
    FLASH_2HZ,
    FLASH_5HZ,
    ON_T,
};


#define CHARGING        (UINT)1
#define NORMAL_BATT     (UINT)1<<1
#define UNDER_VOLTAGE   (UINT)1<<2
#define OVER_VOLTAGE    (UINT)1<<3


#define HW_ERR_FLASH    (UINT)1
#define HW_ERR_GSENSOR  (UINT)1<<1
#define HW_ERR_GPS      (UINT)1<<2
#define HW_ERR_GPRS     (UINT)1<<3
#define HW_ERR_ALL      (HW_ERR_FLASH|HW_ERR_GSENSOR|HW_ERR_GPS|HW_ERR_GPRS)

#define FS_ERR_FORMAT   (UINT)1<<8
#define CFG_ERR_NULL_F  (UINT)1<<9
#define CFG_ERR_YEELINK (UINT)1<<10
#define CFG_ERR_GSM     (UINT)1<<11

typedef struct{
    unsigned char mode;
    int power_level;
    unsigned int sys_err;
}SYSTEM_INFO;



typedef struct{
    BOOL BEEP_ON;                       //打开蜂鸣器
    BOOL SAVE_ENERGY_ON;                //允许低功耗模式
    BOOL WDT_ON;                        //允许启动看门狗
    unsigned int LET_US_GO_MAX_T;       //GPS最长间隔时间,即在无移动时最长多长时间会发送一次坐标
    unsigned int NET_SEND_T_MIN;        //网络允许发送数据最小间隔
    unsigned int ENTER_SAVE_ENERGY_T;   //n秒无振动进入低功耗模式
    unsigned int WAKEUP_SAVE_ENERGY_T;  //n秒自动从低功耗模式唤醒
    unsigned int WAKEUP_SLEEP_T;        //在这个周期内,有一次WAKEUP_SLEEP_T/2秒的连续震动,从休眠模式唤醒
    unsigned int GPRS_RETRY_T;          //如果进入GPX ONLY里面，GPRS重新尝试时间
    unsigned int GPS_SEARCH_T;          //GPS搜星时间
}SYSTEM_CFG;

//电源管理阀值定义
#define V_OPEN_GPRS     3.45            //开启MODEM的最低电压
#define V_CLOSE_GPRS    3.42            //关闭MODEM的最低电压

#define V_PWR_LOW       V_CLOSE_GPRS    //电量低于允许极限
#define V_PWR_ALARM     3.5             //电量告警
#define V_PWR_HIGH      4.3             //电量高于允许极限


//
extern SYSTEM_INFO g_System;
extern float g_BatteryVoltage;
//
void syscfg_init(void);
//
int system_taskctrl(int cmd);
void system_taskinit(void);
int system_task(void);
//
void hw_io_init(int id,int mode);
unsigned int hw_io_ctrl(int id,int mode);
//
void led_taskinit(void);
void led_task(void);
void led_taskctrl(int led_name, unsigned char type, int value);
//
void beep_taskinit(void);
void beep_task(void);
void beep_taskctrl(unsigned char type,int value);
//
void ADC_taskinit(void);
char ADC_task(void);
char ADC_taskctrl(char cmd);
/*---------------------------------------------------*/



#endif



