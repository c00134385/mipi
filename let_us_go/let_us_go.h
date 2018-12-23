#ifndef __LET_US_GO_H_
#define __LET_US_GO_H_

typedef enum{
    M_DIED,             //��������
    M_INIT,             //��ʼ��
    M_RELIVE,           //����
    M_LIVING,           //�����ǻ��
    M_HESITATE,         //�ǻ�
    M_MOVING,           //�ƶ�
    M_STOP,             //ֹͣ
    M_NOCHANGE,         //���ı�
}POS_MOTION_STATE;


typedef struct{
    char gps_state;         //
    int distance;           //�ƶ�����
}GO_STATE_TYPE;


typedef struct{
    bool valid;
    tm time;
    GPS_POS_TYPE pos;
}RT_POS_TYPE;


typedef enum{
    TRACK_IDEL,
    TRACK_START,
    TRACK_GOING,
    TRACK_PAUSE,
    TRACK_END,
}TRACK_STATE;

typedef struct{
    TRACK_STATE state;
    RT_POS_TYPE *last_p;
    RT_POS_TYPE *now_p;
    bool force_update;
    double total_distance;
    double distance;
}TRACK_INFO_TYPE;


char* now_position_to_xml(void);



void iTrack_taskinit(void);
int iTrack_taskctrl(TRACK_STATE cmd);
void iTrack_task(void);

#endif

