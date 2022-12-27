#include "app_gpsarithmetic.h"
#include "app_param.h"
#include "app_serverprotocol.h"
#include "app_task.h"
#include <math.h>
LASTUPLOADGPSINFO lastuploadgpsinfo;

#define PIA 3.1415926
double calculateTheDistanceBetweenTwoPonits(double lat1, double lng1, double lat2, double lng2)
{
    double radLng1, radLng2;
    double a, b, s;
    radLng1 = lng1 * PIA / 180.0;
    radLng2 = lng2 * PIA / 180.0;
    a = (radLng1 - radLng2);
    b = (lat1 - lat2) * PIA / 180.0;
    s = 2 * asin(sqrt(sin(a / 2) * sin(a / 2) + cos(radLng1) * cos(radLng2) * sin(b / 2) * sin(b / 2))) * 6378.137;
    return s;
}

void initLastPoint(GPSINFO *gpsinfo)
{
    lastuploadgpsinfo.datetime = gpsinfo->datetime;
    lastuploadgpsinfo.latitude = gpsinfo->latitude;
    lastuploadgpsinfo.longtitude = gpsinfo->longtitude;
    lastuploadgpsinfo.gpsticksec = gpsinfo->gpsticksec;
    lastuploadgpsinfo.init = 1;
    LogMessage(DEBUG_ALL, "Update last position\n");
}

int8_t calculateDistanceOfPoint(void)
{
    GPSINFO *gpsinfo;
    double distance;
    char debug[100];
    gpsinfo = getCurrentGPSInfo();
    if (gpsinfo->fixstatus == 0)
    {
        return -1;
    }
    if (lastuploadgpsinfo.init == 0)
    {
        initLastPoint(gpsinfo);
        return -2;
    }

    distance = calculateTheDistanceBetweenTwoPonits(gpsinfo->latitude, gpsinfo->longtitude, lastuploadgpsinfo.latitude,
               lastuploadgpsinfo.longtitude) * 1000;
    sprintf(debug, "distance of point =%.2fm\n", distance);
    LogMessage(DEBUG_ALL, debug);
    if (distance > sysparam.fence)
    {
        return 1;
    }
    return 0;
}

int8_t calculateTheGPSCornerPoint(void)
{
    GPSFIFO *gpsfifo;
    char debug[30];
    uint16_t course[5];
    uint8_t positionidnex[5];
    uint16_t coursechange;
    uint8_t cur_index, i;

    if (sysparam.turnalg == 0)
    {
        return 0;
    }
    gpsfifo = getGSPfifo();
    //获取当前最新的gps索引
    cur_index = gpsfifo->currentindex;
    for (i = 0; i < 5; i++)
    {
        //过滤操作
        if (gpsfifo->gpsinfohistory[cur_index].fixstatus == 0 || gpsfifo->gpsinfohistory[cur_index].speed < 6.5)
        {
            return -1;
        }
        if (gpsfifo->gpsinfohistory[cur_index].hadupload == 1)
        {
            return -2;
        }
        //保存5个点的方向
        course[i] = gpsfifo->gpsinfohistory[cur_index].course;
        positionidnex[i] = cur_index;
        //获取上一条索引
        cur_index = (cur_index + GPSFIFOSIZE - 1) % GPSFIFOSIZE;
    }
    //计算转弯角度
    coursechange = abs(course[0] - course[4]);
    if (coursechange > 180)
    {
        coursechange = 360 - coursechange;
    }
    sprintf(debug, "Total course=%d\n", coursechange);
    LogMessage(DEBUG_ALL, debug);
    if (coursechange >= 15 && coursechange <= 45)
    {
        //sendProtocolToServer(PROTOCOL_12,&gpsfifo->gpsinfohistory[positionidnex[0]]);
        sendProtocolToServer(PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[1]]);
        //sendProtocolToServer(PROTOCOL_12,&gpsfifo->gpsinfohistory[positionidnex[2]]);
        sendProtocolToServer(PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[3]]);
        //sendProtocolToServer(PROTOCOL_12,&gpsfifo->gpsinfohistory[positionidnex[4]]);
    }
    else if (coursechange > 45)
    {
        sendProtocolToServer(PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[0]]);
        sendProtocolToServer(PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[1]]);
        sendProtocolToServer(PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[2]]);
        sendProtocolToServer(PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[3]]);
        sendProtocolToServer(PROTOCOL_12, &gpsfifo->gpsinfohistory[positionidnex[4]]);

    }
    return 1;
}

void gpsUploadPointToServer(void)
{
    static uint64_t tick = 0;
    if (sysinfo.GPSStatus == 0 ||  sysparam.gpsuploadgap == 0 || sysparam.gpsuploadgap >= GPS_UPLOAD_GAP_MAX)
    {
    	tick=0;
        return ;
    }
    tick++;
    calculateTheGPSCornerPoint();
    if (tick >= sysparam.gpsuploadgap)
    {
        if (calculateDistanceOfPoint() == 1)
        {
            sendProtocolToServer(PROTOCOL_12, getCurrentGPSInfo());
            initLastPoint(getCurrentGPSInfo());
            LogMessage(DEBUG_ALL, "gpsUploadPointToServer==>Upload point\n");
            tick = 0;
        }
    }
}


