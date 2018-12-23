/*----------------------------------------------------------------------------*/
/* sys_cfg.c                                                                  */
/* 描述:校验/更新/读取系统配置                                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* 2014/7/28 manwjh 建立 V0.1                                                 */
/* STM32中型容量MCU，1K/页                                                    */
/* itrack.cfg, 最大不超过2K字节,即占2个页。                                   */
/*             126页(0x0801F800~0x08001FBFF)                                  */
/*             127页(0x0801FC00~0x08001FFFF)                                  */
/*  flash存储文件结构(16个字节的文件描述+内容)：                              */
/*  DWORD fchecksum      文件内容校验和                                       */
/*  DWORD fsize;         File size                                            */
/*  DWORD fdatetime;     Last modified date&time                              */
/*  WORD 保留;                                                                */
/*  WORD 保留;                                                                */
/*  ...                                                                       */
/* 2014/8/30 manwjh v0.2                                                      */
/*           增加g_SysCfg.WAKEUP_SAVE_ENERGY_T 变量,定时唤醒。                 */
/* 2014/9/2  manwjh v0.3                                                      */
/*           增加g_SysCfg.WAKEUP_SLEEP_T 变量,定时唤醒。                       */
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
    
    //检查FLASH内部配置文件
    check_flash_cfg();
    
    //检查disk中是否有itrack.cfg文件
    printf("\r\nSYS_CFG-->检查是否有配置文件更新!");
    if( f_stat("\\iTracker.cfg",&f_info)!=FR_OK )
        err_log++;
        
    if( (err_log>0)||f_open(&fs,"iTracker.cfg",FA_READ)!=FR_OK )
        err_log++;

    if( err_log>0 ){
        return 0;
    }

    //检查文件大小
    if( f_info.fsize>1024-FLASH_CFG_HEADER_SIZE ){
        f_close(&fs);
        printf("\r\nSYS_CFG-->iTrack.cfg超过限定大小!");
        return 0;
    }
    //检查时间是否有更新
    fl_datetime = f_info.fdate<<16|f_info.ftime;
    if( (cfg_datetime!=fl_datetime)
        ||(cfg_size!=f_info.fsize) )
    {
        cpy_cfg_to_flash(&fs,(uint32_t)FLASH_CFG_ADDR,fl_datetime);
        f_close(&fs);
        printf("\r\nSYS_CFG-->iTracker.cfg更新到FLASH成功！！");
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
    
    printf("\r\nSYS_CFG-->检查FLASH系统配置参数:");
    //read header
    cfg_checksum = *(uint32_t*)(FLASH_CFG_ADDR);
    cfg_size = *(uint32_t*)(FLASH_CFG_ADDR+4);
    cfg_datetime = *(uint32_t*)(FLASH_CFG_ADDR+8);
    
    //校验flash配置区内容
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
    
    //擦除当前块
    FLASH_Unlock();
    FLASH_ErasePage( flash_addr );        /* 擦除页 */
    
    wr_p = (uint8_t*)(flash_addr+FLASH_CFG_HEADER_SIZE);
    //从+16偏移量开始将文件内容COPY到flash
    do{
        if( f_read(fp,&tmp,4,&ByteRead) != FR_OK ){
            printf("\r\nnSYS_CFG-->文件读取错误!");
            err_log++;
            break;
        }
        FLASH_ProgramWord( (uint32_t)wr_p,tmp);   /* 写32位 */
        fsize += ByteRead;
        wr_p += 4;
        if( ByteRead<4 ){
            printf("\r\nSYS_CFG-->文件读取完成!");
            break;
        }
    }while(1);
    //校验和
    wr_p = (uint8_t *)(flash_addr+FLASH_CFG_HEADER_SIZE);
    checksum = 0;
    for( i=0; i<fsize; i++ ){
        checksum += *wr_p++;
    }
    
    //写头部信息
    FLASH_ProgramWord( flash_addr,checksum);   /* 写32位 */
    FLASH_ProgramWord( flash_addr+4,fsize);   /* 写32位 */
    FLASH_ProgramWord( flash_addr+8,f_datetime);   /* 写32位 */
    
    //错误提示
    if( err_log!= 0){
        printf("\r\nnSYS_CFG-->系统严重问题!!!!!!!!!!!!!!!!!!!");
        beep_taskctrl(BEEP_CNT,10);
    }
    //关闭FLASH
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
    
    //搜索=
    while(1){
        tmp = *in_bf++;
        if( tmp== '=' )
            break;
    };
    
    //搜索第一个字母/数字/引号
    while(1){
        tmp = *in_bf++;
        if( ischar(tmp) )
            break;
    };
    
    //COPY字符到out_bf
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
					strcat(yeelink_server.http_header,"\r\n");      //填充一个回车空行信息
                    yeelink_server.http_data = &yeelink_server.http_bf[strlen(yeelink_server.http_header)];    //修改http_data指针位置
                    break;
                }
                //copy到yeelink的post buffer
                strcat(yeelink_server.http_header,tmp_str);     //填充http_header信息
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
                //系统配置
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


