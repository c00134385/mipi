/*----------------------------------------------------------------------------*/
/* gps_receive_task.c                                                         */
/*                                                                            */
/* ���ڽ���GPSģ����Ϣ����ȷ������״̬                                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* 2014/5/20 manwjh ���� V0.1                                                 */
/*                                                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/* include */
#include "includes.h"
#include "gps_algorithm.h"
#include "nmea_parse.c"

#pragma pack(1)

/* define */
#define GPS_COMM_DEBUG  0

/* external */
extern SYSTEM_CFG g_SysCfg;
#if 0
int gps_comm_write(void *p, int len);
int gps_comm_read(void *p, int len);
int gps_comm_clr(void);
int gps_comm_get_len(void);
#endif
/* public */
uint16_t gGpsState;
int gps_taskstate;
GPS_INFO_TYPE g_GpsInfo;        //gps ������Ϣ

int gps_rec_taskinit(void);
int gps_rec_task(void *p);
int nmea_get_section(char *in, char *out, int seg);

/* private */
#define GPS_NMEA_BF_LEN 500
char gps_nmea_bf[GPS_NMEA_BF_LEN];        //NMEA-0183��֡�����210 byte
static unsigned long task_t_mark;

char HexToChar(unsigned char byTemp);
int nmea_get_frame(char *in, int len, char *out, int *frame_len);
int nmea_get_info(char *in, int len);
void gps_parse_proc(char *bf, int bf_len);

/* debug */
#define SET_GPS_STATE_BITS(a)     gGpsState = (gGpsState|a)
#define CLR_GPS_STATE_BITS(a)     gGpsState = (gGpsState&(~a))

#define MAX_FRAME_BF_LEN 210
char frame_bf[MAX_FRAME_BF_LEN];

//------------------------------------------------------------------//
void gps_info_init(void)
{
    g_GpsInfo.pos_update = false;
    g_NMEAInfo.GGA.update = 0;
    g_NMEAInfo.GLL.update = 0;
    g_NMEAInfo.GSA.update = 0;
    g_NMEAInfo.GSV.update = 0;
    g_NMEAInfo.RMC.update = 0;
    g_NMEAInfo.VTG.update = 0;
    g_NMEAInfo.ZDA.update = 0;
}

int gps_taskinit(void)
{
    //char desay_gps_init[] = "$PMTK314,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";
    
    gps_info_init();
    //
    CLR_GPS_STATE_BITS((uint16_t)0xffff);
    //
    gps_taskctrl(GPS_POWER_OFF);
    //
    return true;
}

int GetGpsState(uint16_t BIT_MASK)
{
    return (gGpsState&BIT_MASK);
}


int gps_taskctrl(int cmd)
{
    if( (cmd==GPS_POWER_ON)||(cmd==GPS_POWER_OFF) ){
        gps_taskstate = cmd;
    }
    
    return gps_taskstate;
}


int gps_task(void *p)
{
    int pkg_size;
	char *str_time;
    
    //gps_rec_task�������״̬���ڶ�ȡGPS������Ϣ
    #if 0
    if( gps_comm_get_len() > 0){
        pkg_size=gps_comm_read( gps_nmea_bf, GPS_NMEA_BF_LEN);
        gps_comm_clr();
    }
    #endif
    
    //
    switch(gps_taskstate){
        case GPS_IDLE:
            CLR_GPS_STATE_BITS((uint16_t)0xffff);
            task_t_mark = GetSysTick_Sec();
            break;
            
        case GPS_POWER_ON:
            printf("\r\nGPS-->������Դ");
			gps_info_init();
            hw_io_ctrl(GPS_PWR,OFF);
            Wait10Ms(10);
            hw_io_ctrl(GPS_PWR,ON);
            SET_GPS_STATE_BITS(GPS_POWRERON);
            task_t_mark = GetSysTick_Sec();
            gps_taskstate = GPS_SEARCHING;
            break;
            
        case GPS_POWER_OFF:
            printf("\r\nGPS-->�رյ�Դ");
			gps_info_init();
            hw_io_ctrl(GPS_PWR,OFF);
            CLR_GPS_STATE_BITS((uint16_t)0xffff);
            gps_taskstate = GPS_IDLE;
            break;
            
        case GPS_SEARCHING:
            gps_info_init();
            //���5�������Ҳ�������,������������ڡ�
            if( GetSysTick_Sec()>task_t_mark+g_SysCfg.GPS_SEARCH_T ){
                printf("\r\nGPS-->������������\r\n");
                SET_GPS_STATE_BITS(GPS_ERR_LOCATE_FAIL);
                gps_taskstate = GPS_SEARCHING;
                task_t_mark = GetSysTick_Sec();
            }
            gps_parse_proc(gps_nmea_bf,pkg_size);
            //��ȡ���Ƕ�λģʽ
            if( g_NMEAInfo.GGA.update ){
                switch(g_NMEAInfo.GGA.PositionFix){
                    case '1':
                        printf("\r\nGPS-->GGA:GPS SPSģʽ");
                        break;
                    case '2':
                        printf("\r\nGPS-->GGA:���GPS/SPSģʽ");
                        break;
                    case '6':
                        printf("\r\nGPS-->GGA:��λ����ģʽ");
                }
                printf("\r\nGPS-->ʹ��������:%d,����%f",g_NMEAInfo.GGA.SatellitesUsed,g_GpsInfo.gps.altitude);
            }
            if( g_NMEAInfo.GSA.update ){
                if( (g_GpsInfo.gsa_locate_mode=='3')||(g_GpsInfo.gsa_locate_mode=='2') ){
                    task_t_mark = GetSysTick_Sec();
                    gps_taskstate = GPS_RUNNING;
                    CLR_GPS_STATE_BITS(GPS_ERR_LOCATE_FAIL);
                    SET_GPS_STATE_BITS(GPS_LOCATED);
                }
            }
            break;
            
        case GPS_RUNNING:
            gps_parse_proc(gps_nmea_bf,pkg_size);
            //����GPS��λ״̬
            if( g_NMEAInfo.GSA.update ){
                if( g_GpsInfo.gsa_locate_mode=='1' ){
                    //��ȡ���Ƕ�λģʽ
                    if( g_NMEAInfo.GGA.update ){
                        switch(g_NMEAInfo.GGA.PositionFix){
                            case '1':
                                printf("\r\nGPS-->GGA:GPS SPSģʽ");
                                break;
                            case '2':
                                printf("\r\nGPS-->GGA:���GPS/SPSģʽ");
                                break;
                            case '6':
                                printf("\r\nGPS-->GGA:��λ����ģʽ");
                        }
                        printf("\r\nGPS-->ʹ��������:%d,����%f",g_NMEAInfo.GGA.SatellitesUsed,g_GpsInfo.gps.altitude);
                    }
                    //���5������û�ж�λ��Ϣ,����ת��GPS_SEARCHING
                    if( GetSysTick_Sec()>task_t_mark+g_SysCfg.GPS_SEARCH_T ){
                        CLR_GPS_STATE_BITS(GPS_LOCATED);
                        gps_taskstate = GPS_SEARCHING;
                        task_t_mark = GetSysTick_Sec();
                    }
                }else if( (g_GpsInfo.gsa_locate_mode=='2')||(g_GpsInfo.gsa_locate_mode=='3') ){
                    SET_GPS_STATE_BITS(GPS_LOCATED);
                    task_t_mark = GetSysTick_Sec();
                }
            }
            //
			str_time = tm2str(&g_GpsInfo.time);
            if( g_NMEAInfo.RMC.update ){    //��RMC��ȡUTCʱ�������
				printf("\r\nGPS-->%s,lat:%f,lon:%f,lat_c:%f,lon_c:%4.1f",
				str_time, g_GpsInfo.gps.lat, g_GpsInfo.gps.lon, g_GpsInfo.gps.lon_correct, g_GpsInfo.gps.lat_correct );
            }
            break;
            
        case GPS_HW_ERROR:
			gps_info_init();
            break;
        
    }

	return 0;
}


//����GPS���ڽ��ջ���
void gps_parse_proc(char *bf, int bf_len)
{    
    int i;
	int offset,frame_len;
    
    //�ڳ��Զ�ȡGPS������ʱ,�����ȷϵ����new�ı�־
    g_GpsInfo.pos_update = false;
    g_NMEAInfo.GGA.update = 0;
    g_NMEAInfo.GLL.update = 0;
    g_NMEAInfo.GSA.update = 0;
    g_NMEAInfo.GSV.update = 0;
    g_NMEAInfo.RMC.update = 0;
    g_NMEAInfo.VTG.update = 0;
    g_NMEAInfo.ZDA.update = 0;
    //
    if( bf_len==0 )
        return;
    
    //��������
    offset = 0;
    do{
        i = nmea_get_frame(bf+offset, bf_len, frame_bf, &frame_len);
        if( i==0 )  //����û��������
            break;
        if( frame_len != 0){
            nmea_get_info(frame_bf, frame_len);
        }
        offset += i;
        bf_len -= i;
    }while( bf_len>0 );

    //����ģʽ,��ʾGPS���ܺͽ���״��
#if GPS_COMM_DEBUG
    /* GGA */
    if( g_NMEAInfo.GGA.update ){     //��GGA��ȡ��������
        switch(g_NMEAInfo.GGA.PositionFix){
            case '1':
                printf("GGA:GPS SPSģʽ, ");
                break;
            case '2':
                printf("GGA:���GPS/SPSģʽ, ");
                break;
            case '6':
                printf("GGA:��λ����ģʽ, ");
        }
        printf("ʹ��������:"); printf(Int2DecStr(g_NMEAInfo.GGA.SatellitesUsed));
        printf(",����:");    printf(float2str(g_GpsInfo.gps.altitude));   printf("\r\n");
    }
    /* GLL */
    if( g_NMEAInfo.GLL.update ){    //
        if( g_NMEAInfo.GLL.PositionFix=='A' ){
            printf("GLL:��Ч��λ");
        }else if( g_NMEAInfo.GLL.PositionFix=='V' ){
            printf("GLL:��Ч��λ");
        }
    }
    /* GSA */
    if( g_NMEAInfo.GSA.update ){    //��GSA��ȡ��λģʽ
        switch(g_GpsInfo.gsa_locate_mode){
            case '1':
            printf("GSA:δ��λ\r\n");
            break;
            
            case '2':
            printf("GSA:��ά��λ\r\n");
            break;
            
            case '3':
            printf("GSA:��ά��λ\r\n");
            break;
        }
    }
    /* GSV */
    if( g_NMEAInfo.GSV.update ){    //��GSV��ȡ����Ƶ��������
        printf("GSV.������������:"); printf(Int2DecStr(g_GpsInfo.stlt_in_view)); printf("\r\n");
    }
    /* RMC */
    if( g_NMEAInfo.RMC.update ){    //��RMC��ȡUTCʱ�������
        printf("RMC:%s",printf( tm2str(&g_GpsInfo.time);
        printf(" lat,%f",g_GpsInfo.gps.lat) );
        printf(" lon,%f",g_GpsInfo.gps.lon) );
        printf(" lat_c,%f",g_GpsInfo.gps.lat) );
        printf(" lon_c,%f",g_GpsInfo.gps.lon) );
        printf("\r\n");
    }
    /* VTG */
    if( g_NMEAInfo.VTG.update ){    //��VTG��ȡ������ٶ�
        
    }
#endif

}



//------------------------------------------------------------------//
/*
    int nmea_get_frame(char *in, int in_size, char *out, int *frame_len)
    �ӻ��������ȡ��У��һ��������֡����,���ط������ݵĳ���(����֡��).
    *in ���뻺��ָ��
    in_size 
    *out �������ָ��
    frame_len   ��ȡ��֡����(����'$'��ʼ�ַ�����������*У���֡�
    return: 0 end, ���򷵻ط������ȣ�֡�Ƿ���ȷ��Ҫ��frame_len�Ƿ���Ч��
    
    
    int nmea_get_info(char *in, int len)
    ����һ֡����������,���洢��ȫ�ֱ�����.
*/
//------------------------------------------------------------------//
int nmea_get_frame(char *in, int bf_len, char *out, int *frame_len)
{
    int i=0,len=0;
    unsigned char checksum=0;
    char ck_h,ck_l;
    
    if( (bf_len<10)||(bf_len>GPS_NMEA_BF_LEN) )		//MAX_FRAME_BF_LEN) )
        return 0;
    //Ѱ��'$'ͷ��ʾ
    while( i<bf_len ){
        i++;
        if( *in == '$')     //search frame header '*'
            break;
        in++;
    }
    if( i==bf_len ){     //�޷�ʶ������ݰ�
        *frame_len = 0;
        return 0;
    }
    *out++ = *in++;     //д'$'ͷ
    //��ȡ֡��Ϣ
    while(i<bf_len){
        i++;
        len ++;
        //check�Ƿ�У������
        if( *in == '*' )
            break;
        //
        checksum = checksum^ *in;
		if( len< MAX_FRAME_BF_LEN )
        	*out++ = *in++;
		else
			return 0;
    }
    *out = '\0';
    //У��֡
    if(i<bf_len+2){
        ck_h = HexToChar( (checksum&0xf0)>>4 );
        ck_l = HexToChar( checksum&0x0f );
        if( (ck_h == *(in+1))&&(ck_l==*(in+2)) ){
            *frame_len = len+1; //����0x0������
        }else
            *frame_len = 0;     //��Ч֡
        
        return i+2;
    }
    
    return 0;
}

int nmea_get_info(char *in, int len)
{
    char header[6+1];
    int rslt = false;
    
    memcpy(header, in, 6);
    header[6] = '\0';
    //
    if( strcmp(header, "$GPGGA")==0 ){
        if(len<=74+4){
            g_NMEAInfo.GGA.update = GGA_Parse(&g_GpsInfo,in);       /*��������,��������g_GpsInfo*/
            rslt = true;
        }
    }else if( strcmp(header, "$GPGLL")==0 ){
        if(len<=65+4){
            g_NMEAInfo.GLL.update = GLL_Parse(&g_GpsInfo,in);
            rslt = true;
        }
    }else if( strcmp(header, "$GPGSA")==0 ){
        if(len<=65+4){
            g_NMEAInfo.GSA.update = GSA_Parse(&g_GpsInfo,in);
            rslt = true;
        }
    }else if( strcmp(header, "$GPGSV")==0 ){                        /*������Ϣ,��������g_NMEAInfo.GSV*/
            g_NMEAInfo.GSV.update = GSV_Parse(&g_GpsInfo,in);
            rslt = true;
    }else if( strcmp(header, "$GPRMC")==0 ){
        if(len<=70+4){
            g_NMEAInfo.RMC.update = RMC_Parse(&g_GpsInfo,in);       /*��������,��������g_GpsInfo*/
            rslt = true;
        }
    }else if( strcmp(header, "$GPVTG")==0 ){
        if(len<=34+4){
            g_NMEAInfo.VTG.update = VTG_Parse(&g_GpsInfo,in);
            rslt = true;
        }
    }else if( strcmp(header, "$GPZDA")==0 ){
        if(len<=34+4){
            g_NMEAInfo.ZDA.update = ZDA_Parse(&g_GpsInfo,in);
            rslt = true;
        }
    }

    return rslt;
}

char HexToChar(unsigned char byTemp)  
{  
    byTemp &= 0x0f;  
    if(byTemp >= 10)     // show 'A' - 'F'  
    {  
        byTemp = byTemp - 0xa + 0x41;  
    }  
    else        // show '0' - '9'  
    {  
        byTemp = byTemp + 0x30;  
    }  
    return(byTemp);  
}  


