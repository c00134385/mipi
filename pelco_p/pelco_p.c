
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

#define PELCO_P_INPUT_BUFFER_SIZE   (32)

static int input_buf_index = 0;
static unsigned char input_buf[PELCO_P_INPUT_BUFFER_SIZE];

static PELCO_P_MSG_QUEUE_t msg_queue;


void pelco_p_input_byte(unsigned char byte)
{   
    printf("%02x ", byte);
    return;
    
    if(PELCO_P_MSG_STX == byte)
    {
        input_buf_index = 0;
    }
    input_buf[input_buf_index++] = byte;

    if(PELCO_P_MSG_LEN == input_buf_index)
    {
        //pelco_print_message((PELCO_P_MSG_t*)input_buf);
        pelco_p_push_message((PELCO_P_MSG_t*)input_buf);
        return;
    }

    input_buf_index %= PELCO_P_INPUT_BUFFER_SIZE;
}

int pelco_p_init(void)
{
    printf("\r\n %s() is called", __FUNCTION__);
    msg_queue.rp = 0;
    msg_queue.wp = 0;
    msg_queue.state = MSG_Q_STATE_EMPTY;
    msg_queue.size = sizeof(msg_queue.msg_list)/sizeof(PELCO_P_MSG_t);
    
    uart3_set_rx_callback(pelco_p_input_byte);    
    return 0;
}

void pelco_p_push_message(PELCO_P_MSG_t* msg)
{
    if(MSG_Q_STATE_FULL == msg_queue.state)
    {
        printf("\r\n L:%d pelco_d msg queue is full.", __LINE__);
        return;
    }
    
    memcpy(msg_queue.msg_list[msg_queue.wp++].content, msg, PELCO_P_MSG_LEN);
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
PELCO_P_MSG_t* pelco_p_pull_message(void)
{
    PELCO_P_MSG_t* msg = NULL;
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

void pelco_p_print_message(PELCO_P_MSG_t* msg)
{
    int i = 0;
    printf("\r\n ");
    for(i = 0; i < PELCO_P_MSG_LEN; i ++)
    {
        printf("%02x ", msg->content[i]);
    }
}

bool pelco_p_check_valid(PELCO_P_MSG_t* msg)
{
    int i = 0;
    uint8 sum = 0;
    if((PELCO_P_MSG_STX !=  msg->content[0]) || (PELCO_P_MSG_ETX !=  msg->content[PELCO_P_MSG_LEN - 2]))
    {
        printf("\r\n msg is invalid.");
        return false;
    }

    for(i = 1; i < PELCO_P_MSG_LEN - 2; i++)
    {
        sum ^= msg->content[i];
    }
    if(msg->content[PELCO_P_MSG_LEN - 1] != sum)
    {
        printf("\r\n msg is invalid.");
        return false;
    }

    return true;
}

void pelco_p_handle(PELCO_P_TYPE_e cmd)
{    
    switch(cmd)
    {
        case PELCO_P_TYPE_STOP:
            printf("\r\n cmd: PELCO_P_TYPE_STOP");
            sonylens_set_zoom_stop();
            sonylens_set_focus_stop();
            break;
        case PELCO_P_TYPE_RIGHT:
            printf("\r\n cmd: PELCO_P_TYPE_RIGHT");
            break;
        case PELCO_P_TYPE_LEFT:
            printf("\r\n cmd: PELCO_P_TYPE_LEFT");
            break;
        case PELCO_P_TYPE_UP:
            printf("\r\n cmd: PELCO_P_TYPE_UP");
            break;
        case PELCO_P_TYPE_DOWN:
            printf("\r\n cmd: PELCO_P_TYPE_DOWN");
            break;
        case PELCO_P_TYPE_ZOOM_TELE:
            printf("\r\n cmd: PELCO_P_TYPE_ZOOM_TELE");
            sonylens_set_zoom_tele();
            break;
        case PELCO_P_TYPE_ZOOM_WIDE:
            printf("\r\n cmd: PELCO_P_TYPE_ZOOM_WIDE");
            sonylens_set_zoom_wide();
            break;
        case PELCO_P_TYPE_FOCUS_FAR:
            printf("\r\n cmd: PELCO_P_TYPE_FOCUS_FAR");
            sonylens_set_focus_far();
            break;
        case PELCO_P_TYPE_FOCUS_NEAR:
            printf("\r\n cmd: PELCO_P_TYPE_FOCUS_NEAR");
            sonylens_set_focus_near();
            break;
        case PELCO_P_TYPE_IRIS_OPEN:
            printf("\r\n cmd: PELCO_P_TYPE_IRIS_OPEN");
            break;
        case PELCO_P_TYPE_IRIS_CLOSE:
            printf("\r\n cmd: PELCO_P_TYPE_IRIS_CLOSE");
            break;
        case PELCO_P_TYPE_CAMERA_ON:
            printf("\r\n cmd: PELCO_P_TYPE_CAMERA_ON");
            break;
        case PELCO_P_TYPE_CAMERA_OFF:
            printf("\r\n cmd: PELCO_P_TYPE_CAMERA_OFF");
            break;
        case PELCO_P_TYPE_SCAN_AUTO:
            printf("\r\n cmd: PELCO_P_TYPE_SCAN_AUTO");
            break;
        case PELCO_P_TYPE_SCAN_MANUAL:
            printf("\r\n cmd: PELCO_P_TYPE_SCAN_MANUAL");
            break;
        default:
            printf("\r\n cmd: PELCO_P_TYPE_UNKNOWN");
            break;
    }
    
}

void pelco_p_parse_message(PELCO_P_MSG_t* msg)
{
    int i = 0;
    uint16 comm = 0;
    uint16 data = 0;

    comm = (((uint16)msg->content[2]) << 8) | (msg->content[3]);
    data = (((uint16)msg->content[4]) << 8) | (msg->content[5]);

    if((0 == comm) && (0 == data))
    {
        pelco_p_handle(PELCO_P_TYPE_STOP);
        return;
    }
    
    for(i = 1; i < 16; i++)
    {
        uint8 bit_val = (comm >> i) & 0x1;
        if(0 == bit_val)
        {
            continue;
        }
        else
        {
            if(i <= 10 )
            {
                pelco_p_handle((PELCO_P_TYPE_e)i);
            }
        }
    }
}

int pelco_p_process(void)
{
    PELCO_P_MSG_t* msg = NULL;
    uint8 address;
    msg = pelco_p_pull_message();
    if(NULL != msg)
    {
        pelco_p_print_message(msg);
        if(pelco_p_check_valid(msg))
        {
            address = msg->content[1];
            pelco_p_parse_message(msg);
        }
    }
    
    return 0;
}



