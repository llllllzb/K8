#include "app_user.h"
#include "app_sys.h"
#include "app_kernal.h"
#include "app_port.h"
#include "app_param.h"
#include "app_net.h"
#include "app_task.h"
#include "app_rec.h"
#include "app_atcmd.h"
#include "adc.h"
/**************************/
void myAppConfig(void)
{
    //sysinfo.logmessage = DEBUG_ALL;
    portUartCfg(APPUSART1, 1, atCmdParserFunction);
    paramInit();
    portLowPowerCfg();
    createSystemTask(ledRunTask, 100);
    createSystemTask(outPutNodeCmd, 200);
    sysinfo.SystaskID = createSystemTask(taskRunInOneSecond, 1000);
    HAL_ADCEx_Calibration_Start(&hadc, ADC_SINGLE_ENDED);
}

void myAppRun(void)
{
    kernalRun();
    portPollUart();
    systemRequestTask();
    alarmUploadRequest();
}


void gsensorInMode5(void)
{
    static uint32_t befTick = 0, CurTick = 0;
    static uint8_t tapCnt = 0;
    static uint8_t flag = 0;

    CurTick = portGetDateTimeOfSeconds();
    if (CurTick < befTick)
    {
        CurTick += 86399;
    }
    if (flag == 0)
    {
        LogPrintf(DEBUG_FACTORY, "��ֹʱ��:%d\r\n", CurTick - befTick);
        if (CurTick - befTick >= 10)
        {
            //��ֹ����10�룬��������
            flag = 1;
        }
        befTick = CurTick % 86400;
    }
    else
    {
        tapCnt++;
        LogPrintf(DEBUG_ALL, "TapCnt=%d\r\n", tapCnt);
        if (CurTick - befTick <= 20)
        {
            //20��������𶯳���7�Σ�������Ϊ�������
            if (tapCnt >= 7)
            {

                flag = 0;
                tapCnt = 0;
                befTick = CurTick % 86400;
                LogMessage(DEBUG_ALL, "�����ϱ�\r\n");
                if (sysparam.MODE == MODE5 && sysinfo.runFsm == MODE_DONE)
                {
                    gpsRequestSet(GPS_REQUEST_UPLOAD_ONE);
                    //�����ϱ�
                }
            }
        }
        else
        {

            flag = 0;
            tapCnt = 0;
            befTick = CurTick % 86400;
            LogMessage(DEBUG_ALL, "δ�ﵽ����\r\n");
        }
    }
}
#define MAX_TICK_CNT	10
void tapIntInMode5(void)
{
    static uint32_t tickTime[MAX_TICK_CNT] = {0};
    static uint8_t ind = 0;
    uint32_t nowTick;
    uint8_t i, now, nowind, bef, staticind, flag = 0, cnt;
    if (sysparam.MODE != MODE5)
        return;
    now = ind;
    nowind = now;
    tickTime[ind++] = portGetDateTimeOfSeconds();
    ind %= MAX_TICK_CNT;
    //���Ҽ�¼����û�о�ֹ����10���
    for (i = 0; i < MAX_TICK_CNT - 1; i++)
    {
        bef = (now + MAX_TICK_CNT - 1) % MAX_TICK_CNT;
        nowTick = tickTime[now];
        if (nowTick < tickTime[bef])
        {
            nowTick += 86400;
        }
        if (nowTick - tickTime[bef] >= 60)
        {
            //LogPrintf(DEBUG_ALL, "��ֹʱ��:%d\r\n", nowTick - tickTime[bef]);
            staticind = now;
            flag = 1;
            break;
        }
        now = bef;
    }
    if (flag)
    {
        if (nowind >= staticind)
        {
            cnt = nowind - staticind + 1;
        }
        else
        {
            cnt = nowind + MAX_TICK_CNT  - staticind + 1;
        }

        //LogPrintf(DEBUG_ALL, "������%d\r\n", cnt);

        if (cnt >= 7)
        {
            if (gpsRequestGet(GPS_REQUEST_UPLOAD_ONE) == 0)
            {
                LogMessage(DEBUG_ALL, "�𶯴���\r\n");
                gpsRequestSet(GPS_REQUEST_UPLOAD_ONE);
            }
        }
    }
    if (sysinfo.runFsm == MODE_DONE && sysinfo.GPSRequest == 0 && sysinfo.alarmrequest == 0)
    {
        systemRequestSet(SYSTEM_ENTERSLEEP_REQUEST);
    }
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GSINT_Pin)
    {
        portGsensorInt();
        tapIntInMode5();
    }
    else if (GPIO_Pin == LDR_Pin)
    {
        portLdrInt();
    }
    else if (GPIO_Pin == RI_Pin)
    {
        portRingSignal();
    }
}



