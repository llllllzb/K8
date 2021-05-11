#include "app_gps.h"
#include "app_param.h"
#include "app_task.h"
#include "app_sys.h"


static GPSFIFO gpsfifo;
static analysis_typedef analysis_data;


void addGPSDataToAnalyze(GPSINFO *gpsinfo);

/*
初始化GPS信息队列
*/
void GPSFifoInit(void)
{
    memset(&gpsfifo,0,sizeof(GPSFIFO));
}

//返回0则过滤
uint8_t  gpsFilter(GPSINFO * gpsinfo)
{
    if(gpsinfo->fixstatus==0)
        return 0;
    if(gpsinfo->fixmode!=3)
        return 0;
    if(gpsinfo->pdop>(sysparam.pdop/100.0))
        return 0;
    return 1;
}
void updateLocalRTCTime(DATETIME * datetime)
{
    DATETIME localtime;
    localtime=changeUTCTimeToLocalTime(*datetime,sysparam.utc);
    updateRTCdatetime(localtime.year,localtime.month,localtime.day,localtime.hour,localtime.minute,localtime.second);

    if(sysparam.MODE==MODE1 || sysparam.MODE==MODE21)
    {
        setNextAlarmTime();
    }
    else if(sysparam.MODE==MODE3|| sysparam.MODE==MODE23)
    {
        setNextWakeUpTime();
    }
};
void addNewGPSInfo(GPSINFO *gpsinfo)
{

    //将添加进来的数据复制到队列中

    if(gpsinfo->fixstatus==1)
    {
        //addGPSDataToAnalyze(gpsinfo);
        updateSystemLedStatus(SYSTEM_LED_GPSOK,1);
        terminalGPSFixed();
        if(sysinfo.localrtchadupdate==0)
        {

            if(gpsinfo->datetime.year!=0 && gpsinfo->datetime.month!=0 &&gpsinfo->datetime.day!=0)
            {
                sysinfo.localrtchadupdate=1;
                updateLocalRTCTime(&gpsinfo->datetime);
            }
        }
    }
    else
    {
        updateSystemLedStatus(SYSTEM_LED_GPSOK,0);
        terminalGPSUnFixed();
    }
    if(gpsFilter(gpsinfo)==0)
    {
        gpsinfo->fixstatus=0;
    }
    else
    {
        memcpy(&gpsfifo.lastfixgpsinfo,gpsinfo,sizeof(GPSINFO));
    }
    gpsfifo.currentindex=(gpsfifo.currentindex+1)%GPSFIFOSIZE;
    memcpy(&gpsfifo.gpsinfohistory[gpsfifo.currentindex],gpsinfo,sizeof(GPSINFO));
    memcpy(&gpsfifo.currentgpsinfo,gpsinfo,sizeof(GPSINFO));

}

void gpsClearCurrentGPSInfo(void)
{
    gpsfifo.currentgpsinfo.fixstatus=0;
    memset(gpsfifo.currentgpsinfo.gpsCn,0,sizeof(gpsfifo.currentgpsinfo.gpsCn));
    memset(gpsfifo.currentgpsinfo.beidouCn,0,sizeof(gpsfifo.currentgpsinfo.beidouCn));
    terminalGPSUnFixed();
}
GPSINFO *getCurrentGPSInfo(void)
{
    return &gpsfifo.currentgpsinfo;
}

GPSINFO *getLastFixedGPSInfo(void)
{
    return &gpsfifo.lastfixgpsinfo;
}

GPSFIFO *getGSPfifo(void)
{
    return &gpsfifo;
}



void ayalysisDataInit(void)
{
    uint8_t i;
    for(i=0; i<ANALY_LEN; i++)
        analysis_data.ayaly_fifo[i].time=0;
}

static void analyzeGPSData(void)
{
    uint8_t begin,before,i;
    float speed=0;
    uint8_t gap_time=0;
    uint16_t angle;
    float acclerate_speed=0;

    begin=(analysis_data.index+1)%ANALY_LEN;//得到5秒前的点
    before=(analysis_data.index+ANALY_LEN-1)%ANALY_LEN;//得到一秒前的点

    if(analysis_data.ayaly_fifo[analysis_data.index].time-analysis_data.ayaly_fifo[begin].time>6)
    {
        return;
    }
    /********************************************判断急左转 or 急右转*****************************************************/
    for(i=0; i<ANALY_LEN; i++)
    {
        speed+=analysis_data.ayaly_fifo[(analysis_data.index+i)%ANALY_LEN].speed;
    }
    speed/=ANALY_LEN;
    //LogPrintf(DEBUG_ALL,"均速:%d km/h\r\n",speed);
    angle=abs(analysis_data.ayaly_fifo[analysis_data.index].angle-analysis_data.ayaly_fifo[begin].angle);
    if(angle>=70 && angle <=120 && speed >=28)
    {
        LogMessage(DEBUG_ALL,"转弯\n");
        ayalysisDataInit();
        if(analysis_data.ayaly_fifo[analysis_data.index].angle>=270 && analysis_data.ayaly_fifo[analysis_data.index].angle<360 \
                && analysis_data.ayaly_fifo[begin].angle<=90)
        {
            //左转
            alarmRequestSet(ALARM_RAPIDLEFT_REQUEST);
        }
        else if(analysis_data.ayaly_fifo[begin].angle>=270 && analysis_data.ayaly_fifo[begin].angle<360 \
                && analysis_data.ayaly_fifo[analysis_data.index].angle<=90)
        {
            //右转
            alarmRequestSet(ALARM_RAPIDRIGHT_REQUEST);
        }
        else
        {
            angle=analysis_data.ayaly_fifo[analysis_data.index].angle-analysis_data.ayaly_fifo[begin].angle;
            if(angle>0)
            {
                //右转
                alarmRequestSet(ALARM_RAPIDRIGHT_REQUEST);
            }
            else
            {
                //左转
                alarmRequestSet(ALARM_RAPIDLEFT_REQUEST);
            }
        }
        return ;
    }
    /********************************************判断急加速 or 急减速*****************************************************/
    gap_time=analysis_data.ayaly_fifo[analysis_data.index].time-analysis_data.ayaly_fifo[before].time;//时间间隔
    speed=analysis_data.ayaly_fifo[analysis_data.index].speed-analysis_data.ayaly_fifo[before].speed; //速度差
    acclerate_speed=speed/gap_time;//加速度
    if(acclerate_speed==0)
        return;
    //LogPrintf(DEBUG_ALL,"加速度=%.2f\n",acclerate_speed);
    if(acclerate_speed>0)
    {
        if(acclerate_speed>=9)
        {
            //LogMessage(DEBUG_ALL,"急加速\n");
            alarmRequestSet(ALARM_ACCLERATE_REQUEST);
        }

    }
    else
    {
        acclerate_speed *=(-1);
        if(acclerate_speed>=20)
        {
            //LogMessage(DEBUG_ALL,"急减速\n");
            alarmRequestSet(ALARM_DECELERATE_REQUEST);
        }
    }
}

void addGPSDataToAnalyze(GPSINFO *gpsinfo)
{
    if(gpsinfo->speed<=7.0)
        return ;
    analysis_data.index=(analysis_data.index+1)%ANALY_LEN;
    analysis_data.ayaly_fifo[analysis_data.index].speed=gpsinfo->speed;
    analysis_data.ayaly_fifo[analysis_data.index].time=sysinfo.System_Tick;
    analysis_data.ayaly_fifo[analysis_data.index].angle=gpsinfo->course;
    analyzeGPSData();
}

