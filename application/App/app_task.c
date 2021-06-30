#include "app_task.h"
#include "app_sys.h"
#include "app_param.h"
#include "app_kernal.h"
#include "app_mir3da.h"
#include "app_instructioncmd.h"
#include "app_serverprotocol.h"
#include "app_gpsarithmetic.h"
#include "app_rec.h"
#include "app_ble.h"
/***********************************************************/
/*LED ������ع���*/
static SystemLEDInfo sysledinfo;
static void gpsLedTask(void)
{
    static uint8_t tick = 0;
    if (sysinfo.System_Tick >= 300 && sysparam.ledctrl == 0)
    {
        GPSLEDOFF;
        return ;
    }
    if (sysledinfo.sys_gps_led_on_time == 0)
    {
        GPSLEDOFF;
        return ;
    }
    else if (sysledinfo.sys_gps_led_off_time == 0)
    {
        GPSLEDON;
        return ;
    }
    tick++;
    if (sysledinfo.sys_gps_led_onoff == 1)
    {
        GPSLEDON;
        if (tick >= sysledinfo.sys_gps_led_on_time)
        {
            tick = 0;
            sysledinfo.sys_gps_led_onoff = 0;
        }
    }
    else
    {
        GPSLEDOFF;
        if (tick >= sysledinfo.sys_gps_led_off_time)
        {
            tick = 0;
            sysledinfo.sys_gps_led_onoff = 1;
        }
    }
}

static void signalLedTask(void)
{
    static uint8_t tick = 0;

    if (sysinfo.System_Tick >= 300 && sysparam.ledctrl == 0)
    {
        SIGNALLEDOFF;
        return ;
    }
    if (sysledinfo.sys_led_on_time == 0)
    {
        SIGNALLEDOFF;
        return ;
    }
    else if (sysledinfo.sys_led_off_time == 0)
    {
        SIGNALLEDON;
        return ;
    }

    tick++;
    if (sysledinfo.sys_led_onoff == 1)
    {
        SIGNALLEDON;
        if (tick >= sysledinfo.sys_led_on_time)
        {
            tick = 0;
            sysledinfo.sys_led_onoff = 0;
        }
    }
    else
    {
        SIGNALLEDOFF;
        if (tick >= sysledinfo.sys_led_off_time)
        {
            tick = 0;
            sysledinfo.sys_led_onoff = 1;
        }
    }
}
static void ledSetPeriod(uint8_t ledtype, uint8_t on_time, uint8_t off_time)
{
    if (ledtype == 0)
    {
        //ϵͳ�źŵ�
        sysledinfo.sys_led_on_time = on_time;
        sysledinfo.sys_led_off_time = off_time;
    }
    else
    {
        //ϵͳgps��
        sysledinfo.sys_gps_led_on_time = on_time;
        sysledinfo.sys_gps_led_off_time = off_time;

    }
}
//100ms ����һ��
void ledRunTask(void)
{
    gpsLedTask();
    signalLedTask();
}
//���µ�״̬
void updateSystemLedStatus(uint8_t status, uint8_t onoff)
{
    if (onoff == 1)
    {
        sysinfo.systemledstatus |= status;
    }
    else
    {
        sysinfo.systemledstatus &= ~status;
    }
    if ((sysinfo.systemledstatus & SYSTEM_LED_RUN) == SYSTEM_LED_RUN)
    {

        //����
        ledSetPeriod(SIGNALLED, 10, 10);
        ledSetPeriod(GPSLED, 10, 10);
        //GPS��λ�ɹ�
        if ((sysinfo.systemledstatus & SYSTEM_LED_NETOK) == SYSTEM_LED_NETOK)
        {
            //����
            //ledSetPeriod(SIGNALLED,1,0);
            ledSetPeriod(SIGNALLED, 1, 9);

            //�����ɹ�
            if ((sysinfo.systemledstatus & SYSTEM_LED_GPSOK) == SYSTEM_LED_GPSOK)
            {
                //����
                //ledSetPeriod(GPSLED,1,0);
                ledSetPeriod(SIGNALLED, 1, 0);
            }
        }



    }
    else
    {
        SIGNALLEDOFF;
        GPSLEDOFF;
        ledSetPeriod(SIGNALLED, 0, 1);
        ledSetPeriod(GPSLED, 0, 1);
    }
}


/***********************************************************/
/*���� �¼�����*/
void alarmRequestSet(uint16_t request)
{
    sysinfo.alarmrequest |= request;
}
void alarmRequestClear(uint16_t request)
{
    sysinfo.alarmrequest &= ~request;
}


void alarmUploadRequest(void)
{
    uint8_t alarm;
    if (sysinfo.alarmrequest == 0)
    {
        return ;
    }
    if (isProtocolReday() == 0)
    {
        return ;
    }
    //�йⱨ��
    if (sysinfo.alarmrequest & ALARM_LIGHT_REQUEST)
    {
        alarmRequestClear(ALARM_LIGHT_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>Light Alarm\n");
        terminalAlarmSet(TERMINAL_WARNNING_LIGHT);
        alarm = 0;
        sendProtocolToServer(PROTOCOL_16, &alarm);
    }

    //�͵籨��
    if (sysinfo.alarmrequest & ALARM_LOWV_REQUEST)
    {
        alarmRequestClear(ALARM_LOWV_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>LowVoltage Alarm\n");
        terminalAlarmSet(TERMINAL_WARNNING_LOWV);
        alarm = 0;
        sendProtocolToServer(PROTOCOL_16, &alarm);
    }

    //�����ٱ���
    if (sysinfo.alarmrequest & ALARM_ACCLERATE_REQUEST)
    {
        alarmRequestClear(ALARM_ACCLERATE_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>Rapid Accleration Alarm\n");
        alarm = 9;
        sendProtocolToServer(PROTOCOL_16, &alarm);
    }
    //�����ٱ���
    if (sysinfo.alarmrequest & ALARM_DECELERATE_REQUEST)
    {
        alarmRequestClear(ALARM_DECELERATE_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>Rapid Deceleration Alarm\n");
        alarm = 10;
        sendProtocolToServer(PROTOCOL_16, &alarm);
    }

    //����ת����
    if (sysinfo.alarmrequest & ALARM_RAPIDLEFT_REQUEST)
    {
        alarmRequestClear(ALARM_RAPIDLEFT_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>Rapid LEFT Alarm\n");
        alarm = 11;
        sendProtocolToServer(PROTOCOL_16, &alarm);
    }

    //��������
    if (sysinfo.alarmrequest & ALARM_GUARD_REQUEST)
    {
        alarmRequestClear(ALARM_GUARD_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>Guard Alarm\n");
        alarm = 1;
        sendProtocolToServer(PROTOCOL_16, &alarm);
    }

    //��������
    if (sysinfo.alarmrequest & ALARM_BLE_REQUEST)
    {
        alarmRequestClear(ALARM_BLE_REQUEST);
        LogMessage(DEBUG_ALL, "alarmUploadRequest==>BLE Alarm\n");
        alarm = 20;
        sendProtocolToServer(PROTOCOL_16, &alarm);
    }
}

/***********************************************************/
/*GPS����*/
void gpsRequestSet(uint32_t flag)
{
    char debug[40];
    sprintf(debug, "gpsRequestSet==>0x%04X\n", flag);
    LogMessage(DEBUG_ALL, debug);
    sysinfo.GPSRequest |= flag;
}
void gpsRequestClear(uint32_t flag)
{
    char debug[40];
    sprintf(debug, "gpsRequestClear==>0x%04X\n", flag);
    LogMessage(DEBUG_ALL, debug);
    sysinfo.GPSRequest &= ~flag;
}

uint8_t gpsRequestGet(uint32_t flag)
{
    return sysinfo.GPSRequest & flag;
}

void gpsChangeFsmState(uint8_t state)
{
    sysinfo.GPSRequestFsm = state;
}

static void gpsStartBD(void)
{
    sendModuleCmd(N58_MYGNSSSEL_CMD, "0");
};

void gpsOutputCheckTask(void)
{
    if (sysinfo.GPSStatus == 0)
        return;
    if (sysinfo.System_Tick - sysinfo.gpsUpdatetick >= 20)
    {
        sysinfo.gpsUpdatetick = sysinfo.System_Tick;
        LogMessage(DEBUG_ALL, "No nmea output\n");
        GPSLNAOFF;
        gpsClearCurrentGPSInfo();
        closeModuleGPS();
        updateSystemLedStatus(SYSTEM_LED_GPSOK, 0);
        gpsChangeFsmState(GPSCLOSESTATUS);
        sysinfo.GPSStatus = 0;
    }
}
static void gpsRequestOpen(void)
{

    LogMessage(DEBUG_ALL, "gpsRequestTask==>open GPS\n");
    GPSLNAON;
    startTimer(8000, gpsStartBD, 0);
    sysinfo.gpsUpdatetick = sysinfo.System_Tick;
    if (sysinfo.agpsOpenTick == 0 || (sysinfo.System_Tick >= sysinfo.agpsOpenTick))
    {
        sysinfo.agpsOpenTick = sysinfo.System_Tick + 5400;
        sendModuleCmd(N58_SETSERVER_CMD, "1,gnss-aide.com,2621,freetrial,123456");
        gpsChangeFsmState(GPSOPENWAITSTATUS);

    }
    else
    {
        openModuleGPS();
        gpsChangeFsmState(GPSOPENSTATUS);

    }
}

static void gpsRequestClose(void)
{
    LogMessage(DEBUG_ALL, "gpsRequestTask==>close GSP\n");
    GPSLNAOFF;
    closeModuleGPS();
    gpsClearCurrentGPSInfo();
    updateSystemLedStatus(SYSTEM_LED_GPSOK, 0);
    gpsChangeFsmState(GPSCLOSESTATUS);
}

//�Ƿ����RTCʱ���ж��߼�����
static void modeRequeUpdateRTC(void)
{
    if (sysparam.MODE == MODE1 || sysparam.MODE == MODE2 || sysparam.MODE == MODE21 || sysparam.MODE == MODE5 ||
            sysparam.MODE == MODE4)
    {
        updateRTCtimeRequest();
    }
    else if (sysparam.MODE == MODE23)
    {
        if (getTerminalAccState())
        {
            updateRTCtimeRequest();
        }
    }

}
void gpsRequestTask(void)
{
    static uint8_t tick = 0;
    if (netWorkModuleRunOk() == 0)
    {
        tick = 0;
        return ;
    }
    tick++;
    switch (sysinfo.GPSRequestFsm)
    {
        case GPSCLOSESTATUS:
            if (sysinfo.GPSRequest != 0)
            {
                modeRequeUpdateRTC();
                gpsRequestOpen();
                tick = 0;
            }
            if (sysinfo.GPSStatus == 1 && tick >= 10)
            {
                tick = 0;
                sendModuleCmd(N58_MYGPSSTATE_CMD, NULL);
            }
            break;
        case GPSOPENWAITSTATUS:
            if (tick >= 3)
            {
                openModuleGPS();
                gpsChangeFsmState(GPSOPENSTATUS);
                tick = 0;
            }
            break;
        case GPSOPENSTATUS:
            if (sysinfo.GPSRequest == 0)
            {
                gpsRequestClose();

            }
            if (sysinfo.GPSStatus == 0 && tick >= 10)
            {
                tick = 0;
                sendModuleCmd(N58_MYGPSSTATE_CMD, NULL);
            }
            break;
        default:
            gpsChangeFsmState(GPSCLOSESTATUS);
            break;

    }
}
/***********************************************************/
/*GPS ����һ����*/
void gpsUplodOnePointTask(void)
{
    GPSINFO *gpsinfo;
    static uint16_t runtick = 0;
    static uint8_t	uploadtick = 0;
    //�ж��Ƿ���������¼�
    if (gpsRequestGet(GPS_REQUEST_UPLOAD_ONE) == 0)
    {
        runtick = 0;
        uploadtick = 0;
        return ;
    }
    gpsinfo = getCurrentGPSInfo();
    runtick++;
    if (runtick >= sysinfo.gpsuploadonepositiontime)
    {
        runtick = 0;
        uploadtick = 0;
        sendProtocolToServer(PROTOCOL_12, getCurrentGPSInfo());
        gpsRequestClear(GPS_REQUEST_UPLOAD_ONE);
    }
    else
    {
        if (isProtocolReday() && gpsinfo->fixstatus)
        {
            if (++uploadtick >= 10)
            {
                uploadtick = 0;
                if (sysinfo.flag123)
                {
                    dorequestSend123();
                }
                sendProtocolToServer(PROTOCOL_12, getCurrentGPSInfo());
                gpsRequestClear(GPS_REQUEST_UPLOAD_ONE);
            }
        }
    }


}

/***********************************************************/
/*��վ�ϱ�*/
void lbsUploadRequestTask(void)
{
    static uint8_t step = 0;
    if (sysinfo.lbsrequest == 0)
    {
        step = 0;
        return ;
    }
    if (isProtocolReday() == 0)
        return ;
    switch (step)
    {
        case 0:
        case 1:
            sendModuleCmd(N58_MYLACID_CMD, NULL);
            break;
        case 2:
            sendProtocolToServer(PROTOCOL_19, NULL);
            sysinfo.lbsrequest = 0;
            step = 0;
            break;
        default:
            step = 0;
            break;
    }
    step++;
}

/***********************************************************/
/*WIFI�ϱ�*/
static void wifiScanStart(void)
{
    GPSINFO *gpsinfo;
    gpsinfo = getCurrentGPSInfo();
    if (gpsinfo->fixstatus == 0)
        sendModuleCmd(N58_WIFIAPSCAN_CMD, NULL);
}
void wifiUploadRequestTask(void)
{
    if (sysinfo.wifirequest == 0)
        return ;
    if (isProtocolReday() == 0)
        return ;
    sysinfo.wifirequest = 0;
    startTimer(12000, wifiScanStart, 0);
}

/***********************************************************/
/*ϵͳ����*/
void systemRequestSet(uint16_t request)
{
    sysinfo.systemRequest |= request;
}
void systemRequestClear(uint16_t request)
{
    sysinfo.systemRequest &= ~request;

}
/*ģ�鿪��*/
static void netModuleStartup(void)
{
    if (sysinfo.systempowerlow == 0)
    {
        modulePinConfig(1);
        modulePowerOn();
        appUartConfig(APPUSART2, 1, moduleResponParaser);
        LogPrintf(DEBUG_ALL, "netModuleStartup==>at %d\n", sysinfo.System_Tick);
    }
    else
    {
        LogMessage(DEBUG_ALL, "netModuleStartup==>SystemPowerlow,don't startup\n");
    }
}
/*ģ��ػ�*/
static void netModuleShutdown(void)
{
    appUartConfig(APPUSART2, 0, NULL);
    modulePowerOff();
    modulePinConfig(0);
    updateSystemLedStatus(SYSTEM_LED_NETOK, 0);
    LogPrintf(DEBUG_ALL, "netModuleShutdown==>at %d\n", sysinfo.System_Tick);
}
/*ϵͳ�ػ�*/
void systemShutDown(void)
{
    //��ձ�־
    sysinfo.alarmrequest = 0;
    sysinfo.lbsrequest = 0;
    sysinfo.wifirequest = 0;
    //�ر�GPS
    if (sysinfo.GPSRequest != 0)
    {
        gpsRequestClose();
        sysinfo.GPSRequest = 0;
        sysinfo.GPSStatus = 0;
    }
    //�ر�ģ��
    if (sysinfo.runFsm == MODE_RUNING)
    {
        sysinfo.runFsm = MODE_DONE;
        gsensorConfig(0);
        netModuleShutdown();
        updateSystemLedStatus(SYSTEM_LED_RUN, 0);
    }
    //���gsensor
    memset(sysinfo.onetaprecord, 0, 15);
    sysinfo.gsensortapcount = 0;
    //���������־
    terminalAccoff();
    if (gpsRequestGet(GPS_REQUEST_ACC_CTL))
    {
        gpsRequestClear(GPS_REQUEST_ACC_CTL);
    }
    LogMessage(DEBUG_ALL, "System shutdown\n");
    systemRequestSet(SYSTEM_ENTERSLEEP_REQUEST);
}

void wdtProcess(void)
{
    static uint8_t tick = 0;
    feedWdt();
    if (sysinfo.runFsm == MODE_DONE && sysinfo.alarmrequest == 0 && sysinfo.GPSRequest == 0)
    {
        //���µ�ѹֵ

        if (sysinfo.systempowerlow == 1)
        {
            getBatVoltage();
            getBatVoltage();
            LogPrintf(DEBUG_ALL, "Systempower voltage=%.2f\n", sysinfo.outsidevoltage);
            //MODE2ʱ,��ѹ����60�����3.52ʱ���򿪻�
            if (sysinfo.outsidevoltage >= 3.48)
            {
                LogPrintf(DEBUG_ALL, "System power supply normal %.2fV\n", sysinfo.outsidevoltage);
                if (++tick > 5)
                {
                    tick = 0;
                    if (sysparam.MODE == MODE2 || sysparam.MODE == MODE5)
                    {
                        sysinfo.systempowerlow = 0;
                        systemModeRunStart();
                        return ;
                    }
                    else
                    {
                        LogMessage(DEBUG_ALL, "clear systempower low flag\n");
                        sysinfo.systempowerlow = 0;
                    }
                }

            }
            else
            {
                tick = 0;

            }
        }
        systemRequestSet(SYSTEM_ENTERSLEEP_REQUEST);
    }

    //ÿ��3���Ӽ��һ������
    if (sysparam.MODE == MODE4 && ++sysinfo.mode4checktick >= 18)
    {

        sysinfo.mode4checktick = 0;
        if (isModuleRunNormal())
        {
            N58_ChangeInvokeStatus(N58_CPIN_STATUS);
        }
    }
}

//��������
void hearbeatRequest(void)
{
    static uint8_t count = 0;
    static uint16_t count2 = 0;
    count++;
    if (sysparam.MODE == MODE2 || sysparam.MODE == MODE5)
    {
        if (count % (sysparam.heartbeatgap / 10) == 0)
        {
            if (isProtocolReday())
            {
                sysinfo.hearbeatrequest = 1;
            }
            count = 0;
        }
        if (sysparam.interval_wakeup_minutes != 0)
        {
            count2++;
            if (count2 % (sysparam.interval_wakeup_minutes * 6) == 0)
            {
                sysinfo.lbsrequest = 1;
                sysinfo.wifirequest = 1;
                gpsRequestSet(GPS_REQUEST_UPLOAD_ONE);
                count2 = 0;
            }
        }
    }
}

void uploadRequestInMode4(void)
{
    static uint16_t mode4runtick = 0;
    static uint16_t nonettick;
    if (sysparam.MODE != MODE4)
        return ;


    if (sysparam.interval_wakeup_minutes != 0)
    {
        mode4runtick++;
        if ((mode4runtick / 6) >= sysparam.interval_wakeup_minutes)
        {
            mode4runtick = 0;
            sysinfo.lbsrequest = 1;
            sysinfo.wifirequest = 1;
            gpsRequestSet(GPS_REQUEST_UPLOAD_ONE);
        }
    }
    if (sysinfo.noNetworkFlag)
    {
        nonettick++;
        LogPrintf(DEBUG_ALL, "no network sleep %d0 sec\n", nonettick);
        if ((nonettick / 6) >= sysparam.noNetWakeUpMinutes || sysinfo.GPSRequest)
        {
            nonettick = 0;
            sysinfo.noNetworkFlag = 0;
            if (isModulePowerOn() == 0)
            {
                systemRequestSet(SYSTEM_MODULE_STARTUP_REQUEST);
            }
        }
    }
}
void exitNetCtl(void)
{
    if (sysinfo.netCtrlStop)
    {
        sysinfo.netCtlTick++;//10 * n sec
        LogPrintf(DEBUG_ALL, "net ctrl tick %d\n", sysinfo.netCtlTick * 10);
        if (sysinfo.netCtlTick >= 360 || sysinfo.GPSRequest != 0)
        {
            sysinfo.netCtlTick = 0;

            netExitStopMode();
        }
    }
}

//����ϵͳ��������ٶȴ���
void systemRequestTask(void)
{
    //����ģ������
    if (sysinfo.systemRequest & SYSTEM_MODULE_STARTUP_REQUEST)
    {
        systemRequestClear(SYSTEM_MODULE_STARTUP_REQUEST);
        netModuleStartup();
    }
    //�ر�ģ������
    if (sysinfo.systemRequest & SYSTEM_MODULE_SHUTDOWN_REQUEST)
    {
        systemRequestClear(SYSTEM_MODULE_SHUTDOWN_REQUEST);
        netModuleShutdown();
    }
    //����˯��
    if (sysinfo.systemRequest & SYSTEM_ENTERSLEEP_REQUEST)
    {
        systemRequestClear(SYSTEM_ENTERSLEEP_REQUEST);
        LogMessage(DEBUG_ALL, "systemRequestTask==>Enter Sleep\n");
        HAL_SuspendTick();
        HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
        SystemClock_Config();
        HAL_ResumeTick();
        LogMessage(DEBUG_ALL, "SystemRequestTask==>Exit Sleep\n");
    }
    //ϵͳ��Դ��
    if (sysinfo.systemRequest & SYSTEM_POWERLOW_REQUEST)
    {
        systemRequestClear(SYSTEM_POWERLOW_REQUEST);
        LogMessage(DEBUG_ALL, "SystemRequestTask==>Powerlow\n");
        systemShutDown();
    }
    //�رմ���
    if (sysinfo.systemRequest & SYSTEM_CLOSEUART1_REQUEST)
    {
        systemRequestClear(SYSTEM_CLOSEUART1_REQUEST);
        appUartConfig(APPUSART1, 0, NULL);
        LogMessage(DEBUG_ALL, "System request close uart\n");
    }
    //��������
    if (sysinfo.systemRequest & SYSTEM_OPENUART1_REQUEST)
    {
        systemRequestClear(SYSTEM_OPENUART1_REQUEST);
        appUartConfig(APPUSART1, 1, atCmdParaseFunction);
        LogMessage(DEBUG_ALL, "System request open uart\n");
    }
    //ι��
    if (sysinfo.systemRequest & SYSTEM_WDT_REQUEST)
    {
        systemRequestClear(SYSTEM_WDT_REQUEST);
        wdtProcess();
        hearbeatRequest();
        uploadRequestInMode4();
        exitNetCtl();
    }
    alarmUploadRequest();
}

/***********************************************************/

static void doPoitypeRequest(void)
{
    switch (sysparam.MODE)
    {
        case MODE1:
        case MODE3:
        //case MODE4:
        case MODE5:
        case MODE21:
        case MODE23:
            //only lbs
            if (sysparam.poitype == 0)
            {
                sysinfo.lbsrequest = 1;
            }
            //lbs and gps
            else if (sysparam.poitype == 1)
            {
                sysinfo.lbsrequest = 1;
                gpsRequestSet(GPS_REQUEST_UPLOAD_ONE);
            }
            else
            {
                //lbs gps and wifi
                sysinfo.lbsrequest = 1;
                sysinfo.wifirequest = 1;
                gpsRequestSet(GPS_REQUEST_UPLOAD_ONE);

            }
            break;
        case MODE4:
            LogMessage(DEBUG_ALL, "no need upload gps\n");
            break;
        default:
            gpsRequestSet(GPS_REQUEST_UPLOAD_ONE);
            break;

    }
}

static void modeShutDownQuickly(void)
{
    static uint8_t delaytick = 0;
    if (isProtocolReday() == 0)
    {
        delaytick = 0;
        return;
    }

    if (sysparam.MODE == MODE2)
    {
        return;
    }
    if (sysinfo.GPSRequest == 0 && sysinfo.lbsrequest == 0 && sysinfo.wifirequest == 0 && sysinfo.alarmrequest == 0 &&
            sysinfo.blerequest == 0)
    {
        delaytick++;
        if (delaytick >= 30)
        {
            LogMessage(DEBUG_ALL, "modeShutDownQuickly==>No request,shutdown\n");
            delaytick = 0;
            sysinfo.runFsm = MODE_STOP; //ִ����ϣ��ػ�
        }
    }
    else
    {
        delaytick = 0;
    }
}

void getBatVoltage(void)
{
    sysinfo.outsidevoltage = ((float)getVoltageAdcValue() / 4095) * 1.8 * sysparam.adccal;
}
/*��ѹ�������*/
void voltageCheckTask(void)
{
    static uint16_t lowpowertick = 0, superlowpowertick = 0;
    static uint8_t  delaytick = 0, powernormaltick = 0, clearRunninginfo = 0, lowwflag = 0;
    static float voltage[5] = {0, 0, 0, 0, 0};
    static uint8_t vi = 0;
    uint8_t chargestate, j, i;

    //��ȡ��ѹ
    getBatVoltage();
    chargestate = CHARGEDET;

    vi = (vi++) % 5;
    voltage[vi] = sysinfo.outsidevoltage;
    sysinfo.outsidevoltage = 0;
    j = 0;
    for (i = 0; i < 5; i++)
    {
        if (voltage[i] != 0)
        {
            j++;
            sysinfo.outsidevoltage += voltage[i];
        }
    }
    sysinfo.outsidevoltage /= j;

    //����ж�
    if (getTerminalChargeState() == 0 && chargestate == 1)
    {
        terminalCharge();
        LogMessage(DEBUG_ALL, "Battery charging\n");
        systemRequestSet(SYSTEM_CLOSEUART1_REQUEST);
    }
    else if (getTerminalChargeState() && chargestate == 0)
    {
        terminalunCharge();
        LogMessage(DEBUG_ALL, "Battery uncharged\n");
        systemRequestSet(SYSTEM_OPENUART1_REQUEST);
    }
    //�������ģʽ��¼
    if (clearRunninginfo == 0 && sysinfo.outsidevoltage >= 4.13)
    {
        delaytick++;
        if (delaytick >= 15)
        {
            delaytick = 0;
            LogMessage(DEBUG_ALL, "Battery charge fully,clear mode running info\n");
            clearRunninginfo = 1;
            paramSaveMode2cnt(0);
            paramSaveMode1Timer(0);
        }
    }
    else
    {
        delaytick = 0;
        if (sysinfo.outsidevoltage < 3.8	&& clearRunninginfo == 1)
        {
            clearRunninginfo = 0;
        }
    }

    //log��ӡ
    if (sysinfo.systempowerlow == 1)
    {
        LogPrintf(DEBUG_ALL, "%s,BatV=%.2fV\n", chargestate ? "Battery charging" : "Uncharged", sysinfo.outsidevoltage);
    }


    //�͵籨��
    if (sysinfo.outsidevoltage < sysinfo.lowvoltage && sysinfo.systempowerlow == 0)
    {
        lowpowertick++;
        LogPrintf(DEBUG_ALL, "Low power supply==>Bat %.2fV\n", sysinfo.outsidevoltage);
        if (lowpowertick >= 30	&& lowwflag == 0 && isProtocolReday())
        {
            lowwflag = 1;
            lowpowertick = 0;
            //�͵籨��
            alarmRequestSet(ALARM_LOWV_REQUEST);

        }
    }
    else
    {
        lowpowertick = 0;
    }


    if (sysinfo.outsidevoltage >= sysinfo.lowvoltage + 0.2)
    {
        lowwflag = 0;
    }


    //�͵籣��
    if (sysinfo.outsidevoltage < 3.35)
    {
        powernormaltick = 0;
        superlowpowertick++;
        LogPrintf(DEBUG_ALL, "The power supply is too low,system will sleep (%ds)...\n", superlowpowertick);
        if (superlowpowertick >= 15)
        {
            superlowpowertick = 0;
            sysinfo.alarmrequest = 0;
            sysinfo.systempowerlow = 1;
            LogMessage(DEBUG_ALL, "System sleep...\n");
            systemRequestSet(SYSTEM_POWERLOW_REQUEST);//����͹���
        }
    }
    else if (sysinfo.outsidevoltage >= 3.35 &&  sysinfo.outsidevoltage < 3.48)
    {
        powernormaltick = 0;
        if (sysinfo.systempowerlow == 1)
        {
            superlowpowertick++;
            if (superlowpowertick >= 15)
            {
                superlowpowertick = 0;
                sysinfo.alarmrequest = 0;
                sysinfo.systempowerlow = 1;
                systemRequestSet(SYSTEM_POWERLOW_REQUEST);//����͹���
                LogMessage(DEBUG_ALL, "System power supply too low\n");

            }
        }
        else
        {
            superlowpowertick = 0;//��ֹ��ѹ����
        }
    }
    else if (sysinfo.outsidevoltage >= 3.48)
    {
        powernormaltick++;
        if (sysinfo.systempowerlow == 1 && powernormaltick >= 5)
        {
            powernormaltick = 0;
            sysinfo.systempowerlow = 0;
            systemRequestSet(SYSTEM_MODULE_STARTUP_REQUEST);
        }
    }

}

void systemModeRunStart(void)
{
    if (sysinfo.systempowerlow == 0)
    {
        if (sysinfo.runFsm != MODE_RUNING)
        {
            sysinfo.runFsm = MODE_START;
        }
    }
    else
    {
        LogMessage(DEBUG_ALL, "systemModeRunStart==>system power too low\n");
    }
}

void systemModeRunTask(void)
{
    static uint8_t mode2runtick;
    static uint8_t delaytick;
    switch (sysinfo.runFsm)
    {
        case MODE_START:
            mode2runtick = 0;
            delaytick = 0;
            sysinfo.runStartTick = sysinfo.System_Tick;
            sysinfo.gpsuploadonepositiontime = 180;

            gsensorConfig(1);
            if (sysparam.MODE == MODE1)
            {
                setNextAlarmTime();
                sysparam.mode1startuptime++;
                sysinfo.gpsuploadonepositiontime = 90;
            }
            else if (sysparam.MODE == MODE2)
            {
                if (sysparam.accctlgnss == 0)
                {
                    gpsRequestSet(GPS_REQUEST_GPSKEEPOPEN_CTL);
                }
            }
            else if (sysparam.MODE == MODE3)
            {
                setNextWakeUpTime();
                sysparam.mode1startuptime++;
                sysinfo.gpsuploadonepositiontime = 90;
            }

            else if (sysparam.MODE == MODE4)
            {
                gsensorConfig(0);
                sysinfo.gpsuploadonepositiontime = 90;
            }

            else if (sysparam.MODE == MODE5)
            {
                sysinfo.gpsuploadonepositiontime = 90;
                gsensorConfig(0);
            }
            else if (sysparam.MODE == MODE23)
            {
                setNextWakeUpTime();
                if (getTerminalAccState() == 0)
                    sysparam.mode1startuptime++;
            }
            else if (sysparam.MODE == MODE21)
            {
                setNextAlarmTime();
                if (getTerminalAccState() == 0)
                    sysparam.mode1startuptime++;
            }
            paramSaveMode1Timer(sysparam.mode1startuptime);
            doPoitypeRequest();
            systemRequestSet(SYSTEM_MODULE_STARTUP_REQUEST);
            updateSystemLedStatus(SYSTEM_LED_RUN, 1);
            sysinfo.runFsm = MODE_RUNING;
            sysinfo.csqSearchTime = 30;
            break;
        case MODE_RUNING:

            if (sysparam.MODE == MODE1)
            {
                if ((sysinfo.System_Tick - sysinfo.runStartTick) >= 210)
                {
                    sysinfo.alarmrequest = 0;
                    sysinfo.runFsm = MODE_STOP; //ִ����ϣ��ػ�
                }
                modeShutDownQuickly();

            }
            else if (sysparam.MODE == MODE2)
            {
                mode2runtick++;
                if (mode2runtick >= 180)
                {
                    mode2runtick = 0;
                    sysparam.mode2worktime++;
                    if (sysparam.mode2worktime % 20 == 0)
                    {
                        paramSaveMode2cnt(sysparam.mode2worktime);
                    }
                    //ÿ180���¼һ�ι���ʱ��
                    //ȷ�����Ѿ��ػ��±��й⻽��ʱ��4Gģ�鿪������ʱ����๤��3����
                    if (sysinfo.systempowerlow == 1)
                    {
                        sysinfo.runFsm = MODE_STOP; //ִ����ϣ��ػ�
                    }
                }
            }
            else if (sysparam.MODE == MODE3)
            {
                if ((sysinfo.System_Tick - sysinfo.runStartTick) >= 210)
                {
                    sysinfo.alarmrequest = 0;
                    sysinfo.runFsm = MODE_STOP; //ִ����ϣ��ػ�
                }
                modeShutDownQuickly();
            }

            else if (sysparam.MODE == MODE4)
            {
            }
            //ʵʱ���ߣ��Զ��ϱ�
            else if (sysparam.MODE == MODE5)
            {
                mode2runtick++;
                if (mode2runtick >= 180)
                {
                    mode2runtick = 0;
                    sysparam.mode2worktime++;
                    if (sysparam.mode2worktime % 20 == 0)
                    {
                        paramSaveMode2cnt(sysparam.mode2worktime);
                    }
                }
            }

            //ģʽ21��23����ģʽ2��3����������gsensor�Ƿ��п���
            else if (sysparam.MODE == MODE21 || sysparam.MODE == MODE23)
            {
                modeShutDownQuickly();
                if (getTerminalAccState())
                {
                    mode2runtick++;
                    delaytick = 0;
                    if (mode2runtick >= 180)
                    {
                        mode2runtick = 0;
                        sysparam.mode2worktime++;
                        if (sysparam.mode2worktime % 20 == 0)
                        {
                            paramSaveMode2cnt(sysparam.mode2worktime);
                        }
                        //ÿ180���¼һ�ι���ʱ��
                    }
                }
                else if (++delaytick >= 210)
                {
                    sysinfo.alarmrequest = 0;
                    sysinfo.runFsm = MODE_STOP; //ִ����ϣ��ػ�
                }

            }
            else
            {
                paramSaveMode(MODE1);
            }

            //���ù켣�㷨
            if (sysparam.MODE == MODE2 || sysparam.MODE == MODE21 || sysparam.MODE == MODE23)
            {
                gpsUploadPointToServer();
            }
            voltageCheckTask();
            break;
        case MODE_STOP:
            if (sysparam.MODE != MODE21 && sysparam.MODE != MODE23)
                gsensorConfig(0);
            systemRequestSet(SYSTEM_MODULE_SHUTDOWN_REQUEST);
            systemRequestSet(SYSTEM_ENTERSLEEP_REQUEST);
            updateSystemLedStatus(SYSTEM_LED_RUN, 0);
            sysinfo.runFsm = MODE_DONE;
            break;

        case MODE_DONE:
            if ((getTerminalAccState() || sysinfo.alarmrequest != 0) && sysinfo.runFsm == MODE_DONE)
            {
                sysinfo.runFsm = MODE_START;
            }
            else
            {
                systemRequestSet(SYSTEM_ENTERSLEEP_REQUEST);
            }
            break;
    }

}

/***********************************************************/
/*gsensor �ж�*/
void gsensorTapTask(void)
{
    static uint8_t index = 0;
    static uint32_t tick = 0;
    GPSINFO *gpsinfo;
    uint8_t i;
    uint16_t total = 0;
    //��ģʽ������һ��ģʽʱ��������GSENSOR �㷨
    if (sysparam.MODE != MODE2 && sysparam.MODE != MODE21 && sysparam.MODE != MODE23)
    {
        tick = 0;
        terminalAccoff();
        if (gpsRequestGet(GPS_REQUEST_ACC_CTL))
        {
            gpsRequestClear(GPS_REQUEST_ACC_CTL);
        }
        return ;
    }
    gpsinfo = getCurrentGPSInfo();

    sysinfo.onetaprecord[index++] = sysinfo.gsensortapcount;
    sysinfo.gsensortapcount = 0;
    if (index >= 15)
        index = 0;
    for (i = 0; i < 15; i++)
    {
        total += sysinfo.onetaprecord[i];
    }
    //LogPrintf(DEBUG_ALL,"Triggered %d in 15s \n",total);
    if (total >= 7	|| (sysinfo.gsensorerror == 1 && gpsinfo->fixstatus == 1 && gpsinfo->speed >= 15))
    {
        if (getTerminalAccState() == 0 && sysinfo.systempowerlow == 0)
        {
            sysinfo.csqSearchTime = 30;
            terminalAccon();
            LogMessage(DEBUG_ALL, "Acc on detected by gsensor\n");
            if (isProtocolReday())
            {
                sendProtocolToServer(PROTOCOL_13, NULL);
            }
            if (sysparam.bf == 1)
            {
                alarmRequestSet(ALARM_GUARD_REQUEST);
            }
            gpsRequestSet(GPS_REQUEST_ACC_CTL);
            gpsRequestSet(GPS_REQUEST_UPLOAD_ONE);
            memset(sysinfo.onetaprecord, 0, 15);
        }
    }
    if (total == 0)
    {
        tick++;
        //�ٶ��ж�ACC
        if (gpsinfo->fixstatus == 1 && gpsinfo->speed >= 7)
        {
            tick = 0;
        }
        if (tick >= 90)
        {
            if (getTerminalAccState())
            {

                terminalAccoff();
                LogMessage(DEBUG_ALL, "Acc off detected by gsensor\n");
                if (isProtocolReday())
                {
                    sendProtocolToServer(PROTOCOL_13, NULL);
                }
                gpsRequestClear(GPS_REQUEST_ACC_CTL);

            }

        }
    }
    else
    {
        tick = 0;
    }
}

static void repairGsensor(void)
{
    gsensorConfig(1);
}
void gsensorIntervalCheck(void)
{
    static uint8_t tick = 0;
    static uint8_t errorcount = 0;
    if (sysinfo.gsensoronoff == 0)
    {
        tick = 0;
        return;
    }
    if (tick % 60 == 0)
    {
        tick = 0;
        if (readInterruptConfig() != 0 && sysparam.MODE != MODE1 && sysparam.MODE != MODE3 && sysparam.MODE != MODE5 &&
                sysparam.MODE != MODE4)
        {
            LogMessage(DEBUG_ALL, "gsensor open error\n");
            errorcount++;
            gsensorConfig(0);
            startTimer(500, repairGsensor, 0);
            if (errorcount >= 3)
            {
                errorcount = 0;
                sysinfo.gsensorerror = 1;
                gpsRequestSet(GPS_REQUEST_GPSKEEPOPEN_CTL);
            }
        }
        else
        {
            errorcount = 0;
            sysinfo.gsensorerror = 0;
            if (sysparam.accctlgnss == 1)
            {
                if (gpsRequestGet(GPS_REQUEST_GPSKEEPOPEN_CTL))
                    gpsRequestClear(GPS_REQUEST_GPSKEEPOPEN_CTL);
            }
        }
    }
    tick++;
}

uint8_t autoSleepTask(void)
{
    if (!(sysparam.MODE == MODE2 || sysparam.MODE == MODE5 || sysparam.MODE == MODE4))
        return 1;
    if (sysinfo.GPSRequest != 0)
        return 2;
    if (sysinfo.GPSStatus != 0)
        return 3;
    if (sysinfo.alarmrequest != 0 && sysinfo.netCtrlStop == 0)
        return 4;
    if (sysinfo.lbsrequest != 0 && sysinfo.netCtrlStop == 0)
        return 5;
    if (sysinfo.wifirequest != 0 && sysinfo.netCtrlStop == 0)
        return 6;
    if (sysinfo.hearbeatrequest != 0 && sysinfo.netCtrlStop == 0)
        return 7;
    if (!(isModuleRunNormal() || sysinfo.noNetworkFlag == 1 || sysinfo.netCtrlStop == 1))
        return 8;
    if (sysparam.MODE == MODE4)
        return 9;
    if (sysinfo.instructionqequest != 0)
        return 10;
    if (sysinfo.recordingflag != 0)
        return 11;
    if (gpsIsRun())
        return 12;
    if (recIsRun())
        return 13;
    if (resIsCycleRuning())
        return 14;
    LogMessage(DEBUG_ALL, "auto sleep\n");
    if (sysparam.MODE == MODE4)
    {
        netConnectReset();
    }
    systemRequestSet(SYSTEM_ENTERSLEEP_REQUEST);
    LED1OFF;
    return 0;
}


/***********************************************************/
/*ϵͳ��������*/
void taskRunInOneSecond(void)
{
    sysinfo.System_Tick++;
    disPlaySystemTime();
    networkConnectProcess();
    gpsOutputCheckTask();
    gpsRequestTask();
    gpsUplodOnePointTask();
    systemModeRunTask();
    lbsUploadRequestTask();
    wifiUploadRequestTask();
    gsensorIntervalCheck();
    gsensorTapTask();
    recCycleTask();
    recRecordUploadTask();
#ifdef RI_ENABLE
    autoSleepTask();
#endif
}

