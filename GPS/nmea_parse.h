#ifndef __NMEA_PARSE_H_
#define __NMEA_PARSE_H_


typedef enum{
    MSG_GGA,    //全球定位数据
    MSG_GLL,    //大地坐标
    MSG_GSA,    //卫星PRN数据
    MSG_GSV,    //卫星状态
    MSG_RMC,   //运输定位数据
    MSG_VTG,   //地面速度信息
    MSG_ZDA,   //时间信息
}GPS_MSG_TYPE;

/*----------------------------------------------------------------------------*/
/*
　　$GPGGA,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>,<13>,<14>*<15><CR><LF>
　　<1> UTC时间，格式为hhmmss.sss。
　　<2> 纬度，格式为ddmm.mmmm（前导位数不足则补0）。
　　<3> 纬度半球，N或S（北纬或南纬）。
　　<4> 经度，格式为dddmm.mmmm（前导位数不足则补0）。
　　<5> 经度半球，E或W（东经或西经）。
　　<6> 定位质量指示，0=定位无效，1=定位有效。
　　<7> 使用卫星数量，从00到12（前导位数不足则补0）。
　　<8> 水平精确度，0.5到99.9。
　　<9> 天线离海平面的高度，-9999.9到9999.9米
　　<10> 高度单位，M表示单位米。
　　<11> 大地椭球面相对海平面的高度（-999.9到9999.9）。
　　<12> 高度单位，M表示单位米。
　　<13> 差分GPS数据期限（RTCM SC-104），最后设立RTCM传送的秒数量。
　　<14> 差分参考基站标号，从0000到1023（前导位数不足则补0）。
　　<15> 校验和。
*/
typedef struct{
    char update;
    char PositionFix;
    int SatellitesUsed;
    char *msg;   //msg[72+10];
}GGA_TYPE;


/*----------------------------------------------------------------------------*/
/*
　　$GPGSA,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>,<13>,<14>,<15>,<16>,<17>*<18><CR><LF>
　　<1> 模式2：M = 手动， A = 自动。
　　<2> 模式1：定位型式1 = 未定位，2 = 二维定位，3 = 三维定位。
　　<3> 第1信道正在使用的卫星PRN码编号（Pseudo Random Noise，伪随机噪声码），01至32（前导位数不足则补0，最多可接收12颗卫星信息）。
　　<4> 第2信道正在使用的卫星PRN码编号
　　<5> 第3信道正在使用的卫星PRN码编号
　　<6> 第4信道正在使用的卫星PRN码编号
　　<7> 第5信道正在使用的卫星PRN码编号
　　<8> 第6信道正在使用的卫星PRN码编号
　　<9> 第7信道正在使用的卫星PRN码编号
　　<10> 第8信道正在使用的卫星PRN码编号
　　<11> 第9信道正在使用的卫星PRN码编号
　　<12> 第10信道正在使用的卫星PRN码编号
　　<13> 第11信道正在使用的卫星PRN码编号
　　<14> 第12信道正在使用的卫星PRN码编号
　　<15> PDOP综合位置精度因子（0.5 - 99.9）
　　<16> HDOP水平精度因子（0.5 - 99.9）
　　<17> VDOP垂直精度因子（0.5 - 99.9）
　　<18> 校验和
*/
typedef struct{
    char update;
    char *msg;  //msg[65+10];
}GSA_TYPE;


/*----------------------------------------------------------------------------*/
/*
　　$GPGSV, <1>,<2>,<3>,<4>,<5>,<6>,<7>,...,<4>,<5>,<6>,<7>*<8><CR><LF>
　　<1> 总的GSV语句电文数。
　　<2> 当前GSV语句号。
　　<3> 可视卫星总数，00至12。
　　<4> 卫星编号，01至32。
　　<5> 卫星仰角，00至90度。
　　<6> 卫星方位角，000至359度。实际值。
　　<7> 信噪比（C/No），00至99dB；无表未接收到讯号。
　　<8> 校验和。
　　注：每条语句最多包括四颗卫星的信息，每颗卫星的信息有四个数据项，即：卫星编号、卫星仰角、卫星方位角、信噪比。
*/
typedef struct{
    int ID;             //卫星编号，01至32
    int Elevation;      //卫星仰角，00至90度
    int Azimuth;        //卫星方位角，000至359度。实际值
    int SNR;            //信噪比（C/No），00至99dB；无表未接收到讯号。
}STLT_STATE;


typedef struct{
    char update;
    unsigned char stlt_in_view;
    //STLT_STATE stlt[13];         //编号,仰角,方位角,信噪比
}GSV_TYPE;


/*----------------------------------------------------------------------------*/
/*
　　GPRMC
　　推荐最小数据量的GPS信息（Recommended Minimum Specific GPS/TRANSIT Data）
　　$GPRMC,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>*<13><CR><LF>
　　<1> UTC（Coordinated Universal Time）时间，hhmmss（时分秒）格式
　　<2> 定位状态，A=有效定位，V=无效定位
　　<3> Latitude，纬度ddmm.mmmm（度分）格式（前导位数不足则补0）
　　<4> 纬度半球N（北半球）或S（南半球）
　　<5> Longitude，经度dddmm.mmmm（度分）格式（前导位数不足则补0）
　　<6> 经度半球E（东经）或W（西经）
　　<7> 地面速率（000.0~999.9节，Knot，前导位数不足则补0）
　　<8> 地面航向（000.0~359.9度，以真北为参考基准，前导位数不足则补0）
　　<9> UTC日期，ddmmyy（日月年）格式
　　<10> Magnetic Variation，磁偏角（000.0~180.0度，前导位数不足则补0）
　　<11> Declination，磁偏角方向，E（东）或W（西）
　　<12> Mode Indicator，模式指示（仅NMEA0183 3.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效）
　　<13> 校验和。
*/
typedef struct{
    char update;
    char *msg;      //msg[70+10];
}RMC_TYPE;


/*----------------------------------------------------------------------------*/
/*
　　地面速度信息(GPVTG)
　　$GPVTG,<1>,T,<2>,M,<3>,N,<4>,K,<5>*hh
　　<1> 以真北为参考基准的地面航向(000~359度，前面的0也将被传输)
　　<2> 以磁北为参考基准的地面航向(000~359度，前面的0也将被传输)
　　<3> 地面速率(000.0~999.9节，前面的0也将被传输)
　　<4> 地面速率(0000.0~1851.8公里/小时，前面的0也将被传输)
　　<5> 模式指示(仅NMEA0183 3.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效
*/
typedef struct{
    char update;
    int CourseOfTrueNorth;
    int CourseOfMagneticNorth;
    double Speed;
    char *msg;      //msg[34+10];
}VTG_TYPE;


/*----------------------------------------------------------------------------*/
/*
　　定位地理信息(GPGLL)
　　$GPGLL,<1>,<2>,<3>,<4>,<5>,<6>,<7>*hh
　　<1> 纬度，格式：ddmm.mmmm
　　<2> 南北半球 (N表示北半球；S表示南半球)
　　<3> 经度，格式：dddmm.mmmm
　　<4> 东西半球 (E表示东经；W表示西经)
　　<5> UTC时间，格式：hhmmss (000000.00~235959.99)
    <6> 定位状态 (A=有效定位，V=无效定位)
    <7> 模式指示（NMEA0183 3.00版本输出）A=自主定位，D=差分，E=估算，N=数据无效
*/
typedef struct{
    char update;
    char PositionFix;
    char *msg;      //msg[46+10];
}GLL_TYPE;

/*----------------------------------------------------------------------------*/
/*
　　GPZDA(输出UTC时间和日期信息)
　　$GPZDA,<1>,<2>,<3>,<4>,<5>*<6><CR><LF>
　　<1> 时间：hhmmss.ss——0000000.00～235959.99
　　<2> 日——00～31
　　<3> 月——00～12
　　<4> 年——0000～9999
　　<5> 地方时与世界时之差
　　<6> 校验和。
*/
typedef struct{
    char update;
    char TimeZone;
    char *msg;
}ZDA_TYPE;

/*----------------------------------------------------------------------------*/
typedef struct{
    GGA_TYPE GGA;    //全球定位数据
    GLL_TYPE GLL;    //大地坐标
    GSA_TYPE GSA;    //卫星PRN数据
    GSV_TYPE GSV;    //卫星状态
    RMC_TYPE RMC;    //运输定位数据
    VTG_TYPE VTG;    //地面速度信息
    ZDA_TYPE ZDA;    //输出UTC时间
}NMEA_INFO_TYPE;

/*----------------------------------------------------------------------------*/
typedef struct{
    double lat;         //纬度
    double lon;         //经度
    int speed;       //地面速率
    float altitude;    //海拔高度
    float course;      //地面航向
    double lat_correct; //校准纬度
    double lon_correct; //校准经度
}GPS_POS_TYPE;


typedef struct{
    bool pos_update;
    char located;                       //定位有效位，RMC字节中
    char gsa_locate_mode;               //定位模式，2D或3D
    int stlt_in_view;
    int stlt_used;
    tm time;
    GPS_POS_TYPE gps;
    double ele;                        //仰角
    double PDOP;                       //位置精度
    double HDOP;                       //水平精度
    double VDOP;                       //垂直精度
}GPS_INFO_TYPE;

/*----------------------------------------------------------------------------*/
int nmea_get_section(char *in, char *out, int seg);
bool GGA_Parse(GPS_INFO_TYPE *out_p, char *in_p);
bool GGA_Parse(GPS_INFO_TYPE *out_p, char *in_p);
bool RMC_Parse(GPS_INFO_TYPE *out_p, char *in_p);
bool GSA_Parse(GPS_INFO_TYPE *out_p, char *in_p);
bool GLL_Parse(GPS_INFO_TYPE *out_p, char *in_p);
bool VTG_Parse(GPS_INFO_TYPE *out_p, char *in_p);
bool ZDA_Parse(GPS_INFO_TYPE *out_p, char *in_p);


#endif


