/*----------------------------------------------------------------------------*/
/* route_meter.c                                                              */
/*                                                                            */
/* 依据GPS信息，实时测量移动距离                                              */
/* 2014/5/30 manwjh 建立                                                      */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/* include */
#include "gps_algorithm.h"



/* external */
float abs(float __x);


/* public */
float DistanceCalcs(const double *lat1, const double *lon1, double *lat2, double *lon2);

/* private */
static double pi = 3.14159265358979324;

/* debug */


/*----------------------------------------------------------------------------*/
/*                                  Functions                                 */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* 函数名称: void DistanceCalcs(void)                                         */
/* 函数描述: 计算2个坐标之间的距离                                            */
/* 输入: (lat1,lon1) (lat2,lon2)                                              */
/* 输出: gRoute.distance,gRoute.total_distance                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
/*
附：C#代码：
private const double EARTH_RADIUS = 6378.137;//地球半径
private static double rad(double d)
{
   return d * Math.PI / 180.0;
}

public static double GetDistance(double lat1, double lng1, double lat2, double lng2)
{
   double radLat1 = rad(lat1);
   double radLat2 = rad(lat2);
   double a = radLat1 - radLat2;
   double b = rad(lng1) - rad(lng2);

   double s = 2 * Math.Asin(Math.Sqrt(Math.Pow(Math.Sin(a/2),2) +
    Math.Cos(radLat1)*Math.Cos(radLat2)*Math.Pow(Math.Sin(b/2),2)));
   s = s * EARTH_RADIUS;
   s = Math.Round(s * 10000) / 10000;
   return s;
}

http://www.cnblogs.com/ycsfwhh/archive/2010/12/20/1911232.html
*/
const double EARTH_RADIUS = 6378.137;   //地球半径
    
double rad(double d)
{
   return (d * pi) / 180.0;
}

float DistanceCalcs(const double *lat1, const double *lon1, double *lat2, double *lon2)
{
   float dis;
    
   double radLat1 = rad(*lat1);
   double radLat2 = rad(*lat2);
   double a = radLat1 - radLat2;
   double b = rad(*lon1) - rad(*lon2);

   double s = 2 * sin(sqrt(pow(sin(a/2),2) +
    cos(radLat1)*cos(radLat2)*pow(sin(b/2),2)));
    
   dis = (float)(s * EARTH_RADIUS * 1000);

   return dis;
}


/*----------------------------------------------------------------------------*/
/* 函数名称: void transform(double wgLat, double wgLon, double *lat, double *lon)*/
/* 函数描述: Gps坐标偏移校准算法                                              */
/*                                                                            */
/*----------------------------------------------------------------------------*/

/** 
GpsCorrect
 *http://blog.csdn.net/junfeng120125/article/details/9966857
 * gps纠偏算法，适用于google,高德体系的地图 
 * @author Administrator 
 */  
static double a = 6378245.0;  
static double ee = 0.00669342162296594323;  


static bool outOfChina(double lat, double lon)
{  
    if (lon < 72.004 || lon > 137.8347)  
        return true;  
    if (lat < 0.8293 || lat > 55.8271)  
        return true;  
    return false;  
}  

static double transformLat(double x, double y) {  
    double ret = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(abs(x));  
    ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;  
    ret += (20.0 * sin(y * pi) + 40.0 * sin(y / 3.0 * pi)) * 2.0 / 3.0;  
    ret += (160.0 * sin(y / 12.0 * pi) + 320 * sin(y * pi / 30.0)) * 2.0 / 3.0;  
    return ret;  
}  

static double transformLon(double x, double y) 
{
    double ret = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(abs(x));  
    ret += (20.0 * sin(6.0 * x * pi) + 20.0 * sin(2.0 * x * pi)) * 2.0 / 3.0;  
    ret += (20.0 * sin(x * pi) + 40.0 * sin(x / 3.0 * pi)) * 2.0 / 3.0;  
    ret += (150.0 * sin(x / 12.0 * pi) + 300.0 * sin(x / 30.0 * pi)) * 2.0 / 3.0;  
    return ret;  
}  


void transform(double wgLat, double wgLon, double *lat, double *lon) 
{
    double dLat,dLon,radLat,magic,sqrtMagic;
    
    if (outOfChina(wgLat, wgLon)) {  
        *lat = wgLat;
        *lon = wgLon;  
        return;  
    }  
    dLat = transformLat(wgLon - 105.0, wgLat - 35.0);  
    dLon = transformLon(wgLon - 105.0, wgLat - 35.0);  
    radLat = wgLat / 180.0 * pi;  
    magic = sin(radLat);  
    magic = 1 - ee * magic * magic;  
    sqrtMagic = sqrt(magic);  
    dLat = (dLat * 180.0) / ((a * (1 - ee)) / (magic * sqrtMagic) * pi);  
    dLon = (dLon * 180.0) / (a / sqrtMagic * cos(radLat) * pi);  

    *lat = wgLat + dLat;  
    *lon = wgLon + dLon;  
}  
