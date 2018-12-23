/*----------------------------------------------------------------------------*/
/* sim800.c                                                                   */
/* ����:gprsͨѶ����                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* 2014/6/7 manwjh ���� V0.1                                                  */
/* 2014/7/12 manwjh v0.2  modem��ʼ����ɺ�,ֻ��ȡ״̬�;��쳣������ת.       */
/* 2014/7/22 manwjh v0.3                                                      */
/*  ȥ����ѹ��ⲿ�ִ���,MODEM���ֵ�ѹ���˱�����Լ�����power off.            */
/*  ϵͳ����test_modem(),��AT�������MODEM�Ƿ�����,��Ӧ��˵��MODEM����POWEROFF*/
/*  ����MODEM�͹���ģʽ                                                       */
/* 2014/8/6 manwjh v0.4  ����MODEM�ܹ�                                        */
/* 2014/8/13 manwjh v0.5 ������Դ������ɵ�Ӱ��,�޸�����BUG.����AT+CBC����    */
/*                       �������ź�����AT+CGAT(GPRS����)�����ı���.           */
/*                                                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* SIM800L ˵�� */
/*  1. ��ѹ��Χ: 3.4V~4.4V, ͻ��ģʽʱ��ֵ����2A. 
       VBAT�ĵ��䲻Ҫ����3.0V,����ģ����ܻ�ػ�.
       VBAT��ѹ<=3.5V,ģ���Զ�����"UNDER-VOLTAGE WARNNING, <3.4V ģ�������Զ��ػ�"UNDER-VOLTAGE POWER DOWN"
    2. ��Ƶ: GSM850, EGSM900, DCS1800, PCS1900. ����GSM Phase 2/2+
    3. GPRSʱ϶ȱʡΪ�ȼ�12, ʱ϶ class1~12��ѡ, �ƶ�̨�ȼ�B
*/



/* include */
#include "includes.h"


/* external */
extern char gprs_rx_buffer[],gprs_tx_buffer[];

/* public */
uint16_t gModemState;
float gModemVoltage;

/* private */
static unsigned long task_t_mark;   //�������
static uint16_t gprs_taskstate;          //gprs_task״̬��
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
    WINSOCKET_STATUS state;         //socket����״̬
    int send_flg;                   //��������һ�����ݰ���־λ
    unsigned char *dt;              //���ݻ����ַ
    int dt_len;                     //���ݻ��峤��
    unsigned long send_t_start;     //socket�������ݿ�ʼʱ��
    unsigned long send_time;        //socket����һ�����ݰ�����ʱ��
    int send_err_cnt;               //socket�������ݰ������������
	long send_ok_cnt;                //socket���ͳɹ�����
}winsocket;

char winsocket_server_open_cmd[100];    //ģ��TCP�����

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
    CIP_INITIAL,        //��ʼ��
    CIP_START,          //��������
    CIP_CONFIG,         //���ó���
    CIP_GPRSACT,        //�����Ѽ���
    CIP_IPSTATUS,       //��ñ���IP��ַ
    CIP_CONNECTING,     //TCP������
    CIP_CONNECTED,      //���ӽ����ɹ�
    CIP_CLOSING,       //���ڹر�TCP����
    CIP_CLOSED,         //TCP���ӶϿ�
    CIP_PDP_DEACT,      //�������ͷ�
    CIP_UNKNOW,         //δ֪״̬
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

#if 1   //��������,���Դ���323 BYTESò�����ݾͻᱻ�е�. �������ĳƴ���384�ᱻ�ְ���
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
		printf("\r\nGPRS-->��������ʧ��!");
		rslt = SOCKET_SEND_FAIL;
	}
    winsocket.dt_len = 0;

    return rslt;
}
#else	//�������, Yeelink��֧�ְַ�
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
        printf("\r\nGPRS-->��������:%s,��������������:%s\r\n",txlen,acklen);
        if( DecStr2Int(txlen,4)==DecStr2Int(acklen,4) )
            rslt = 1;
        
    }else{
		printf_strbf("[���ݷ���״̬��ѯ]",gprs_rx_buffer);
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

//�ⲿ��������
void gprs_taskctrl(int cmd)
{
    task_t_mark = GetSysTick_Sec();    
    switch(cmd){
        case MODEM_IDEL:
        case MODEM_POWER_ON:
            //���MODEM������ȫOK״̬,��ϵͳ���ظ�����
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
            send_atcmd("AT+CIPSHUT","SHUT OK",100);     //�رպ�״̬ΪIP INITIAL
            gprs_taskstate = cmd;
            break;

        case MODEM_HW_ERROR:
            printf("\r\nGPRS-->Ӳ������\r\n");
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
    //SIM���Ȳ�μ��,��SIM��ֱ�ӹر�MODEM
    if( GetModemState(MD_SIM_IN)==MD_SIM_IN ){
        if( check_sim_card()==0 )
            gprs_taskctrl(MODEM_POWER_OFF);
    }else
        check_sim_card();
    
    //gprs task main loop
    switch(gprs_taskstate)
    {
        case MODEM_IDEL:        //һ���յ�״̬
            task_t_mark = GetSysTick_Sec();
            break;

        /*-------xxxxxxxx--------��MODEM-----xxxxxxxx----------*/
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

        /*-------xxxxxxxx--------�ر�MODEM-----xxxxxxxx----------*/
        case MODEM_POWER_OFF:
            winsocket.send_err_cnt = 0;
    		winsocket.send_flg = 0;
            //
            modem_poweroff_flow();
            break;

        /*-------xxxxxxxx--------��������-----xxxxxxxx----------*/
        case WINSOCKET_SEND:
            //WINSOCKET��ʼ��
            if( winsocket.send_flg ){
                CLR_MD_STATE_BITS(MD_IPNET_IDEL);
                winsocket.send_t_start = GetSysTick_Sec();
				winsocket.state = SOCKET_OPEN;
				winsocket.send_flg = 0;
                led_taskctrl(LED2,ON,0);     //hw_io_ctrl(LED2,ON);
				printf("\r\nGPRS-->Winscoket Connecting");
				send_atcmd(winsocket_server_open_cmd,"ERROR",100);
            }

            //�������ӳ�ʱ
            if( GetSysTick_Sec()>winsocket.send_t_start+10 ){
                printf("\r\nGPRS-->�������ӳ�ʱ!\r\n");
                winsocket.state = SOCKET_SEND_FAIL;
                gprs_taskctrl( WINSOCKET_ERROR );
                break;
            }
            
            //WINSOCKET����
            if( winsocket.state==SOCKET_OPEN ){
                if( socket_open()==SOCKET_LINKED )
                    winsocket.state = SOCKET_SEND;
                else
                    break;
            }
            //WINSOCKET����
            if( winsocket.state==SOCKET_SEND ){
                if( socket_send()==SOCKET_SEND_FAIL ){
                    printf("\r\nGPRS-->��������ʧ��!\r\n");
					gprs_taskctrl( WINSOCKET_ERROR );
    			}
    			else{
    			    gprs_taskctrl( WINSOCKET_SEND_QUERY );
                	task_t_mark = GetSysTick_Sec();
				}
    		}
            break;

        case WINSOCKET_SEND_QUERY:
            //���ͳ�ʱ
            if( GetSysTick_Sec()>winsocket.send_t_start+20 ){
                printf("\r\nGPRS-->���ݽ�����Ӧ��ʱ!\r\n");
                winsocket.state = SOCKET_SEND_FAIL;
                gprs_taskctrl( WINSOCKET_ERROR );
                break;
            }
            //
            if( uart3_get_reclen() ){
                if( atcmd_ack_is(gprs_rx_buffer,"SEND OK") ){
					printf("\r\nGPRS-->SEND OK");
                    socket_query();
                    if( winsocket.dt_len>0 ){           //���δ���������������ʣ������
                        winsocket.state = SOCKET_SEND;
                        gprs_taskctrl( WINSOCKET_SEND );
                    }else{
                        winsocket.state = SOCKET_SEND_OK;
                        gprs_taskctrl( WINSOCKET_FINISHED );    //������ɳ���
                    }
                }else{
                    printf("\r\nGPRS-->δȷ���������Ƿ���յ�!");
                    winsocket.state = SOCKET_SEND_FAIL;
                    gprs_taskctrl( WINSOCKET_ERROR );           //���ʹ������
                }
            }
            break;
        //WINSOCKET �������
        case WINSOCKET_ERROR:
            SET_MD_STATE_BITS(MD_IPNET_IDEL);
            send_atcmd("AT+CIPCLOSE","CLOSE OK",10);
            //
            winsocket.send_time = GetSysTick_Sec()-winsocket.send_t_start;
            printf("\r\nGPRS-->Winsocket ERROR! time:%d%s",winsocket.send_time,"\r\n");
            led_taskctrl(LED2,FLASH_1HZ,0);     //hw_io_ctrl(LED2,OFF);
            //
            if( winsocket.send_err_cnt++>3 ){       //�����������ʧ��,����ڷ�������󾡿�������ӡ�
				winsocket.send_err_cnt = 0;
                gprs_taskctrl(MODEM_RETRY);
            }else
                gprs_taskctrl(MODEM_CHECK);
            break;

        //WINSOCKET �ɹ�����
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

        /*-------xxxxxxxx--------���MODEM״̬-----xxxxxxxx----------*/
        case MODEM_CHECK:
            //״̬���,3����һ��
            if( GetSysTick_Sec()>task_t_mark+1 ){
                task_t_mark = GetSysTick_Sec();
                //MODEM������Ӧ����
                if( test_modem()==0 ){
                    printf("\r\nGPRS-->MODEM���ʧ��\r\n");
                    CLR_MD_STATE_BITS(MD_CSQ_OK|MD_GSM_OK|MD_GPRS_OK);
                    gprs_taskctrl(MODEM_HW_ERROR);
                }else{
                    //�ź�����,GSM,GPRS���
                    get_CSQ();
                    get_gsm_status();
                    get_gprs_status();
                    check_ipnet_status();
				}
			}
            break;
            
        case MODEM_RETRY:
            if( test_modem()==0 ){
                printf("\r\nGPRS-->MODEM���ʧ��\r\n");
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

        /*-------xxxxxxxx--------MODEM ����״̬-----xxxxxxxx----------*/
        //�������ͣ�MODEMû����,SIM��û��,��������Ӧ,�����޷�����
        //#define MD_ERR_POWERON          (UINT)(1<<8)        //MODEMû����
        //#define MD_ERR_NO_ACK           (UINT)(1<<10)       //ϵͳ����Ӧ��ʧ��
        //#define MD_ERR_SIM              (UINT)(1<<12)       //��SIM��,�����
        //#define MD_ERR_CONFIG           (UINT)(1<<13)       //�����޷�����
        case MODEM_HW_ERROR:
            if( GetSysTick_Sec()>task_t_mark+1 ){
                task_t_mark = GetSysTick_Sec();
                //
                printf("\r\nGPRS-->MODEM����:%4x\r\n",gModemState);
                if( gModemState&MD_ERR_POWERON ){
                    printf("    -->û��(�޷�)����");
                }
                if( gModemState&MD_ERR_NO_ACK ){
                    printf("    -->������������Ӧ");
                }
                if( gModemState&MD_ERR_SIM ){
                    printf("    -->��SIM��");
                }
                if( gModemState&MD_ERR_NET_NULL ){
                    printf("    -->���Ӳ�������");
                }
                if( gModemState&MD_ERR_VBAT ){
                    printf("    -->MODEM��ѹ�쳣");
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
            printf("\r\nGPRS-->AT CMD��Ӧ��ʱ!");

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

//�������յ�������,�Ƿ��з���Ԥ�ڵ����ݴ���.
char *atcmd_ack_is(char *bf, const char *hope_str)
{
    char *pos=NULL;
    
    pos = strstr((const char*)bf, hope_str);
    
    return pos;
}


//�ȴ�MODEM��������
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
    
    //����:
    do{
        if( *in=='\0' )
            return 0;
    }while( *in++!=':' );
    
    //������һ�����ǿո���ַ�
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
    
    if( send_atcmd("AT+CIPSTATUS","OK",10)==RESPOND_OK ){    //��ѯ����״̬
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

const char gsm_stat_1[]="ע�ᵽ��������";
const char gsm_stat_5[]="ע�ᵽ��������";
const char gsm_stat_2[]="����Ѱ����Ӫ��...";
const char gsm_stat_0[]="û��ע��/ע�ᱻ��/δ֪";

int get_gsm_status(void)
{
    char n[8],stat[12];
        
    //
    if( send_atcmd("AT+CREG?","OK",10)==RESPOND_OK ){   //��ȡ����ע����Ϣ
        //����+CREG:0,1��+CREG:0,5ʱ��˵��ģ��ע�ᵽGPRS����
        atcmd_get_ack_section(n,0);
        atcmd_get_ack_section(stat,1);
#if 0
        printf("\r\nGPRS-->GSM����ע��:");
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
        
    //��ʾѡ�����Ӫ����Ϣ
#if 0
    if( gModemState&MD_GSM_OK ){
        if( send_atcmd("AT+COPS?","+COPS",10)==RESPOND_OK ){
            atcmd_get_ack_section(stat,2);
            printf("\r\nGPRS-->��Ӫ��:%s",stat);
        }
    }
#endif
    //    
    return (gModemState&MD_GSM_OK);
}


int get_gprs_status(void)
{
    char n[8],stat[8];
    
    if( send_atcmd("AT+CGREG?","OK",10)==RESPOND_OK ){   //��ȡGPRS����ע����Ϣ
        //����+CGREG:0,1��+CGREG:0,5ʱ��˵��ģ��ע�ᵽGPRS����
        atcmd_get_ack_section(n,0);
        atcmd_get_ack_section(stat,1);
#if 0
        printf("\r\nGPRS-->GPRS����ע��:");
        switch(stat[0]){
            case '0':   printf("δע��");       break;
            case '1':   printf("��ע��,����");  break;
            case '2':   printf("��������...");  break;
            case '3':   printf("ע�ᱻ�ܾ�");   break;
            case '5':   printf("��ע�ᣬ����"); break;
            default:    printf("δ֪ԭ��");     break;
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

//�ź�ǿ����15~35֮��ȽϺ�
int get_CSQ(void)
{
    char n[8];
	int tmp;
    
    if( send_atcmd("AT+CSQ","+CSQ",10)==RESPOND_OK ){
        atcmd_get_ack_section(n,0);
#if 0
		printf("\r\nGPRS-->�ź�ǿ��:%s\r\n",n);
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
        case MD_POWER_ON:   //�������
            led_taskctrl(LED2,FLASH_5HZ,0);
            printf("\r\nGPRS-->����...\r\n");
            CLR_MD_STATE_BITS((uint16_t)0xffff);     //������б�־
            SET_MD_MODE(MODEM_PWRON);   //MODEM����
            md_poweron_step = 1;
            break;

        case 1:     //Ӳ������
	        if( g_BatteryVoltage>V_OPEN_GPRS ){
	            if( test_modem()==0 ){
	                hw_io_ctrl(GSM_PWR,POWER_ON);
	                pwronoff_t_mark = GetSysTick_Sec();
	                md_poweron_step = 2;
                }else{
                    md_poweron_step = 3;
                    printf("\r\nGPRS-->���ڿ���״̬");
            	}
        	}else{
        	    SET_MD_STATE_BITS(MD_ERR_VBAT);
        	    md_poweron_step = MD_POWER_ON_ERROR;
            	printf("\r\nGPRS-->�����ѹ����,�޷�����");
        	}
        	break;
        case 2:  //delay 2~3��
            if( GetSysTick_Sec()>pwronoff_t_mark+3 )
                md_poweron_step = 3;
            else
                printf(".");
            
        case 3:  //�ȴ��������
			if( test_modem() ){
                SET_MD_STATE_BITS(MD_PWR_OK);
                md_poweron_step = 4;
			}
            //
            if( GetSysTick_Sec()>task_t_mark+8 )
                md_poweron_step = MD_POWER_ON_ERROR;
            break;

        case 4: //�����ɹ�?
		    if( test_modem() ){
                //����ģ��Ӳ������
                send_atcmd("ATE0","OK",10);                 //�رջ���
    			//send_atcmd("AT+IPR=57600","OK",100);		//�趨������
    			//��ȡģ��汾
                if( send_atcmd("ATI","OK",10)==RESPOND_OK ){
                    printf_strbf("[ģ��汾]",gprs_rx_buffer);
                    md_poweron_step = 5;
                }
            }
            //
            if( GetSysTick_Sec()>task_t_mark+10 )
                md_poweron_step = MD_POWER_ON_ERROR;
            break;

        case 5: //����MODEM����
            //��ʱ�Լ�����������Ӧ���
            if( GetSysTick_Sec()>task_t_mark+60 ){
                md_poweron_step = MD_POWER_ON_ERROR;
	        }else if( test_modem()==0 ){
                md_poweron_step = MD_POWER_ON_ERROR;
	        }
            //��⿪������״̬
            get_CSQ();
            get_gsm_status();
            if( GetModemState(MD_GSM_OK|MD_CSQ_OK)!=(MD_GSM_OK|MD_CSQ_OK) )
                break;
            //
            send_atcmd("AT+CIPSHUT","SHUT OK",100);
            err_log=0;
            //���GPRS����״̬
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
            //����GPRS�ƶ�̨���
            if( (err_log==0)&&(send_atcmd("AT+CGCLASS?","+CGCLASS:",10)==RESPOND_OK) ){
                atcmd_get_ack_section(n,0);
                if( strcmp(n,"\"B\"")!=0 ){
                    if( send_atcmd("AT+CGCLASS=\"B\"","OK",100)!=RESPOND_OK )              //����GPRS�ƶ�̨���ΪB,֧�ְ����������ݽ��� 
	                    err_log++;
	            }
            }else
                err_log++;

            if( (err_log==0)&&(send_atcmd("AT+CGDCONT?","+CGDCONT:",10)==RESPOND_OK) ){
                atcmd_get_ack_section(n,1);
                atcmd_get_ack_section(stat,2);
                if( (strcmp(n,"\"IP\"")!=0)||(strcmp(stat,"\"CMNET\"")!=0) ){
                    if( send_atcmd("AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",100)!=RESPOND_OK ) //�ƶ�,����PDP������,��������Э��,��������Ϣ
	                    err_log++;
                }
            }else
                err_log++;
            //��ʾ������Ϣ
            if( err_log == 0 ){
	            printf("\r\nGPRS-->���ò���:�ɹ�!");
                //��ʾIP���ò���
                if( send_atcmd("AT+CIPSCONT?","OK",10)==RESPOND_OK ){
                    printf_strbf("[����IP������Ϣ]",gprs_rx_buffer);
                }
                led_taskctrl(LED2,FLASH_1HZ,0);
                SET_MD_MODE(MODEM_NORMAL);
	            md_poweron_step = MD_POWER_ON_END;
	        }
            break;
            
        case MD_POWER_ON_ERROR:
            printf("\r\nGPRS-->���ò���:ʧ��!");
            led_taskctrl(LED2,OFF,0);
            SET_MD_STATE_BITS(MD_ERR_POWERON);
            md_poweron_step = MD_POWER_ON_END;
            break;
            
        case MD_POWER_ON_END:
            break;
            
    }
    
    return md_poweron_step;
}


//�����������û��ʧ�ܿ���
int modem_poweroff_flow(void)
{
    switch(md_poweroff_step){
        case MD_POWER_OFF:
            CLR_MD_STATE_BITS((uint16_t)0xffff);     //������б�־
            SET_MD_MODE(MODEM_PWROFF);
            if( test_modem()==true ){
                md_poweroff_step = 1;
            }else{
                md_poweroff_step = 3;
                printf("\r\nGPRS-->���ڹػ�״̬\r\n");
            }
            break;

        case 1:
            printf("\r\nGPRS-->��ػ�...\r\n");
            if( send_atcmd("AT+CPOWD=1","NORMAL POWER DOWN",100)==RESPOND_OK ){
                led_taskctrl(LED2,OFF,0);
                md_poweroff_step = MD_POWER_OFF_END;
            }else{
                md_poweroff_step = 2;
            }
            break;

    	case 2:
    	    printf("\r\nGPRS-->Ӳ�ػ�...\r\n");
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

