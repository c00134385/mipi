#ifndef __YEELINK_H_
#define __YEELINK_H_

typedef enum{
    GPS_POS = 1,
    SPEED_SENSOR,     //general sensor， 泛传感器
    GPS_OFFSET,
    IMAGE_SENSOR,       //图像传感器
    VALUE_SENSOR,       //数值传感器
}SENSOR_TYPE;



typedef struct{
    const char *siteUrl;
    const char *Host;
    const char *U_ApiKey;
    const char *ContentType;
    const char *Length;
}POST_TYPE;


#define iPOS_NUM    15

typedef struct iPOS_BUFF{
    int wr_id;
    int rd_id;
    int pos_num;
    int jump_step;       //坐标跳跃数
    int jump_cnt;
    RT_POS_TYPE GPS[iPOS_NUM];
}UPLOAD_POS_BF;



typedef struct yeelink_cfg{
    char *http_header;
    char *product_sn;
    char *api_key;
}YEELINK_CFG_TYPE;


void yeelink_http_init(void);
void yeelink_taskinit(void);
void yeelink_task(void);
void iPos_clr(void);
int iPos_get_num(void);

#endif

