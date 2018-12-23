
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

#define PELCO_D_INPUT_BUFFER_SIZE   (32)

static int input_buf_index = 0;
static unsigned char input_buf[PELCO_D_INPUT_BUFFER_SIZE];

static PELCO_D_MSG_QUEUE_t msg_queue;


void pelco_d_input_byte(unsigned char byte)
{
    //printf("\r\n byte:%02x ", byte);
    pelco_fill_message(byte);
}

int pelco_d_init(void)
{
    printf("\r\n %s() is called", __FUNCTION__);
    msg_queue.rp = 0;
    msg_queue.wp = 0;
    msg_queue.state = MSG_Q_STATE_EMPTY;
    msg_queue.size = sizeof(msg_queue.msg_list)/sizeof(PELCO_D_MSG_t);
    
    uart3_set_rx_callback(pelco_d_input_byte);    
    return 0;
}

void pelco_fill_message(unsigned char byte)
{
    PELCO_D_MSG_t* msg = NULL;
    
    if(MSG_Q_STATE_FULL == msg_queue.state)
    {
        printf("\r\n L:%d pelco msg queue is full.", __LINE__);
        return;
    }
    
    msg = &msg_queue.msg_list[msg_queue.wp];
    if(PELCO_D_MSG_STX == byte)
    {
        msg->type = PELCO_D;
        msg->wp = 0;
    }
    else if(PELCO_P_MSG_STX == byte)
    {
        msg->type = PELCO_P;
        msg->wp = 0;
    }

    msg->content[msg->wp++] = byte;

    if(((PELCO_D == msg->type) && (msg->wp == PELCO_D_MSG_LEN))
        || ((PELCO_P == msg->type) && (msg->wp == PELCO_P_MSG_LEN))
        || (msg->wp >= PELCO_P_MSG_BUFF_LEN))
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

PELCO_D_MSG_t* pelco_d_pull_message(void)
{
    PELCO_D_MSG_t* msg = NULL;
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

void pelco_print_message(PELCO_D_MSG_t* msg)
{
    int i = 0;
    printf("\r\n type:%s len:%d \r\n", msg->type?"PELCO_P":"PELCO_D", msg->wp);
    for(i = 0; i < msg->wp; i ++)
    {
        printf("%02x ", msg->content[i]);
    }
}

bool pelco_d_check_valid(PELCO_D_MSG_t* msg)
{
    int i = 0;
    
    if(PELCO_D == msg->type)
    {
        uint32 sum = 0;
        for(i = 1; i < PELCO_D_MSG_LEN - 1; i++)
        {
            sum += msg->content[i];
        }
        if(msg->content[PELCO_D_MSG_LEN - 1] != (sum%0x100))
        {
            printf("\r\n d msg is invalid.");
            return false;
        }
    }
    else if(PELCO_P == msg->type)
    {
        uint8 checkcode = 0;
        //for(i = 1; i < PELCO_P_MSG_LEN - 2; i++)
        for(i = 0; i < PELCO_P_MSG_LEN - 2; i++)
        {
            checkcode ^= msg->content[i];
        }

        if(msg->content[PELCO_P_MSG_LEN - 1] != checkcode)
        {
            printf("\r\n p msg is invalid.");
            return false;
        }
    }
    else
    {
        return false;

    }

    return true;
    
}

void pelco_d_handle(PELCO_D_TYPE_e cmd)
{    
    switch(cmd)
    {
        case PELCO_D_TYPE_STOP:
            printf("\r\n cmd: PELCO_D_TYPE_STOP");
            sonylens_set_zoom_stop();
            sonylens_set_focus_stop();
            break;
        case PELCO_D_TYPE_RIGHT:
            printf("\r\n cmd: PELCO_D_TYPE_RIGHT");
            break;
        case PELCO_D_TYPE_LEFT:
            printf("\r\n cmd: PELCO_D_TYPE_LEFT");
            break;
        case PELCO_D_TYPE_UP:
            printf("\r\n cmd: PELCO_D_TYPE_UP");
            break;
        case PELCO_D_TYPE_DOWN:
            printf("\r\n cmd: PELCO_D_TYPE_DOWN");
            break;
        case PELCO_D_TYPE_ZOOM_TELE:
            printf("\r\n cmd: PELCO_D_TYPE_ZOOM_TELE");
            if(sonylens_remote_is_menu_on())
            {
                sonylens_remote_navi_left();
            }
            else
            {
                sonylens_set_zoom_tele();
            }
            break;
        case PELCO_D_TYPE_ZOOM_WIDE:
            printf("\r\n cmd: PELCO_D_TYPE_ZOOM_WIDE");
            if(sonylens_remote_is_menu_on())
            {
                sonylens_remote_navi_right();
            }
            else
            {
                sonylens_set_zoom_wide();
            }
            break;
        case PELCO_D_TYPE_FOCUS_FAR:
            printf("\r\n cmd: PELCO_D_TYPE_FOCUS_FAR");
            if(sonylens_remote_is_menu_on())
            {
                sonylens_remote_navi_up();
            }
            else
            {
                sonylens_set_focus_far();
            }
            break;
        case PELCO_D_TYPE_FOCUS_NEAR:
            printf("\r\n cmd: PELCO_D_TYPE_FOCUS_NEAR");
            if(sonylens_remote_is_menu_on())
            {
                sonylens_remote_navi_down();
            }
            else
            {
                sonylens_set_focus_near();
            }
            break;
        case PELCO_D_TYPE_IRIS_OPEN:
            printf("\r\n cmd: PELCO_D_TYPE_IRIS_OPEN");
            break;
        case PELCO_D_TYPE_IRIS_CLOSE:
            printf("\r\n cmd: PELCO_D_TYPE_IRIS_CLOSE");
            break;
        case PELCO_D_TYPE_CAMERA_ON:
            printf("\r\n cmd: PELCO_D_TYPE_CAMERA_ON");
            break;
        case PELCO_D_TYPE_CAMERA_OFF:
            printf("\r\n cmd: PELCO_D_TYPE_CAMERA_OFF");
            break;
        case PELCO_D_TYPE_SCAN_AUTO:
            printf("\r\n cmd: PELCO_D_TYPE_SCAN_AUTO");
            break;
        case PELCO_D_TYPE_SCAN_MANUAL:
            printf("\r\n cmd: PELCO_D_TYPE_SCAN_MANUAL");
            break;
        case PELCO_D_TYPE_NAVI_C:
            sonylens_remote_navi_ok();
            break;
        default:
            printf("\r\n cmd: PELCO_D_TYPE_UNKNOWN");
            break;
    }
    
}

void pelco_handle_cmd(PELCO_CMD_e cmd) {
    switch(cmd)
    {
        case PELCO_CMD_F1:
            sonylens_control_f1();
            break;
        case PELCO_CMD_F2:
            sonylens_control_f2();
            break;
        case PELCO_CMD_F3:
            sonylens_control_f3();
            break;
        case PELCO_CMD_F4:
            sonylens_control_f4();
            break;
        case PELCO_CMD_F5:
            sonylens_control_f5();
            break;
        case PELCO_CMD_F6:
            sonylens_control_f6();
            break;
        case PELCO_CMD_F7:
            sonylens_control_f7();
            break;
        case PELCO_CMD_F8:
            sonylens_control_f8();
            break;
        case PELCO_CMD_F9:
            sonylens_control_f9();
            break;
        case PELCO_CMD_F10:
            sonylens_control_f10();
            break;
        case PELCO_CMD_F11:
            sonylens_control_f11();
            break;
        case PELCO_CMD_F12:
            sonylens_control_f12();
            break;
        default:
            break;
    }
}

void pelco_d_parse_message(PELCO_D_MSG_t* msg)
{
    int i = 0;
    uint8 address;
    uint16 comm = 0;
    uint16 data = 0;

    address = msg->content[1];
    if(sonylens_address_get() != address)
    {
        printf("\r\n address is not match.");
        return;
    }
    
    comm = (((uint16)msg->content[2]) << 8) | (msg->content[3]);
    data = (((uint16)msg->content[4]) << 8) | (msg->content[5]);

    if((0 == comm) && (0 == data))
    {
        pelco_d_handle(PELCO_D_TYPE_STOP);
        return;
    }
    
    for(i = 0; i <= 8; i++)
    {
        uint8 bit_val = (comm >> i) & 0x1;
        if(0 == bit_val)
        {
            continue;
        }
        else
        {
            #if 0
            if(11 == i)
            {
                if(1 == (comm >> 15))
                {
                    pelco_d_handle(PELCO_D_TYPE_CAMERA_ON);
                }
                else
                {
                    pelco_d_handle(PELCO_D_TYPE_CAMERA_OFF);
                }
            }
            else if(12 == i)
            {
                if(1 == (comm >> 15))
                {
                    pelco_d_handle(PELCO_D_TYPE_SCAN_AUTO);
                }
                else
                {
                    pelco_d_handle(PELCO_D_TYPE_SCAN_MANUAL);
                }
            }
            else if(13 == i)
            {
                pelco_d_handle(PELCO_D_TYPE_NAVI_C);
            }
            else
            {
                pelco_d_handle((PELCO_D_TYPE_e)i);
            }
            #endif
            pelco_d_handle((PELCO_D_TYPE_e)i);
        }
    }

    pelco_handle_cmd((PELCO_CMD_e)(comm >> 10));
    
}

void pelco_p_parse_message(PELCO_D_MSG_t* msg)
{
    int i = 0;
    uint8 address;
    uint16 comm = 0;
    uint16 data = 0;

    address = msg->content[1];
    if(sonylens_address_get() != address)
    {
        printf("\r\n address is not match.");
        return;
    }

    comm = (((uint16)msg->content[2]) << 8) | (msg->content[3]);
    data = (((uint16)msg->content[4]) << 8) | (msg->content[5]);

    if((0 == comm) && (0 == data))
    {
        pelco_d_handle(PELCO_D_TYPE_STOP);
        return;
    }
    
    for(i = 1; i <= 9; i++)
    {
        uint8 bit_val = (comm >> i) & 0x1;
        if(0 == bit_val)
        {
            continue;
        }

        switch(i)
        {
            case 1:
                pelco_d_handle(PELCO_D_TYPE_RIGHT);
                break;
            case 2:
                pelco_d_handle(PELCO_D_TYPE_LEFT);
                break;
            case 3:
                pelco_d_handle(PELCO_D_TYPE_UP);
                break;
            case 4:
                pelco_d_handle(PELCO_D_TYPE_DOWN);
                break;
            case 5:
                pelco_d_handle(PELCO_D_TYPE_ZOOM_TELE);
                break;
            case 6:
                pelco_d_handle(PELCO_D_TYPE_ZOOM_WIDE);
                break;
            case 8:
                pelco_d_handle(PELCO_D_TYPE_FOCUS_FAR);
                break;
            case 9:
                pelco_d_handle(PELCO_D_TYPE_FOCUS_NEAR);
                break;
        }        
    }

    pelco_handle_cmd((PELCO_CMD_e)(comm >> 10));
}

int pelco_d_process(void)
{
    PELCO_D_MSG_t* msg = NULL;
    msg = pelco_d_pull_message();
    if(NULL != msg)
    {
        pelco_print_message(msg);
        if(pelco_d_check_valid(msg))
        {
            if(PELCO_D == msg->type)
            {
                pelco_d_parse_message(msg);
            }
            else if(PELCO_P == msg->type)
            {
                pelco_p_parse_message(msg);
            }
        }

        msg->wp = 0;
    }
    
    return 0;
}



