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

    getRtcDateTime(&year, &month, &date, &hour, &minute, &second);
    sprintf(param, "0,REC%.2d%.2d%.2d%.2d%.2d%.2d.amr,3", year % 100, month, date, hour, minute, second);
    sendModuleCmd(N58_RECMODE_CMD, param);
    sendModuleCmd(N58_RECF_CMD, "1");
    sysinfo.recordingflag = 1;
    LogMessage(DEBUG_ALL, "Start Record\n");
}

void recStop(void)
{
    sysinfo.recordingflag = 0;
    sendModuleCmd(N58_RECF_CMD, "0");
    LogMessage(DEBUG_ALL, "Stop Record\n");
}

void recStopAndUpload(void)
{
    rf.nofileflag = 0;
    recStop();
}

void recNoFileToread(void)
{
    LogMessage(DEBUG_ALL, "no rec file\n");
    rf.nofileflag = 1;
}

RECORDFILE *recGetRecStruct(void)
{
    return &rf;
}

void recUpdateFileName(char *filename)
{
    if (filename != NULL)
    {
        strcpy(rf.filename, filename);
    }
}

void recUpdateFileSize(uint32_t totalsize)
{
    char param[50];
    rf.totalsize = totalsize;
    rf.havereaddata = 0;
    rf.nofileflag = 0;

    if (rf.totalsize == 0)
    {
        if (rf.filename[0] != 0)
        {
            sprintf(param, "\"%s\"", rf.filename);
            sendModuleCmd(N58_FSDF_CMD, param);
        }
        return ;
    }

    if (rf.filename[0] != 0 && rf.totalsize != 0)
    {
        if (rf.fsmstate != FSM_RECORD_SEARCH)
            return ;
        rf.fsmstate = FSM_RECORD_READ;
    }

}
uint8_t recIsRun(void)
{
    if (rf.nofileflag == 0)
        return 1;
    return 0;
}
void recReadFileOk(void)
{
    rf.readfileok = 1;
}
//100ms

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
    if (rf.reccycletick == 60)
    {
        LogMessage(DEBUG_ALL, "Í£Ö¹Ñ­»·Â¼Òô\n");
        recStopAndUpload();
        sendModuleCmd(N58_FSLIST_CMD, NULL);
    }
    if (rf.reccycletick >= 62)
    {
        rf.reccycletick = 0;
    }
    if (rf.reccycletick == 0)
    {
        LogMessage(DEBUG_ALL, "¿ªÊ¼Ñ­»·Â¼Òô\n");
        recStart();
    }
    rf.reccycletick++;

}

void recRecordUploadTask(void)
{
    char param[100];
    if (isProtocolReday() == 0)
    {
        rf.fsmstate = FSM_RECORD_SEARCH;
        rf.waittick = 0;
        return;
    }
    if (rf.nofileflag == 1)
    {
        rf.fsmstate = FSM_RECORD_SEARCH;
        rf.waittick = 0;
        return ;
    }
    switch (rf.fsmstate)
    {
        case FSM_RECORD_SEARCH:
            if (++rf.waittick % 10 == 0)
            {
                sendModuleCmd(N58_FSLIST_CMD, NULL);
            }
            break;
        case FSM_RECORD_READ:
            sprintf(param, "0,\"%s\"", rf.filename);
            sendModuleCmd(N58_SNDTRANS_CMD, param);
            rf.fsmstate = FSM_RECORD_WAIT;
            rf.waittick = 0;
            break;
        case FSM_RECORD_WAIT:
            rf.waittick++;
            if (rf.waittick >= 10)
            {
                sprintf(param, "\"%s\"", rf.filename);
                sendModuleCmd(N58_FSDF_CMD, param);
                rf.fsmstate = FSM_RECORD_SEARCH;
                LogPrintf(DEBUG_ALL, "Delete file %s\n", rf.filename);
                if (rf.reccycle == 1)
                {
                    rf.nofileflag = 1;
                }
            }
            break;
        default:
            rf.fsmstate = FSM_RECORD_SEARCH;
            break;
    }
}




