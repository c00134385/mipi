/*----------------------------------------------------------------------------*/
/* let_us_go.c                                                                */
/* 描述: 根据GPS,加速度传感器判断运动状态,提供给应用层作为调度的判断          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* 2014/6/7 manwjh 建立 V0.1                                                  */
/*                                                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/* include */
#include "includes.h"

/* external */
extern GPS_INFO_TYPE g_GpsInfo;        //gps 数据信息
extern int gUsbLinked;                  //USB是否连接
extern SYSTEM_CFG g_SysCfg;

/* public */
TRACK_INFO_TYPE iTrack;
RT_POS_TYPE g_NewPosition;
RT_POS_TYPE mark_start_p;

/* private */
unsigned long iTrack_mark_t;

void remember_now_gps_pos( RT_POS_TYPE *mem_p, GPS_INFO_TYPE *in_p );

int motion_analysis_by_gps(void);      //通过gps来判断用户行为
int motion_analysis_by_gyro(void);     //通过陀螺仪来判断用户行为

/* debug */


/*
步行速度: 一般是4~7km/h，走得快的，10km/h，个别疾步如飞者，能达到15km/h。
跑步速度: 
骑行速度: 平地爆发速度50km/h,爬山速度10km/h, 长途 15~20km/h.
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
    //有USB连接时,不进行处理。
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
            //GPS开始有坐标时,立即更新第一个坐标到g_NewPosition缓冲。
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
            //等待5秒后进入空闲状态
            if( GetSysTick_Sec()>iTrack_mark_t+5 ){
                iTrack.state = TRACK_IDEL;
                iTrack_mark_t = GetSysTick_Sec();
            }
            break;
	}
}


/*----------------------------------------------------------------------------*/
/*                                                                            */
/* 描述:通过gps坐标移动速度来决定记录间隔.                                    */
/* 1S移动距离小于10米                                                         */
/*                                                                            */
/*----------------------------------------------------------------------------*/
int motion_analysis_by_gps(void)
{
    RT_POS_TYPE now_p;
    unsigned long now_systick_sec = GetSysTick_Sec();
    float move_distance;   //移动距离
    long time_interval;     //时间间隔(秒)
    int speed_eval;

    g_NewPosition.valid = false;     //分析前，先清零该标志。
    
    //------------------------------------------------------------//
    // 获取新的坐标                                               //
    //------------------------------------------------------------//
    if( g_GpsInfo.pos_update==true )         //有GPS模块是否有新的数据进来
    {
        remember_now_gps_pos(&now_p,&g_GpsInfo);
    }else{
        return 0;
    }
    
    //------------------------------------------------------------//
    // 如果mark时间点没有建立,立即记录mark点                      //
    //------------------------------------------------------------//
    if( mark_start_p.valid !=true ){
        remember_now_gps_pos(&mark_start_p,&g_GpsInfo);
        return 0;
    }

    //------------------------------------------------------------//
    // 根据gSensor判断静止坐标                                    //
    //------------------------------------------------------------//



    //------------------------------------------------------------//
    // mark点与now点的时间间隔超过最大值时,重新标定mark点。       //
    // 出现未来/过去时间,设备长时间没有GPS信号。                  //
    //------------------------------------------------------------//
    time_interval = now_p.time.tick-mark_start_p.time.tick;
    
    //时间出现负数,抛弃掉mark点和now点.(可能是异常,也可能是跨天了)
    //now与mark超过1小时,重新更新mark点
    if( (time_interval<0)
      ||(time_interval>1*60*60) )
    {
        mark_start_p.valid = false;
        return 0;
	}

    //------------------------------------------------------------//
    // mark点与now点的移动速度超出常理,舍弃掉当前点。1000KM/hr    //
    // mark点与now点的距离差距超过一定里程,重新标定mark点。       //
    //------------------------------------------------------------//
    move_distance = DistanceCalcs( &mark_start_p.pos.lat, &mark_start_p.pos.lon, &now_p.pos.lat, &now_p.pos.lon);
    speed_eval = move_distance/time_interval;
    //超过合理速度,抛弃掉mark点和now点.(飞点坐标)
    //mark与now点差距50公里,系统重新开始.(过远距离的mark点和now,不如直接抛弃坐标)
    if( (speed_eval>277)   //277米/秒(1000KM/hr)
      ||(move_distance >50000) ){
        mark_start_p.valid = false;
        return 0;
    }
    
    //------------------------------------------------------------//
    //以下是基于GPS信号正常分析                                   //
    // mark点到now点的速度，移动距离，时间间隔分析。              //
    //------------------------------------------------------------//
    printf("\r\nLet's go-->速度:%3d,距离:%f,时间间隔:%d",now_p.pos.speed,move_distance,time_interval);
    if( (time_interval>g_SysCfg.LET_US_GO_MAX_T)          //最长时间间隔5*60秒
        || ((now_p.pos.speed<3)&&(move_distance>20))                          //徘徊模式
        || ((now_p.pos.speed>=3)&&(now_p.pos.speed<10)&&(move_distance>50))   //步行模式  4Km/hr~10Km/Hr
        || ((now_p.pos.speed>=10)&&(now_p.pos.speed<60)&&(move_distance>80))  //城市道路  10Km/Hr~60Km/Hr
        || ((now_p.pos.speed>=60)&&(now_p.pos.speed<80)&&(move_distance>150)) //快速干道  60Km/Hr~80Km/Hr
        || ((now_p.pos.speed>=80)&&(move_distance>200))
        || (iTrack.force_update == true)                       //高速运动  >80Km/Hr
    )
    {
        remember_now_gps_pos(&g_NewPosition,&g_GpsInfo); //更新新坐标
        remember_now_gps_pos(&mark_start_p,&g_GpsInfo); //更新mark点
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

/* 该函数除了被用于打印,还被用于GPX文件记录,不能改变其格式 */
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

