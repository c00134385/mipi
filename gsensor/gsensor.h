#ifndef __GSENSOR_H_
#define __GSENSOR_H_


typedef struct{
    int gsensor_work_state;
    int new_flg;
    int shock_cnt;
    int shock_level;
    unsigned long shock_time;         //连续振动时间
    unsigned long static_time;        //连续静止时间
    unsigned long stat_period;        //统计时间
    unsigned long stat_t_start;     //统计开始
}GSENSOR_INFO;

/* Private function prototypes -----------------------------------------------*/
void I2C_Configuration(void);
FunctionalState I2C_WriteByte(uint8_t SendByte, uint16_t WriteAddress, uint8_t DeviceAddress);
FunctionalState I2C_ReadBytes(uint8_t* pBuffer,   uint16_t length,   uint16_t ReadAddress,  uint8_t DeviceAddress);

int MBA220init(void);
int MBA220_INT(void);
int gSensor_taskinit(void);
void gsensor_task(void);


//void gsensor_task(void);
void set_gsensor_ctrl(int ctrl , int value);
GSENSOR_INFO *get_gsensor_state(void);


enum{
    GSNESOR_WORK_MODE,  //GSENSOR_NORMAL GSENSOR_SLEEP  GSENSOR_LOW_POWER
    GSENSOR_RESET_CAL,
};  // GSENSOR_CTRL;


enum{
    GSENSOR_NORMAL,
    GSENSOR_SLEEP,
    GSENSOR_LOW_POWER,
};  // GSENSOR_MODE;

#endif
