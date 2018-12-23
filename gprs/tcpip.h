#ifndef __TCPIP_H_
#define __TCPIP_H_


typedef struct{
    char Active;
	char LinkId[2];
    char IPv4_addr[18];     //
    char HostName[30];
    char port[6];
    char procotl[6];
    //HTTP���
    char *http_bf;
    char *http_header;
    char *http_data;
}SOCKET;




//TCPIPӦ��
typedef enum{
    SOCKET_INIT,            //��ʼ���ͼ��SOCKET����������
    SOCKET_OPEN,            //��SOCKET
    SOCKET_OPEN_OK,         //��SOCKET OK
    SOCKET_LINKED,          //����������
    SOCKET_SEND,            //��������
    SOCKET_SEND_OK,         //��������OK
    //
    SOCKET_OPEN_FAIL=0x10,  //����SOCKETʧ��
    SOCKET_SEND_FAIL,       //��������ʧ��
    SOCKET_ERROR,           //SOCKETʧ��,��������Ϊ��������
}WINSOCKET_STATUS;


WINSOCKET_STATUS WinSocketInit(SOCKET *s);
WINSOCKET_STATUS WinSocketSend(void *bf, int len);
WINSOCKET_STATUS WinSocketGetState(void);
WINSOCKET_STATUS WinSocketClrState(SOCKET *s);

#endif
