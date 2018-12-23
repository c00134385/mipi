#ifndef		_LT8918_H
#define		_LT8918_H

#include "stm32f10x.h"
#include "sys.h"
#include "integer.h"

#define LT8918L_RST PAout(7)// PA7


typedef  enum
{
	LOW    = 0,
	HIGH   = !LOW
}
Pin_Status;



#define 	CLK_CHANGE_MARGIN   		(3*1024)  //CLK_CHANGE_MARGIN*1KHz
#define 	CLK_ACTIVE_MIN_LIMIT   	(16*1024)  //CLK_ACTIVE_MIN_LIMIT should be lower than 25M(480P).

#define 	HBLANK_MIN_LIMIT		10
#define 	VBLANK_MIN_LIMIT	 	5

#define		HACTIVE_MIN_LIMIT	400
#define		VACTIVE_MIN_LIMIT	600
#define		FAULT_TOLERANT	10

#define Lt8918LAdr 0x40

typedef enum
{			
	VESA_1920x1080_60,
	VESA_1920x1080_50,
	VESA_1920x1080_30,
	VESA_1920x1080_25,
	VESA_1920x1080I_60,
	VESA_1920x1080I_50,
	VESA_1280x720_60,
	VESA_1280x720_50,
	VESA_1280x720_30,
	VESA_1280x720_25,
	SignnalOff
}
Resolution_Type;

typedef enum
{			
	Htotal,
	Vtotal,
	Hactive,
	Vactive
}
Parameter_Type;

typedef struct {
    u16 hs;
    u16 hbp;
    u16 hactive;
    u16 hfp;
    
    u16 vs;
    u16 vbp;
    u16 vactive;
    u16 vfp;
} Timing_Param;

extern const u8 LVDSCK_DLY[7];

void SetRegisterBank(u8 adr);
void InitLt8918State(void);
u32 ClkFreqIndicate(void);
bool Lt8918_ClkFreqCheck(void);
u16 VideoParamIndicate(Parameter_Type param);
bool Lt8918_VideoJudge(void);
void Lt8918_VideoCheck(void);
void Lt8918_TxPllConfig(void);
void Lt8918_DesscFreqSet(void);
void Lt8918_TxDPhyConfig(void);
void Lt8918_TxPrtclConfig(void);
void Lt8918_MLRXInit(void);
void Lt8918_LvdsClkPhaseAdjust(u8 value);
void lt8918l_Process(void);
void Lt8918_PtnDataConfig_Pattern( void );
void Lt8918_TxPrtclConfig_pattern( void );

void Lt8918L_init(void);
void Lt8918L_reset(void);
u32 Lt8918L_deviceId(void);
void Lt8918_VideoCheck_lvds( void );
void Lt8918_checkPixClk(void);
void Lt8918_checkByteClk(void);
void Lt8918_setVideoResolution(uint8 format);


#endif
