#ifndef __VISCA_P_H
#define __VISCA_P_H

#define VISCA_MSG_STX   (0x80)
#define VISCA_MSG_ETX   (0xFF)
#define VISCA_MSG_BROADCAST   (0x08)


#define VISCA_MSG_BUFF_LEN (16)

typedef struct VISCA_MSG {
    int wp;
    unsigned char content[16];
} VISCA_MSG_t;

typedef struct VISCA_MSG_QUEUE {
    int rp;
    int wp;
    int size;
    MSG_Q_STATE_e state;
    VISCA_MSG_t msg_list[VISCA_MSG_BUFF_LEN];
} VISCA_MSG_QUEUE_t;


int sony_visca_init(void);
void visca_fill_message(unsigned char byte);
VISCA_MSG_t* visca_pull_message(void);
void visca_print_message(VISCA_MSG_t* msg);
void visca_parse_message(VISCA_MSG_t* msg);
int visca_process(void);


#endif
