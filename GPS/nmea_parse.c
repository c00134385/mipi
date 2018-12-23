/*----------------------------------------------------------------------------*/
/* nmea_parse.c                                                               */
/*                                                                            */
/* 解析NMEA协议,将数据从ASCII转成可运算的变量                                 */
/*----------------------------------------------------------------------------*/
/* 2014/5/20 manwjh 建立 V0.1                                                 */
/*                                                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/* include */



/* external */


/* public */
NMEA_INFO_TYPE g_NMEAInfo;      //gps nmea格式信息


/* private */


/* debug */



/*----------------------------------------------------------------------------*/
/*                                  Functions                                 */
/*----------------------------------------------------------------------------*/
int nmea_get_section(char *out, char *in, int seg)
{
    int i=0,j=0;
    
    do{
        if( *in==',' ){
            j++;
            if(j>seg)
                break;
        }else{
            if( j==seg){
                i++;
                *out++ = *in;
            }
        }
    }while( *in++!= '\0' );
	
	*out = '\0';

    return i;
}


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
//仅取海拔值
/*----------------------------------------------------------------------------*/
bool GGA_Parse(GPS_INFO_TYPE *out_p, char *in_p)
{
        
    char c_tmp[100];
	int len;
    bool rslt = false;
    
    //定位状态
    if( nmea_get_section(c_tmp,in_p,6)==1 ){
        g_NMEAInfo.GGA.PositionFix = c_tmp[0];
        //1:GPS SPS模式,有效 2:差分GPS/SPS模式,有效 6:航位推算模式,有效
        if( (c_tmp[0]=='1')||(c_tmp[0]=='2')||(c_tmp[0]=='6') ){
            //<11> 大地椭球面相对海平面的高度（-999.9到9999.9）。
            if( nmea_get_section(c_tmp,in_p,11)>0 ){        //获取的section如果长度是0则说明是没有信息。
                out_p->gps.altitude = atof(c_tmp);
            }
        }
        rslt = true;
    }
    //使用卫星数量，从00到12
	len = nmea_get_section(c_tmp,in_p,7);
    if( len>0 ){
        g_NMEAInfo.GGA.SatellitesUsed = DecStr2Int(c_tmp,len);
        out_p->stlt_used = g_NMEAInfo.GGA.SatellitesUsed;
        rslt = true;
    }
    
    return rslt;
}



/*----------------------------------------------------------------------------*/
/*
　　GPRMC
　　推荐最小数据量的GPS信息（Recommended Minimum Specific GPS/TRANSIT Data）
　　$GPRMC,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>*<13><CR><LF>
　　<1> UTC（Coordinated Universal Time）时间，hhmmss.ss（时分秒）格式
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
/*----------------------------------------------------------------------------*/
bool RMC_Parse(GPS_INFO_TYPE *out_p, char *in_p)
{
    char c_tmp[100];
    char d[4];
    char m[10];
    int err_flg=0;
    
    //定位状态
    if( nmea_get_section(c_tmp,in_p,2)==1 ){
        if( (c_tmp[0]=='A')||(c_tmp[0]=='V') ){
            out_p->located = c_tmp[0];
        }else
            err_flg=1;
    }else
        err_flg=1;

    //时间hhmmss（时分秒）格式
    if( (err_flg==0)&&(nmea_get_section(c_tmp,in_p,1)>=9) ){
        out_p->time.hour = DecStr2Int(&c_tmp[0],2);
        out_p->time.min = DecStr2Int(&c_tmp[2],2);
        out_p->time.sec = DecStr2Int(&c_tmp[4],2);
        out_p->time.tick = out_p->time.hour*3600+out_p->time.min*60+out_p->time.sec;
    }else
        err_flg++;
    //日期
    if( (err_flg==0)&&(nmea_get_section(c_tmp,in_p,9)>=6) ){
        out_p->time.mday = DecStr2Int(&c_tmp[0],2);
        out_p->time.month = DecStr2Int(&c_tmp[2],2);
        out_p->time.year = DecStr2Int(&c_tmp[4],2);
        out_p->time.year += 2000;
    }else
        err_flg++;
    
    //时间转换成当地时间
    if(err_flg==0){
        UTCTime2LocalTime(&out_p->time,&out_p->time,8);
        //如果定位有效，获取坐标
        if( out_p->located=='V' )
            err_flg++;
    }

    //<3> Latitude，纬度ddmm.mmmm（度分格式）
    if( (err_flg==0)&&(nmea_get_section(c_tmp,in_p,3)>=9) ){
        memcpy(d,c_tmp,2);  d[2]='\0';
        memcpy(m,&c_tmp[2],7);  m[7]='\0';
        out_p->gps.lat = atof(d)+atof(m)/(double)60;
    }else
        err_flg++;
    //<4> 纬度半球N（北半球）或S（南半球）
    if( (err_flg==0)&&(nmea_get_section(c_tmp,in_p,4)==1) ){
        if(c_tmp[0]=='S')   //南半球为负数
            out_p->gps.lat = 0-out_p->gps.lat;
    }else
        err_flg++;
    //<5> Longitude，经度dddmm.mmmm（度分格式）
    if( (err_flg==0)&&(nmea_get_section(c_tmp,in_p,5)>=10) ){
        memcpy(d,c_tmp,3);  d[3]='\0';
        memcpy(m,&c_tmp[3],7);  m[7]='\0';
        out_p->gps.lon = atof(d)+atof(m)/(double)60;
    }else
        err_flg++;
    //<6> 经度半球E（东经）或W（西经）
    if( (err_flg==0)&&(nmea_get_section(c_tmp,in_p,4)==1) ){
        if(c_tmp[0]=='W')   //西半球为负数
            out_p->gps.lon = 0-out_p->gps.lon;
    }else
        err_flg++;

    //<7> 地面速率（000.0~999.9节，Knot，前导位数不足则补0）
    if( (err_flg==0)&&(nmea_get_section(c_tmp,in_p,7)>0) ){
        c_tmp[5] = '\0';
        out_p->gps.speed = atof(c_tmp)*1.609;    //转换成"公里/小时"
    }else
        err_flg++;

    //<8> 地面航向（000.0~359.9度，以真北为参考基准，前导位数不足则补0）
    if( (err_flg==0)&&(nmea_get_section(c_tmp,in_p,8)>0) ){
        c_tmp[5] = '\0';
        out_p->gps.course = atof(c_tmp);
        //纠偏
    }

    if( err_flg==0 ){
#if 1
        transform(out_p->gps.lat,out_p->gps.lon, &out_p->gps.lat_correct, &out_p->gps.lon_correct);
#else
        out_p->lat_correct = out_p->lat;
        out_p->lon_correct = out_p->lon;
#endif
        out_p->pos_update = true;  //有效坐标更新
        
        return true;
    }
    
    out_p->pos_update = false;  //有效坐标更新

    return false;
}

/*----------------------------------------------------------------------------*/
/*
　　地面速度信息(GPVTG)
　　$GPVTG,<1>,<T>,<3>,<M>,<5>,<N>,<7>,K,<9>*hh
　　<1> 以真北为参考基准的地面航向(000~359度，前面的0也将被传输)
　　<3> 以磁北为参考基准的地面航向(000~359度，前面的0也将被传输)
　　<5> 地面速率(000.0~999.9节，前面的0也将被传输)
　　<7> 地面速率(0000.0~1851.8公里/小时，前面的0也将被传输)
　　<9> 模式指示(仅NMEA0183 3.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效
*/
/*----------------------------------------------------------------------------*/
bool VTG_Parse(GPS_INFO_TYPE *out_p, char *in_p)
{
    char c_tmp[10];
    bool rslt=false;
    
    //定位状态
    if( nmea_get_section(c_tmp,in_p,1)>0 ){
        g_NMEAInfo.VTG.CourseOfTrueNorth = DecStr2Int( c_tmp,3 );
		if( nmea_get_section(c_tmp,in_p,3)>0 ){
        	g_NMEAInfo.VTG.CourseOfMagneticNorth = DecStr2Int( c_tmp,3 );
			if( nmea_get_section(c_tmp,in_p,7)>0 ){
        		g_NMEAInfo.VTG.Speed = atof(c_tmp);
				rslt = true;
    		}
		}
    }

	return rslt;	
}

/*----------------------------------------------------------------------------*/
/*
　　GPGSA
　　GPS精度指针及使用卫星格式
　　$GPGSA,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>,<13>,<14>,<15>,<16>,<17>*<18><CR><LF>
　　<1> 模式2：M = 手动， A = 自动。
　　<2> 模式1：定位型式1 = 未定位，2 = 二维定位，3 = 三维定位。
　　<3> 第1信道正在使用的卫星PRN码编号
       （Pseudo Random Noise，伪随机噪声码）,01至32（前导位数不足则补0，最多可接收12颗卫星信息）
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
/*----------------------------------------------------------------------------*/
bool GSA_Parse(GPS_INFO_TYPE *out_p, char *in_p)
{
    char c_tmp[10];
    
    //定位模式
    if( nmea_get_section(c_tmp,in_p,2)==1 ){
        if( (c_tmp[0]=='1')||(c_tmp[0]=='2')||(c_tmp[0]=='3') ){
            out_p->gsa_locate_mode = c_tmp[0];
            return true;
        }
    }
    return false;
}



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
/*----------------------------------------------------------------------------*/
bool GLL_Parse(GPS_INFO_TYPE *out_p, char *in_p)
{
    char c_tmp[10];
	bool rslt = false;
    
    //定位模式
    if( nmea_get_section(c_tmp,in_p,6)==1 ){
        if( (c_tmp[0]=='A')||(c_tmp[0]=='V') ){
            g_NMEAInfo.GLL.PositionFix = c_tmp[0];
            rslt = true;
        }
    }
    return rslt;
}




/*----------------------------------------------------------------------------*/
/*
　　GPGSV
　　可视卫星状态输出语句
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

    GSV_Parse仅统计搜索到卫星的数量
*/
bool GSV_Parse(GPS_INFO_TYPE *out_p, char *in_p)
{
    char c_tmp[100];
	int len;
    
    len = nmea_get_section(c_tmp,in_p,3);
    if( len>0 ){
        out_p->stlt_in_view = DecStr2Int(c_tmp,len);
        return true;
    }

    return false;
}

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
bool ZDA_Parse(GPS_INFO_TYPE *out_p, char *in_p)
{
    char c_tmp[100];
	int len;
	bool rslt = false;
    
    len = nmea_get_section(c_tmp,in_p,5);
    if(len>0){
        g_NMEAInfo.ZDA.TimeZone = DecStr2Int(c_tmp,len);
		rslt = true;
    }

	return rslt;
}


