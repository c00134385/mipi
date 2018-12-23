/*****************************************************************************
 *   type.h:  Type definition Header file for Philips LPC214x Family 
 *   Microprocessors
 *
 *   Copyright(C) 2006, Philips Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2005.10.01  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __TYPE_H__
#define __TYPE_H__

#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef NON_NULL
#define NON_NULL ((void *)1)
#endif

#ifndef OK
#define OK      (0)
#endif

#ifndef ERROR
#define ERROR   (1)
#endif

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif

#ifndef true
#define true    (1)
#endif

#ifndef false
#define false   (0)
#endif

#ifndef EJ_ERR_UNKNOWN
#define EJ_ERR_UNKNOWN  0x01
#endif

#ifndef EJ_ERR_NONE
#define EJ_ERR_NONE     0x0
#endif


typedef unsigned long RESULT;

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;  
typedef unsigned int   BOOL;
typedef unsigned int   bool;

typedef unsigned int UINT;
typedef unsigned char UINT8;
typedef unsigned char u8;
typedef unsigned short UINT16;
typedef unsigned int u16;
typedef unsigned long UINT32;
typedef unsigned long u32;

typedef int INT16;
typedef int INT32;

#define INT8U	UINT8
#define INT16U	UINT16	
#define INT32U	UINT32


typedef struct{
    UINT8 *Ptr;
    UINT16 Size;
    UINT16 Count;
}BUFFER_TYPE;


typedef struct{
    UINT8 Flag;
    UINT8 Msg;
}MSG_BOX;

// my struct define
typedef struct{
    int   ChgFlag;
    int   NowState;
    int   NewState;
    int   Error;
}StateMechine;

typedef struct{
    UINT8 update;
    UINT8 tm_sec;       /* seconds */
    UINT8 tm_min;       /* minutes */
    UINT8 tm_hour;      /* hours */
    UINT8 tm_mday;      /* day of month */
    UINT8 tm_mon;       /* month(0~30) */
    UINT8 tm_year;
    UINT8 tm_wday;      /* Weekday 0~6; sunday = 0; */
}TIME_TYPE;

typedef struct SD_STRUCT 
{	 
	INT16U RCA;						/* 卡逻辑地址 */ 
 
	INT8U card_type;				/* 卡类型 */ 
	INT32U sector_count;    		/* SD卡中sector个数，即容量 */ 
	INT32U block_size;
	INT16U sector_size;				/* sector中byte个数 */ 
	 
	INT32U timeout_read;			/* 读块超时时间(单位: 8 SPI clock) */ 
	INT32U timeout_write;			/* 写块超时时间(单位: 8 SPI clock) */ 
	INT32U timeout_erase;			/* 擦块超时时间(单位: 8 SPI clock) */ 
}sd_struct;

typedef struct{
    bool flag;      //0:NULL 1:PRESS 2:HOLD 3:RELEASE
    u16 value;
    u16 hold_key;
    u16 hold_count;
}KEY_TYPE;


//
#define HIGH                1
#define LOW                 0
#define NEGATIVE_EDGE       0x01    //下降沿
#define RISING_EDGE         0x02    //上升沿
#define BOTH_EDGE           0x03    //

typedef struct{
    INT8U rising_edge;
    INT8U negative_edge;
    INT8U level;
}SIGNAL_TYPE;

#endif  /* __TYPE_H__ */
