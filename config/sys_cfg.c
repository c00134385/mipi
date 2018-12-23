/*----------------------------------------------------------------------------*/
/* sys_cfg.c                                                                  */
/* ����:У��/����/��ȡϵͳ����                                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* 2014/7/28 manwjh ���� V0.1                                                 */
/* STM32��������MCU��1K/ҳ                                                    */
/* itrack.cfg, ��󲻳���2K�ֽ�,��ռ2��ҳ��                                   */
/*             126ҳ(0x0801F800~0x08001FBFF)                                  */
/*             127ҳ(0x0801FC00~0x08001FFFF)                                  */
/*  flash�洢�ļ��ṹ(16���ֽڵ��ļ�����+����)��                              */
/*  DWORD fchecksum      �ļ�����У���                                       */
/*  DWORD fsize;         File size                                            */
/*  DWORD fdatetime;     Last modified date&time                              */
/*  WORD ����;                                                                */
/*  WORD ����;                                                                */
/*  ...                                                                       */
/* 2014/8/30 manwjh v0.2                                                      */
/*           ����g_SysCfg.WAKEUP_SAVE_ENERGY_T ����,��ʱ���ѡ�                 */
/* 2014/9/2  manwjh v0.3                                                      */
/*           ����g_SysCfg.WAKEUP_SLEEP_T ����,��ʱ���ѡ�                       */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/* include */
#include "includes.h"

/* external */
extern SOCKET yeelink_server;
extern SYSTEM_CFG g_SysCfg;

/* public */


/* private */
void cpy_cfg_to_flash(FIL *fp, uint32_t flash_addr, uint32_t f_datetime);
bool check_flash_cfg(void);

/* define */
#define CFG_FILE_NAME   "itrack.cfg"
#define FLASH_CFG_ADDR  (uint8_t*)0x0801F800
#define FLASH_CFG_HEADER_SIZE   16

uint32_t cfg_checksum;
uint32_t cfg_size;
uint32_t cfg_datetime;


int check_itarcker_cfg(void)
{
    FIL fs;       /*Work area (file system object) for logical drive*/
    FILINFO f_info;
    //
    uint32_t fl_datetime;
    uint16_t err_log=0;
    
    //���FLASH�ڲ������ļ�
    check_flash_cfg();
    
    //���disk���Ƿ���itrack.cfg�ļ�
    printf("\r\nSYS_CFG-->����Ƿ��������ļ�����!");
    if( f_stat("\\iTracker.cfg",&f_info)!=FR_OK )
        err_log++;
        
    if( (err_log>0)||f_open(&fs,"iTracker.cfg",FA_READ)!=FR_OK )
        err_log++;

    if( err_log>0 ){
        return 0;
    }

    //����ļ���С
    if( f_info.fsize>1024-FLASH_CFG_HEADER_SIZE ){
        f_close(&fs);
        printf("\r\nSYS_CFG-->iTrack.cfg�����޶���С!");
        return 0;
    }
    //���ʱ���Ƿ��и���
    fl_datetime = f_info.fdate<<16|f_info.ftime;
    if( (cfg_datetime!=fl_datetime)
        ||(cfg_size!=f_info.fsize) )
    {
        cpy_cfg_to_flash(&fs,(uint32_t)FLASH_CFG_ADDR,fl_datetime);
        f_close(&fs);
        printf("\r\nSYS_CFG-->iTracker.cfg���µ�FLASH�ɹ�����");
        return 1;
    }
   
    f_close(&fs);
    return 0;
}

bool check_flash_cfg(void)
{
    uint32_t checksum;
    uint16_t i;
    
    uint8_t *cfg_fl_p = FLASH_CFG_ADDR;
    
    printf("\r\nSYS_CFG-->���FLASHϵͳ���ò���:");
    //read header
    cfg_checksum = *(uint32_t*)(FLASH_CFG_ADDR);
    cfg_size = *(uint32_t*)(FLASH_CFG_ADDR+4);
    cfg_datetime = *(uint32_t*)(FLASH_CFG_ADDR+8);
    
    //У��flash����������
    if( cfg_size<=1024-FLASH_CFG_HEADER_SIZE ){
        cfg_fl_p = FLASH_CFG_ADDR+FLASH_CFG_HEADER_SIZE;
        checksum = 0;
        for( i=0; i<cfg_size; i++ ){
            checksum += *cfg_fl_p++;
        }
        //
        if( cfg_checksum==checksum ){
            printf("OK");
            return true;
        }
    }
    
    printf("FAIL");
    return false;
}

void cpy_cfg_to_flash(FIL *fp, uint32_t flash_addr, uint32_t f_datetime)
{
    uint32_t checksum=0;
    uint32_t tmp;
    uint8_t *wr_p;
    unsigned int ByteRead;
    uint32_t fsize=0;
    int err_log=0;
    int i;
    
    //������ǰ��
    FLASH_Unlock();
    FLASH_ErasePage( flash_addr );        /* ����ҳ */
    
    wr_p = (uint8_t*)(flash_addr+FLASH_CFG_HEADER_SIZE);
    //��+16ƫ������ʼ���ļ�����COPY��flash
    do{
        if( f_read(fp,&tmp,4,&ByteRead) != FR_OK ){
            printf("\r\nnSYS_CFG-->�ļ���ȡ����!");
            err_log++;
            break;
        }
        FLASH_ProgramWord( (uint32_t)wr_p,tmp);   /* д32λ */
        fsize += ByteRead;
        wr_p += 4;
        if( ByteRead<4 ){
            printf("\r\nSYS_CFG-->�ļ���ȡ���!");
            break;
        }
    }while(1);
    //У���
    wr_p = (uint8_t *)(flash_addr+FLASH_CFG_HEADER_SIZE);
    checksum = 0;
    for( i=0; i<fsize; i++ ){
        checksum += *wr_p++;
    }
    
    //дͷ����Ϣ
    FLASH_ProgramWord( flash_addr,checksum);   /* д32λ */
    FLASH_ProgramWord( flash_addr+4,fsize);   /* д32λ */
    FLASH_ProgramWord( flash_addr+8,f_datetime);   /* д32λ */
    
    //������ʾ
    if( err_log!= 0){
        printf("\r\nnSYS_CFG-->ϵͳ��������!!!!!!!!!!!!!!!!!!!");
        beep_taskctrl(BEEP_CNT,10);
    }
    //�ر�FLASH
    FLASH_Lock();
}



static const uint8_t *cfg_rd_p;

int cfg_get_line(char *out_bf)
{
    char *bf = out_bf;
    int rd_bytes=0;
    uint8_t tmp;
    
    while( cfg_rd_p<=(uint8_t*)(FLASH_CFG_ADDR+cfg_size) )
    {
        tmp = *cfg_rd_p;
        *bf = tmp;
        bf++;
        cfg_rd_p++;
        rd_bytes++;
        if( tmp =='\r' ){
            *bf++ = '\n',
            *bf = 0x0;
            rd_bytes += 2;
            cfg_rd_p++;
            break;
        }
    }
    
    return rd_bytes;
}

int cfg_get_segment(char *in_bf, char *out_bf)
{
    char tmp;
    
    //����=
    while(1){
        tmp = *in_bf++;
        if( tmp== '=' )
            break;
    };
    
    //������һ����ĸ/����/����
    while(1){
        tmp = *in_bf++;
        if( ischar(tmp) )
            break;
    };
    
    //COPY�ַ���out_bf
    *out_bf++ = tmp;
    while(1){
        tmp = *in_bf++;
        if( ischar(tmp) )
            *out_bf++ = tmp;
        else{
            *out_bf = 0x0;
            break;
        }
    }

    return 1;
}


bool load_sys_config(void)
{
    char n[10];
    char tmp_str[100];

    syscfg_init();
    //
    if( check_flash_cfg()==false ){
        g_System.sys_err |= CFG_ERR_NULL_F;
        return false;
    }
    //        
    cfg_checksum = *(uint32_t*)(FLASH_CFG_ADDR);
    cfg_size = *(uint32_t*)(FLASH_CFG_ADDR+4);
    cfg_datetime = *(uint32_t*)(FLASH_CFG_ADDR+8);
    cfg_rd_p = (uint8_t *)(FLASH_CFG_ADDR+FLASH_CFG_HEADER_SIZE);
    //
    while(cfg_get_line(tmp_str)>0){
        //[SECTION:HTTP HEADER]
        if( strstr(tmp_str,"[SECTION:HTTP HEADER]") ){
            yeelink_http_init();
            while(cfg_get_line(tmp_str)>0){
                if( strstr(tmp_str,"[SECTION END]")||strstr(tmp_str,"[FILE END]") ){
					strcat(yeelink_server.http_header,"\r\n");      //���һ���س�������Ϣ
                    yeelink_server.http_data = &yeelink_server.http_bf[strlen(yeelink_server.http_header)];    //�޸�http_dataָ��λ��
                    break;
                }
                //copy��yeelink��post buffer
                strcat(yeelink_server.http_header,tmp_str);     //���http_header��Ϣ
            }
            printf("\r\n[HTTP HEADER]\r\n%s",yeelink_server.http_header);
        }
        //[SECTION:SERVER INFO]
        if( strstr(tmp_str,"[SECTION:SERVER INFO]") ){
            while(cfg_get_line(tmp_str)>0){
                if( strstr(tmp_str,"[SECTION END]")||strstr(tmp_str,"[FILE END]") ){
                    break;
                }
                //
                if( strstr(tmp_str,"HOSTName") ){
                    cfg_get_segment(tmp_str,yeelink_server.HostName);
                }else if( strstr(tmp_str,"PORT") ){
                    cfg_get_segment(tmp_str,yeelink_server.port);
                }else if( strstr(tmp_str,"PROCOTL") ){
                    cfg_get_segment(tmp_str,yeelink_server.procotl);
                }
            }
        }
        //[SECTION:SYS CONFIG]
        if( strstr(tmp_str,"[SECTION:SYS CONFIG]") ){
            while(cfg_get_line(tmp_str)>0){
                if( strstr(tmp_str,"[SECTION END]")||strstr(tmp_str,"[FILE END]") ){
                    break;
                }
                //ϵͳ����
                if( strstr(tmp_str,"BEEP_ON") ){
                    cfg_get_segment(tmp_str,n);
                    if( isdecstring(n)==1 ){
                        if( n[0]=='0' )
                            g_SysCfg.BEEP_ON = false;
                        else if( n[0]=='1' )
                            g_SysCfg.BEEP_ON = true;
                    }
                }else if( strstr(tmp_str,"LET_US_GO_MAX_T") ){
                    cfg_get_segment(tmp_str,n);
                    if( isdecstring(n)<6 ){
                        g_SysCfg.LET_US_GO_MAX_T = DecStr2Int(n,5);
                    }
                }else if( strstr(tmp_str,"NET_SEND_T_MIN") ){
                    cfg_get_segment(tmp_str,n);
                    if( isdecstring(n)<6 ){
                        g_SysCfg.NET_SEND_T_MIN = DecStr2Int(n,5);
                    }
                }else if( strstr(tmp_str,"SAVE_ENERGY_ON") ){
					cfg_get_segment(tmp_str,n);
                    if( isdecstring(n)==1 ){
                        if( n[0]=='0' )
                            g_SysCfg.SAVE_ENERGY_ON = false;
                        else if( n[0]=='1' )
                            g_SysCfg.SAVE_ENERGY_ON = true;
                    }
                }else if( strstr(tmp_str,"ENTER_SAVE_ENERGY_T") ){
                    cfg_get_segment(tmp_str,n);
                    if( isdecstring(n)<6 ){
                        g_SysCfg.ENTER_SAVE_ENERGY_T = DecStr2Int(n,5);
                    }
                }else if( strstr(tmp_str,"WAKEUP_SAVE_ENERGY_T") ){
                    cfg_get_segment(tmp_str,n);
                    if( isdecstring(n)<6 ){
                        g_SysCfg.WAKEUP_SAVE_ENERGY_T = DecStr2Int(n,5);
                    }
                }else if( strstr(tmp_str,"WAKEUP_SLEEP_T") ){
                    cfg_get_segment(tmp_str,n);
                    if( isdecstring(n)<6 ){
                        g_SysCfg.WAKEUP_SLEEP_T = DecStr2Int(n,5);
                    }
                }else if( strstr(tmp_str,"GPRS_RETRY_T") ){
                    cfg_get_segment(tmp_str,n);
                    if( isdecstring(n)<6 ){
                        g_SysCfg.GPRS_RETRY_T = DecStr2Int(n,5);
					}
                }else if( strstr(tmp_str,"GPS_SEARCH_T") ){
                    cfg_get_segment(tmp_str,n);
                    if( isdecstring(n)<6 ){
                        g_SysCfg.GPS_SEARCH_T = DecStr2Int(n,5);
					}
                }else if( strstr(tmp_str,"WDT_ON") ){
                    cfg_get_segment(tmp_str,n);
                    if( isdecstring(n)==1 ){
                        if( n[0]=='0' )
                            g_SysCfg.WDT_ON = false;
                        else if( n[0]=='1' )
                            g_SysCfg.WDT_ON = true;
                    }
                }
            }
        }
    }
    if( strlen(yeelink_server.http_header)==0 ){
        g_System.sys_err |= CFG_ERR_YEELINK;
    }

	return false;
}


