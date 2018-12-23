#ifndef __SYSTEM_H_
#define __SYSTEM_H_


enum{
    BEEP_IDLE,
    BEEP_ON_T,
    BEEP_ON,
    BEEP_OFF,
    BEEP_CNT,
};

/* ϵͳ״̬ */
enum{
    SYS_NORMAL_TRY=0,          //��ͨģʽ����
    SYS_NORMAL=10,             //��ͨģʽ
    SYS_ONLY_GPX=20,           //ֻ���ش洢
    SYS_SAVE_ENERGY=30,        //����ģʽ
    SYS_SLEEP=40,              //����ģʽ
    USB_MODE=50,               //USBģʽ
    SYS_ERROR=60               //ϵͳ����
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
    BOOL BEEP_ON;                       //�򿪷�����
    BOOL SAVE_ENERGY_ON;                //����͹���ģʽ
    BOOL WDT_ON;                        //�����������Ź�
    unsigned int LET_US_GO_MAX_T;       //GPS����ʱ��,�������ƶ�ʱ��೤ʱ��ᷢ��һ������
    unsigned int NET_SEND_T_MIN;        //����������������С���
    unsigned int ENTER_SAVE_ENERGY_T;   //n�����񶯽���͹���ģʽ
    unsigned int WAKEUP_SAVE_ENERGY_T;  //n���Զ��ӵ͹���ģʽ����
    unsigned int WAKEUP_SLEEP_T;        //�����������,��һ��WAKEUP_SLEEP_T/2���������,������ģʽ����
    unsigned int GPRS_RETRY_T;          //�������GPX ONLY���棬GPRS���³���ʱ��
    unsigned int GPS_SEARCH_T;          //GPS����ʱ��
}SYSTEM_CFG;

//��Դ����ֵ����
#define V_OPEN_GPRS     3.45            //����MODEM����͵�ѹ
#define V_CLOSE_GPRS    3.42            //�ر�MODEM����͵�ѹ

#define V_PWR_LOW       V_CLOSE_GPRS    //��������������
#define V_PWR_ALARM     3.5             //�����澯
#define V_PWR_HIGH      4.3             //��������������


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



