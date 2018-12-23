#ifndef __GPS_RECEIVE_TASK_H_
#define __GPS_RECEIVE_TASK_H_

int gps_taskctrl(int cmd);
int gps_taskinit(void);
int gps_task(void *p);

int GetGpsState(uint16_t BIT_MASK);


enum{
    GPS_IDLE,
    GPS_POWER_ON,
    GPS_POWER_OFF,
    GPS_SEARCHING,
    GPS_RUNNING,
    GPS_HW_ERROR
};


//正常状态
#define GPS_POWRERON             (UINT)(1)
#define GPS_LCATING              (UINT)(1<<1)       //GPS定位中
#define GPS_LOCATED              (UINT)(1<<2)       //GPS定位成功
    
#define GPS_ERR_HW               (UINT)(1<<8)       //通常是串口没有信号过来
#define GPS_ERR_LOCATE_FAIL      (UINT)(1<<9)       //GPS定位失败

#endif
