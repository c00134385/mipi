#ifndef __TCPIP_H_
#define __TCPIP_H_


typedef struct{
    char Active;
	char LinkId[2];
    char IPv4_addr[18];     //
    char HostName[30];
    char port[6];
    char procotl[6];
    //HTTP相关
    char *http_bf;
    char *http_header;
    char *http_data;
}SOCKET;




//TCPIP应用
typedef enum{
    SOCKET_INIT,            //初始化和检查SOCKET建立的请求
    SOCKET_OPEN,            //打开SOCKET
    SOCKET_OPEN_OK,         //打开SOCKET OK
    SOCKET_LINKED,          //建立连接中
    SOCKET_SEND,            //发送数据
    SOCKET_SEND_OK,         //发送数据OK
    //
    SOCKET_OPEN_FAIL=0x10,  //建立SOCKET失败
    SOCKET_SEND_FAIL,       //发送数据失败
    SOCKET_ERROR,           //SOCKET失败,可能是因为网络问题
}WINSOCKET_STATUS;


WINSOCKET_STATUS WinSocketInit(SOCKET *s);
WINSOCKET_STATUS WinSocketSend(void *bf, int len);
WINSOCKET_STATUS WinSocketGetState(void);
WINSOCKET_STATUS WinSocketClrState(SOCKET *s);

#endif
