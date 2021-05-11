#include "app_rec.h"
#include "app_net.h"
#include "app_port.h"
#include "app_sys.h"
#include "app_serverprotocol.h"
RECORDFILE rf;

void recStart(void)
{
    char param[50];
    uint16_t year;
    uint8_t  month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    getRtcDateTime(&year,&month,&date,&hour,&minute,&second);
    sprintf(param,"0,REC%.2d%.2d%.2d%.2d%.2d%.2d.amr,3",year%100,month,date,hour,minute,second);
    sendModuleCmd(N58_RECMODE_CMD,param);
    sendModuleCmd(N58_RECF_CMD,"1");
    sysinfo.recordingflag=1;
    LogMessage(DEBUG_ALL,"Start Record\n");
}

void recStop(void)
{
    sysinfo.recordingflag=0;
    sendModuleCmd(N58_RECF_CMD,"0");
    LogMessage(DEBUG_ALL,"Stop Record\n");
}

void recStopAndUpload(void)
{
	rf.nofileflag = 0;
	recStop();
}

void recNoFileToread(void)
{
    rf.nofileflag=1;
}

RECORDFILE * recGetRecStruct(void)
{
    return &rf;
}

void recUpdateRestoreFileNameAndTotalSize(char *filename,uint32_t totalsize)
{
    char txdata[100];
    if(filename!=NULL)
    {
        strcpy(rf.filename,filename);
    }
    rf.totalsize=totalsize;
    rf.havereaddata = 0;
    rf.nofileflag = 0;
    if (rf.filename[0] != 0 && rf.totalsize != 0)
    {
        if (rf.fsmstate != FSM_RECORD_SEARCH)
            return ;
        rf.fsmstate = FSM_RECORD_READ;
        createProtocol61(txdata, rf.filename + 3, rf.totalsize, 0, RECORD_UPLOADONEPACKSIZE);
    }

}
uint8_t recIsRun(void)
{
    if(rf.nofileflag==0)
        return 1;
    return 0;
}
void recReadFileOk(void)
{
    rf.readfileok=1;
}
//100ms
void recordUploadRun(void)
{
    char param[100];
    char debug[200];
    if(isProtocolReday()==0 || gpsIsRun()==1)
    {
        rf.fsmstate=FSM_RECORD_SEARCH;
        rf.waittick=0;
        return;
    }
    if(rf.nofileflag==1)
    {
        return ;
    }
    switch (rf.fsmstate)
    {
    case FSM_RECORD_SEARCH:
        if(rf.waittick++ %5==0)
        {
            sendModuleCmd(N58_FSLIST_CMD,NULL);
        }
        break;
    case FSM_RECORD_READ:
        rf.needreadsize=rf.totalsize-rf.havereaddata;
        if (rf.needreadsize==0)
        {

            LogMessage(DEBUG_ALL,"Read restore data complete\n\n");
            sprintf(param,"\"%s\"",rf.filename);
            sendModuleCmd(N58_FSDF_CMD,param);
            rf.fsmstate=FSM_RECORD_SEARCH;

            return ;
        }
        if(rf.needreadsize>RECORD_MAXREADRESTORESIZE)
        {
            rf.needreadsize=RECORD_MAXREADRESTORESIZE;
        }
        sprintf(debug,"recordUploadRun==>从%s文件中偏移 %d,读取%d个字节\n",rf.filename,rf.havereaddata,rf.needreadsize);
        LogMessage(DEBUG_ALL,debug);
        sprintf(param,"\"%s\",1,%d,%d",rf.filename,rf.needreadsize,rf.havereaddata);
        rf.readfileok=0;
        rf.waittick=0;
        rf.fsmstate=FSM_RECORD_WAIT;
        sendModuleCmd(N58_FSRF_CMD,param);
        break;
    case FSM_RECORD_WAIT:
        rf.waittick++;
        if(rf.readfileok==1)
        {
            rf.havereaddata+=rf.needreadsize;
            rf.fsmstate=FSM_RECORD_READ;
        }
        else
        {
            if(rf.waittick>20)
            {
                rf.fsmstate=FSM_RECORD_READ;
            }

        }
        break;
    default:
        rf.fsmstate=FSM_RECORD_SEARCH;
        break;
    }
}

uint8_t resIsCycleRuning(void)
{
	return rf.reccycle;
}

void recCycleStart(void)
{
    rf.reccycle = 1;
}

void recCycleStop(void)
{
    if (rf.reccycle == 1)
    {
        rf.reccycle = 0;
        recStopAndUpload();
    }
}

void recCycleTask(void)
{
    if (rf.reccycle == 0)
    {
        rf.reccycletick = 0;
        return ;
    }
    if (rf.reccycletick >=60)
    {
        LogMessage(DEBUG_ALL, "停止循环录音\n");
        recStop();
        rf.reccycletick = 0;
    }
    if (rf.reccycletick == 0)
    {
        LogMessage(DEBUG_ALL, "开始循环录音\n");
        recStart();
    }
    rf.reccycletick++;

}

