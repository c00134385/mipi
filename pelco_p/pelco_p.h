#ifndef __PELCO_P_H
#define __PELCO_P_H


#define PELCO_P_CMD_RIGHT       (1<<1)
#define PELCO_P_CMD_LEFT        (1<<2)
#define PELCO_P_CMD_UP          (1<<3)
#define PELCO_P_CMD_DOWN        (1<<4)
#define PELCO_P_CMD_ZOOM_TELE   (1<<5)
#define PELCO_P_CMD_ZOOM_WIDE   (1<<6)

#define PELCO_P_CMD_FOCUS_FAR   (1<<8)
#define PELCO_P_CMD_FOCUS_NEAR  (1<<9)
#define PELCO_P_CMD_IRIS_OPEN   (1<<10)
#define PELCO_P_CMD_IRIS_CLOSE  (1<<11)
#define PELCO_P_CMD_CAMERA_ON_OFF    (1<<12)
#define PELCO_P_CMD_SCAN_AUTO_MANUAL (1<<13)
#define PELCO_P_CMD_SENSE       (1<<15)


#define PELCO_P_MSG_LEN   (8)
#define PELCO_P_MSG_STX   (0xA0)
#define PELCO_P_MSG_ETX   (0xAF)


typedef enum PELCO_P_TYPE {
    PELCO_P_TYPE_STOP = 0,
    PELCO_P_TYPE_RIGHT,
    PELCO_P_TYPE_LEFT,
    PELCO_P_TYPE_UP,
    PELCO_P_TYPE_DOWN,
    PELCO_P_TYPE_ZOOM_TELE,
    PELCO_P_TYPE_ZOOM_WIDE,
    PELCO_P_TYPE_FOCUS_FAR,
    PELCO_P_TYPE_FOCUS_NEAR,
    PELCO_P_TYPE_IRIS_OPEN,
    PELCO_P_TYPE_IRIS_CLOSE,
    PELCO_P_TYPE_CAMERA_ON = 21,
    PELCO_P_TYPE_CAMERA_OFF,
    PELCO_P_TYPE_SCAN_AUTO,
    PELCO_P_TYPE_SCAN_MANUAL,
    PELCO_P_TYPE_UNKNOWN,
} PELCO_P_TYPE_e;

typedef struct PELCO_P_MSG {
    unsigned char content[PELCO_P_MSG_LEN];
} PELCO_P_MSG_t;

typedef struct PELCO_P_MSG_QUEUE {
    int rp;
    int wp;
    int size;
    MSG_Q_STATE_e state;
    PELCO_P_MSG_t msg_list[10];
} PELCO_P_MSG_QUEUE_t;


int pelco_p_init(void);
void pelco_p_push_message(PELCO_P_MSG_t* msg);
PELCO_P_MSG_t* pelco_p_pull_message(void);
void pelco_p_print_message(PELCO_P_MSG_t* msg);
bool pelco_p_check_valid(PELCO_P_MSG_t* msg);
void pelco_p_handle(PELCO_P_TYPE_e cmd);
void pelco_p_parse_message(PELCO_P_MSG_t* msg);
int pelco_p_process(void);


#endif
