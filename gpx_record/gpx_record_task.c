/*----------------------------------------------------------------------------*/
/* nmea_parse.c                                                               */
/*                                                                            */
/* ����NMEAЭ��,�����ݴ�ASCIIת�ɿ�����ı���                                 */
/*----------------------------------------------------------------------------*/
/* 2014/5/20 manwjh ���� V0.1                                                 */
/*                                                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/* include */
#include "stdio.h"
#include "datetime.h"
#include "ff.h"
#include "nmea_parse.h"
#include "let_us_go.h"

#include "gpx_record_task.h"

/* define */
#ifndef NULL
#define	NULL	0
#endif

//external
extern TRACK_INFO_TYPE iTrack;
extern RT_POS_TYPE g_NewPosition;
extern int gUsbLinked;                  //USB�Ƿ�����
extern FATFS *fat_fs;       /*Work area (file system object) for logical drive*/

//public

//private
FIL gpx_fp;


FRESULT gpx_creat_fl(FIL *fp);
FRESULT gpx_wr_fl_trkpt(FIL *fp);
FRESULT gpx_wr_fl_header(FIL *fp);
FRESULT gpx_wr_fl_end(FIL *fp);



/*----------------------------------------------------------------------------*/
/*                                  Functions                                 */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* ��������: int gpx_record_taskinit(void)                                    */
/* ��������: ��ʼ��gpx_record_task()����                                      */
/*                                                                            */
/* ��������: int gpx_record_task(void *p)                                     */
/* ��������: gpx_record_task����,��ȡ���꣬���ܵ�����¼�ܶȣ�д��gpx�ļ�      */
/*----------------------------------------------------------------------------*/
int gpx_record_state;

int gpx_record_taskinit(void)
{
	gpx_fp.fs = NULL;
	gpx_record_state = GPX_REC_IDEL;
    //�ļ�ϵͳ
    return 0;
}

void gpx_record_taskctrl(int cmd)
{
    if( (cmd==GPX_REC_END)&&(gpx_record_state==GPX_RECORDING) ){
        if( gpx_fp.fs!=NULL )
            gpx_wr_fl_end(&gpx_fp);
        gpx_record_state = GPX_REC_IDEL;
    }
}

int gpx_record_task(void *p)
{
    unsigned long disk_free_size;
    
    //�ļ�����1M����
    if(gpx_fp.fs!=NULL){
        if( gpx_fp.fsize>1*1024*1024 ){
            gpx_wr_fl_end(&gpx_fp);
            f_close(&gpx_fp);
        }
    }
    
    //
    f_getfree(0,&disk_free_size,&fat_fs);
    
    //������˶�״̬����GPXû�п�ʼ¼��,��ʼ�켣��¼
    if( iTrack.state==TRACK_GOING ){
        if( (gpx_record_state!=GPX_RECORDING)&&(gUsbLinked==0)&&(gpx_record_state!=GPX_REC_ERROR)&&(disk_free_size>2) ){
            gpx_record_state = GPX_REC_START;
        }
    }
    //����˶����,���Ҳ��ڶ�Ӧ״̬,�������GPX�켣��¼
    if( (iTrack.state==TRACK_END)||(iTrack.state==TRACK_IDEL)||(gUsbLinked==1)||(disk_free_size<=2) ){
        if( (gpx_record_state!=GPX_REC_END)&&(gpx_record_state!=GPX_REC_IDEL)&&(gpx_record_state!=GPX_REC_ERROR) ){
            gpx_record_state = GPX_REC_END;
        }
    }
    //
    switch(gpx_record_state)
    {
        case GPX_REC_IDEL:
            break;
            
        case GPX_REC_START:
            if( disk_free_size<=2 ){
                gpx_record_state = GPX_REC_IDEL;
                break;
            }
            //�������г�
            gpx_creat_fl(&gpx_fp);
            //
            if(gpx_fp.fs!=NULL){
                gpx_wr_fl_header(&gpx_fp);
                gpx_record_state = GPX_RECORDING;
            }
            break;
        
        case GPX_RECORDING:
            if( g_NewPosition.valid==true ){
                if(gpx_fp.fs!=NULL){
                    printf("\r\nGPX_REC-->*mark position!");
				    gpx_wr_fl_trkpt(&gpx_fp);
				}
            }
            break;
        
	    case GPX_REC_END:
	        if( gpx_fp.fs!=NULL )
			    gpx_wr_fl_end(&gpx_fp);
            gpx_record_state = GPX_REC_IDEL;
            break;
           
        case GPX_REC_ERROR:
            break;
            
        default:
            
            break;
    }
    
    return true;
}

/*----------------------------------------------------------------------------*/
/* ��������: char gpx_wr_trkpt(FILE *fp)                                      */
/* ��������: ��g_GpsInfo���ݽṹ�ж�ȡgps������Ϣ                             */
/*           ת����gpx�ļ���Ϣ,��д�ļ�ϵͳ��                                 */
/*                                                                            */
/* ��������: char gpx_wr_fl_header(FILE *fp)                                  */
/* ��������: дgpx�ļ��Ĺ̶�ͷ����                                            */
/*                                                                            */
/* ��������: char gpx_wr_fl_end(FILE *fp)                                     */
/* ��������: дgpx�ļ��Ĺ̶�β����,���ر��ļ�                                 */
/*----------------------------------------------------------------------------*/
/*
sample:
<trkpt lat="53.831915000" lon="-1.780495000">
  <ele>103.600000</ele>
<time>2008-08-14T15:26:32Z</time>
  <course>82.750000</course>
  <speed>0.000000</speed>
  <fix>3d</fix>
  <sat>9</sat>
  <hdop>0.900000</hdop>
</trkpt>
*/
FRESULT gpx_wr_fl_trkpt(FIL *fp)
{
    if( f_printf(fp,"%s",now_position_to_xml())!=EOF )
        if( f_printf(fp,"%s","</trkpt>\r\n")!=EOF )
            return FR_OK; 
    
    return FR_DISK_ERR;
}

FRESULT gpx_creat_fl(FIL *fp)
{
    char gpx_file_name[15];
    FRESULT rslt;
	tm *t=sys_time(NULL);
    
    sprintf(gpx_file_name,"%02d%02d%02d%02d%s",t->month,t->mday,t->hour,t->min,".gpx");
	rslt = f_open(fp,gpx_file_name,FA_CREATE_ALWAYS|FA_WRITE);
    if( rslt==FR_OK ){
        printf("\r\nGPX_REC-->%s - create ok!",gpx_file_name);
    }else{
        printf("\r\nGPX_REC-->%s - create fail!",gpx_file_name);
        gpx_file_name[0] = 0x0;
    }
    
    return rslt;
}


#if 1
const char gpx_file_header[]=
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n\
<gpx version=\"1.0\" creator=\"CarBetter\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.topografix.com/GPX/1/0\" xsi:schemaLocation=\"http://www.topografix.com/GPX/1/0 http://www.topografix.com/GPX/1/0/gpx.xsd\">\r\n\
<bounds minlat=\"39.941354000\" minlon=\"115.012329000\" maxlat=\"40.022721000\" maxlon=\"115.075414000\"/>\r\n\
<trk>\r\n<trkseg>\r\n    ";
#endif

FRESULT gpx_wr_fl_header(FIL *fp)
{
    //fwriteд����������ǻ��дһ��0,��������header�������ж������һ�������ַ�
//	rslt = f_write(fp,gpx_file_header,sizeof(gpx_file_header),&ByteWritten);
    if( f_printf(fp,"%s",gpx_file_header)!= EOF )
		return FR_OK;

    return FR_INT_ERR;
}


const char gpx_file_end[]=
"</trkseg>\r\n\
</trk>\r\n\
</gpx>\r\n";

FRESULT gpx_wr_fl_end(FIL *fp)
{
	int rslt;
    
    rslt = f_printf(fp,"%s",gpx_file_end);
    
    if( rslt==EOF ){
        printf("\r\nGPX_REC--> error end record");
    }else{
        printf("\r\nGPX_REC--> record end!");
    }
    f_close(fp);

    return (rslt == EOF) ? FR_INT_ERR : FR_OK;
}

void gpx_file_save(void)
{
    FIL *fp = &gpx_fp;
    
    if( gpx_fp.fs!=NULL )
        f_sync(fp);
}

void gpx_file_saveexit(void)
{
    if( gpx_fp.fs!=NULL )
        gpx_wr_fl_end(&gpx_fp);
}




