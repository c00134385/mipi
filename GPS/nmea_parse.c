/*----------------------------------------------------------------------------*/
/* nmea_parse.c                                                               */
/*                                                                            */
/* ����NMEAЭ��,�����ݴ�ASCIIת�ɿ�����ı���                                 */
/*----------------------------------------------------------------------------*/
/* 2014/5/20 manwjh ���� V0.1                                                 */
/*                                                                            */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/* include */



/* external */


/* public */
NMEA_INFO_TYPE g_NMEAInfo;      //gps nmea��ʽ��Ϣ


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
����$GPGGA,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>,<13>,<14>*<15><CR><LF>
����<1> UTCʱ�䣬��ʽΪhhmmss.sss��
����<2> γ�ȣ���ʽΪddmm.mmmm��ǰ��λ��������0����
����<3> γ�Ȱ���N��S����γ����γ����
����<4> ���ȣ���ʽΪdddmm.mmmm��ǰ��λ��������0����
����<5> ���Ȱ���E��W����������������
����<6> ��λ����ָʾ��0=��λ��Ч��1=��λ��Ч��
����<7> ʹ��������������00��12��ǰ��λ��������0����
����<8> ˮƽ��ȷ�ȣ�0.5��99.9��
����<9> �����뺣ƽ��ĸ߶ȣ�-9999.9��9999.9��
����<10> �߶ȵ�λ��M��ʾ��λ�ס�
����<11> �����������Ժ�ƽ��ĸ߶ȣ�-999.9��9999.9����
����<12> �߶ȵ�λ��M��ʾ��λ�ס�
����<13> ���GPS�������ޣ�RTCM SC-104�����������RTCM���͵���������
����<14> ��ֲο���վ��ţ���0000��1023��ǰ��λ��������0����
����<15> У��͡�
*/
//��ȡ����ֵ
/*----------------------------------------------------------------------------*/
bool GGA_Parse(GPS_INFO_TYPE *out_p, char *in_p)
{
        
    char c_tmp[100];
	int len;
    bool rslt = false;
    
    //��λ״̬
    if( nmea_get_section(c_tmp,in_p,6)==1 ){
        g_NMEAInfo.GGA.PositionFix = c_tmp[0];
        //1:GPS SPSģʽ,��Ч 2:���GPS/SPSģʽ,��Ч 6:��λ����ģʽ,��Ч
        if( (c_tmp[0]=='1')||(c_tmp[0]=='2')||(c_tmp[0]=='6') ){
            //<11> �����������Ժ�ƽ��ĸ߶ȣ�-999.9��9999.9����
            if( nmea_get_section(c_tmp,in_p,11)>0 ){        //��ȡ��section���������0��˵����û����Ϣ��
                out_p->gps.altitude = atof(c_tmp);
            }
        }
        rslt = true;
    }
    //ʹ��������������00��12
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
����GPRMC
�����Ƽ���С��������GPS��Ϣ��Recommended Minimum Specific GPS/TRANSIT Data��
����$GPRMC,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>*<13><CR><LF>
����<1> UTC��Coordinated Universal Time��ʱ�䣬hhmmss.ss��ʱ���룩��ʽ
����<2> ��λ״̬��A=��Ч��λ��V=��Ч��λ
����<3> Latitude��γ��ddmm.mmmm���ȷ֣���ʽ��ǰ��λ��������0��
����<4> γ�Ȱ���N�������򣩻�S���ϰ���
����<5> Longitude������dddmm.mmmm���ȷ֣���ʽ��ǰ��λ��������0��
����<6> ���Ȱ���E����������W��������
����<7> �������ʣ�000.0~999.9�ڣ�Knot��ǰ��λ��������0��
����<8> ���溽��000.0~359.9�ȣ����汱Ϊ�ο���׼��ǰ��λ��������0��
����<9> UTC���ڣ�ddmmyy�������꣩��ʽ
����<10> Magnetic Variation����ƫ�ǣ�000.0~180.0�ȣ�ǰ��λ��������0��
����<11> Declination����ƫ�Ƿ���E��������W������
����<12> Mode Indicator��ģʽָʾ����NMEA0183 3.00�汾�����A=������λ��D=��֣�E=���㣬N=������Ч��
����<13> У��͡�
*/
/*----------------------------------------------------------------------------*/
bool RMC_Parse(GPS_INFO_TYPE *out_p, char *in_p)
{
    char c_tmp[100];
    char d[4];
    char m[10];
    int err_flg=0;
    
    //��λ״̬
    if( nmea_get_section(c_tmp,in_p,2)==1 ){
        if( (c_tmp[0]=='A')||(c_tmp[0]=='V') ){
            out_p->located = c_tmp[0];
        }else
            err_flg=1;
    }else
        err_flg=1;

    //ʱ��hhmmss��ʱ���룩��ʽ
    if( (err_flg==0)&&(nmea_get_section(c_tmp,in_p,1)>=9) ){
        out_p->time.hour = DecStr2Int(&c_tmp[0],2);
        out_p->time.min = DecStr2Int(&c_tmp[2],2);
        out_p->time.sec = DecStr2Int(&c_tmp[4],2);
        out_p->time.tick = out_p->time.hour*3600+out_p->time.min*60+out_p->time.sec;
    }else
        err_flg++;
    //����
    if( (err_flg==0)&&(nmea_get_section(c_tmp,in_p,9)>=6) ){
        out_p->time.mday = DecStr2Int(&c_tmp[0],2);
        out_p->time.month = DecStr2Int(&c_tmp[2],2);
        out_p->time.year = DecStr2Int(&c_tmp[4],2);
        out_p->time.year += 2000;
    }else
        err_flg++;
    
    //ʱ��ת���ɵ���ʱ��
    if(err_flg==0){
        UTCTime2LocalTime(&out_p->time,&out_p->time,8);
        //�����λ��Ч����ȡ����
        if( out_p->located=='V' )
            err_flg++;
    }

    //<3> Latitude��γ��ddmm.mmmm���ȷָ�ʽ��
    if( (err_flg==0)&&(nmea_get_section(c_tmp,in_p,3)>=9) ){
        memcpy(d,c_tmp,2);  d[2]='\0';
        memcpy(m,&c_tmp[2],7);  m[7]='\0';
        out_p->gps.lat = atof(d)+atof(m)/(double)60;
    }else
        err_flg++;
    //<4> γ�Ȱ���N�������򣩻�S���ϰ���
    if( (err_flg==0)&&(nmea_get_section(c_tmp,in_p,4)==1) ){
        if(c_tmp[0]=='S')   //�ϰ���Ϊ����
            out_p->gps.lat = 0-out_p->gps.lat;
    }else
        err_flg++;
    //<5> Longitude������dddmm.mmmm���ȷָ�ʽ��
    if( (err_flg==0)&&(nmea_get_section(c_tmp,in_p,5)>=10) ){
        memcpy(d,c_tmp,3);  d[3]='\0';
        memcpy(m,&c_tmp[3],7);  m[7]='\0';
        out_p->gps.lon = atof(d)+atof(m)/(double)60;
    }else
        err_flg++;
    //<6> ���Ȱ���E����������W��������
    if( (err_flg==0)&&(nmea_get_section(c_tmp,in_p,4)==1) ){
        if(c_tmp[0]=='W')   //������Ϊ����
            out_p->gps.lon = 0-out_p->gps.lon;
    }else
        err_flg++;

    //<7> �������ʣ�000.0~999.9�ڣ�Knot��ǰ��λ��������0��
    if( (err_flg==0)&&(nmea_get_section(c_tmp,in_p,7)>0) ){
        c_tmp[5] = '\0';
        out_p->gps.speed = atof(c_tmp)*1.609;    //ת����"����/Сʱ"
    }else
        err_flg++;

    //<8> ���溽��000.0~359.9�ȣ����汱Ϊ�ο���׼��ǰ��λ��������0��
    if( (err_flg==0)&&(nmea_get_section(c_tmp,in_p,8)>0) ){
        c_tmp[5] = '\0';
        out_p->gps.course = atof(c_tmp);
        //��ƫ
    }

    if( err_flg==0 ){
#if 1
        transform(out_p->gps.lat,out_p->gps.lon, &out_p->gps.lat_correct, &out_p->gps.lon_correct);
#else
        out_p->lat_correct = out_p->lat;
        out_p->lon_correct = out_p->lon;
#endif
        out_p->pos_update = true;  //��Ч�������
        
        return true;
    }
    
    out_p->pos_update = false;  //��Ч�������

    return false;
}

/*----------------------------------------------------------------------------*/
/*
���������ٶ���Ϣ(GPVTG)
����$GPVTG,<1>,<T>,<3>,<M>,<5>,<N>,<7>,K,<9>*hh
����<1> ���汱Ϊ�ο���׼�ĵ��溽��(000~359�ȣ�ǰ���0Ҳ��������)
����<3> �Դű�Ϊ�ο���׼�ĵ��溽��(000~359�ȣ�ǰ���0Ҳ��������)
����<5> ��������(000.0~999.9�ڣ�ǰ���0Ҳ��������)
����<7> ��������(0000.0~1851.8����/Сʱ��ǰ���0Ҳ��������)
����<9> ģʽָʾ(��NMEA0183 3.00�汾�����A=������λ��D=��֣�E=���㣬N=������Ч
*/
/*----------------------------------------------------------------------------*/
bool VTG_Parse(GPS_INFO_TYPE *out_p, char *in_p)
{
    char c_tmp[10];
    bool rslt=false;
    
    //��λ״̬
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
����GPGSA
����GPS����ָ�뼰ʹ�����Ǹ�ʽ
����$GPGSA,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>,<13>,<14>,<15>,<16>,<17>*<18><CR><LF>
����<1> ģʽ2��M = �ֶ��� A = �Զ���
����<2> ģʽ1����λ��ʽ1 = δ��λ��2 = ��ά��λ��3 = ��ά��λ��
����<3> ��1�ŵ�����ʹ�õ�����PRN����
       ��Pseudo Random Noise��α��������룩,01��32��ǰ��λ��������0�����ɽ���12��������Ϣ��
����<4> ��2�ŵ�����ʹ�õ�����PRN����
����<5> ��3�ŵ�����ʹ�õ�����PRN����
����<6> ��4�ŵ�����ʹ�õ�����PRN����
����<7> ��5�ŵ�����ʹ�õ�����PRN����
����<8> ��6�ŵ�����ʹ�õ�����PRN����
����<9> ��7�ŵ�����ʹ�õ�����PRN����
����<10> ��8�ŵ�����ʹ�õ�����PRN����
����<11> ��9�ŵ�����ʹ�õ�����PRN����
����<12> ��10�ŵ�����ʹ�õ�����PRN����
����<13> ��11�ŵ�����ʹ�õ�����PRN����
����<14> ��12�ŵ�����ʹ�õ�����PRN����
����<15> PDOP�ۺ�λ�þ������ӣ�0.5 - 99.9��
����<16> HDOPˮƽ�������ӣ�0.5 - 99.9��
����<17> VDOP��ֱ�������ӣ�0.5 - 99.9��
����<18> У���
*/
/*----------------------------------------------------------------------------*/
bool GSA_Parse(GPS_INFO_TYPE *out_p, char *in_p)
{
    char c_tmp[10];
    
    //��λģʽ
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
������λ������Ϣ(GPGLL)
����$GPGLL,<1>,<2>,<3>,<4>,<5>,<6>,<7>*hh
����<1> γ�ȣ���ʽ��ddmm.mmmm
����<2> �ϱ����� (N��ʾ������S��ʾ�ϰ���)
����<3> ���ȣ���ʽ��dddmm.mmmm
����<4> �������� (E��ʾ������W��ʾ����)
����<5> UTCʱ�䣬��ʽ��hhmmss (000000.00~235959.99)
    <6> ��λ״̬ (A=��Ч��λ��V=��Ч��λ)
    <7> ģʽָʾ��NMEA0183 3.00�汾�����A=������λ��D=��֣�E=���㣬N=������Ч
*/
/*----------------------------------------------------------------------------*/
bool GLL_Parse(GPS_INFO_TYPE *out_p, char *in_p)
{
    char c_tmp[10];
	bool rslt = false;
    
    //��λģʽ
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
����GPGSV
������������״̬������
����$GPGSV, <1>,<2>,<3>,<4>,<5>,<6>,<7>,...,<4>,<5>,<6>,<7>*<8><CR><LF>
����<1> �ܵ�GSV����������
����<2> ��ǰGSV���š�
����<3> ��������������00��12��
����<4> ���Ǳ�ţ�01��32��
����<5> �������ǣ�00��90�ȡ�
����<6> ���Ƿ�λ�ǣ�000��359�ȡ�ʵ��ֵ��
����<7> ����ȣ�C/No����00��99dB���ޱ�δ���յ�Ѷ�š�
����<8> У��͡�
����ע��ÿ������������Ŀ����ǵ���Ϣ��ÿ�����ǵ���Ϣ���ĸ�������������Ǳ�š��������ǡ����Ƿ�λ�ǡ�����ȡ�

    GSV_Parse��ͳ�����������ǵ�����
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
����GPZDA(���UTCʱ���������Ϣ)
����$GPZDA,<1>,<2>,<3>,<4>,<5>*<6><CR><LF>
����<1> ʱ�䣺hhmmss.ss����0000000.00��235959.99
����<2> �ա���00��31
����<3> �¡���00��12
����<4> �ꡪ��0000��9999
����<5> �ط�ʱ������ʱ֮��
����<6> У��͡�
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


