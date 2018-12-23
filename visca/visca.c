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

/* macro */
#define VISCA_INPUT_BUFFER_SIZE            (32) /* size of the local packet buffer */
#define VISCA_NO_RESPONSE_TIME_OUT         (500) /* 2000 ms */

/* constant */
/**********************/
/* Common constant */
/**********************/
static unsigned char ack_msg[] = {0x90, 0x41, 0xff};
static unsigned char completion_msg[] = {0x90, 0x51, 0xff};
static unsigned char ack_completion_msg[] = {0x90, 0x41, 0xff, 0x90, 0x51, 0xff};
static unsigned char network_change[] = {0x90,0x38,0xff};
static unsigned char command_not_executable[] = {0x90,0x61,0x41,0xff};
static unsigned char reply_for_set_address[] = {0x88,0x30,0x02,0xff};
static unsigned char reply_for_if_clear[] = {0x90,0x50,0xff};
static unsigned char inq_result_msg[] = {0x90, 0x50};


/* variable */
static int input_buf_index = 0;
static unsigned char input_buf[VISCA_INPUT_BUFFER_SIZE];
VISCA_state_e g_state = VISCA_state_idle;
VISCA_result_e g_result = VISCA_result_ok;
unsigned long visca_start_time = 0;




/* internal */
void visca_print(unsigned char* msg, int len) {
    int i = 0;
    printf("\r\n len:%d", len);
    while(i < len) {
        if(i == 0) {
            printf("\r\n msg: 0x%02x", msg[i]);
        } else {
            printf(" 0x%02x", msg[i]);
        }
        i ++;
    }
}

bool visca_is_ack(unsigned char* data, int len) {
    bool result = true;
    int i = 0;
    if(len != sizeof(ack_msg)) {
        return false;
    }
    
    for(i = 0; i < len; i++) {
        if(data[i] != ack_msg[i]) {
            result = false;
            break;
        }
    }
    return result;
}

bool visca_is_completion(unsigned char* data, int len) {
    bool result = true;
    int i = 0;
    if(len != sizeof(completion_msg)) {
        return false;
    }
    
    for(i = 0; i < len; i++) {
        if(data[i] != completion_msg[i]) {
            result = false;
            break;
        }
    }
    return result;
}

bool visca_is_ack_completion(unsigned char* data, int len) {
    bool result = true;
    int i = 0;
    if(len != sizeof(ack_completion_msg)) {
        return false;
    }
    
    for(i = 0; i < len; i++) {
        if(data[i] != ack_completion_msg[i]) {
            result = false;
            break;
        }
    }
    return result;
}


bool visca_is_network_change(unsigned char* data, int len) {
    bool result = true;
    int i = 0;
    if(len != sizeof(network_change)) {
        return false;
    }
    
    for(i = 0; i < len; i++) {
        if(data[i] != network_change[i]) {
            result = false;
            break;
        }
    }
    return result;
}

bool visca_is_command_not_executable(unsigned char* data, int len) {
    bool result = true;
    int i = 0;
    if(len != sizeof(command_not_executable)) {
        return false;
    }
    
    for(i = 0; i < len; i++) {
        if(data[i] != command_not_executable[i]) {
            result = false;
            break;
        }
    }
    return result;
}


bool visca_is_reply(unsigned char* src, int src_len, unsigned char* dst, int dst_len) {
    bool result = true;
    int i = 0;

    if(src_len != dst_len) {
        printf("\r\n src:");
        visca_print(src, src_len);
        printf("\r\n dst:");
        visca_print(dst, dst_len);
        return false;
    }
    
    for(i = 0; i < src_len; i++) {
        if(src[i] != dst[i]) {
            result = false;
            break;
        }
    }

    if(!result) {
        printf("\r\n src:");
        visca_print(src, src_len);
        printf("\r\n dst:");
        visca_print(dst, dst_len);
    }
    
    return result;
}

bool visca_is_inq_result(unsigned char* data, int len) {
    bool result = true;
    int i = 0;
    if(len <= sizeof(inq_result_msg)) {
        return false;
    }
    
    for(i = 0; i < sizeof(inq_result_msg); i++) {
        if(data[i] != inq_result_msg[i]) {
            result = false;
            break;
        }
    }
    return result;
}



void visca_set_reply_msg(int address) {
    uint8 header = (0x8 + address)<<4;
    ack_msg[0] = header;
    completion_msg[0] = header;
    command_not_executable[0] = header;
    inq_result_msg[0] = header;
}
void visca_set_result(VISCA_result_e r) {
    g_result = r;
}

void visca_input_byte(unsigned char byte) {
    input_buf[input_buf_index++] = byte;
    #if 0
    if(byte == 0xff) {
        //printf("\r\n it is terminator");
        
        if(visca_is_ack(input_buf, input_buf_index)) {
            visca_set_state(VISCA_state_wait_completion);
            //printf("\r\n it is ack");
        } else if(visca_is_completion(input_buf, input_buf_index)) {
            visca_set_result(VISCA_result_ok);
            visca_set_state(VISCA_state_completion);    
            printf("\r\n it is completion");
        } else if(visca_is_network_change(input_buf, input_buf_index)) {
            visca_set_result(VISCA_result_network_change);
            visca_set_state(VISCA_state_completion);
            printf("\r\n it is network change");
        } else if(visca_is_command_not_executable(input_buf, input_buf_index)) {
            visca_set_result(VISCA_result_command_not_executable);
            visca_set_state(VISCA_state_completion);
            printf("\r\n it is command_not_executable");
        } else {
            if(VISCA_state_wait_set_address_done == visca_get_state()) {
                printf("\r\n set address done?");
                if(visca_is_reply(input_buf, input_buf_index, reply_for_set_address, sizeof(reply_for_set_address))) {
                    printf("\r\n set address done!");
                    visca_set_result(VISCA_result_ok);
                    visca_set_state(VISCA_state_completion);
                }
            } else if(VISCA_state_wait_if_clear_done == visca_get_state()) {
                printf("\r\n if clear done?");
                if(visca_is_reply(input_buf, input_buf_index, reply_for_if_clear, sizeof(reply_for_if_clear))) {
                    printf("\r\n if clear done!");
                    visca_set_result(VISCA_result_ok);
                    visca_set_state(VISCA_state_completion);
                }
            } else if(VISCA_state_wait_result == visca_get_state()) {
                printf("\r\n inquiry done?");
                if(visca_is_inq_result(input_buf, input_buf_index)) {
                    printf("\r\n inquiry done!");
                    visca_set_result(VISCA_result_ok);
                    visca_set_state(VISCA_state_completion);
                } else {
                    visca_print(input_buf, input_buf_index);
                }
            } else {
                printf("\r\n unknown situation....");
                visca_set_result(VISCA_result_fail);
                visca_set_state(VISCA_state_completion);
                visca_print(input_buf, input_buf_index);
            }
        }

        input_buf_index = 0;
    }
    #endif
}

void visca_print_packet(VISCA_packet_t* packet) {
    int i = 0;
    printf("\r\n packet len:%d", packet->length);
    while(i < packet->length) {
        if(i == 0) {
            printf("\r\n packet: 0x%02x", packet->bytes[i]);
        } else {
            printf(" 0x%02x", packet->bytes[i]);
        }
        i ++;
    }
}

void visca_send_packet(VISCA_packet_t *packet) {
    visca_print_packet(packet);
    memset(input_buf, 0, sizeof(input_buf));
    input_buf_index = 0;
    visca_start_time = GetSysTick_10Ms();
    uart2_comm_write(packet->bytes, packet->length);
}

void visca_send_packet_without_print(VISCA_packet_t *packet) {
    memset(input_buf, 0, sizeof(input_buf));
    input_buf_index = 0;
    visca_start_time = GetSysTick_10Ms();
    uart2_comm_write(packet->bytes, packet->length);
}

bool visca_is_no_response() {
    return ((GetSysTick_10Ms() - visca_start_time) > VISCA_NO_RESPONSE_TIME_OUT);
}

/* external */
void visca_init() {
    uart2_set_input_byte_callback(visca_input_byte);    
}

void visca_append_byte(VISCA_packet_t *packet, unsigned char byte)
{
    packet->bytes[packet->length]=byte;
    (packet->length)++;
}


void visca_init_packet(VISCA_packet_t *packet, int address)
{
    // we start writing at byte 1, the first byte will be filled by the
    // packet sending function. This function will also append a terminator.
    packet->bytes[0] = 0x80 | address;
    packet->length=1;
}

void visca_init_broadcast_packet(VISCA_packet_t *packet)
{
    // we start writing at byte 1, the first byte will be filled by the
    // packet sending function. This function will also append a terminator.
    packet->bytes[0] = 0x88;
    packet->length=1;
}

void visca_set_state(VISCA_state_e s) {
    g_state = s;
}

VISCA_state_e visca_get_state() {
    return g_state;
}

VISCA_result_e visca_get_result() {
    return g_result;
}

VISCA_result_e visca_set_address(int address) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_broadcast_packet(&packet);
    visca_append_byte(&packet, 0x30);
    visca_append_byte(&packet, address);;
    visca_append_byte(&packet, VISCA_TERMINATOR);

    reply_for_set_address[2] = address + 1;
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_set_address_done);

    printf("\r\n set address done?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index > 0) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_reply(input_buf, input_buf_index, reply_for_set_address, sizeof(reply_for_set_address))) {
                printf("\r\n set address done!");
                ret = VISCA_result_ok;
                break;
            } else if(visca_is_network_change(input_buf, input_buf_index)){
                printf("\r\n network change!");
                ret = VISCA_result_network_change;
                break;
            } else {
                printf("\r\n unknown message!");
                ret = VISCA_result_unknown;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_if_clear(int address) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_INTERFACE);
    visca_append_byte(&packet, 0x01);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    reply_for_if_clear[0] = (0x8 + address)<<4;
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_if_clear_done);

    printf("\r\n if clear done?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index > 0) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_reply(input_buf, input_buf_index, reply_for_if_clear, sizeof(reply_for_if_clear))) {
                printf("\r\n if clear done!");
                ret = VISCA_result_ok;
                break;
            } else if(visca_is_network_change(input_buf, input_buf_index)){
                printf("\r\n network change!");
                ret = VISCA_result_network_change;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                printf("\r\n unknown message!");
                ret = VISCA_result_unknown;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}
VISCA_result_e visca_get_version(int address, uint32* model, uint32* rom, uint32* socket)
{
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_INQUIRY);
    visca_append_byte(&packet, VISCA_CATEGORY_INTERFACE);
    visca_append_byte(&packet, 0x02);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_result);

    printf("\r\n get_version?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index > 0) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_inq_result(input_buf, input_buf_index)) {
                printf("\r\n get_version done!");
                *model = (((uint32)input_buf[4])<<8) | input_buf[5];
                *rom = (((uint32)input_buf[6])<<8) | input_buf[7];
                *socket = input_buf[8];
                ret = VISCA_result_ok;
                break;
            } else {
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

// self-check state inquiry
VISCA_result_e visca_get_self_check_state(int address, uint8 *state) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_INQUIRY);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0xB4);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_result);

    printf("\r\n get self-check state?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index > 0) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_inq_result(input_buf, input_buf_index)) {
                printf("\r\n get self-check state done! state:%d", input_buf[2]);
                *state = input_buf[2];
                ret = VISCA_result_ok;
                break;
            } else {
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_power(int address, uint8 *power) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_INQUIRY);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x00);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_result);

    printf("\r\n get power?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index > 0) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_inq_result(input_buf, input_buf_index)) {
                printf("\r\n get power done!");
                *power = input_buf[2];
                ret = VISCA_result_ok;
                break;
            } else {
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_set_power(int address, uint8 power) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x00);
    visca_append_byte(&packet, power);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set power?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set power done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_register(int address, uint8 reg, uint8 *value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_INQUIRY);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x24);
    visca_append_byte(&packet, (unsigned char)reg);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
        
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_result);


    printf("\r\n get register?");
    
    while(true) {
        Wait10Ms(1);
        if((input_buf_index > 0) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_inq_result(input_buf, input_buf_index)) {
                printf("\r\n get register done!");
                *value = ((input_buf[2]&0x0f)<<4) | (input_buf[3]&0x0f);
                ret = VISCA_result_ok;
                break;
            }
            else
            {
                ret = VISCA_result_fail;
                break;
            }
        }
        else if((input_buf_index == sizeof(command_not_executable)) && (input_buf[input_buf_index-1] == 0xFF))
        {
            if(visca_is_command_not_executable(input_buf, input_buf_index))
            {
                printf("\r\n please get register again.");
                ret = VISCA_result_command_not_executable;
                break;
            }
            else
            {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_set_register(int address, uint8 reg, uint8 value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x24);
    visca_append_byte(&packet, (unsigned char)reg);
    visca_append_byte(&packet, (value & 0xf0) >> 4);
    visca_append_byte(&packet, value & 0x0f);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set register?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set register done!");
                ret = VISCA_result_ok;
                break;
            }
            else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if((input_buf_index == sizeof(command_not_executable)) && (input_buf[input_buf_index-1] == 0xFF))
        {
            if(visca_is_command_not_executable(input_buf, input_buf_index))
            {
                printf("\r\n please set register again.");
                ret = VISCA_result_command_not_executable;
                break;
            }
            else
            {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_zoom_value(int address, uint32 *value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_INQUIRY);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x47);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
        
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_result);

    printf("\r\n get zoom_value?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index > 0) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_inq_result(input_buf, input_buf_index)) {
                printf("\r\n get zoom_value done. %x-%x-%x-%x", input_buf[2], input_buf[3], input_buf[4], input_buf[5]); 
                *value=(((uint32)input_buf[2])<<12)+(((uint32)input_buf[3])<<8)+(((uint32)input_buf[4])<<4)+(uint32)input_buf[5];
                ret = VISCA_result_ok;
                break;
            } else {
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }
}
VISCA_result_e visca_set_zoom_value(int address, uint32 value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x47);
    visca_append_byte(&packet, (value & 0xF000) >> 12);
    visca_append_byte(&packet, (value & 0x0F00) >>  8);
    visca_append_byte(&packet, (value & 0x00F0) >>  4);
    visca_append_byte(&packet, (value & 0x000F));
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set zoom_value?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set zoom_value done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if((input_buf_index == sizeof(command_not_executable)) && (input_buf[input_buf_index-1] == 0xFF))
        {
            if(visca_is_command_not_executable(input_buf, input_buf_index))
            {
                printf("\r\n please set zoom_value again.");
                ret = VISCA_result_command_not_executable;
                break;
            }
            else
            {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}


VISCA_result_e visca_get_title_display(int address, uint8 *enable) {
    VISCA_result_e ret = VISCA_result_unknown;
    return ret;
}
VISCA_result_e visca_set_title_display(int address, uint8 lines, uint8 enable) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x74);
    if(enable) {
        visca_append_byte(&packet, (0x20) | (lines & 0xF));
    } else {
        visca_append_byte(&packet, (0x30) | (lines & 0xF));
    }
    
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set title_display?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set title_display done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_title_clear(int address) {
    VISCA_result_e ret = VISCA_result_unknown;
    return ret;
}
VISCA_result_e visca_set_title_clear(int address, uint8 lines) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x74);
    visca_append_byte(&packet, (0x10) | (lines & 0xF));
    
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set title_clear?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set title_clear done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if((input_buf_index == sizeof(command_not_executable)) && (input_buf[input_buf_index-1] == 0xFF))
        {
            if(visca_is_command_not_executable(input_buf, input_buf_index))
            {
                printf("\r\n please title_clear again.");
                ret = VISCA_result_command_not_executable;
                break;
            }
            else
            {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_title_param(int address){
    VISCA_result_e ret = VISCA_result_unknown;
    return ret;
}
VISCA_result_e visca_set_title_param(int address, uint8 line, uint8 pos, uint8 color, uint8 blink) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x73);
    visca_append_byte(&packet, (0x10 | line));

    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, pos);
    visca_append_byte(&packet, color);  // h Pos
    visca_append_byte(&packet, blink);  // color
    visca_append_byte(&packet, 0);  // blink
    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, 0);
    
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set title_param?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set title_param done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_title(int address) {
    VISCA_result_e ret = VISCA_result_unknown;
    return ret;
}
VISCA_result_e visca_set_title(int address, uint8 line, unsigned char* title) {
    VISCA_result_e ret = VISCA_result_unknown;
    int i;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x73);
    visca_append_byte(&packet, (0x20 | line));
    for (i = 0; i < 10; i++) {
        visca_append_byte(&packet, title[i]);
    }
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);

    visca_send_packet_without_print(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set set_title part1?");

    while(true) {
        //Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set set_title part1 done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    if(ret != VISCA_result_ok) {
        return ret;
    }

    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x73);
    visca_append_byte(&packet, (0x30 | line));

    for (i = 10; i < 20; i++) {
        visca_append_byte(&packet, title[i]);
    }
    
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet_without_print(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set set_title part2?");

    while(true) {
        //Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set set_title part2 done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_set_zoom_stop(int address) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x07);
    visca_append_byte(&packet, 0x00);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set set_zoom_stop?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set set_zoom_stop done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}


VISCA_result_e visca_set_zoom_tele(int address) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x07);
    visca_append_byte(&packet, 0x02);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set set_zoom_tele?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set set_zoom_tele done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_set_zoom_wide(int address) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x07);
    visca_append_byte(&packet, 0x03);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set set_zoom_wide?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set set_zoom_wide done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}


VISCA_result_e visca_get_zoom_tele_speed(int address, uint8 *speed) {
    VISCA_result_e ret = VISCA_result_unknown;
    return ret;
}

VISCA_result_e visca_set_zoom_tele_speed(int address, uint8 speed) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x07);
    visca_append_byte(&packet, 0x20 | (speed & 0x7));
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set zoom_tele_speed?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set zoom_tele_speed done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_zoom_wide_speed(int address, uint8 *speed) {
    VISCA_result_e ret = VISCA_result_unknown;
    return ret;
}
VISCA_result_e visca_set_zoom_wide_speed(int address, uint8 speed) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x07);
    visca_append_byte(&packet, 0x30 | (speed & 0x7));
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set zoom_wide_speed?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set zoom_wide_speed done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_digital_zoom(int address, uint8 *enable) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_INQUIRY);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x06);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
        
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_result);

    printf("\r\n get_digital_zoom?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index > 0) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_inq_result(input_buf, input_buf_index)) {
                printf("\r\n get_digital_zoom done!");
                *enable = input_buf[2];
                ret = VISCA_result_ok;
                break;
            } else {
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}
VISCA_result_e visca_set_digital_zoom(int address, uint8 enable) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x06);
    visca_append_byte(&packet, enable);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set digital_zoom?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set digital_zoom done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_zoom_osd(int address, uint8 *enable) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_zoom_osd(int address, uint8 enable) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}

VISCA_result_e visca_get_zoom_display(int address, uint8 *enable) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_zoom_display(int address, uint8 enable) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}

VISCA_result_e visca_set_focus_far(int address) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x08);
    visca_append_byte(&packet, 0x02);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set_focus_far?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set_focus_far done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if((input_buf_index == sizeof(command_not_executable)) && (input_buf[input_buf_index-1] == 0xFF))
        {
            if(visca_is_command_not_executable(input_buf, input_buf_index))
            {
                printf("\r\n please set_focus_far again.");
                ret = VISCA_result_command_not_executable;
                break;
            }
            else
            {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_set_focus_near(int address) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x08);
    visca_append_byte(&packet, 0x03);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set_focus_near?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set_focus_near done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if((input_buf_index == sizeof(command_not_executable)) && (input_buf[input_buf_index-1] == 0xFF))
        {
            if(visca_is_command_not_executable(input_buf, input_buf_index))
            {
                printf("\r\n please set_focus_near again.");
                ret = VISCA_result_command_not_executable;
                break;
            }
            else
            {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_set_focus_far_speed(int address, uint8 speed) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x08);
    visca_append_byte(&packet, 0x20 | (speed & 0x7));
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set_focus_far_speed?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set_focus_far_speed done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if((input_buf_index == sizeof(command_not_executable)) && (input_buf[input_buf_index-1] == 0xFF))
        {
            if(visca_is_command_not_executable(input_buf, input_buf_index))
            {
                printf("\r\n please set_focus_near again.");
                ret = VISCA_result_command_not_executable;
                break;
            }
            else
            {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_set_focus_near_speed(int address, uint8 speed) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x08);
    visca_append_byte(&packet, 0x30 | (speed & 0x7));
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set_focus_near_speed?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set_focus_near_speed done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if((input_buf_index == sizeof(command_not_executable)) && (input_buf[input_buf_index-1] == 0xFF))
        {
            if(visca_is_command_not_executable(input_buf, input_buf_index))
            {
                printf("\r\n please set_focus_near_speed again.");
                ret = VISCA_result_command_not_executable;
                break;
            }
            else
            {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_set_focus_stop(int address) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x08);
    visca_append_byte(&packet, 0x00);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set_focus_stop?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set_focus_stop done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if((input_buf_index == sizeof(command_not_executable)) && (input_buf[input_buf_index-1] == 0xFF))
        {
            if(visca_is_command_not_executable(input_buf, input_buf_index))
            {
                printf("\r\n please set_focus_stop again.");
                ret = VISCA_result_command_not_executable;
                break;
            }
            else
            {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_focus_mode(int address, uint8* mode) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_INQUIRY);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x38);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
        
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_result);

    printf("\r\n get_focus_mode?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index > 0) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_inq_result(input_buf, input_buf_index)) {
                printf("\r\n get_focus_mode(0x%x) done.", input_buf[2]); 
                *mode = input_buf[2];
                ret = VISCA_result_ok;
                break;
            } else {
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }
}
VISCA_result_e visca_set_focus_mode(int address, uint8 mode) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x38);
    visca_append_byte(&packet, mode);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set focus_mode?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set focus_mode done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_focus_af_sens(int address, uint8* mode)
{
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_focus_af_sens(int address, uint8 mode)
{
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x58);
    visca_append_byte(&packet, mode);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set focus_af_sens?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set focus_af_sens done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_focus_light(int address, uint8* mode)
{
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_focus_light(int address, uint8 mode)
{
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x11);
    visca_append_byte(&packet, mode);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set focus_light?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set focus_light done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_focus_near_limit(int address, uint32 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_focus_near_limit(int address, uint32 value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x28);
    visca_append_byte(&packet, (value & 0xF000) >> 12);
    visca_append_byte(&packet, (value & 0x0F00) >>  8);
    visca_append_byte(&packet, (value & 0x00F0) >>  4);
    visca_append_byte(&packet, (value & 0x000F));
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set zoom_value?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set zoom_value done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_focus_position(int address, uint32 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_focus_position(int address, uint32 value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x48);
    visca_append_byte(&packet, (value & 0xF000) >> 12);  // 0x1000 ~ 0xF000
    visca_append_byte(&packet, (value & 0x0F00) >>  8);
    visca_append_byte(&packet, (value & 0x00F0) >>  4);
    visca_append_byte(&packet, (value & 0x000F));
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set_focus_position?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set_focus_position done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_exposure_ae_mode(int address, uint8 *mode) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_INQUIRY);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x39);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
        
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_result);

    printf("\r\n get exposure_ae?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index > 0) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_inq_result(input_buf, input_buf_index)) {
                printf("\r\n get exposure_ae done.0x%02x", input_buf[2]);
                *mode = input_buf[2];
                ret = VISCA_result_ok;
                break;
            } else {
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }
}
VISCA_result_e visca_set_exposure_ae_mode(int address, uint8 mode) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x39);
    visca_append_byte(&packet, mode); //0x00:Full Auto  0x03:Manual  0x0A:Shutter Pri  0x0B:Iris Pri  0x0D:Bright
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set exposure_ae_mode?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set exposure_ae_mode done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_exposure_shutter(int address, uint32 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_exposure_shutter(int address, uint32 value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x4A);
    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, (value & 0x00F0) >>  4);
    visca_append_byte(&packet, (value & 0x000F));
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set_exposure_shutter?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set_exposure_shutter done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if((input_buf_index == sizeof(command_not_executable)) && (input_buf[input_buf_index-1] == 0xFF))
        {
            if(visca_is_command_not_executable(input_buf, input_buf_index))
            {
                ret = VISCA_result_command_not_executable;
                break;
            }
            else
            {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_exposure_iris(int address, uint32 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_exposure_iris(int address, uint32 value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x4B);
    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, (value & 0x00F0) >>  4);
    visca_append_byte(&packet, (value & 0x000F));
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set_exposure_iris?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set_exposure_iris done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if((input_buf_index == sizeof(command_not_executable)) && (input_buf[input_buf_index-1] == 0xFF))
        {
            if(visca_is_command_not_executable(input_buf, input_buf_index))
            {
                ret = VISCA_result_command_not_executable;
                break;
            }
            else
            {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}


VISCA_result_e visca_get_exposure_gain(int address, uint32 *gain) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_exposure_gain(int address, uint32 gain) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x4C);
    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, (gain & 0x00F0) >>  4);
    visca_append_byte(&packet, (gain & 0x000F));
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set exposure_gain?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set exposure_gain done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        }
        else if((input_buf_index == sizeof(command_not_executable)) && (input_buf[input_buf_index-1] == 0xFF))
        {
            if(visca_is_command_not_executable(input_buf, input_buf_index))
            {
                ret = VISCA_result_command_not_executable;
                break;
            }
            else
            {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_exposure_slow_ae(int address, uint8 *value) {
    VISCA_result_e ret = VISCA_result_unknown;
    return ret;
}
VISCA_result_e visca_set_exposure_slow_ae(int address, uint8 value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x5D);
    visca_append_byte(&packet, value);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set exposure_slow_ae?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set exposure_slow_ae done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_exposure_slow_shutter(int address, uint8 *value) {
    VISCA_result_e ret = VISCA_result_unknown;
    return ret;
}
VISCA_result_e visca_set_exposure_slow_shutter(int address, uint8 value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x5A);
    visca_append_byte(&packet, value); 
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set exposure_slow_shutter?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set exposure_slow_shutter done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_wb_mode(int address, uint8 *mode) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_INQUIRY);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x35);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
        
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_result);

    printf("\r\n get wb_mode?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index > 0) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_inq_result(input_buf, input_buf_index)) {
                printf("\r\n get wb_mode done.0x%02x", input_buf[2]);
                *mode = input_buf[2];
                ret = VISCA_result_ok;
                break;
            } else {
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }
}
VISCA_result_e visca_set_wb_mode(int address, uint8 mode) {
    // ATW:0x35 04 
    // MANUAL: 0x35 0x05
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x35);
    visca_append_byte(&packet, mode);  // 0x04:ATW  0x05:Manual
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set wb_mode?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set wb_mode done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_wb_red_gain(int address, uint32 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_wb_red_gain(int address, uint32 value) {
    // 0x43
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x43);
    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, (value & 0x00F0) >>  4);
    visca_append_byte(&packet, (value & 0x000F));
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set wb_red_gain?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set wb_red_gain done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_wb_blue_gain(int address, uint32 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_wb_blue_gain(int address, uint32 value) {
    // 0x44
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x44);
    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, (value & 0x00F0) >>  4);
    visca_append_byte(&packet, (value & 0x000F));
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set wb_blue_gain?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set wb_blue_gain done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_set_wb_one_push(int address) {
    //0x10 05
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x10);
    visca_append_byte(&packet, 0x05);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set wb_one_push?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set wb_one_push done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_advance_blc(int address, uint8 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_advance_blc(int address, uint8 value) {
    //0x33
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x33);
    visca_append_byte(&packet, value);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set advance_blc?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set advance_blc done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_advance_expcomp_value(int address, uint32 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_advance_expcomp_value(int address, uint32 value) {
    //0x4e
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x4E);
    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, 0);
    visca_append_byte(&packet, (value & 0xF0)>4);
    visca_append_byte(&packet, (value & 0x0F));
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set advance_expcomp?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set advance_expcomp done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_advance_expcomp(int address, uint8 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_advance_expcomp(int address, uint8 value) {
    //0x4e
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x3E);
    visca_append_byte(&packet, (value & 0xFF));
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set advance_expcomp?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set advance_expcomp done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_advance_wdr(int address, uint8 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_advance_wdr(int address, uint8 value) {
    VISCA_result_e ret = VISCA_result_ok;
    //unknown
    return ret;
}

VISCA_result_e visca_get_advance_ircut(int address, uint8 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_advance_ircut(int address, uint8 value) {
    VISCA_result_e ret = VISCA_result_ok;
    //unknown
    return ret;
}

VISCA_result_e visca_get_advance_nr_level(int address, uint8 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_advance_nr_level(int address, uint8 value) {
    //0x53
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x53);
    visca_append_byte(&packet, value);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set advance_nr_level?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set advance_nr_level done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_advance_stabilizer(int address, uint8 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_advance_stabilizer(int address, uint8 value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x34);
    visca_append_byte(&packet, value);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set stabilizer?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set stabilizer done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_advance_defog_mode(int address, uint8 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_advance_defog_mode(int address, uint8 value, uint8 level) {
    //0x37
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x37);
    visca_append_byte(&packet, value);
    visca_append_byte(&packet, level);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set advance_defog_mode?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set advance_defog_mode done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_advance_alarm_out(int address, uint8 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_advance_alarm_out(int address, uint8 value) {
    VISCA_result_e ret = VISCA_result_ok;
    //unknown
    return ret;
}

VISCA_result_e visca_get_image_aperture(int address, uint8 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_image_aperture(int address, uint8 value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x42);
    visca_append_byte(&packet, 0x0);
    visca_append_byte(&packet, 0x0);
    visca_append_byte(&packet, (value & 0xF0) > 4);
    visca_append_byte(&packet, (value & 0xF));
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set image_aperture?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set image_aperture done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_image_color_gain(int address, uint8 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_image_color_gain(int address, uint8 value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x49);
    visca_append_byte(&packet, 0x0);
    visca_append_byte(&packet, 0x0);
    visca_append_byte(&packet, 0x0);
    visca_append_byte(&packet, value);// 0x0~0xE:  60% ~ 200%
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set image color_gain?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set image color_gain done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_image_color_hue(int address, uint8 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_image_color_hue(int address, uint8 value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x4F);
    visca_append_byte(&packet, 0x0);
    visca_append_byte(&packet, 0x0);
    visca_append_byte(&packet, 0x0);
    visca_append_byte(&packet, value); // 0x0~0xE:  -14 degress ~14 degress
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set image color_hue?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set image color_hue done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_image_chroma_suppress(int address, uint8 *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_image_chroma_suppress(int address, uint8 value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x5F);
    visca_append_byte(&packet, value); //0x00: off  0x01~0x03: (3 levels)
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set image mirror?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set image mirror done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_image_gamma(int address, int *value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_image_gamma(int address, int value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x1E);
    visca_append_byte(&packet, 0x00);
    visca_append_byte(&packet, 0x00);
    visca_append_byte(&packet, 0x00);
    if(value >= 0)
    {
        visca_append_byte(&packet, 0x00);
        visca_append_byte(&packet, (value & 0xF0) > 4);
        visca_append_byte(&packet, (value & 0x0F));
    }
    else
    {
        visca_append_byte(&packet, 0x01);
        visca_append_byte(&packet, ((-value) & 0xF0) > 4);
        visca_append_byte(&packet, ((-value) & 0x0F));
    }
    
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set image_gamma?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set image_gamma done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_image_mirror(int address, uint8* mirror) {
    VISCA_result_e ret = VISCA_result_unknown;
    return ret;
}
VISCA_result_e visca_set_image_mirror(int address, uint8 mirror) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x61);
    visca_append_byte(&packet, mirror);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set image mirror?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set image mirror done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_image_flip(int address, uint8* flip) {
    VISCA_result_e ret = VISCA_result_unknown;
    return ret;
}
VISCA_result_e visca_set_image_flip(int address, uint8 flip) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x66);
    visca_append_byte(&packet, flip);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set image flip?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set image flip done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}


VISCA_result_e visca_get_image_freeze(int address, uint8* mirror) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_image_freeze(int address, uint8 value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x62);
    visca_append_byte(&packet, value); // 0x02:on  0x03:off
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set image_freeze?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set image_freeze done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_image_pic_effect(int address, uint8* value) {
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_image_pic_effect(int address, uint8 value) {
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x63);
    visca_append_byte(&packet, value); // 0x00: off  0x02:Neg.Art  0x04:Black&White
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set image mirror?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set image mirror done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_privacy_mask(int address)
{
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_privacy_mask(int address, uint8 mask, uint8 w, uint8 h)
{
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x76);
    visca_append_byte(&packet, mask);
    visca_append_byte(&packet, 0x00);  // 0x00 resetting  0x01:setting newly
    visca_append_byte(&packet, (w & 0xF0)>4);
    visca_append_byte(&packet, w & 0x0F);
    visca_append_byte(&packet, (h & 0xF0)>4);
    visca_append_byte(&packet, h & 0x0F);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set privacy_mask?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set privacy_mask done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_privacy_non_interlock_mask(int address)
{
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_privacy_non_interlock_mask(int address, uint8 mask, uint8 x, uint8 y, uint8 w, uint8 h)
{
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x6F);
    visca_append_byte(&packet, mask);
    visca_append_byte(&packet, (x & 0xF0)>4);
    visca_append_byte(&packet, x & 0x0F);
    visca_append_byte(&packet, (y & 0xF0)>4);
    visca_append_byte(&packet, y & 0x0F);
    visca_append_byte(&packet, (w & 0xF0)>4);
    visca_append_byte(&packet, w & 0x0F);
    visca_append_byte(&packet, (h & 0xF0)>4);
    visca_append_byte(&packet, h & 0x0F);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set non_interlock_mask?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set non_interlock_mask done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

VISCA_result_e visca_get_privacy_display(int address, uint32* display)
{
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_INQUIRY);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x77);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
        
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_result);


    printf("\r\n get privacy_display?");
    
    while(true) {
        Wait10Ms(1);
        if((input_buf_index > 0) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_inq_result(input_buf, input_buf_index)) {
                printf("\r\n get privacy_display done!");
                *display = (input_buf[2]<<12) | (input_buf[3] << 8) | (input_buf[4] << 4) | (input_buf[5]);
                ret = VISCA_result_ok;
                break;
            } else {
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}
VISCA_result_e visca_set_privacy_display(int address, uint32 display)
{
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x77);
    visca_append_byte(&packet, (display>>12) & 0x3F);
    visca_append_byte(&packet, (display>>8) & 0x3F);
    visca_append_byte(&packet, (display>>4) & 0x3F);
    visca_append_byte(&packet, display & 0x3F); // A B C D
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set privacy_display?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set privacy_display done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;

}

VISCA_result_e visca_get_center_line_display(int address)
{
    VISCA_result_e ret = VISCA_result_ok;
    return ret;
}
VISCA_result_e visca_set_center_line_display(int address, uint8 enable)
{
    VISCA_result_e ret = VISCA_result_unknown;
    VISCA_packet_t packet;
    visca_set_state(VISCA_state_send);
    visca_init_packet(&packet, address);
    visca_append_byte(&packet, VISCA_COMMAND);
    visca_append_byte(&packet, VISCA_CATEGORY_CAMERA1);
    visca_append_byte(&packet, 0x7C);
    visca_append_byte(&packet, enable);
    visca_append_byte(&packet, VISCA_TERMINATOR);

    visca_set_reply_msg(address);
    
    visca_send_packet(&packet);
    visca_set_state(VISCA_state_wait_ack);

    printf("\r\n set center_line_display?");

    while(true) {
        Wait10Ms(1);
        if((input_buf_index == sizeof(ack_completion_msg)) && (input_buf[input_buf_index-1] == 0xFF)) {
            if(visca_is_ack_completion(input_buf, sizeof(ack_completion_msg))) {
                printf("\r\n set center_line_display done!");
                ret = VISCA_result_ok;
                break;
            } else {
                visca_print(input_buf, input_buf_index);
                ret = VISCA_result_fail;
                break;
            }
        } else if(visca_is_no_response()){
            visca_print(input_buf, input_buf_index);
            ret = VISCA_result_no_response;
            break;
        }
    }

    visca_set_state(VISCA_state_idle);
    return ret;
}

