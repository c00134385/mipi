/*----------------------------------------------------------------------------*/
/* yeelink.c                                                                  */
/*                                                                            */
/* 项目公用的基础函数集                                                       */
/* 2014/5/30 manwjh 建立                                                      */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/* include */
#include "includes.h"

/* external */
extern RT_POS_TYPE g_NewPosition;   //new GPS坐标
extern SYSTEM_CFG g_SysCfg;

/* public */
SOCKET yeelink_server;

/* private */
UPLOAD_POS_BF iPos;

char yeelink_http_buffer[323];

void gpspos_to_yeelinktype(RT_POS_TYPE *gps, char *pos_str);
unsigned long g_YeelinkNextTime=0;


/* debug */



/**/
/*----------------------------------------------------------------------------*/
/* yeelink_task                                                               */
/*                                                                            */
/*----------------------------------------------------------------------------*/
void yeelink_http_init(void)
{
    yeelink_server.http_bf = yeelink_http_buffer;
    yeelink_server.http_header = yeelink_http_buffer;
    yeelink_server.http_header[0] = 0x0;
	yeelink_server.http_data = yeelink_http_buffer;		//临时性定义
}


void yeelink_taskinit(void)
{
    //
    iPos.wr_id = -1;
    iPos.rd_id = -1;
    iPos.jump_cnt = 0;
    g_YeelinkNextTime = 0;
    iPos_clr();
    //
#if 0
    strcpy(yeelink_server.IPv4_addr,"\"42.96.164.52\"");
    strcpy(yeelink_server.port,"80");
    strcpy(yeelink_server.procotl,"\"TCP\"");
    strcpy(yeelink_server.HostName,"\"api.yeelink.net\"");
    strcpy(yeelink_server.LinkId,"1");
    //
    yeelink_server.http_header = &yeelink_http_buffer[0];
    //
    strcpy(yeelink_server.http_header,"POST /v1.1/device/10635/sensor/17422/datapoints HTTP/1.1\r\n");       	//易工
    //strcpy(yeelink_server.http_header,"POST /v1.1/device/10635/sensor/20369/datapoints HTTP/1.1\r\n");	    //蔡工
    //strcpy(yeelink_server.http_header,"POST /v1.1/device/10635/sensor/20850/datapoints HTTP/1.1\r\n");       //飞鸽roger
    //strcpy(yeelink_server.http_header,"POST /v1.1/device/10635/sensor/21011/datapoints HTTP/1.1\r\n");	    //阿海
    //
    strcat(yeelink_server.http_header,"Host: api.yeelink.net\r\n");
    //strcat(yeelink_server.http_header,"Accept: */*\r\n");
    strcat(yeelink_server.http_header,"Content-Length: 95\r\n");
    strcat(yeelink_server.http_header,"U-ApiKey: 66b5604ec2ab4758ab9af8b97e77a8c2\r\n");
    //strcat(yeelink_server.http_header,"Content-Type: application/x-www-form-urlencoded\r\n");
    //strcat(yeelink_server.http_header,"Connection: close\r\n");
    strcat(yeelink_server.http_header,"\r\n");
    //
    yeelink_server.http_data = &yeelink_http_buffer[strlen(yeelink_server.http_header)];
	//
#endif
#if 0
	WinSocketInit(&yeelink_server);
#endif
}

int iPos_get_num(void)
{
    int i,cnt=0;
    
    for(i=0; i<iPOS_NUM; i++){
        if( iPos.GPS[i].valid ){
            cnt++;
        }
    }
    
    return cnt;
}

void iPos_clr(void)
{
    int i;
    
    for(i=0; i<iPOS_NUM; i++)
        iPos.GPS[i].valid = 0;
}

void yeelink_task(void)
{
    int i,cnt=0;
	char *bf_p;
 
    //注入缓冲
    if( g_NewPosition.valid==true ){
        printf("\r\nYeelink-->缓冲坐标数:%d",iPos.pos_num);
        if( iPos.pos_num<=3 ){
            iPos.jump_step = 0;
        }else if( (iPos.pos_num>3)&&(iPos.pos_num<=6) ){
            iPos.jump_step = 1;
        }else if( (iPos.pos_num>6)&&(iPos.pos_num<=9) ){
            iPos.jump_step = 3;
        }else if( (iPos.pos_num>9)&&(iPos.pos_num<=12) ){
            iPos.jump_step = 6;
        }else if( iPos.pos_num>12 ){
            iPos.jump_step = 10;
        }
        //跳点存储
        if( (iPos.jump_cnt==0)||(iPos.jump_cnt>iPos.jump_step) ){
            iPos.jump_cnt = iPos.jump_step;
            //
            iPos.wr_id++;
            if( iPos.wr_id>=iPOS_NUM )
                iPos.wr_id = 0;
            memcpy(&iPos.GPS[iPos.wr_id],&g_NewPosition,sizeof(RT_POS_TYPE));
        }else{
            iPos.jump_cnt--;
        }
        iPos.pos_num = iPos_get_num();
    }
    
    //检查上次传送是否成功
    #if 0
    if( WinSocketGetState()==SOCKET_SEND_OK ){
        if( (iPos.rd_id>=0)&&(iPos.rd_id<iPOS_NUM) ){
            iPos.GPS[iPos.rd_id].valid = false;
        }
        iPos.pos_num = iPos_get_num();
    }
    if( (WinSocketGetState()==SOCKET_SEND_OK)||(WinSocketGetState()==SOCKET_SEND_FAIL) ){\
        WinSocketClrState(&yeelink_server);
        g_YeelinkNextTime = GetSysTick_Sec()+g_SysCfg.NET_SEND_T_MIN;
    }
    #endif
    //保障与服务器一定的数据上传间隔
    if( GetSysTick_Sec()<g_YeelinkNextTime )
        return ;
    
    //检查SOCKET是否准备就绪
    #if 0
    if( GetModemState(MD_IPNET_OK|MD_IPNET_IDEL)!=(MD_IPNET_OK|MD_IPNET_IDEL) )
        return ;
	#endif
    
#if 0   //批量传送
        sprintf( yeelink_server.http_data,"[" );
        bf_p = yeelink_server.http_data+1;
        for(i=0; i<iPOS_NUM; i++){
            if( iPos.GPS[i].valid ){
//                sprintf( bf_p,"\r\n" );
//                bf_p += 2;
                gpspos_to_yeelinktype(&iPos.GPS[i],bf_p);
                cnt++;
                bf_p += strlen(bf_p);
                sprintf( bf_p,"," );
                bf_p++;
                iPos.GPS[i].valid = 0;
            }
        }
        sprintf( bf_p-1,"]        " );
#else   //单个点传送
        bf_p = yeelink_server.http_data;
        if( (iPos.wr_id<0)||(iPos.wr_id>=iPOS_NUM) )
            return;
        //
        iPos.rd_id = iPos.wr_id+1;
        for(i=0; i<iPOS_NUM; i++){
            if( iPos.rd_id>=iPOS_NUM )
                iPos.rd_id = 0;
            if( iPos.GPS[iPos.rd_id].valid ){
                gpspos_to_yeelinktype(&iPos.GPS[iPos.rd_id],bf_p);
                strcat(bf_p,"\r\n         ");
                cnt++;
                break;
            }
			iPos.rd_id++;
        }
#endif
#if 0
    if( cnt>0 )
    {
        printf("\r\nYeelink-->send pos");
		WinSocketSend(yeelink_http_buffer,strlen(yeelink_http_buffer));
		//printf("%s%s",yeelink_http_buffer,"\r\n");
		cnt = 0;
    }
	#endif
}


void gpspos_to_yeelinktype(RT_POS_TYPE *gps, char *pos_str)
{
    strcpy( pos_str, "{\"timestamp\":\"xxxx-xx-xxTxx:xx:xx\"," );
    
    sprintf( pos_str+14,"%04d%s%02d%s%02d%s%02d%s%02d%s%02d",
        gps->time.year,"-",gps->time.month,"-",gps->time.mday,
        "T",
        gps->time.hour,":",gps->time.min,":",gps->time.sec);
    
    strcpy( pos_str+33,"\"," );

    sprintf(pos_str+35,"%s%f%s%f%s%d%s",
            "\"value\":{\"lat\":",
            gps->pos.lat_correct,
            ",\"lng\":",
            gps->pos.lon_correct,
            ",\"speed\":",
            gps->pos.speed,
//            ",\"offset\":\"yes\"",
            "}}"
            );
}


