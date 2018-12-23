#ifndef __SONY_UI_H
#define __SONY_UI_H

#define UI_LINES_COUNT        (11)
#define UI_CHARACTERS_COUNT   (20)

typedef struct UI_LINE {
    BOOL need_display;
    uint8 line_pos;
    uint8 color;
    uint8 blink;
    unsigned char title[UI_CHARACTERS_COUNT];
} UI_LINE_t;


typedef struct UI_DESC {
    BOOL need_display;
    BOOL need_clear;
    
    UI_LINE_t lines[UI_LINES_COUNT];
} UI_DESC_t;

int sony_ui_init(void);
int sony_ui_display_all(BOOL display);
int sony_ui_display_line(uint8 line_num, BOOL display);
int sony_ui_clear_all(void);
int sony_ui_clear_line(uint8 line_num);
int sony_ui_set_title(uint8 line_num, char* head, char* left, char* right);
int sony_ui_set_char(uint8 line_num, uint8 line_pos, char c);
int sony_ui_set_color(uint8 line_num, uint8 color);
int sony_ui_process(void);

#endif
