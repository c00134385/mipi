
//正常状态

#define MODEM_PWRON          (uint16_t)(1)     //MODEM状态:开机
#define MODEM_PWROFF         (uint16_t)(1<<1)       //MODEM状态:关机
#define MODEM_NORMAL         (uint16_t)(1<<2)    //MODEM状态:正常工作
#define MODEM_SLEEP          (uint16_t)(1<<3)    //MODEM状态:睡眠状态

#define MD_SIM_IN            (uint16_t)(1<<4)
#define MD_PWR_OK            (uint16_t)(1<<5)
#define MD_CSQ_OK            (uint16_t)(1<<6)
#define MD_GSM_OK            (uint16_t)(1<<7)
#define MD_GPRS_OK           (uint16_t)(1<<8)
#define MD_IPNET_OK          (uint16_t)(1<<9)
#define MD_IPNET_IDEL        (uint16_t)(1<<10)

#define MD_MODE_BIT_MASK       (uint16_t)(0xf)
#define MD_OK_ALL               (uint16_t)(MODEM_NORMAL|MD_CSQ_OK|MD_SIM_IN|MD_GSM_OK|MD_GPRS_OK)

//错误状态
#define MD_ERR_POWERON          (uint16_t)(1<<11)       //MODEM没开机
#define MD_ERR_NO_ACK           (uint16_t)(1<<12)       //系统长期应答失败
#define MD_ERR_NET_NULL         (uint16_t)(1<<13)       //没有网络
#define MD_ERR_SIM              (uint16_t)(1<<14)       //无SIM卡,或错误
#define MD_ERR_VBAT             (uint16_t)(1<<15)       //电压异常
#define MD_ERR_ALL              ((uint16_t)(MD_ERR_SIM))


typedef enum{
    AT_ACK_NULL,
    AT_ACK_UNKNOW,
    AT_ACK_OK,
    AT_ACK_ERROR
}AT_ACK_TYPE;

#define ATERR_TIMEOUT  1
#define ATERR_HW       1<<1

//
enum{
	MODEM_IDEL,
    MODEM_POWER_ON,
    MODEM_CHECK,
	MODEM_RETRY,
    MODEM_POWER_OFF,
	//
	WINSOCKET_SEND =10,
	WINSOCKET_SEND_QUERY,
	WINSOCKET_ERROR,
	WINSOCKET_FINISHED,
    //
	MODEM_HW_ERROR = 100,
};


enum{
    MD_POWER_OFF = 0,
    MD_POWER_OFF_END = 0xff,
};


enum{
    MD_POWER_ON = 0,
    MD_POWER_ON_ERROR = 0xfe,
    MD_POWER_ON_END = 0xff,
};

void gprs_task(void);
void gprs_taskinit(void);
void gprs_taskctrl(int cmd);
uint16_t GetModemState(uint16_t BIT_MASK);
uint16_t GetModemMode(void);


