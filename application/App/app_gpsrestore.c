#include "app_gpsrestore.h"
#include "app_net.h"
#include "app_sys.h"

static GPSReadFromModule  grfm;


/*��4Gģ����д�ļ�*/
void gpsRestoreWriteData(GPSRestoreStruct *gpsres)
{
    uint8_t restore[42];
    uint8_t *data;
    data=(uint8_t *)gpsres;
    sendModuleCmd(N58_FSWF_CMD,"\"gpssave.data\",1,20,1500");
    changeByteArrayToHexString(data,restore,20);
    restore[40]=0;
    LogMessage(DEBUG_ALL,"Save:");
    LogMessage(DEBUG_ALL,(char *)restore);
    LogMessage(DEBUG_ALL,"\n");
    CreateNodeCmd((char *)data,20,N58_FSWF_CMD);
}

/**********************************************************************/

/*��λ�ṹ�壬���¿�ʼ*/
void gpsReadRestoreReset(void)
{
    grfm.nofileflag=0;
    grfm.fsmstate=FSM_GPSRESTORE_SEARCH;
}

/*������Ҫ���ص��ļ����ļ���С*/
void gpsUpdateRestoreFileNameAndTotalSize(char *filename,uint32_t totalsize)
{
    if(filename!=NULL)
    {
        strcpy(grfm.filename,filename);
    }
    grfm.totalsize=totalsize;
    grfm.fsmstate=FSM_GPSRESTORE_READ;
    grfm.havereaddata=0;
    grfm.nofileflag=0;
}
uint8_t gpsIsRun(void)
{
	if(grfm.nofileflag==0)
		return 1;
	return 0;
}
void gpsNoRestoreFileToRead(void)
{
    grfm.nofileflag=1;
}

/*�˺�������������״̬�¿ɵ���*/
/*ִ�ж��ļ��Ĳ���������ƫ�Ƽ�¼�����ɾ���ļ�*/
void gpsReadFromModuleAndSendtoServe(void)
{
    uint32_t needreadsize;
    char param[100];
    char debug[100];
    if(grfm.nofileflag==1)
    {
        return ;
    }
    switch (grfm.fsmstate)
    {
    case FSM_GPSRESTORE_SEARCH:
        sendModuleCmd(N58_FSLIST_CMD,NULL);
        break;
    case FSM_GPSRESTORE_READ:
        needreadsize=grfm.totalsize-grfm.havereaddata;
        if (needreadsize==0)
        {
            LogMessage(DEBUG_ALL,"Read restore data complete\n\n");
            sprintf(param,"\"%s\"",grfm.filename);
            sendModuleCmd(N58_FSDF_CMD,param);
            grfm.fsmstate=FSM_GPSRESTORE_SEARCH;
            return ;
        }
        if(needreadsize>GPS_MAXREADRESTORESIZE)
        {
            needreadsize=GPS_MAXREADRESTORESIZE;
        }
        sprintf(debug,"GPS��%s�ļ���ƫ�� %d,��ȡ%d���ֽ�\n",grfm.filename,grfm.havereaddata,needreadsize);
        LogMessage(DEBUG_ALL,debug);
        sprintf(param,"\"%s\",1,%d,%d",grfm.filename,needreadsize,grfm.havereaddata);
        sendModuleCmd(N58_FSRF_CMD,param);
        grfm.havereaddata+=needreadsize;
        break;
    default:
        grfm.fsmstate=FSM_GPSRESTORE_SEARCH;
        break;
    }
}



