
#ifndef		_DCS_H
#define		_DCS_H

#include "stm32f10x.h"

extern u8 len;
extern u8 Buf[];

extern u8 ShortPacketECC(void);
extern u8 LongPacketECC(void);
extern void test_CRC(void);
extern void DcsSendEn(void);
extern void LT1603Initial(void);
extern void DcsShortPktRead(u8 DataID,u8 Data0,u8 Data1);
//extern void DcsShortPktWrite(u8 DataID,u8 Data0,u8 Data1);
extern void DcsLongPktWrite(u8 DataID,u8 Len);
extern void ScreenInitial(void);
extern void ScreenInitial_400x1280(void);
extern void DcsShortPktWrite(u8 DataID,u8 Data0,u8 Data1);
extern void InitialCode(void);

#endif