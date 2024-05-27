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
	memset(&sysinfo, 0, sizeof(sysinfo));
    //sysinfo.logmessage = DEBUG_ALL;
    portUartCfg(APPUSART1, 1, atCmdParserFunction);
    paramInit();
    portLowPowerCfg();
    noNetWakeUpGapInit();
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

#define MAX_TICK_CNT	10

void tapInt(void)
{
    static uint32_t tickTime[MAX_TICK_CNT];
    static uint8_t idleFlag = 0;
    uint8_t i;
    uint32_t tickTemp;

    if (sysparam.staticTime == 0)
    {
        return;
    }

    for (i = MAX_TICK_CNT - 1; i > 0; i--)
    {
        tickTime[i] = tickTime[i - 1];
    }
    tickTemp = tickTime[0] = portGetDateTimeOfSeconds();
    //    for (i = 0; i < MAX_TICK_CNT; i++)
    //    {
    //        LogPrintf(DEBUG_ALL, "{[%d]:%d} ", i, tickTime[i]);
    //    }
    //    LogPrintf(DEBUG_ALL, "\r\n");
    if (tickTemp < tickTime[1])
    {
        tickTemp += 86400;
    }
    if (tickTemp - tickTime[1] >= 60)
    {
        idleFlag = 1;
    }
    tickTemp = tickTime[0];
    if (tickTemp < tickTime[MAX_TICK_CNT - 1])
    {
        tickTemp += 86400;
    }
    if (idleFlag)
    {
        if (tickTemp - tickTime[MAX_TICK_CNT - 1] <= sysparam.staticTime)
        {
            if (gpsRequestGet(GPS_REQUEST_UPLOAD_ONE) == 0)
            {
                idleFlag = 0;
                LogMessage(DEBUG_ALL, "Õð¶¯´¥·¢\r\n");
                gpsRequestSet(GPS_REQUEST_UPLOAD_ONE);
                alarmRequestSet(ALARM_GUARD_REQUEST);
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
        tapInt();
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



