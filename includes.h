//编译系统函数
#include "stdlib.h"
#include "stdio.h"
#include "memory.h"
#include "math.h"
#include "string.h"

//硬件基础函数
#include "stm32f10x.h"

//数据类型定义
#include "integer.h"

//系统硬件配置函数
#include "sof_iic.h"
#include "system.h"
#include "systick.h"
#include "uart1.h"
#include "uart2.h"
#include "uart3.h"
#include "sys_cfg.h"

//USB
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "mass_mal.h"



////按键
#include "key.h"

////定时器
#include "timer.h"

//外设驱动函数
#include "gsensor.h"
//#include "sim800.h"
#include "pelco_d.h"
//#include "pelco_p.h"
#include "visca_p.h"
#include "visca.h"
#include "sony_ui.h"
#include "sonylens.h"
#include "w25xx.h"
#include "eeprom.h"


//基础函数
//#include "basic_f.h"
#include "my_string.h"
#include "datetime.h"

//网络应用
#include "tcpip.h"

//应用函数
#include "gps_algorithm.h"
#include "ff.h"                 //文件系统

//任务进程
#include "gps_receive_task.h"
#include "nmea_parse.h"
#include "let_us_go.h"
#include "yeelink.h"
#include "gpx_record_task.h"



