/*----------------------------------------------------------------------------*/
/* let_us_go.c                                                                */
/* ����: ����GPS,���ٶȴ������ж��˶�״̬,�ṩ��Ӧ�ò���Ϊ���ȵ��ж�          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* 2014/6/7 manwjh ���� V0.1                                                  */
/*                                                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/* include */
#include "includes.h"

/* external */
extern GPS_INFO_TYPE g_GpsInfo;        //gps ������Ϣ
extern int gUsbLinked;                  //USB�Ƿ�����
extern SYSTEM_CFG g_SysCfg;

/* public */
TRACK_INFO_TYPE iTrack;
RT_POS_TYPE g_NewPosition;
RT_POS_TYPE mark_start_p;

/* private */
unsigned long iTrack_mark_t;

void remember_now_gps_pos( RT_POS_TYPE *mem_p, GPS_INFO_TYPE *in_p );

int motion_analysis_by_gps(void);      //ͨ��gps���ж��û���Ϊ
int motion_analysis_by_gyro(void);     //ͨ�����������ж��û���Ϊ

/* debug */


/*
�����ٶ�: һ����4~7km/h���ߵÿ�ģ�10km/h�����𼲲�����ߣ��ܴﵽ15km/h��
�ܲ��ٶ�: 
�����ٶ�: ƽ�ر����ٶ�50km/h,��ɽ�ٶ�10km/h, ��; 15~20km/h.
*/

void iTrack_taskinit(void)
{
    iTrack.state = TRACK_IDEL;
}

int iTrack_taskctrl(TRACK_STATE cmd)
{
    switch(cmd)
    {
        case TRACK_START:
        case TRACK_END:
            iTrack_mark_t = GetSysTick_Sec();
            iTrack.state = cmd;
            break;
    }

	return iTrack.state;
}


void iTrack_task(void)
{
    //��USB����ʱ,�����д���
    if( gUsbLinked==1 )
        return;

    //
    switch(iTrack.state){
        case TRACK_IDEL:
            if( GetGpsState(GPS_LOCATED)!=GPS_LOCATED ){
                iTrack.state = TRACK_START;
            }
            break;
            
        case TRACK_START:
            //GPS��ʼ������ʱ,�������µ�һ�����굽g_NewPosition���塣
            if( g_GpsInfo.pos_update==true ){
                iTrack.force_update = true;
                iTrack.state = TRACK_GOING;
            }
            break;
            
        case TRACK_GOING:
            motion_analysis_by_gps();
            break;
            
        case TRACK_PAUSE:
            if( g_GpsInfo.pos_update==true ){
                iTrack.force_update = true;
                iTrack.state = TRACK_GOING;
            }
            break;
            
        case TRACK_END:
            //�ȴ�5���������״̬
            if( GetSysTick_Sec()>iTrack_mark_t+5 ){
                iTrack.state = TRACK_IDEL;
                iTrack_mark_t = GetSysTick_Sec();
            }
            break;
	}
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/* ����:ͨ��gps�����ƶ��ٶ���������¼���.                                    */
/* 1S�ƶ�����С��10��                                                         */
/*                                                                            */
/*----------------------------------------------------------------------------*/
int motion_analysis_by_gps(void)
{
    RT_POS_TYPE now_p;
    unsigned long now_systick_sec = GetSysTick_Sec();
    float move_distance;   //�ƶ�����
    long time_interval;     //ʱ����(��)
    int speed_eval;

    g_NewPosition.valid = false;     //����ǰ��������ñ�־��
    
    //------------------------------------------------------------//
    // ��ȡ�µ�����                                               //
    //------------------------------------------------------------//
    if( g_GpsInfo.pos_update==true )         //��GPSģ���Ƿ����µ����ݽ���
    {
        remember_now_gps_pos(&now_p,&g_GpsInfo);
    }else{
        return 0;
    }
    
    //------------------------------------------------------------//
    // ���markʱ���û�н���,������¼mark��                      //
    //------------------------------------------------------------//
    if( mark_start_p.valid !=true ){
        remember_now_gps_pos(&mark_start_p,&g_GpsInfo);
        return 0;
    }

    //------------------------------------------------------------//
    // ����gSensor�жϾ�ֹ����                                    //
    //------------------------------------------------------------//



    //------------------------------------------------------------//
    // mark����now���ʱ�����������ֵʱ,���±궨mark�㡣       //
    // ����δ��/��ȥʱ��,�豸��ʱ��û��GPS�źš�                  //
    //------------------------------------------------------------//
    time_interval = now_p.time.tick-mark_start_p.time.tick;
    
    //ʱ����ָ���,������mark���now��.(�������쳣,Ҳ�����ǿ�����)
    //now��mark����1Сʱ,���¸���mark��
    if( (time_interval<0)
      ||(time_interval>1*60*60) )
    {
        mark_start_p.valid = false;
        return 0;
	}

    //------------------------------------------------------------//
    // mark����now����ƶ��ٶȳ�������,��������ǰ�㡣1000KM/hr    //
    // mark����now��ľ����೬��һ�����,���±궨mark�㡣       //
    //------------------------------------------------------------//
    move_distance = DistanceCalcs( &mark_start_p.pos.lat, &mark_start_p.pos.lon, &now_p.pos.lat, &now_p.pos.lon);
    speed_eval = move_distance/time_interval;
    //���������ٶ�,������mark���now��.(�ɵ�����)
    //mark��now����50����,ϵͳ���¿�ʼ.(��Զ�����mark���now,����ֱ����������)
    if( (speed_eval>277)   //277��/��(1000KM/hr)
      ||(move_distance >50000) ){
        mark_start_p.valid = false;
        return 0;
    }
    
    //------------------------------------------------------------//
    //�����ǻ���GPS�ź���������                                   //
    // mark�㵽now����ٶȣ��ƶ����룬ʱ����������              //
    //------------------------------------------------------------//
    printf("\r\nLet's go-->�ٶ�:%3d,����:%f,ʱ����:%d",now_p.pos.speed,move_distance,time_interval);
    if( (time_interval>g_SysCfg.LET_US_GO_MAX_T)          //�ʱ����5*60��
        || ((now_p.pos.speed<3)&&(move_distance>20))                          //�ǻ�ģʽ
        || ((now_p.pos.speed>=3)&&(now_p.pos.speed<10)&&(move_distance>50))   //����ģʽ  4Km/hr~10Km/Hr
        || ((now_p.pos.speed>=10)&&(now_p.pos.speed<60)&&(move_distance>80))  //���е�·  10Km/Hr~60Km/Hr
        || ((now_p.pos.speed>=60)&&(now_p.pos.speed<80)&&(move_distance>150)) //���ٸɵ�  60Km/Hr~80Km/Hr
        || ((now_p.pos.speed>=80)&&(move_distance>200))
        || (iTrack.force_update == true)                       //�����˶�  >80Km/Hr
    )
    {
        remember_now_gps_pos(&g_NewPosition,&g_GpsInfo); //����������
        remember_now_gps_pos(&mark_start_p,&g_GpsInfo); //����mark��
        iTrack.force_update = false;
    }

    return 1;
}


void remember_now_gps_pos( RT_POS_TYPE *mem_p, GPS_INFO_TYPE *in_p )
{
    memcpy( &mem_p->time,&in_p->time,sizeof(tm));
    memcpy( &mem_p->pos,&in_p->gps,sizeof(GPS_POS_TYPE) );
    mem_p->valid = true;
}

/* �ú������˱����ڴ�ӡ,��������GPX�ļ���¼,���ܸı����ʽ */
char* now_position_to_xml(void)
{
    static char pos_msg[100]="";
    
    sprintf(pos_msg,"%s%f%s%f%s",
            "<trkpt lat=\"",
            g_NewPosition.pos.lat,
            "\" lon=\"",
            g_NewPosition.pos.lon,
            "\">\r\n"
            );
    sprintf(&pos_msg[strlen(pos_msg)],"%s%.1f%s",
            " <ele>",g_NewPosition.pos.altitude,"</ele>\r\n"
            );
    sprintf(&pos_msg[strlen(pos_msg)],"%s%04d%s%02d%s%02d%s%02d%s%02d%s%02d%s",
            " <time>",
            g_NewPosition.time.year,"-",g_NewPosition.time.month,"-",g_NewPosition.time.mday,
            "T",
            g_NewPosition.time.hour,":",g_NewPosition.time.min,":",g_NewPosition.time.sec,
            "Z</time>\r\n"
            );
    return pos_msg;
}            

