#ifndef __KEY_H
#define __KEY_H	 
//#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//按键驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////   	 

#include "sys.h"
#include "integer.h"

//#define KEY0 PEin(4)   	//PE4
//#define KEY1 PEin(3)	//PE3 
//#define KEY2 PEin(2)	//PE2
//#define KEY3 PAin(0)	//PA0  WK_UP
#if 0
#define KEY0  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)//读取按键0
#define KEY1  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)//读取按键1
#define KEY2  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)//读取按键2 
#define KEY3  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//读取按键3(WK_UP) 


#endif

#define KEY_U  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)//读取按键U
#define KEY_D  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)//读取按键D
#define KEY_L  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)//读取按键L
#define KEY_R  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)//读取按键R
#define KEY_M  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)//读取按键M
#define KEY_F1  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)//读取按键F1
#define KEY_F2  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_14)//读取按键F2
#define KEY_F3  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_15)//读取按键F3
#define KEY_F4  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//读取按键F4
#define KEY_F5  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1)//读取按键F5
#define KEY_F6  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5)//读取按键F6
#define KEY_F7  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8)//读取按键F7
#define KEY_F8  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9)//读取按键F8




#define KEY_ID_F0    (0)
#define KEY_ID_F1    (1)
#define KEY_ID_F2    (2)
#define KEY_ID_F3    (3)
#define KEY_ID_F4    (4)
#define KEY_ID_F5    (5)
#define KEY_ID_F6    (6)
#define KEY_ID_F7    (7)
#define KEY_ID_F8    (8)
#define KEY_ID_F9    (9)
#define KEY_ID_U     (10)
#define KEY_ID_D     (11)
#define KEY_ID_L     (12)
#define KEY_ID_R     (13)
#define KEY_ID_M     (14)



#define KEY_U_DOWN               0x1
#define KEY_D_DOWN               0x2
#define KEY_L_DOWN               0x3
#define KEY_R_DOWN               0x4
#define KEY_M_DOWN               0x5
#define KEY_F1_DOWN              0x6
#define KEY_F2_DOWN              0x7
#define KEY_F3_DOWN              0x8
#define KEY_F4_DOWN              0x9
#define KEY_F5_DOWN              0xA
#define KEY_F6_DOWN              0xB
#define KEY_F7_DOWN              0xC
#define KEY_F8_DOWN              0xD

#define KEY_U_DOWN_UP               0x81
#define KEY_D_DOWN_UP               0x82
#define KEY_L_DOWN_UP               0x83
#define KEY_R_DOWN_UP               0x84
#define KEY_M_DOWN_UP               0x85
#define KEY_F1_DOWN_UP              0x86
#define KEY_F2_DOWN_UP              0x87
#define KEY_F3_DOWN_UP              0x88
#define KEY_F4_DOWN_UP              0x89
#define KEY_F5_DOWN_UP              0x8A
#define KEY_F6_DOWN_UP              0x8B
#define KEY_F7_DOWN_UP              0x8C
#define KEY_F8_DOWN_UP              0x8D


#if 0
#define KEY_UP 		4
#define KEY_LEFT	3
#define KEY_DOWN	2
#define KEY_RIGHT	1

#endif

typedef void (*key_handler)(void);


#define KEY_LONG_CLICK_TIME   (300)
typedef struct Key_Click {
    
    u8 key_value;
    unsigned long key_down_time;
    unsigned long key_up_time; 
} Key_Click_t;

typedef enum KEY_STATE {
    KEY_STATE_DOWN = 0,
    KEY_STATE_UP = 1,
} KEY_STATE_e;

typedef struct KEY_INFO {
    u8 id;
    char* name;

    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
    
    KEY_STATE_e key_state;
    unsigned long key_press_time;
    unsigned long key_unpress_time;
    unsigned long key_holding_time;
    bool event_pressed;
    bool event_unpressed;

    struct {
        void (*press)(void);
        void (*unpress)(void);
        void (*hold)(void);
        unsigned long hold_time_threshold;
    } handler;
} KEY_INFO_t;


void KEY_Init(void);//IO初始化
u8 KEY_Scan(u8);  	//按键扫描函数		

u8 Do_KEY_Down(void);

void key_scan_process(void);
void key_register_hander(u8 key_id, key_handler press, key_handler unpress, key_handler hold, unsigned long time);
void key_print(void);
void key_print_by_id(u8 key_id);
BOOL key_condition(u8 key_id, unsigned long time);
BOOL key_is_pressed(u8 key_id);
BOOL key_is_unpressed(u8 key_id);
BOOL key_is_long_pressed(u8 key_id, unsigned long* time);
void gennum_3g_enable(BOOL on);
void video_enable(BOOL on);
void system_reset(void);

#if 0
void gennum_reset(void);
void gennum_reset_low(void);
void gennum_reset_high(void);
#endif

#endif
