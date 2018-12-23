
/*----------------------------------------------------------------------------*/
/* sonylens.c                                                                   */
/* 描述:sony 变倍机型通讯程序                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/* 2017/10/17 yihuagang建立 V0.1                                                  */
/* 2017/10/17   */
/* 2017/10/17 yihuagang  v0.3                                                      */
/*            */
/*  */
/*                     */
/*                               */
/*   */
/*                      */
/*                                                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/


/* include */
#include "includes.h"

static VISCA_MSG_QUEUE_t msg_queue;

void sony_visca_input_byte(unsigned char byte)
{   
    printf("%02x ", byte);
    visca_fill_message(byte);
}

int sony_visca_init(void)
{
    printf("\r\n %s() is called...", __FUNCTION__);
    msg_queue.rp = 0;
    msg_queue.wp = 0;
    msg_queue.state = MSG_Q_STATE_EMPTY;
    msg_queue.size = sizeof(msg_queue.msg_list)/sizeof(VISCA_MSG_t);
    
    uart3_set_rx_callback(sony_visca_input_byte);    
}

void visca_fill_message(unsigned char byte)
{
    VISCA_MSG_t* msg = NULL;
    
    if(MSG_Q_STATE_FULL == msg_queue.state)
    {
        printf("\r\n L:%d msg queue is full.", __LINE__);
        return;
    }
    
    msg = &msg_queue.msg_list[msg_queue.wp];
    if(VISCA_MSG_STX == (byte & 0xF0))
    {
        msg->wp = 0;
    }
    

    msg->content[msg->wp++] = byte;

    if((VISCA_MSG_ETX == byte) || (msg->wp >= VISCA_MSG_BUFF_LEN))
    {
        msg_queue.wp++;
        msg_queue.wp %= msg_queue.size;
        if(msg_queue.wp == msg_queue.rp)
        {
            msg_queue.state = MSG_Q_STATE_FULL;
        }
        else
        {
            msg_queue.state = MSG_Q_STATE_NORMAL;
        }
    }
}
VISCA_MSG_t* visca_pull_message(void)
{
    VISCA_MSG_t* msg = NULL;
    if(MSG_Q_STATE_EMPTY == msg_queue.state)
    {
        return msg;
    }

    msg = &msg_queue.msg_list[msg_queue.rp++];
    msg_queue.rp %= msg_queue.size;
    if(msg_queue.wp == msg_queue.rp)
    {
        msg_queue.state = MSG_Q_STATE_EMPTY;
    }
    else
    {
        msg_queue.state = MSG_Q_STATE_NORMAL;
    }
    return msg;
}

void visca_print_message(VISCA_MSG_t* msg)
{
    int i = 0;
    printf("\r\n len:%d \r\n", msg->wp);
    for(i = 0; i < msg->wp; i ++)
    {
        printf("%02x ", msg->content[i]);
    }
}

void visca_parse_message(VISCA_MSG_t* msg)
{
    int i = 0;
    uint8 address;
    uint8 comm = 0;
    uint8 data = 0;

    address = msg->content[0] & 0x0F;
    if((sonylens_address_get() != address) && (VISCA_MSG_BROADCAST != address))
    {
        printf("\r\n address is not match.");
        return;
    }

    comm = msg->content[3];
    data = msg->content[4];

    if((0x07 == comm) && ((0x2 == data) || (0x20 == (data & 0xF0))))
    {
        sonylens_set_zoom_tele();
    }
    else if((0x07 == comm) && ((0x3 == data) || (0x30 == (data & 0xF0))))
    {
        sonylens_set_zoom_wide();
    }
    else if((0x07 == comm) && (0x0 == data))
    {
        sonylens_set_zoom_stop();
    }
    else if((0x08 == comm) && (0x2 == data))
    {
        sonylens_set_focus_far();
    }
    else if((0x08 == comm) && (0x3 == data))
    {
        sonylens_set_focus_near();
    }
    else if((0x08 == comm) && (0x0 == data))
    {
        sonylens_set_focus_stop();
    }
    else
    {
        printf("\r\n unknown message.");
    }
}

int visca_process(void)
{
    VISCA_MSG_t* msg = NULL;
    msg = visca_pull_message();
    if(NULL != msg)
    {
        visca_print_message(msg);
        visca_parse_message(msg);
        msg->wp = 0;
    }
    
    return 0;
}


