/*----------------------------------------------------------------------------*/
/* sim800.c                                                                   */
/* 描述:gprs通讯程序                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* 2014/6/7 manwjh 建立 V0.1                                                  */
/* 2014/7/12 manwjh v0.2  modem初始化完成后,只读取状态和就异常进行跳转.       */
/* 2014/7/22 manwjh v0.3                                                      */
/*  去掉电压检测部分代码,MODEM发现电压低了本身会自己进入power off.            */
/*  系统增加test_modem(),用AT命令测试MODEM是否正常,无应答说明MODEM进入POWEROFF*/
/*  增加MODEM低功耗模式                                                       */
/* 2014/8/6 manwjh v0.4  调整MODEM架构                                        */
/* 2014/8/13 manwjh v0.5 评估电源跌落造成的影响,修复其他BUG.评估AT+CBC命令    */
/*                       评估无信号区域AT+CGAT(GPRS附着)开机的表现.           */
/*                                                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* SIM800L 说明 */
/*  1. 电压范围: 3.4V~4.4V, 突发模式时峰值电流2A. 
       VBAT的跌落不要超过3.0V,否则模块可能会关机.
       VBAT电压<=3.5V,模块自动发送"UNDER-VOLTAGE WARNNING, <3.4V 模块立即自动关机"UNDER-VOLTAGE POWER DOWN"
    2. 四频: GSM850, EGSM900, DCS1800, PCS1900. 符合GSM Phase 2/2+
    3. GPRS时隙缺省为等级12, 时隙 class1~12可选, 移动台等级B
*/



/* include */
#include "includes.h"


/* external */
extern char gprs_rx_buffer[],gprs_tx_buffer[];

/* public */
uint16_t gModemState;
float gModemVoltage;

/* private */
static unsigned long task_t_mark;   //秒计数器
static uint16_t gprs_taskstate;          //gprs_task状态机
//
int md_poweron_step=0;
int md_poweroff_step=0;
int md_check_fail_cnt=0;
int modem_poweron_flow(void);
int modem_poweroff_flow(void);
//
char send_atcmd(const char *atcmd, const char *hope_respond, int wait_respond_t);
void printf_strbf(const char *tag, char *bf);
char *atcmd_ack_is(char *bf, const char *hope_str);
int atcmd_wait_respond(int wait_t);
int atcmd_get_ack_section(char *out, int seg);
int test_modem(void);
//
int get_cipstatus(void);
int get_gsm_status(void);
int get_gprs_status(void);
int get_CSQ(void);
int check_sim_card(void);
int check_ipnet_status(void);
//
WINSOCKET_STATUS socket_send(void);
int socket_query(void);

struct{
    WINSOCKET_STATUS state;         //socket工作状态
    int send_flg;                   //启动发送一次数据包标志位
    unsigned char *dt;              //数据缓冲地址
    int dt_len;                     //数据缓冲长度
    unsigned long send_t_start;     //socket发送数据开始时间
    unsigned long send_time;        //socket发送一个数据包消耗时间
    int send_err_cnt;               //socket发送数据包连续错误次数
	long send_ok_cnt;                //socket发送成功次数
}winsocket;

char winsocket_server_open_cmd[100];    //模块TCP命令缓冲

//
const char CTRL_Z=0x1A;
const char ESC=0x1B;
//
#define SET_MD_STATE_BITS(a)     gModemState = (gModemState|a)
#define CLR_MD_STATE_BITS(a)     gModemState = (gModemState&(~a))
#define SET_MD_MODE(a)           gModemState = ((gModemState&(~MD_MODE_BIT_MASK))|a)


/* debug */
typedef enum{
    CMNET,
    CHN_UNICOM
}NET_NAME;

typedef enum{
    CIP_INITIAL,        //初始化
    CIP_START,          //启动任务
    CIP_CONFIG,         //配置场景
    CIP_GPRSACT,        //场景已激活
    CIP_IPSTATUS,       //获得本地IP地址
    CIP_CONNECTING,     //TCP连接中
    CIP_CONNECTED,      //连接建立成功
    CIP_CLOSING,       //正在关闭TCP连接
    CIP_CLOSED,         //TCP连接断开
    CIP_PDP_DEACT,      //场景被释放
    CIP_UNKNOW,         //未知状态
}CIP_STATUS;


enum{
    NO_ACK,
    RESPOND_OK,
	RESPOND_UNMATCH,
};


/*----------------------------------------------------------------------------*/
/* WinSocket                                                                  */
/*                                                                            */
/*----------------------------------------------------------------------------*/
WINSOCKET_STATUS WinSocketClrState(SOCKET *s)
{
    winsocket.send_flg = 0;
    winsocket.state = SOCKET_INIT;

	return winsocket.state;
}

WINSOCKET_STATUS WinSocketInit(SOCKET *s)
{
    winsocket.state = SOCKET_INIT;
    winsocket.send_err_cnt = 0;
    winsocket.send_ok_cnt = 0;
    winsocket.send_flg = 0;
    sprintf(winsocket_server_open_cmd,"%s%s%s%s%s%s","AT+CIPSTART=",s->procotl,",",s->HostName,",",s->port);
    printf("\r\n%s",winsocket_server_open_cmd);
    
    return winsocket.state;
}

WINSOCKET_STATUS WinSocketGetState(void)
{
    return winsocket.state;
}

WINSOCKET_STATUS WinSocketSend(void *bf, int len)
{
    if( GetModemState(MD_IPNET_OK)==MD_IPNET_OK ){
        printf("\r\n%s\r\n",(char *)bf);
        gprs_taskstate = WINSOCKET_SEND;
        winsocket.dt = bf;
        winsocket.dt_len = len;
	    winsocket.send_flg = 1;
    }else{
		printf("\r\nGPRS-->IPNET NOT OK!");
    }

	return winsocket.state;
}


WINSOCKET_STATUS socket_open(void)
{
    WINSOCKET_STATUS rslt=SOCKET_OPEN;
    
    int cip_state = get_cipstatus();
    
    if( cip_state==CIP_CONNECTED ){
		rslt = SOCKET_LINKED;
	}else if( cip_state==CIP_CLOSED ){
	    send_atcmd("AT+CIPCLOSE","OK",10);
        if( send_atcmd(winsocket_server_open_cmd,"ERROR",10)==RESPOND_OK ){
            printf("\r\nGPRS-->SOCKET CREATE Fail!");
            rslt = SOCKET_OPEN_FAIL;
        }else{
            printf("\r\nGPRS-->SOCKET CREATE OK!\r\n");
            rslt = SOCKET_OPEN_OK;
        }
    }else{
		printf(".");
	}
	
	return rslt;
}

#if 1   //单包发送,测试大于323 BYTES貌似数据就会被切掉. 网上有文称大于384会被分包。
WINSOCKET_STATUS socket_send(void)
{
    WINSOCKET_STATUS rslt=SOCKET_SEND;
    char at_cipsend_cmd[20];
    
    sprintf(at_cipsend_cmd,"%s%d","AT+CIPSEND=",winsocket.dt_len);
    
    if( send_atcmd(at_cipsend_cmd,">",200) ){
        delay_nus(100);
        uart3_clr();
        uart3_sendbf(winsocket.dt,winsocket.dt_len);
    }else{
		printf("\r\nGPRS-->传送数据失败!");
		rslt = SOCKET_SEND_FAIL;
	}
    winsocket.dt_len = 0;

    return rslt;
}
#else	//拆包发送, Yeelink不支持分包
#define SOCKET_MAX_LEN      323

WINSOCKET_STATUS socket_send(void)
{
    int rslt=0;
    int len=0;
    char at_cipsend_cmd[20];

    if(winsocket.dt_len>SOCKET_MAX_LEN){
        len = SOCKET_MAX_LEN;
    }else{
        len = winsocket.dt_len;
    }
    
    sprintf(at_cipsend_cmd,"%s%d","AT+CIPSEND=",len);
    printf("\r\nGPRS-->%s",at_cipsend_cmd);
    //
    if( send_atcmd(at_cipsend_cmd,">",20) ){
		uart3_clr();
        uart3_sendbf(winsocket.dt+winsocket.send_cnt,len);
        rslt = 1;
    }
    winsocket.dt_len -= len;
    winsocket.send_cnt += len;

    return rslt;
}
#endif

int socket_query(void)
{
    char txlen[8],acklen[8];
    int rslt=0;

    if( send_atcmd("AT+CIPACK","+CIPACK",100) ){
        atcmd_get_ack_section(txlen,0);
        atcmd_get_ack_section(acklen,1);
        printf("\r\nGPRS-->发送数据:%s,服务器接收数据:%s\r\n",txlen,acklen);
        if( DecStr2Int(txlen,4)==DecStr2Int(acklen,4) )
            rslt = 1;
        
    }else{
		printf_strbf("[数据发送状态查询]",gprs_rx_buffer);
	}
   
    return rslt;
}

/*----------------------------------------------------------------------------*/
/* gprs_task()                                                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
void gprs_taskinit(void)
{
    led_taskctrl(LED2,OFF,0);
    CLR_MD_STATE_BITS((uint16_t)0xffff);
    gprs_taskctrl(MODEM_IDEL);
    
}

//外部控制任务
void gprs_taskctrl(int cmd)
{
    task_t_mark = GetSysTick_Sec();    
    switch(cmd){
        case MODEM_IDEL:
        case MODEM_POWER_ON:
            //如果MODEM处于完全OK状态,则系统不重复开机
            if( GetModemState(MD_OK_ALL)!= MD_OK_ALL ){
                gprs_taskstate = cmd;
                md_poweron_step = 0;
            }
            break;

        case MODEM_POWER_OFF:
            if( GetModemMode()!=MODEM_PWROFF ){
                gprs_taskstate = cmd;
                md_poweroff_step = 0;
            }
            break;
            
        case MODEM_CHECK:
            gprs_taskstate = cmd;
            break;
            
        case MODEM_RETRY:
            send_atcmd("AT+CIPSHUT","SHUT OK",100);     //关闭后状态为IP INITIAL
            gprs_taskstate = cmd;
            break;

        case MODEM_HW_ERROR:
            printf("\r\nGPRS-->硬件错误\r\n");
            gprs_taskstate = cmd;
            break;

        case WINSOCKET_SEND:
	    case WINSOCKET_SEND_QUERY:
	    case WINSOCKET_ERROR:
	    case WINSOCKET_FINISHED:
	        gprs_taskstate = cmd;
	        break;
    }
}

uint16_t GetModemState(uint16_t BIT_MASK)
{
    return (gModemState&BIT_MASK);
}

uint16_t GetModemMode(void)
{
    return (gModemState&MD_MODE_BIT_MASK);
}


void gprs_task(void)
{
    //SIM卡热插拔检测,无SIM卡直接关闭MODEM
    if( GetModemState(MD_SIM_IN)==MD_SIM_IN ){
        if( check_sim_card()==0 )
            gprs_taskctrl(MODEM_POWER_OFF);
    }else
        check_sim_card();
    
    //gprs task main loop
    switch(gprs_taskstate)
    {
        case MODEM_IDEL:        //一个空的状态
            task_t_mark = GetSysTick_Sec();
            break;

        /*-------xxxxxxxx--------打开MODEM-----xxxxxxxx----------*/
        case MODEM_POWER_ON:
            winsocket.send_err_cnt = 0;
    		winsocket.send_flg = 0;
            if( modem_poweron_flow()==MD_POWER_ON_END ){
                if( GetModemMode()==MODEM_NORMAL )
    		        gprs_taskctrl(MODEM_CHECK);
                else
                    gprs_taskctrl(MODEM_HW_ERROR);
    		}
            break;

        /*-------xxxxxxxx--------关闭MODEM-----xxxxxxxx----------*/
        case MODEM_POWER_OFF:
            winsocket.send_err_cnt = 0;
    		winsocket.send_flg = 0;
            //
            modem_poweroff_flow();
            break;

        /*-------xxxxxxxx--------发送数据-----xxxxxxxx----------*/
        case WINSOCKET_SEND:
            //WINSOCKET初始化
            if( winsocket.send_flg ){
                CLR_MD_STATE_BITS(MD_IPNET_IDEL);
                winsocket.send_t_start = GetSysTick_Sec();
				winsocket.state = SOCKET_OPEN;
				winsocket.send_flg = 0;
                led_taskctrl(LED2,ON,0);     //hw_io_ctrl(LED2,ON);
				printf("\r\nGPRS-->Winscoket Connecting");
				send_atcmd(winsocket_server_open_cmd,"ERROR",100);
            }

            //建立连接超时
            if( GetSysTick_Sec()>winsocket.send_t_start+10 ){
                printf("\r\nGPRS-->建立连接超时!\r\n");
                winsocket.state = SOCKET_SEND_FAIL;
                gprs_taskctrl( WINSOCKET_ERROR );
                break;
            }
            
            //WINSOCKET连接
            if( winsocket.state==SOCKET_OPEN ){
                if( socket_open()==SOCKET_LINKED )
                    winsocket.state = SOCKET_SEND;
                else
                    break;
            }
            //WINSOCKET发送
            if( winsocket.state==SOCKET_SEND ){
                if( socket_send()==SOCKET_SEND_FAIL ){
                    printf("\r\nGPRS-->发送数据失败!\r\n");
					gprs_taskctrl( WINSOCKET_ERROR );
    			}
    			else{
    			    gprs_taskctrl( WINSOCKET_SEND_QUERY );
                	task_t_mark = GetSysTick_Sec();
				}
    		}
            break;

        case WINSOCKET_SEND_QUERY:
            //发送超时
            if( GetSysTick_Sec()>winsocket.send_t_start+20 ){
                printf("\r\nGPRS-->数据接收响应超时!\r\n");
                winsocket.state = SOCKET_SEND_FAIL;
                gprs_taskctrl( WINSOCKET_ERROR );
                break;
            }
            //
            if( uart3_get_reclen() ){
                if( atcmd_ack_is(gprs_rx_buffer,"SEND OK") ){
					printf("\r\nGPRS-->SEND OK");
                    socket_query();
                    if( winsocket.dt_len>0 ){           //如果未发送完则继续发送剩余数据
                        winsocket.state = SOCKET_SEND;
                        gprs_taskctrl( WINSOCKET_SEND );
                    }else{
                        winsocket.state = SOCKET_SEND_OK;
                        gprs_taskctrl( WINSOCKET_FINISHED );    //发送完成出口
                    }
                }else{
                    printf("\r\nGPRS-->未确定服务器是否接收到!");
                    winsocket.state = SOCKET_SEND_FAIL;
                    gprs_taskctrl( WINSOCKET_ERROR );           //发送错误出口
                }
            }
            break;
        //WINSOCKET 错误出口
        case WINSOCKET_ERROR:
            SET_MD_STATE_BITS(MD_IPNET_IDEL);
            send_atcmd("AT+CIPCLOSE","CLOSE OK",10);
            //
            winsocket.send_time = GetSysTick_Sec()-winsocket.send_t_start;
            printf("\r\nGPRS-->Winsocket ERROR! time:%d%s",winsocket.send_time,"\r\n");
            led_taskctrl(LED2,FLASH_1HZ,0);     //hw_io_ctrl(LED2,OFF);
            //
            if( winsocket.send_err_cnt++>3 ){       //如果发送连续失败,如何在发现网络后尽快完成连接。
				winsocket.send_err_cnt = 0;
                gprs_taskctrl(MODEM_RETRY);
            }else
                gprs_taskctrl(MODEM_CHECK);
            break;

        //WINSOCKET 成功出口
        case WINSOCKET_FINISHED:
            SET_MD_STATE_BITS(MD_IPNET_IDEL);
            send_atcmd("AT+CIPCLOSE","CLOSE OK",10);
            //
            winsocket.send_time = GetSysTick_Sec()-winsocket.send_t_start;
            printf("\r\nGPRS-->Winsocket time:%d%s",winsocket.send_time,"\r\n");
            led_taskctrl(LED2,FLASH_1HZ,0);     //hw_io_ctrl(LED2,OFF);
            gprs_taskctrl(MODEM_CHECK);
            //
            winsocket.send_ok_cnt++;
            winsocket.send_err_cnt=0;
            break;

        /*-------xxxxxxxx--------检测MODEM状态-----xxxxxxxx----------*/
        case MODEM_CHECK:
            //状态检测,3秒检测一次
            if( GetSysTick_Sec()>task_t_mark+1 ){
                task_t_mark = GetSysTick_Sec();
                //MODEM命令响应测试
                if( test_modem()==0 ){
                    printf("\r\nGPRS-->MODEM检测失败\r\n");
                    CLR_MD_STATE_BITS(MD_CSQ_OK|MD_GSM_OK|MD_GPRS_OK);
                    gprs_taskctrl(MODEM_HW_ERROR);
                }else{
                    //信号质量,GSM,GPRS检测
                    get_CSQ();
                    get_gsm_status();
                    get_gprs_status();
                    check_ipnet_status();
				}
			}
            break;
            
        case MODEM_RETRY:
            if( test_modem()==0 ){
                printf("\r\nGPRS-->MODEM检测失败\r\n");
                CLR_MD_STATE_BITS(MD_CSQ_OK|MD_GSM_OK|MD_GPRS_OK);
                gprs_taskctrl(MODEM_HW_ERROR);
            }
            //
            if( GetSysTick_Sec()>task_t_mark+10 ){
                get_CSQ();
                get_gsm_status();
                get_gprs_status();
                check_ipnet_status();
                if( GetModemState(MD_IPNET_OK)==MD_IPNET_OK )
                    gprs_taskctrl(MODEM_CHECK);
            }
            //
            if( GetSysTick_Sec()>task_t_mark+20 ){
                SET_MD_STATE_BITS(MD_ERR_NET_NULL);
                gprs_taskctrl(MODEM_HW_ERROR);
            }
            break;

        /*-------xxxxxxxx--------MODEM 错误状态-----xxxxxxxx----------*/
        //错误类型：MODEM没开机,SIM卡没插,命令无响应,参数无法配置
        //#define MD_ERR_POWERON          (UINT)(1<<8)        //MODEM没开机
        //#define MD_ERR_NO_ACK           (UINT)(1<<10)       //系统长期应答失败
        //#define MD_ERR_SIM              (UINT)(1<<12)       //无SIM卡,或错误
        //#define MD_ERR_CONFIG           (UINT)(1<<13)       //参数无法配置
        case MODEM_HW_ERROR:
            if( GetSysTick_Sec()>task_t_mark+1 ){
                task_t_mark = GetSysTick_Sec();
                //
                printf("\r\nGPRS-->MODEM错误:%4x\r\n",gModemState);
                if( gModemState&MD_ERR_POWERON ){
                    printf("    -->没有(无法)开启");
                }
                if( gModemState&MD_ERR_NO_ACK ){
                    printf("    -->长期命令无响应");
                }
                if( gModemState&MD_ERR_SIM ){
                    printf("    -->无SIM卡");
                }
                if( gModemState&MD_ERR_NET_NULL ){
                    printf("    -->连接不上网络");
                }
                if( gModemState&MD_ERR_VBAT ){
                    printf("    -->MODEM电压异常");
                }
                printf("\r\n");
                //
                if( (g_BatteryVoltage>V_OPEN_GPRS)&&(GetModemState(MD_SIM_IN)==MD_SIM_IN) )
                    gprs_taskctrl(MODEM_POWER_ON);
            }
            break;
    }
}


char send_atcmd(const char *atcmd, const char *hope_respond, int wait_respond_t)
{
    int get_len;
	unsigned char tmp;
    char atcmd_bf[150];
    
	uart3_clr();
	
	if( (u32)atcmd<0xff ){
		tmp = (unsigned char)atcmd;
		uart3_sendbf(&tmp,1);
	}else{
        strcpy(atcmd_bf,atcmd);
        strcat(atcmd_bf,"\r\n");
		uart3_printf(atcmd_bf);
    }
    
    
    if(wait_respond_t){
        get_len = atcmd_wait_respond(wait_respond_t);
        if( get_len==0 )
            printf("\r\nGPRS-->AT CMD响应超时!");

        if( (u32)hope_respond==0x0 ){
            if(get_len>0)
                return RESPOND_OK;
        }else{
            if( get_len>0 ){
                if( atcmd_ack_is(gprs_rx_buffer,hope_respond) )
                    return RESPOND_OK;
                else
                    return RESPOND_UNMATCH;
            }
        }
    }
    return NO_ACK;
}

//分析接收到的数据,是否有符合预期的数据存在.
char *atcmd_ack_is(char *bf, const char *hope_str)
{
    char *pos=NULL;
    
    pos = strstr((const char*)bf, hope_str);
    
    return pos;
}


//等待MODEM返回数据
int atcmd_wait_respond(int wait_t)
{
    unsigned long end_t = GetSysTick_10Ms()+wait_t;
    int get_len;
    
    while(GetSysTick_10Ms()<end_t){
        get_len = uart3_get_reclen();
        if( get_len ){
/*
            if( get_len>ATCMD_REC_BUFF_LEN )
                get_len = ATCMD_REC_BUFF_LEN;
            uart3_read(atcmd_rec_buff,get_len);
*/
            break;
        }
	}

    return get_len;
}

int atcmd_get_ack_section(char *out, int seg)
{
    char *in=gprs_rx_buffer;
    int i=0,j=0;
    
    //搜索:
    do{
        if( *in=='\0' )
            return 0;
    }while( *in++!=':' );
    
    //搜索第一个不是空格的字符
    do{
        if( *in=='\0' )
            return 0;
    }while( *in++!=' ' );
    
    //
    do{
        if( *in==',' ){
            j++;
            if(j>seg)
                break;
        }else{
            if( j==seg){
                 i++;
                 *out++ = *in;
            }
        }
        in++;
    }while( (*in!= '\0')&&(*in!='\r')&&(*in!='\n') );
	
	*out = '\0';

    return i;
}
//
int check_sim_card(void)
{
	int rslt =hw_io_ctrl(SIM_DET,GET);

	if( rslt ){
	    SET_MD_STATE_BITS(MD_SIM_IN);
	}else{
	    CLR_MD_STATE_BITS(MD_SIM_IN);
	}
	
    return( gModemState&MD_SIM_IN );
}


/*
<server state>
    OPENING
    LISTENING
    CLOSING
<client state>
    INITAL
    CONNECTING
    CONNECTED
    REMOTE CLOSING
    CLOSING
    CLOSED
<state>
    ...
*/ 
int get_cipstatus(void)
{
    int rslt = CIP_UNKNOW;
    
    if( send_atcmd("AT+CIPSTATUS","OK",10)==RESPOND_OK ){    //查询连接状态
        if( atcmd_ack_is(gprs_rx_buffer,"IP INITIAL") )
            rslt = CIP_INITIAL;
        else if( atcmd_ack_is(gprs_rx_buffer,"IP START") )
            rslt = CIP_START;
        else if( atcmd_ack_is(gprs_rx_buffer,"IP CONFIG") )
            rslt = CIP_CONFIG;
        else if( atcmd_ack_is(gprs_rx_buffer,"IP GPRSACT") )
            rslt = CIP_GPRSACT;
        else if( atcmd_ack_is(gprs_rx_buffer,"IP STATUS") )
            rslt = CIP_IPSTATUS;
        else if( atcmd_ack_is(gprs_rx_buffer,"TCP CONNECTING") )
            rslt = CIP_CONNECTING;
        else if( atcmd_ack_is(gprs_rx_buffer,"CONNECT OK") )
            rslt = CIP_CONNECTED;
        else if( atcmd_ack_is(gprs_rx_buffer,"TCP CLOSING") )
            rslt = CIP_CLOSING;
        else if( atcmd_ack_is(gprs_rx_buffer,"TCP CLOSED") )
            rslt = CIP_CLOSED;
        else if( atcmd_ack_is(gprs_rx_buffer,"PDP DEACT") )
            rslt = CIP_PDP_DEACT;
    }
    
    return rslt;
}

const char gsm_stat_1[]="注册到归属网络";
const char gsm_stat_5[]="注册到漫游网络";
const char gsm_stat_2[]="正在寻找运营商...";
const char gsm_stat_0[]="没有注册/注册被拒/未知";

int get_gsm_status(void)
{
    char n[8],stat[12];
        
    //
    if( send_atcmd("AT+CREG?","OK",10)==RESPOND_OK ){   //获取网络注册信息
        //返回+CREG:0,1或+CREG:0,5时都说明模块注册到GPRS网络
        atcmd_get_ack_section(n,0);
        atcmd_get_ack_section(stat,1);
#if 0
        printf("\r\nGPRS-->GSM网络注册:");
        switch(stat[0]){
            case '1':   printf(gsm_stat_1);    break;
            case '5':   printf(gsm_stat_5);    break;
            case '2':   printf(gsm_stat_2); break;
            case '3':
            case '4':
            case '0':
            default:    printf(gsm_stat_0);   break;
        }
#endif
        if( (stat[0]=='1')||(stat[0]=='5') ){
            SET_MD_STATE_BITS(MD_GSM_OK);
        }else{
            CLR_MD_STATE_BITS(MD_GSM_OK);
        }
    }
        
    //显示选择的运营商信息
#if 0
    if( gModemState&MD_GSM_OK ){
        if( send_atcmd("AT+COPS?","+COPS",10)==RESPOND_OK ){
            atcmd_get_ack_section(stat,2);
            printf("\r\nGPRS-->运营商:%s",stat);
        }
    }
#endif
    //    
    return (gModemState&MD_GSM_OK);
}


int get_gprs_status(void)
{
    char n[8],stat[8];
    
    if( send_atcmd("AT+CGREG?","OK",10)==RESPOND_OK ){   //获取GPRS网络注册信息
        //返回+CGREG:0,1或+CGREG:0,5时都说明模块注册到GPRS网络
        atcmd_get_ack_section(n,0);
        atcmd_get_ack_section(stat,1);
#if 0
        printf("\r\nGPRS-->GPRS网络注册:");
        switch(stat[0]){
            case '0':   printf("未注册");       break;
            case '1':   printf("已注册,本网");  break;
            case '2':   printf("正在搜索...");  break;
            case '3':   printf("注册被拒绝");   break;
            case '5':   printf("已注册，漫游"); break;
            default:    printf("未知原因");     break;
        }
#endif
        //
        if( (stat[0]=='1')||(stat[1]=='5') )
            SET_MD_STATE_BITS(MD_GPRS_OK);
        else
            CLR_MD_STATE_BITS(MD_GPRS_OK);
    }
        
    return (gModemState&MD_GPRS_OK);
}

//信号强度在15~35之间比较好
int get_CSQ(void)
{
    char n[8];
	int tmp;
    
    if( send_atcmd("AT+CSQ","+CSQ",10)==RESPOND_OK ){
        atcmd_get_ack_section(n,0);
#if 0
		printf("\r\nGPRS-->信号强度:%s\r\n",n);
#endif
		//
		tmp = DecStr2Int(n,2);
		if( (tmp>=5) )
		    SET_MD_STATE_BITS(MD_CSQ_OK);
        else
            CLR_MD_STATE_BITS(MD_CSQ_OK);
    }

	return gModemState;
}

int check_ipnet_status(void)
{
    int cip_state;
    
    CLR_MD_STATE_BITS(MD_IPNET_IDEL);
    CLR_MD_STATE_BITS(MD_IPNET_OK);
    //
	if( GetModemState(MD_OK_ALL)==MD_OK_ALL ){
        cip_state = get_cipstatus();
        
        if( cip_state==CIP_INITIAL )
            send_atcmd("AT+CSTT","OK",30);
        else if( cip_state==CIP_START )
            send_atcmd("AT+CIICR","OK",30);
        else if( cip_state==CIP_GPRSACT )
            send_atcmd("AT+CIFSR","OK",30);
        else if( (cip_state==CIP_IPSTATUS)||(cip_state==CIP_CLOSED) ){
            SET_MD_STATE_BITS(MD_IPNET_IDEL);
            SET_MD_STATE_BITS(MD_IPNET_OK);
        }
    }

    return GetModemState(MD_IPNET_OK);
}

void printf_strbf(const char *tag, char *bf)
{
    printf("\r\n-----------------------\r\n");
    printf(tag); printf("\r\n");
    printf(bf);
    printf("\r\n-----------------------\r\n");
}


int test_modem(void)
{
    int i=2;
	char n[6];
	int v_tmp;
    
    do{
        if( send_atcmd("AT","OK",30)==RESPOND_OK ){
            CLR_MD_STATE_BITS((uint16_t)MD_ERR_NO_ACK);
#if 0
            if( send_atcmd("AT+CBC","OK",10)==RESPOND_OK ){
                atcmd_get_ack_section(n,2);
				v_tmp = DecStr2Int(n,4);
				if(v_tmp>3000){
                	gModemVoltage = (float)v_tmp/1000;
                	printf("\r\nGPRS-->Modem voltage:%f",gModemVoltage);
				}
            }
#endif
            break;
        }else{
            SET_MD_STATE_BITS((uint16_t)MD_ERR_NO_ACK);
            Wait10Ms(1);
        }
    }while(i--);
    
    return ( (gModemState&MD_ERR_NO_ACK)? 0 : 1);
}

/*----------------------------------------------------------------------------*/
/* md_poweron_flow()                                                          */
/* md_poweroff_flow()                                                         */
/*                                                                            */
/*----------------------------------------------------------------------------*/
static unsigned long pwronoff_t_mark;   //

int modem_poweron_flow(void)
{
	char n[8],stat[8];
    int err_log=0;

	if( GetSysTick_Sec()>task_t_mark+50 ){
		md_poweron_step =  MD_POWER_ON;
		task_t_mark = GetSysTick_Sec();
	}
    
    switch(md_poweron_step)
    {
        case MD_POWER_ON:   //软件开机
            led_taskctrl(LED2,FLASH_5HZ,0);
            printf("\r\nGPRS-->开机...\r\n");
            CLR_MD_STATE_BITS((uint16_t)0xffff);     //清除所有标志
            SET_MD_MODE(MODEM_PWRON);   //MODEM开机
            md_poweron_step = 1;
            break;

        case 1:     //硬件开机
	        if( g_BatteryVoltage>V_OPEN_GPRS ){
	            if( test_modem()==0 ){
	                hw_io_ctrl(GSM_PWR,POWER_ON);
	                pwronoff_t_mark = GetSysTick_Sec();
	                md_poweron_step = 2;
                }else{
                    md_poweron_step = 3;
                    printf("\r\nGPRS-->处于开机状态");
            	}
        	}else{
        	    SET_MD_STATE_BITS(MD_ERR_VBAT);
        	    md_poweron_step = MD_POWER_ON_ERROR;
            	printf("\r\nGPRS-->供电电压过低,无法开机");
        	}
        	break;
        case 2:  //delay 2~3秒
            if( GetSysTick_Sec()>pwronoff_t_mark+3 )
                md_poweron_step = 3;
            else
                printf(".");
            
        case 3:  //等待开机完成
			if( test_modem() ){
                SET_MD_STATE_BITS(MD_PWR_OK);
                md_poweron_step = 4;
			}
            //
            if( GetSysTick_Sec()>task_t_mark+8 )
                md_poweron_step = MD_POWER_ON_ERROR;
            break;

        case 4: //开机成功?
		    if( test_modem() ){
                //配置模块硬件参数
                send_atcmd("ATE0","OK",10);                 //关闭回显
    			//send_atcmd("AT+IPR=57600","OK",100);		//设定波特率
    			//读取模块版本
                if( send_atcmd("ATI","OK",10)==RESPOND_OK ){
                    printf_strbf("[模块版本]",gprs_rx_buffer);
                    md_poweron_step = 5;
                }
            }
            //
            if( GetSysTick_Sec()>task_t_mark+10 )
                md_poweron_step = MD_POWER_ON_ERROR;
            break;

        case 5: //配置MODEM参数
            //超时以及基础命令响应检测
            if( GetSysTick_Sec()>task_t_mark+60 ){
                md_poweron_step = MD_POWER_ON_ERROR;
	        }else if( test_modem()==0 ){
                md_poweron_step = MD_POWER_ON_ERROR;
	        }
            //检测开机网络状态
            get_CSQ();
            get_gsm_status();
            if( GetModemState(MD_GSM_OK|MD_CSQ_OK)!=(MD_GSM_OK|MD_CSQ_OK) )
                break;
            //
            send_atcmd("AT+CIPSHUT","SHUT OK",100);
            err_log=0;
            //检查GPRS附着状态
/*
            if( (err_log==0)&&(send_atcmd("AT+CGATT?","OK",10)==RESPOND_OK) ){
				Wait10Ms(1);
                atcmd_get_ack_section(n,0);
                if( n[0]!='1' ){
                    if( send_atcmd("AT+CGATT=1","OK",100)!=RESPOND_OK )
                        err_log++;
                }
            }else
                err_log++;*/
            //设置GPRS移动台类别
            if( (err_log==0)&&(send_atcmd("AT+CGCLASS?","+CGCLASS:",10)==RESPOND_OK) ){
                atcmd_get_ack_section(n,0);
                if( strcmp(n,"\"B\"")!=0 ){
                    if( send_atcmd("AT+CGCLASS=\"B\"","OK",100)!=RESPOND_OK )              //设置GPRS移动台类别为B,支持包交换和数据交换 
	                    err_log++;
	            }
            }else
                err_log++;

            if( (err_log==0)&&(send_atcmd("AT+CGDCONT?","+CGDCONT:",10)==RESPOND_OK) ){
                atcmd_get_ack_section(n,1);
                atcmd_get_ack_section(stat,2);
                if( (strcmp(n,"\"IP\"")!=0)||(strcmp(stat,"\"CMNET\"")!=0) ){
                    if( send_atcmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",100)!=RESPOND_OK ) //移动,设置PDP上下文,互联网接协议,接入点等信息
	                    err_log++;
                }
            }else
                err_log++;
            //显示配置信息
            if( err_log == 0 ){
	            printf("\r\nGPRS-->配置参数:成功!");
                //显示IP配置参数
                if( send_atcmd("AT+CIPSCONT?","OK",10)==RESPOND_OK ){
                    printf_strbf("[本机IP配置信息]",gprs_rx_buffer);
                }
                led_taskctrl(LED2,FLASH_1HZ,0);
                SET_MD_MODE(MODEM_NORMAL);
	            md_poweron_step = MD_POWER_ON_END;
	        }
            break;
            
        case MD_POWER_ON_ERROR:
            printf("\r\nGPRS-->配置参数:失败!");
            led_taskctrl(LED2,OFF,0);
            SET_MD_STATE_BITS(MD_ERR_POWERON);
            md_poweron_step = MD_POWER_ON_END;
            break;
            
        case MD_POWER_ON_END:
            break;
            
    }
    
    return md_poweron_step;
}


//这个流程里面没有失败可言
int modem_poweroff_flow(void)
{
    switch(md_poweroff_step){
        case MD_POWER_OFF:
            CLR_MD_STATE_BITS((uint16_t)0xffff);     //清除所有标志
            SET_MD_MODE(MODEM_PWROFF);
            if( test_modem()==true ){
                md_poweroff_step = 1;
            }else{
                md_poweroff_step = 3;
                printf("\r\nGPRS-->处于关机状态\r\n");
            }
            break;

        case 1:
            printf("\r\nGPRS-->软关机...\r\n");
            if( send_atcmd("AT+CPOWD=1","NORMAL POWER DOWN",100)==RESPOND_OK ){
                led_taskctrl(LED2,OFF,0);
                md_poweroff_step = MD_POWER_OFF_END;
            }else{
                md_poweroff_step = 2;
            }
            break;

    	case 2:
    	    printf("\r\nGPRS-->硬关机...\r\n");
    	    hw_io_ctrl(GSM_PWR,POWER_OFF);
    	    md_poweroff_step = 3;
    	    break;

    	case 3:
    	    if( GetSysTick_Sec()>task_t_mark+2 ){
    	        led_taskctrl(LED2,OFF,0);
    	        CLR_MD_STATE_BITS(MD_PWR_OK);
    	        md_poweroff_step = MD_POWER_OFF_END;
    	    }
    	    break;

    	case MD_POWER_OFF_END:
    	    
    	    break;
    }
    
    return md_poweroff_step;
}

