#ifndef __PELCO_D_H
#define __PELCO_D_H


#define PELCO_D_CMD_RIGHT       (1<<1)
#define PELCO_D_CMD_LEFT        (1<<2)
#define PELCO_D_CMD_UP          (1<<3)
#define PELCO_D_CMD_DOWN        (1<<4)
#define PELCO_D_CMD_ZOOM_TELE   (1<<5)
#define PELCO_D_CMD_ZOOM_WIDE   (1<<6)
#define PELCO_D_CMD_FOCUS_FAR   (1<<7)
#define PELCO_D_CMD_FOCUS_NEAR  (1<<8)
#define PELCO_D_CMD_IRIS_OPEN   (1<<9)
#define PELCO_D_CMD_IRIS_CLOSE  (1<<10)
#define PELCO_D_CMD_CAMERA_ON_OFF    (1<<11)
#define PELCO_D_CMD_SCAN_AUTO_MANUAL (1<<12)
#define PELCO_D_CMD_MENU          (1<<13)
#define PELCO_D_CMD_SENSE       (1<<15)


#define PELCO_D_MSG_LEN   (7)
#define PELCO_D_MSG_STX   (0xFF)

#define PELCO_P_MSG_LEN   (8)
#define PELCO_P_MSG_STX   (0xA0)
#define PELCO_P_MSG_ETX   (0xAF)

#define PELCO_P_MSG_BUFF_LEN  (10)


typedef enum PELCO_TYPE {
    PELCO_D = 0,
    PELCO_P,
    PELCO_NULL,
} PELCO_TYPE_e;


typedef enum PELCO_D_TYPE {
    PELCO_D_TYPE_STOP = 0,
    PELCO_D_TYPE_RIGHT,
    PELCO_D_TYPE_LEFT,
    PELCO_D_TYPE_UP,
    PELCO_D_TYPE_DOWN,
    PELCO_D_TYPE_ZOOM_TELE,
    PELCO_D_TYPE_ZOOM_WIDE,
    PELCO_D_TYPE_FOCUS_FAR,
    PELCO_D_TYPE_FOCUS_NEAR,
    PELCO_D_TYPE_IRIS_OPEN,
    PELCO_D_TYPE_IRIS_CLOSE,
    PELCO_D_TYPE_CAMERA_ON = 21,
    PELCO_D_TYPE_CAMERA_OFF,
    PELCO_D_TYPE_SCAN_AUTO,
    PELCO_D_TYPE_SCAN_MANUAL,
    PELCO_D_TYPE_NAVI_C,
    PELCO_D_TYPE_NAVI_U,
    PELCO_D_TYPE_NAVI_D,
    PELCO_D_TYPE_NAVI_L,
    PELCO_D_TYPE_NAVI_R,
    PELCO_D_TYPE_UNKNOWN,
} PELCO_D_TYPE_e;

typedef enum PELCO_CMD {
    PELCO_CMD_F1 = 1,
    PELCO_CMD_F2,
    PELCO_CMD_F3,
    PELCO_CMD_F4,
    PELCO_CMD_F5,
    PELCO_CMD_F6,
    PELCO_CMD_F7,
    PELCO_CMD_F8,
    PELCO_CMD_F9,
    PELCO_CMD_F10,
    PELCO_CMD_F11,
    PELCO_CMD_F12,
} PELCO_CMD_e;


typedef enum MSG_Q_STATE {
    MSG_Q_STATE_EMPTY = 0,
    MSG_Q_STATE_NORMAL,
    MSG_Q_STATE_FULL,
} MSG_Q_STATE_e;


typedef struct PELCO_D_MSG {
    PELCO_TYPE_e type;
    int wp;
    unsigned char content[PELCO_P_MSG_BUFF_LEN];
} PELCO_D_MSG_t;

typedef struct PELCO_D_MSG_QUEUE {
    int rp;
    int wp;
    int size;
    MSG_Q_STATE_e state;
    PELCO_D_MSG_t msg_list[10];
} PELCO_D_MSG_QUEUE_t;


int pelco_d_init(void);
void pelco_fill_message(unsigned char byte);
PELCO_D_MSG_t* pelco_d_pull_message(void);
void pelco_print_message(PELCO_D_MSG_t* msg);
bool pelco_d_check_valid(PELCO_D_MSG_t* msg);
void pelco_d_handle(PELCO_D_TYPE_e cmd);
void pelco_handle_cmd(PELCO_CMD_e cmd);
void pelco_d_parse_message(PELCO_D_MSG_t* msg);
int pelco_d_process(void);


#endif
