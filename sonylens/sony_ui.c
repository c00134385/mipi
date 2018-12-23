
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

/* global */
UI_DESC_t ui_desc;
unsigned char camera_id = 1;


/* private */
unsigned char sony_ui_get_char(char in)
{
    unsigned char ret = 0x1B;
    if(isnumber(in)) {
        if(in == '0') {
            ret = 0x27;
        } else {
            ret = 0x1E + (in - '1');
        }
    } else if(isalphabet(in)) {
        if((in>='a')&&(in<='z')) {
            ret = 0x0 + (in - 'a');
        } else {
            ret = 0x0 + (in - 'A');
        }
    } else if(in == ' ') {
        ret = 0x1B;
    } else if(in == '?') {
        ret = 0x1C;
    } else if(in == '-') {
        ret = 0x4F;
    } else if(in == '/') {
        ret = 0x4E;
    } else if(in == '.') {
        ret = 0x4C;
    } else {
        ret = 0x48;
    }

    return ret;
}

int sony_ui_init(void)
{
    int i = 0;
    ui_desc.need_display = false;
    ui_desc.need_clear = false;

    for(i = 0; i < UI_LINES_COUNT; i++)
    {
        ui_desc.lines[i].need_display = false;
        ui_desc.lines[i].line_pos = 0;
        memset(ui_desc.lines[i].title, 0x1B, UI_CHARACTERS_COUNT);
    }

    camera_id = 1;
    return 0;
}

int sony_ui_display_all(BOOL display)
{
    VISCA_result_e result;
    result = visca_set_title_display(camera_id, 0xF, display);
    if(result != VISCA_result_ok) {
        return -1;
    }
    return 0;
}

int sony_ui_display_line(uint8 line_num, BOOL display)
{
    VISCA_result_e result;
    result = visca_set_title_display(camera_id, line_num, display);
    if(result != VISCA_result_ok) {
        return -1;
    }
    return 0;
}

int sony_ui_clear_all(void)
{
    VISCA_result_e result;
    result = visca_set_title_clear(camera_id, 0xF);
    if(result != VISCA_result_ok) {
        return -1;
    }
    return 0;
}

int sony_ui_clear_line(uint8 line_num)
{
    VISCA_result_e result;
    result = visca_set_title_clear(camera_id, line_num);
    if(result != VISCA_result_ok) {
        return -1;
    }
    return 0;
}

int sony_ui_set_title(uint8 line_num, char* head, char* left, char* right)
{
    VISCA_result_e result;
    int i = 0;
    int len = 0;
    int len_head = 0;
    UI_LINE_t* p_ui_line = &ui_desc.lines[line_num];
    memset(p_ui_line->title, 0x1B, UI_CHARACTERS_COUNT);

    if(NULL != head)
    {
        len_head = strlen(head);
        for(i = 0; i < len_head; i++)
        {
            p_ui_line->title[i] = sony_ui_get_char(head[i]);
        }
    }

    if(NULL != left)
    {
        len = strlen(left);
        for(i = 0; i < len; i++)
        {
            p_ui_line->title[i + len_head] = sony_ui_get_char(left[i]);
        }
    }

    if(NULL != right)
    {
        len = strlen(right);
        for(i = 0; i < len; i++)
        {
            p_ui_line->title[UI_CHARACTERS_COUNT - len + i] = sony_ui_get_char(right[i]);
        }
    }

    result = visca_set_title(camera_id, line_num, p_ui_line->title);
    if(result != VISCA_result_ok) {
        return -1;
    }

    return 0;
}

int sony_ui_set_char(uint8 line_num, uint8 line_pos, char c)
{
    ui_desc.lines[line_num].title[line_pos] = sony_ui_get_char(c);
    ui_desc.lines[line_num].need_display = true;
    return 0;
}

int sony_ui_set_color(uint8 line_num, uint8 color)
{
    VISCA_result_e result;
    result = visca_set_title_param(camera_id, line_num, ui_desc.lines[line_num].line_pos, color, 0);
    if(result != VISCA_result_ok) {
        return -1;
    }
    return 0;
}

int sony_ui_process(void)
{
    VISCA_result_e result;
    int i = 0;
    
    if(ui_desc.need_display)
    {
        result = visca_set_title_display(camera_id, 0xF, 1);
        if(result != VISCA_result_ok) {
            return -1;
        }
        for(i = 0; i < UI_LINES_COUNT; i++)
        {
            ui_desc.lines[i].need_display = false;
        }
    
        ui_desc.need_display = false;
    }
    else
    {
        #if 0
        for(i = 0; i < UI_LINES_COUNT; i++)
        {
            if(ui_desc.lines[i].need_display)
            {
                result = visca_set_title_display(camera_id, i, 1);
                if(result != VISCA_result_ok) {
                    return -1;
                }
                ui_desc.lines[i].need_display = false;
            }
        }
        #endif
    }
    return 0;
}

