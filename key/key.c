#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
//#include "delay.h"
#include "includes.h"

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


Key_Click_t key_click;

KEY_INFO_t key_info_list[] = {
    #if 0
    {KEY_ID_M,  "CENTER",    GPIOB, GPIO_Pin_12, KEY_STATE_UP, 0, 0},
    {KEY_ID_L,  "LEFT",  GPIOB, GPIO_Pin_13, KEY_STATE_UP, 0, 0},
    {KEY_ID_R,  "RIGHT",  GPIOB, GPIO_Pin_14, KEY_STATE_UP, 0, 0},
    {KEY_ID_D,  "DOWN",  GPIOB, GPIO_Pin_15, KEY_STATE_UP, 0, 0},
    {KEY_ID_U,  "UP",  GPIOA, GPIO_Pin_8,  KEY_STATE_UP, 0, 0},
    {KEY_ID_F1, "F1",  GPIOC, GPIO_Pin_13,  KEY_STATE_UP, 0, 0},
    {KEY_ID_F2, "F2",  GPIOC, GPIO_Pin_14,  KEY_STATE_UP, 0, 0},
    {KEY_ID_F3, "F3",  GPIOC, GPIO_Pin_15,  KEY_STATE_UP, 0, 0},
    {KEY_ID_F4, "F4",  GPIOA, GPIO_Pin_0,  KEY_STATE_UP, 0, 0},
    {KEY_ID_F5, "F5",  GPIOA, GPIO_Pin_1,  KEY_STATE_UP, 0, 0},
    {KEY_ID_F6, "F6",  GPIOB, GPIO_Pin_5,  KEY_STATE_UP, 0, 0},
    {KEY_ID_F7, "F7",  GPIOB, GPIO_Pin_8,  KEY_STATE_UP, 0, 0},
    {KEY_ID_F8, "F8",  GPIOB, GPIO_Pin_9,  KEY_STATE_UP, 0, 0},
    #endif
    {KEY_ID_F1, "F1",  GPIOA, GPIO_Pin_5,  KEY_STATE_UP, 0, 0},
};
								    
//按键初始化函数
void KEY_Init(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
	//初始化KEY_U-->GPIOA.8,KEY_F4-->GPIOA.0   KEY_F5-->GPIOA.1上拉输入
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO,ENABLE);//使能PORTA,PORTEB,PORTC时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;//PA0 1 8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOa0,1,8

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2| GPIO_Pin_4;	//PB1:VIDEO_EN  PB2:SYS_RST  PB4:3G_Enable
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
    GPIO_SetBits(GPIOB,GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_4);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;	             //PA4:  WORK_LED
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
    GPIO_SetBits(GPIOA,GPIO_Pin_4);
}
//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//0，没有任何按键按下
//1，KEY0按下
//2，KEY1按下
//3，KEY2按下 
//4，KEY3按下 WK_UP
//注意此函数有响应优先级,KEY0>KEY1>KEY2>KEY3!!
u8 KEY_Scan(u8 mode)
{	 
    static u8 key_up=1;//按键按松开标志 
    u8 key_down_value = 0;
    
    if(mode)key_up=1;  //支持连按		  

    if(key_up)
    {
        //delay_ms(10);//去抖动 

        key_up=0;
        delay_nus(10000);
        if(KEY_M == 0){ key_down_value = KEY_M_DOWN ;}
        else if(KEY_R == 0) {key_down_value = KEY_R_DOWN ;}
        else if(KEY_L == 0) { key_down_value = KEY_L_DOWN ;}
        else if(KEY_D == 0) { key_down_value = KEY_D_DOWN ;}
        else if(KEY_U == 0) { key_down_value = KEY_U_DOWN ;}
        else if(KEY_F1 == 0){ key_down_value = KEY_F1_DOWN ;}
        else if(KEY_F2 == 0){ key_down_value = KEY_F2_DOWN ;}
        else if(KEY_F3 == 0){  key_down_value = KEY_F3_DOWN ;}
        else if(KEY_F4 == 0){  key_down_value = KEY_F4_DOWN ;}
        else if(KEY_F5 == 0){  key_down_value = KEY_F5_DOWN ;}
        else if(KEY_F6 == 0){  key_down_value = KEY_F6_DOWN ;}
        else if(KEY_F7 == 0){  key_down_value = KEY_F7_DOWN ;}
        else if(KEY_F8 == 0){  key_down_value = KEY_F8_DOWN ;}

        if(key_down_value != 0) {
            if(key_click.key_value == 0) {
                key_click.key_down_time = GetSysTick_10Ms(); 
                key_click.key_value = key_down_value;
            }
        } else {
            key_click.key_up_time = GetSysTick_10Ms(); 
        }

        //Wait10Ms(1);  ////去抖动 
        key_up = 1;
    } else {    /////按键弹起
      	if(KEY_M == 0) return 0;
    	else if(KEY_R == 0) return 0;
    	else if(KEY_L == 0) return 0;
    	else if(KEY_D == 0) return 0;
    	else if(KEY_U == 0) return 0;
    	else if(KEY_F1 == 0) return 0;
    	else if(KEY_F2 == 0) return 0;
    	else if(KEY_F3 == 0) return 0;
    	else if(KEY_F4 == 0) return 0;
    	else if(KEY_F5 == 0) return 0;
    	else if(KEY_F6 == 0) return 0;
    	else if(KEY_F7 == 0) return 0;
    	else if(KEY_F8 == 0) return 0;
    	key_up = 1;
    		
    }
	
 	return key_down_value;// 无按键按下
}


u8 Do_KEY_Down()
{
    bool isLongClick = FALSE;
    if(key_click.key_value == 0) {
        return 0;
    }
    if(key_click.key_up_time < key_click.key_down_time) {
        return 0;
    }

    if(key_click.key_up_time - key_click.key_down_time >= KEY_LONG_CLICK_TIME) {

        isLongClick = true;
    }

    printf("\r\nkey:%d is clicked. click time:%ld isLongClick:%d", key_click.key_value, key_click.key_up_time - key_click.key_down_time, isLongClick);
    switch(key_click.key_value)
   	{
   	    case KEY_M_DOWN:
            if(isLongClick) {
                sonylens_push_event(SONY_EVENT_MENU_ON, NULL);
            } else {
                sonylens_push_event(SONY_EVENT_NAVI_CENTER, NULL);
            }
			break;
		case KEY_R_DOWN:
            //sonylens_push_event(SONY_EVENT_ZOOM_UP, NULL);
            sonylens_push_event(SONY_EVENT_NAVI_RIGHT, NULL);
			break;
		case KEY_L_DOWN:
            //sonylens_push_event(SONY_EVENT_ZOOM_DOWN, NULL);
            sonylens_push_event(SONY_EVENT_NAVI_LEFT, NULL);
			break;
		case KEY_D_DOWN:
            //sonylens_push_event(SONY_EVENT_MONITOR_MODE_DOWN, NULL);
            sonylens_push_event(SONY_EVENT_NAVI_DOWN, NULL);
			break;
		case KEY_U_DOWN:
            //sonylens_push_event(SONY_EVENT_MONITOR_MODE_UP, NULL);
            sonylens_push_event(SONY_EVENT_NAVI_UP, NULL);
			break;
		case KEY_F1_DOWN:
			break;
		case KEY_F2_DOWN:
			break;
		case KEY_F3_DOWN:
			break;
		case KEY_F4_DOWN:
			break;
		case KEY_F6_DOWN:
			break;
		case KEY_F7_DOWN:
			break;
		case KEY_F8_DOWN:
			break;
		default:
			break;
   	}

    key_click.key_value = 0;////清掉键值
    return 0;
}

void key_scan_process(void)
{
    int i = 0;
    int count = sizeof(key_info_list)/sizeof(KEY_INFO_t);
    for(i = 0; i < count; i++)
    {
        KEY_STATE_e key_state = (KEY_STATE_e)GPIO_ReadInputDataBit(key_info_list[i].GPIOx, key_info_list[i].GPIO_Pin);
        if(key_state != key_info_list[i].key_state)
        {
            Wait10Ms(1);
            key_state = (KEY_STATE_e)GPIO_ReadInputDataBit(key_info_list[i].GPIOx, key_info_list[i].GPIO_Pin);
        }
        if((key_state != key_info_list[i].key_state) && (key_state == KEY_STATE_DOWN))
        {
            key_info_list[i].key_press_time = GetSysTick_10Ms();
            key_info_list[i].event_pressed = TRUE;
            key_info_list[i].event_unpressed = FALSE;
            key_print_by_id(key_info_list[i].id);
        }
        else if((key_state != key_info_list[i].key_state) && (key_state == KEY_STATE_UP))
        {
            key_info_list[i].event_pressed = FALSE;
            key_print_by_id(key_info_list[i].id);
            if((key_info_list[i].id == KEY_ID_U) || (key_info_list[i].id == KEY_ID_D) 
                || (key_info_list[i].id == KEY_ID_L) || (key_info_list[i].id == KEY_ID_R))
            {
                key_info_list[i].event_unpressed = TRUE;
            }
        }
        else
        {
            if(key_state == KEY_STATE_DOWN)
            {
                key_info_list[i].key_holding_time = GetSysTick_10Ms() - key_info_list[i].key_press_time;
            }
            else
            {
                key_info_list[i].key_press_time = 0;
                key_info_list[i].key_holding_time = 0;
            }
        }

        key_info_list[i].key_state = key_state;
    }
}

void key_print(void)
{
    int i = 0;
    int count = sizeof(key_info_list)/sizeof(KEY_INFO_t);
    for(i = 0; i < count; i++)
    {
        if((key_info_list[i].key_state == KEY_STATE_DOWN) || (key_info_list[i].event_pressed) || (key_info_list[i].event_unpressed))
        {
            printf("\r\n key:%s state:%d pressed:%d unpressed:%d time:%ld %ld", 
                key_info_list[i].name, key_info_list[i].key_state,
                key_info_list[i].event_pressed, key_info_list[i].event_unpressed,
                key_info_list[i].key_press_time, key_info_list[i].key_holding_time);  
        }   
    }
}

void key_print_by_id(u8 key_id)
{
    int i = 0;
    int count = sizeof(key_info_list)/sizeof(KEY_INFO_t);
    for(i = 0; i < count; i++)
    {
        if(key_id == key_info_list[i].id) {
            printf("\r\n key:%s state:%d pressed:%d unpressed:%d time:%ld %ld", 
                key_info_list[i].name, key_info_list[i].key_state,
                key_info_list[i].event_pressed, key_info_list[i].event_unpressed,
                key_info_list[i].key_press_time, key_info_list[i].key_holding_time);
        } 
    }  
}

BOOL key_condition(u8 key_id, unsigned long time)
{
    int i = 0;
    int count = sizeof(key_info_list)/sizeof(KEY_INFO_t);
    for(i = 0; i < count; i++)
    {
        if(key_id == key_info_list[i].id)
        {
            if((key_info_list[i].key_state == KEY_STATE_DOWN) && (key_info_list[i].event_pressed) && (key_info_list[i].key_holding_time > time))
            {
                key_info_list[i].event_pressed = FALSE;
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }

    return FALSE;
}

BOOL key_is_pressed(u8 key_id)
{
    int i = 0;
    int count = sizeof(key_info_list)/sizeof(KEY_INFO_t);
    for(i = 0; i < count; i++)
    {
        if(key_id == key_info_list[i].id)
        {
            if((key_info_list[i].key_state == KEY_STATE_DOWN) && (key_info_list[i].event_pressed == TRUE))
            {
                key_info_list[i].event_pressed = FALSE;
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }

    return FALSE;
}

BOOL key_is_unpressed(u8 key_id)
{
    int i = 0;
    int count = sizeof(key_info_list)/sizeof(KEY_INFO_t);
    for(i = 0; i < count; i++)
    {
        if(key_id == key_info_list[i].id)
        {
            if((key_info_list[i].key_state == KEY_STATE_UP)  && (key_info_list[i].event_unpressed == TRUE))
            {
                key_info_list[i].event_unpressed = FALSE;
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }

    return FALSE;
}

BOOL key_is_long_pressed(u8 key_id, unsigned long* time)
{
    int i = 0;
    int count = sizeof(key_info_list)/sizeof(KEY_INFO_t);
    for(i = 0; i < count; i++)
    {
        if(key_id == key_info_list[i].id)
        {
            if(key_info_list[i].key_state == KEY_STATE_DOWN)
            {
                *time = key_info_list[i].key_holding_time;
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }

    return FALSE;
}


void gennum_3g_enable(BOOL on)
{
    if(on)
    {
        PBout(4) = 1;  // PB4 high level
    }
    else
    {
        PBout(4) = 0;  // PB4 low level
    }
}

#define VIDEO_ENABLE PBout(1)// PE5
void video_enable(BOOL on)
{
    printf("\r\n %s() is called. enable:%d", __FUNCTION__, on);
    if(on)
    {
        VIDEO_ENABLE = 1;  // PB4 high level
    }
    else
    {
        VIDEO_ENABLE = 0;  // PB4 low level
    }
}


#define SYSTEM_RESET PBout(2)// PE5
void system_reset(void)
{
    SYSTEM_RESET = 0;
}

#if 0

void gennum_reset(void)
{
    PBout(2) = 0;
    Wait10Ms(50);
    PBout(2) = 1;
}

void gennum_reset_low(void)
{
    PBout(2) = 0;
}

void gennum_reset_high(void)
{
    PBout(2) = 1;
}
#endif

