#include "app_gps.h"
#include "app_param.h"
#include "app_task.h"
#include "app_sys.h"


static GPSFIFO gpsfifo;
static analysis_typedef analysis_data;


void addGPSDataToAnalyze(GPSINFO *gpsinfo);

/*
��ʼ��GPS��Ϣ����
*/
void GPSFifoInit(void)
{
    memset(&gpsfifo,0,sizeof(GPSFIFO));
}

//����0�����
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

    //����ӽ��������ݸ��Ƶ�������

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

    begin=(analysis_data.index+1)%ANALY_LEN;//�õ�5��ǰ�ĵ�
    before=(analysis_data.index+ANALY_LEN-1)%ANALY_LEN;//�õ�һ��ǰ�ĵ�

    if(analysis_data.ayaly_fifo[analysis_data.index].time-analysis_data.ayaly_fifo[begin].time>6)
    {
        return;
    }
    /********************************************�жϼ���ת or ����ת*****************************************************/
    for(i=0; i<ANALY_LEN; i++)
    {
        speed+=analysis_data.ayaly_fifo[(analysis_data.index+i)%ANALY_LEN].speed;
    }
    speed/=ANALY_LEN;
    //LogPrintf(DEBUG_ALL,"����:%d km/h\r\n",speed);
    angle=abs(analysis_data.ayaly_fifo[analysis_data.index].angle-analysis_data.ayaly_fifo[begin].angle);
    if(angle>=70 && angle <=120 && speed >=28)
    {
        LogMessage(DEBUG_ALL,"ת��\n");
        ayalysisDataInit();
        if(analysis_data.ayaly_fifo[analysis_data.index].angle>=270 && analysis_data.ayaly_fifo[analysis_data.index].angle<360 \
                && analysis_data.ayaly_fifo[begin].angle<=90)
        {
            //��ת
            alarmRequestSet(ALARM_RAPIDLEFT_REQUEST);
        }
        else if(analysis_data.ayaly_fifo[begin].angle>=270 && analysis_data.ayaly_fifo[begin].angle<360 \
                && analysis_data.ayaly_fifo[analysis_data.index].angle<=90)
        {
            //��ת
            alarmRequestSet(ALARM_RAPIDRIGHT_REQUEST);
        }
        else
        {
            angle=analysis_data.ayaly_fifo[analysis_data.index].angle-analysis_data.ayaly_fifo[begin].angle;
            if(angle>0)
            {
                //��ת
                alarmRequestSet(ALARM_RAPIDRIGHT_REQUEST);
            }
            else
            {
                //��ת
                alarmRequestSet(ALARM_RAPIDLEFT_REQUEST);
            }
        }
        return ;
    }
    /********************************************�жϼ����� or ������*****************************************************/
    gap_time=analysis_data.ayaly_fifo[analysis_data.index].time-analysis_data.ayaly_fifo[before].time;//ʱ����
    speed=analysis_data.ayaly_fifo[analysis_data.index].speed-analysis_data.ayaly_fifo[before].speed; //�ٶȲ�
    acclerate_speed=speed/gap_time;//���ٶ�
    if(acclerate_speed==0)
        return;
    //LogPrintf(DEBUG_ALL,"���ٶ�=%.2f\n",acclerate_speed);
    if(acclerate_speed>0)
    {
        if(acclerate_speed>=9)
        {
            //LogMessage(DEBUG_ALL,"������\n");
            alarmRequestSet(ALARM_ACCLERATE_REQUEST);
        }

    }
    else
    {
        acclerate_speed *=(-1);
        if(acclerate_speed>=20)
        {
            //LogMessage(DEBUG_ALL,"������\n");
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

