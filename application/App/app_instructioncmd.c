#include "app_instructioncmd.h"
#include "app_sys.h"
#include "app_sn.h"
#include "app_rec.h"
#include "app_net.h"
#include "app_kernal.h"
#include "app_param.h"
#include "app_task.h"
#include "app_mir3da.h"
#include "app_port.h"
#include "app_serverprotocol.h"
#include "app_ble.h"
void sendMessageWithDifMode(uint8_t *buf, uint16_t len, DOINSTRUCTIONMODE mode, char *telnum)
{
    switch (mode)
    {
        case AT_SMS_MODE:
            LogMessage(DEBUG_FACTORY, "----------Content----------\n");
            LogMessage(DEBUG_FACTORY, (char *)buf);
            LogMessage(DEBUG_FACTORY, "\n------------------------------\n");
            break;
        case SHORTMESSAGE_MODE:
            if (sysparam.smsRespon)
            {
                sendMessage(buf, len, telnum);
            }
            else if (sysinfo.smsReplyUpload == 1)
            {
                sysinfo.smsReplyUpload = 0;
                sendMessage(buf, len, telnum);
            }
            startTimer(1500, deleteMessage, 0);
            break;
        case NETWORK_MODE:
            sendProtocolToServer(PROTOCOL_21, (void *)buf);
            break;
        case BLE_MODE:
            appBleSendData(buf, len);
            break;
        default:
            LogPrintf(DEBUG_ALL, "sendMessageWithDifMode==>unknow mode %d\n", mode);
            break;
    }
}

void doParamInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    uint8_t i;
    char message[150];
    sysinfo.smsReplyUpload = 1;
    memset(sysparam.SN, 0, sizeof(sysparam.SN));
    paramGetSnNumber(sysparam.SN, 15);
    paramGetServer(sysparam.Server, &sysparam.ServerPort);
    sprintf(message, "SN:%s;IP:%s:%d;", sysparam.SN, sysparam.Server, sysparam.ServerPort);
    sprintf(message + strlen(message), "APN:%s;", sysparam.apn);
    sprintf(message + strlen(message), "UTC:%s%d;", sysparam.utc >= 0 ? "+" : "", sysparam.utc);
    switch (sysparam.MODE)
    {
        case MODE1:
        case MODE21:
            if (sysparam.MODE == MODE1)
            {
                sprintf(message + strlen(message), "Mode1:");

            }
            else
            {
                sprintf(message + strlen(message), "Mode21:");

            }
            for (i = 0; i < 5; i++)
            {
                if (sysparam.AlarmTime[i] != 0xFFFF)
                {
                    sprintf(message + strlen(message), " %.2d:%.2d", sysparam.AlarmTime[i] / 60, sysparam.AlarmTime[i] % 60);
                }

            }
            sprintf(message + strlen(message), ", Every %d day;", sysparam.MODE1_GAP_DAY);
            break;
        case MODE2:

            if (sysparam.gpsuploadgap == 0)
            {
                if (sysparam.gapMinutes == 0)
                {
                    //保持在线，不上送
                    sprintf(message + strlen(message), "Mode2: online;");
                }
                else
                {
                    //保持在线，不检测震动，每隔m分钟，自动上送
                    sprintf(message + strlen(message), "Mode2: %dM;", sysparam.gapMinutes);
                }
            }
            else
            {

                if (sysparam.gapMinutes == 0)
                {
                    //检测到震动，n 秒上送
                    sprintf(message + strlen(message), "Mode2: %dS;", sysparam.gpsuploadgap);
                }
                else
                {
                    //检测到震动，n 秒上送，未震动，m分钟自动上送
                    sprintf(message + strlen(message), "Mode2: %dS,%dM;", sysparam.gpsuploadgap, sysparam.gapMinutes);

                }
            }
            break;

        case MODE3:
            sprintf(message + strlen(message), "Mode3: %d minutes;", sysparam.gapMinutes);
            break;

        case MODE4:
            sprintf(message + strlen(message), "Mode4: %d M,%dM;", sysparam.gapMinutes, sysparam.noNetWakeUpMinutes);
            break;
        case MODE23:
            sprintf(message + strlen(message), "Mode23: %d minutes;", sysparam.gapMinutes);
            break;
    }
    sprintf(message + strlen(message), "MODE1:%d;MODE2:%d;", sysparam.startUpCnt, sysparam.runTime);
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}


void doStatusInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    GPSINFO *gpsinfo;
    portUpdateStep();
    sprintf(message, "BAT-VOLTAGE=%.2fV;", sysinfo.outsidevoltage);
    if (sysinfo.GPSStatus)
    {
        gpsinfo = getCurrentGPSInfo();
        sprintf(message + strlen(message), "GPS=%s;", gpsinfo->fixstatus ? "Fixed" : "Invalid");
        sprintf(message + strlen(message), "PDOP=%.2f;", gpsinfo->pdop);
    }
    else
    {
        sprintf(message + strlen(message), "GPS=Close;");
    }

    sprintf(message + strlen(message), "ACC=%s;", getTerminalAccState() > 0 ? "On" : "Off");
    sprintf(message + strlen(message), "SIGNAL=%d;", getModuleRssi());
    sprintf(message + strlen(message), "BATTERY=%s;", getTerminalChargeState() > 0 ? "Charging" : "Uncharged");
    sprintf(message + strlen(message), "STEP=%d;", sysinfo.step);
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

void doSNInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char debug[50];
    char IMEI[15];
    uint8_t sndata[30];
    if (item->item_data[1])
        if (my_strpach(item->item_data[1], "ZTINFO") && my_strpach(item->item_data[2], "SN"))
        {
            sprintf(debug, "%s\n", "Update sn number;");
            LogMessage(DEBUG_ALL, debug);
            changeHexStringToByteArray(sndata, (uint8_t *)item->item_data[3], strlen(item->item_data[3]) / 2);
            decryptSN(sndata, IMEI);
            sprintf(debug, "Decrypt:%s\n", IMEI);
            LogMessage(DEBUG_ALL, debug);
            paramSaveSnNumber((uint8_t *)IMEI, 15);
            paramGetSnNumber(sysparam.SN, 15);
        }
}

void doServerInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    paramSaveServer((uint8_t *)item->item_data[2], (uint32_t)atol((const char *)item->item_data[3]));
    paramGetServer(sysparam.Server, &sysparam.ServerPort);
    sprintf(message, "Update domain %s:%d;", sysparam.Server, sysparam.ServerPort);
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
    startTimer(4000, reConnectServer, 0);
}
void doVersionInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    sprintf(message, "Version:%s;Compile:%s %s;", CODEVERSION, __DATE__, __TIME__);
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);

}
void doHbtInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        paramGetHeartbeatInterval(&sysparam.heartbeatgap);
        sprintf(message, "The time of the heartbeat interval is %d seconds;", sysparam.heartbeatgap);
    }
    else
    {
        sysparam.heartbeatgap = (uint16_t)atoi((const char *)item->item_data[1]);
        paramSaveHeartbeatInterval(sysparam.heartbeatgap);
        sprintf(message, "Update the time of the heartbeat interval to %d seconds;", sysparam.heartbeatgap);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

void doModeInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    uint8_t workmode, i, j, timecount = 0, gapday = 1;
    uint16_t mode1time[7];
    uint16_t valueofminute;
    char message[200];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        paramGetGPSUploadInterval(&sysparam.gpsuploadgap);
        sprintf(message, "Current Mode %d", sysparam.MODE);
        sprintf(message + strlen(message), ",gps upload gap %ds,%dm,%dm", sysparam.gpsuploadgap,
                sysparam.gapMinutes, sysparam.noNetWakeUpMinutes);
    }
    else
    {
        workmode = atoi(item->item_data[1]);
        gpsRequestClear(GPS_REQUEST_GPSKEEPOPEN_CTL);
        //开后再关，容易造成设备复位，硬件问题
        switch (workmode)
        {
            case 1:
            case 21:
                //内容项如果大于2，说明有时间或日期
                if (item->item_cnt > 2)
                {
                    for (i = 0; i < item->item_cnt - 2; i++)
                    {
                        if (strlen(item->item_data[2 + i]) <= 4 && strlen(item->item_data[2 + i]) >= 3)
                        {
                            valueofminute = atoi(item->item_data[2 + i]);
                            mode1time[timecount++] = valueofminute / 100 * 60 + valueofminute % 100;
                        }
                        else
                        {
                            gapday = atoi(item->item_data[2 + i]);
                        }
                    }

                    for (i = 0; i < (timecount - 1); i++)
                    {
                        for (j = 0; j < (timecount - 1 - i); j++)
                        {
                            if (mode1time[j] > mode1time[j + 1]) //相邻两个元素作比较，如果前面元素大于后面，进行交换
                            {
                                valueofminute = mode1time[j + 1];
                                mode1time[j + 1] = mode1time[j];
                                mode1time[j] = valueofminute;
                            }
                        }
                    }

                }

                for (i = 0; i < 5; i++)
                {
                    sysparam.AlarmTime[i] = 0xFFFF;
                }
                //重现写入AlarmTime
                for (i = 0; i < timecount; i++)
                {
                    sysparam.AlarmTime[i] = mode1time[i];
                }
                paramSaveAlarmTime();
                paramSaveMode1GapDay(gapday);
                if (workmode == MODE1)
                {
                    terminalAccoff();
                    if (gpsRequestGet(GPS_REQUEST_ACC_CTL))
                    {
                        gpsRequestClear(GPS_REQUEST_ACC_CTL);
                    }
                    paramSaveMode(MODE1);
                    portGsensorCfg(0);
                }
                else
                {
                    paramSaveMode(MODE21);
                    portGsensorCfg(1);
                }
                sprintf(message, "Change to Mode%d,and work on at", workmode);
                for (i = 0; i < timecount; i++)
                {
                    sprintf(message + strlen(message), " %.2d:%.2d", sysparam.AlarmTime[i] / 60, sysparam.AlarmTime[i] % 60);
                }
                sprintf(message + strlen(message), ",every %d day", gapday);
                portSetNextAlarmTime();
                break;
            case 2:
                //MODE,2,0,0
                //MODE,2,0,M
                //MODE,2,N,M

                portGsensorCfg(1);
                paramSaveMode(MODE2);

                sysparam.gpsuploadgap = (uint8_t)atoi((const char *)item->item_data[2]);
                paramSaveGPSUploadInterval(sysparam.gpsuploadgap);
                sysparam.gapMinutes = atoi(item->item_data[3]);
                paramSaveInterval();

                if (sysparam.accctlgnss == 0)
                {
                    gpsRequestSet(GPS_REQUEST_GPSKEEPOPEN_CTL);
                }
                if (sysparam.gpsuploadgap == 0)
                {
                    //运动不自动传GPS
                    if (sysparam.gapMinutes == 0)
                    {

                        sprintf(message, "The device switches to mode 2 without uploading the location");
                    }
                    else
                    {
                        sprintf(message, "The device switches to mode 2 and uploads the position every %d minutes all the time",
                                sysparam.gapMinutes);
                    }
                }
                else
                {

                    if (sysparam.gapMinutes == 0)
                    {
                        sprintf(message, "The device switches to mode 2 and uploads the position every %d seconds when moving",
                                sysparam.gpsuploadgap);

                    }
                    else
                    {
                        sprintf(message,
                                "The device switches to mode 2 and uploads the position every %d seconds when moving, and every %d minutes when standing still",
                                sysparam.gpsuploadgap, sysparam.gapMinutes);
                    }
                    if (getTerminalAccState())
                    {
                        gpsRequestSet(GPS_REQUEST_ACC_CTL);
                        gpsRequestSet(GPS_REQUEST_UPLOAD_ONE);
                    }
                }
                break;
            case 3:
            case 4:
            case 23:
                sysparam.noNetWakeUpMinutes = 0;
                if (workmode == 3)
                {
                    sysparam.gapMinutes = atoi(item->item_data[2]);
                    if (sysparam.gapMinutes <= 5)
                    {
                        sysparam.gapMinutes = 5;
                    }
                    terminalAccoff();
                    if (gpsRequestGet(GPS_REQUEST_ACC_CTL))
                    {
                        gpsRequestClear(GPS_REQUEST_ACC_CTL);
                    }
                    gpsRequestClear(GPS_REQUEST_ACC_CTL);
                    paramSaveMode(MODE3);
                    portGsensorCfg(0);
                    portSetNextWakeUpTime();
                }
                else if (workmode == 4)
                {
                    sysparam.gapMinutes = atoi(item->item_data[2]);
                    sysparam.noNetWakeUpMinutes = atoi(item->item_data[3]);
                    if (sysparam.noNetWakeUpMinutes == 0)
                    {
                        sysparam.noNetWakeUpMinutes = 60;
                    }
                    paramSaveNoNetWakeUpMinutes(sysparam.noNetWakeUpMinutes);
                    terminalAccoff();
                    if (gpsRequestGet(GPS_REQUEST_ACC_CTL))
                    {
                        gpsRequestClear(GPS_REQUEST_ACC_CTL);
                    }
                    gpsRequestClear(GPS_REQUEST_ACC_CTL);
                    paramSaveMode(MODE4);
                }
                else
                {
                    sysparam.gapMinutes = atoi(item->item_data[2]);
                    if (sysparam.gapMinutes <= 5)
                    {
                        sysparam.gapMinutes = 5;
                    }
                    paramSaveMode(MODE23);
                    portSetNextWakeUpTime();
                    portGsensorCfg(1);
                }
                paramSaveInterval();
                sprintf(message, "Change to mode %d and update the startup interval time to %d minutes", workmode,
                        sysparam.gapMinutes);
                if (sysparam.noNetWakeUpMinutes != 0)
                {
                    sprintf(message + strlen(message), ",sleep %d minutes", sysparam.noNetWakeUpMinutes);
                }
                break;
            default:
                strcpy(message, "Unsupport mode");
                break;
        }
        updateSystemStartTime();
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

void doTTSInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char param[4];
    uint8_t decode[300];
    uint16_t decodelen;
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        sendMessageWithDifMode((uint8_t *)"Welcome to use TTS", 18, mode, telnum);
    }
    else
    {
        decodelen = strlen(item->item_data[1]);
        if (decodelen > 140)
        {
            sendMessageWithDifMode((uint8_t *)"Message too long", 16, mode, telnum);
        }
        else
        {
            LogMessage(DEBUG_ALL, "Play:");
            LogMessage(DEBUG_ALL, (char *)item->item_data[1]);
            LogMessage(DEBUG_ALL, "\n");
            changeByteArrayToHexString((uint8_t *)item->item_data[1], decode, decodelen);
            decode[decodelen * 2] = 0;
            sendMessageWithDifMode((uint8_t *)"The playback was successful", 28, mode, telnum);
            //播放长度
            sprintf(param, "%d", decodelen);
            //发送指令
            sendModuleCmd(N58_TTSPLAY_CMD, param);
            //发送TTS内容
            CreateNodeCmd((char *)decode, decodelen * 2, N58_TTSPLAY_CMD);
        }
    }

}

void doJtInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    int16_t recordtime, micl = 6;
    char param[4];
    char message[100];
    if (sysinfo.recordingflag || resIsCycleRuning())
    {
        sprintf(message, "The device is recording,please try again later");
        goto Done;
    }



    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        recordtime = 30;
        sprintf(param, "%d", micl);
        sendModuleCmd(N58_MICL_CMD, param);
        recStart();
        startTimer(recordtime * 1000, recStopAndUpload, 0);
        sprintf(message, "Start recording for %d seconds\n", recordtime);
    }
    else
    {
        recordtime = atoi(item->item_data[1]);
        if (recordtime > 300 || recordtime <= 0)
            recordtime = 30;
        if (item->item_data[2][0] != NULL)
        {
            micl = atoi(item->item_data[2]);
            if (micl > 9 || micl < 0)
            {
                micl = 3;
            }
        }
        sprintf(param, "%d", micl);
        sendModuleCmd(N58_MICL_CMD, param);
        recStart();
        startTimer(recordtime * 1000, recStopAndUpload, 0);
        sprintf(message, "Start recording for %d seconds\n", recordtime);

    }
Done :
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}


//(357784086584883)<Local Time:2020-09-23 16:09:33>http://maps.google.com/maps?q=22.58799,113.85864

static DOINSTRUCTIONMODE mode123;
static char telnum123[30];

void dorequestSend123(void)
{
    char message[150];
    uint16_t year;
    uint8_t  month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    GPSINFO *gpsinfo;

    portGetSystemDateTime(&year, &month, &date, &hour, &minute, &second);
    sysinfo.flag123 = 0;
    gpsinfo = getCurrentGPSInfo();
    sprintf(message, "(%s)<Local Time:%.2d/%.2d/%.2d %.2d:%.2d:%.2d>http://maps.google.com/maps?q=%s%f,%s%f", sysparam.SN, \
            year, month, date, hour, minute, second, \
            gpsinfo->NS == 'N' ? "" : "-", gpsinfo->latitude, gpsinfo->EW == 'E' ? "" : "-", gpsinfo->longtitude);
    reCover123InstructionId();
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode123, telnum123);
}

void do123Instruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    mode123 = mode;
    strcpy(telnum123, telnum);
    if (sysparam.poitype == 0)
    {
        sysinfo.lbsrequest = 1;
        LogMessage(DEBUG_ALL, "Only LBS reporting\n");
    }
    else if (sysparam.poitype == 1)
    {
        sysinfo.lbsrequest = 1;
        gpsRequestSet(GPS_REQUEST_UPLOAD_ONE);
        LogMessage(DEBUG_ALL, "LBS and GPS reporting\n");


    }
    else
    {
        sysinfo.lbsrequest = 1;
        sysinfo.wifirequest = 1;
        gpsRequestSet(GPS_REQUEST_UPLOAD_ONE);
        LogMessage(DEBUG_ALL, "LBS ,WIFI and GPS reporting\n");
    }
    sysinfo.flag123 = 1;
    save123InstructionId();
    updateSystemStartTime();

}

void doAPNInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[200];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        paramGetApnName(sysparam.apn);
        paramGetApnUser(sysparam.apnuser);
        paramGetApnPswd(sysparam.apnpassword);
        sprintf(message, "APN:%s,APN User:%s,APN Password:%s", sysparam.apn, sysparam.apnuser, sysparam.apnpassword);
    }
    else
    {
        if (item->item_data[1][0] != NULL && item->item_cnt >= 2)
        {
            paramSaveApnUser(NULL);
            paramSaveApnPswd(NULL);
            sysparam.apn[0] = 0;
            sysparam.apnuser[0] = 0;
            sysparam.apnpassword[0] = 0;
            strcpy((char *)sysparam.apn, item->item_data[1]);
            paramSaveApnName(sysparam.apn);
        }
        if (item->item_data[2][0] != NULL && item->item_cnt >= 3)
        {
            strcpy((char *)sysparam.apnuser, item->item_data[2]);
            paramSaveApnUser(sysparam.apnuser);

        }
        if (item->item_data[3][0] != NULL && item->item_cnt >= 4)
        {
            strcpy((char *)sysparam.apnpassword, item->item_data[3]);
            paramSaveApnPswd(sysparam.apnpassword);

        }
        sprintf(message, "Update APN:%s,APN User:%s,APN Password:%s", sysparam.apn, sysparam.apnuser, sysparam.apnpassword);
        startTimer(4000, moduleReset, 0);
    }

    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}


void doUPSInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[200];
    uint16_t port;
    uint8_t server[50];
    if (item->item_data[1][0] == NULL)
    {
        paramGetUPSServer(server, &port);
    }
    else
    {
        if (item->item_cnt == 3)
        {
            port = atoi(item->item_data[2]);
            paramSaveUPSServer((uint8_t *)item->item_data[1], port);
            paramGetUPSServer(server, &port);
        }
    }
    sprintf(message, "The device will download the firmware from %s:%d in 5 seconds", server, port);
    paramSaveUpdateStatus(1);
    startTimer(4000, portSystemReset, 0);
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

void doLOWWInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        paramGetLoww(&sysparam.lowvoltage);
        sysinfo.lowvoltage = sysparam.lowvoltage / 10.0;
        sprintf(message, "The low voltage param is %.1fV", sysinfo.lowvoltage);

    }
    else
    {
        sysparam.lowvoltage = atoi(item->item_data[1]);
        if (sysparam.lowvoltage > 38)
            sysparam.lowvoltage = 36;
        sysinfo.lowvoltage = sysparam.lowvoltage / 10.0;
        paramSaveLoww(sysparam.lowvoltage);
        sprintf(message, "When the voltage is below %.1fV, the device will upload a alarm.", sysinfo.lowvoltage);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

void doLEDInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        sprintf(message, "LED was %s", sysparam.ledctrl ? "open" : "close");

    }
    else
    {
        sysparam.ledctrl = atoi(item->item_data[1]);
        paramSaveLedCtl(sysparam.ledctrl);
        sprintf(message, "%s", sysparam.ledctrl ? "LED ON" : "LED OFF");
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}


void doPOITYPEInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        switch (sysparam.poitype)
        {
            case 0:
                strcpy(message, "Current poitype is only LBS reporting");
                break;
            case 1:
                strcpy(message, "Current poitype is LBS and GPS reporting");
                break;
            case 2:
                strcpy(message, "Current poitype is LBS ,WIFI and GPS reporting");
                break;
        }
    }
    else
    {
        if (strstr(item->item_data[1], "AUTO") != NULL)
        {
            sysparam.poitype = 2;
        }
        else
        {
            sysparam.poitype = atoi(item->item_data[1]);
        }
        //强制使用POITYP2
        sysparam.poitype = 2;
        switch (sysparam.poitype)
        {
            case 0:
                strcpy(message, "Set to only LBS reporting");
                break;
            case 1:
                strcpy(message, "Set to LBS and GPS reporting");
                break;
            case 2:
                strcpy(message, "Set to LBS ,WIFI and GPS reporting");
                break;
            default:
                sysparam.poitype = 2;
                strcpy(message, "Unknow type,default set to LBS ,WIFI and GPS reporting");
                break;
        }
        paramSavePoitype(sysparam.poitype);

    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

void doResetInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[50];
    sprintf(message, "System will reset after 5 seconds");
    startTimer(5000, portSystemReset, 0);
    paramSaveMode2cnt(sysparam.runTime);
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

void doUTCInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        sprintf(message, "System time zone:UTC %s%d", sysparam.utc >= 0 ? "+" : "", sysparam.utc);
        updateRTCtimeRequest();
    }
    else
    {
        sysparam.utc = atoi(item->item_data[1]);
        updateRTCtimeRequest();
        LogPrintf(DEBUG_ALL, "utc=%d\n", sysparam.utc);
        if (sysparam.utc < -12 || sysparam.utc > 12)
            sysparam.utc = 8;
        paramSaveUTC(sysparam.utc);
        sprintf(message, "Update the system time zone to UTC %s%d", sysparam.utc >= 0 ? "+" : "", sysparam.utc);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}
void doAlarmModeInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        sprintf(message, "The light-sensing alarm function was %s", sysparam.Light_Alarm_En ? "enable" : "disable");
    }
    else
    {
        if (my_strpach(item->item_data[1], "L1"))
        {
            sysparam.Light_Alarm_En = 1;
            sprintf(message, "Enables the light-sensing alarm function successfully");
        }
        else if (my_strpach(item->item_data[1], "L0"))
        {
            sysparam.Light_Alarm_En = 0;
            sprintf(message, "Disable the light-sensing alarm function successfully");

        }
        else
        {
            sysparam.Light_Alarm_En = 1;
            sprintf(message, "Unknow cmd,enable the light-sensing alarm function by default");
        }
        paramSaveLightAlarmMode(sysparam.Light_Alarm_En);

    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

//DEBUG,MODULE,AT+GMR=1,3,2,3
void doDebugInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[256];
    uint16_t year;
    uint8_t  month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;


    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        portGetSystemDateTime(&year, &month, &date, &hour, &minute, &second);
        sprintf(message, "Time:%.2d/%.2d/%.2d %.2d:%.2d:%.2d;", year, month, date, hour, minute, second);
        sprintf(message + strlen(message), "Sysrun:%.2d:%.2d:%.2d;gpsrequest:%02X;gpslast:%.2d:%.2d:%.2d;",
                sysinfo.System_Tick / 3600, sysinfo.System_Tick % 3600 / 60, sysinfo.System_Tick % 60, sysinfo.GPSRequest,
                sysinfo.gpsUpdatetick / 3600, sysinfo.gpsUpdatetick % 3600 / 60, sysinfo.gpsUpdatetick % 60);
        sprintf(message + strlen(message), "HF:%d;MF:%d;", sysparam.hardfault, sysparam.mallocfault);
    }
    else
    {
        if (my_strpach(item->item_data[1], "DEBUGUART"))
        {
            if (item->item_data[2][0] == '1')
            {
                strcpy(message, "Debug:open uart1");
                systemRequestSet(SYSTEM_OPENUART1_REQUEST);
            }
            else if (item->item_data[2][0] == '0')
            {
                strcpy(message, "Debug:close uart1");
                systemRequestSet(SYSTEM_CLOSEUART1_REQUEST);
            }
        }
        else if (my_strpach(item->item_data[1], "MODECNTCLEAR"))
        {
            paramSaveMode2cnt(0);
            paramSaveMode1Timer(0);
            strcpy(message, "Debug:clear mode count");
        }
        else if (my_strpach(item->item_data[1], "MODULEVER"))
        {
            sprintf(message, "Debug:%s", sysinfo.moduleGMR);
        }
        else if (my_strpach(item->item_data[1], "MODULE"))
        {
            sprintf(message, "%s", item->item_data[2]);
            if (item->item_data[3][0] != 0)
            {
                sprintf(message + strlen(message), "=%s", item->item_data[3]);
            }
            if (item->item_data[4][0] != 0)
            {
                sprintf(message + strlen(message), ",%s", item->item_data[4]);
            }
            if (item->item_data[5][0] != 0)
            {
                sprintf(message + strlen(message), ",%s", item->item_data[5]);
            }
            if (item->item_data[6][0] != 0)
            {
                sprintf(message + strlen(message), ",%s", item->item_data[6]);
            }
            sprintf(message + strlen(message), "%s", "\r\n");
            portUartSend(&usart2_ctl, (uint8_t *)message, strlen(message));
            strcpy(message, "Debug:Send OK");
        }
        else if (my_strpach(item->item_data[1], "STEPCLEAR"))
        {
            portClearStep();
            strcpy(message, "Debug:Clear OK");
        }
        else
        {
            strcpy(message, "Debug:Unknow cmd");
        }
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

void doACCCTLGNSSInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        sprintf(message, "%s", sysparam.accctlgnss ? "GPS is automatically controlled by the program" :
                "The GPS is always be on");
    }
    else
    {
        sysparam.accctlgnss = (uint8_t)atoi((const char *)item->item_data[1]);
        if (sysparam.MODE == MODE2)
        {
            if (sysparam.accctlgnss == 0)
            {
                gpsRequestSet(GPS_REQUEST_GPSKEEPOPEN_CTL);
            }
            else
            {
                gpsRequestClear(GPS_REQUEST_GPSKEEPOPEN_CTL);
            }
        }
        sprintf(message, "%s", sysparam.accctlgnss ? "GPS will be automatically controlled by the program" :
                "The GPS will always be on");
        paramSaveACCCTLGNSS(sysparam.accctlgnss);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);

}



void doPdopInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        sprintf(message, "Current podp filter is %.2f", sysparam.pdop / 100.0);
    }
    else
    {
        sysparam.pdop = atoi(item->item_data[1]);
        paramSavePdop(sysparam.pdop);
        sprintf(message, "Update podp filter to %.2f", sysparam.pdop / 100.0);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}



void doSetblemacInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        if (sysparam.blemac[0] == 0)
        {
            sprintf(message, "The ble function is disable");
        }
        else
        {
            sprintf(message, "The ble function is enable ,and the ble mac is %s", sysparam.blemac);
        }
    }
    else
    {
        if (strlen(item->item_data[1]) == 12)
        {
            sysparam.blemac[0] = item->item_data[1][0];
            sysparam.blemac[1] = item->item_data[1][1];
            sysparam.blemac[2] = ':';
            sysparam.blemac[3] = item->item_data[1][2];
            sysparam.blemac[4] = item->item_data[1][3];
            sysparam.blemac[5] = ':';
            sysparam.blemac[6] = item->item_data[1][4];
            sysparam.blemac[7] = item->item_data[1][5];
            sysparam.blemac[8] = ':';
            sysparam.blemac[9] = item->item_data[1][6];
            sysparam.blemac[10] = item->item_data[1][7];
            sysparam.blemac[11] = ':';
            sysparam.blemac[12] = item->item_data[1][8];
            sysparam.blemac[13] = item->item_data[1][9];
            sysparam.blemac[14] = ':';
            sysparam.blemac[15] = item->item_data[1][10];
            sysparam.blemac[16] = item->item_data[1][11];
            sysparam.blemac[17] = 0;
            paramSaveBleMac(sysparam.blemac);
            sprintf(message, "Enable the ble function,and update the ble mac to %s", sysparam.blemac);
        }
        else
        {
            memset(sysparam.blemac, 0, 19);
            sprintf(message, "Disable the ble function,and the ble mac was clear");
        }
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}


void doBFInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    paramSaveBF(1);
    strcpy(message, "BF OK");
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}


void doCFInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    paramSaveBF(0);
    strcpy(message, "CF OK");
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

void doFactoryTestInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    uint8_t total, i;
    GPSINFO *gpsinfo;
    char message[300];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        //感光检测
        sprintf(message, "Light:%s;", LDRDET ? "darkness" : "brightness");
        //acc线检测
        sprintf(message + strlen(message), "CSQ:%d;", getModuleRssi());
        //Gsensor检测
        if (read_gsensor_id() != 0)
        {
            sprintf(message + strlen(message), "Gsensor:ERROR;");
        }
        else
        {
            sprintf(message + strlen(message), "Gsensor:OK;");
        }
        //电压检测
        getBatVoltage();
        sprintf(message + strlen(message), "vBat:%.3fV;", sysinfo.outsidevoltage);
        //Apn
        sprintf(message + strlen(message), "APN:%s;APN User:%s;APN Password:%s;", sysparam.apn, sysparam.apnuser,
                sysparam.apnpassword);
        //gps
        gpsinfo = getCurrentGPSInfo();
        total = sizeof(gpsinfo->gpsCn);
        sprintf(message + strlen(message), "GPS CN:");

        for (i = 0; i < total; i++)
        {
            if (gpsinfo->gpsCn[i] != 0)
            {
                sprintf(message + strlen(message), "%d,", gpsinfo->gpsCn[i]);
            }
        }

        sprintf(message + strlen(message), ";BeiDou CN:");
        total = sizeof(gpsinfo->beidouCn);

        for (i = 0; i < total; i++)
        {
            if (gpsinfo->beidouCn[i] != 0)
            {
                sprintf(message + strlen(message), "%d,", gpsinfo->beidouCn[i]);
            }
        }
        sprintf(message + strlen(message), ";");
    }
    else
    {

    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

void doFenceInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        sprintf(message, "The static drift fence is %d meters", sysparam.fence);
    }
    else
    {
        sysparam.fence = atol(item->item_data[1]);
        paramSaveFence(sysparam.fence);
        sprintf(message, "Set the static drift fence to %d meters", sysparam.fence);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

void doFactoryInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];

    if (my_strpach(item->item_data[1], "ZTINFO8888"))
    {
        paramDefaultInit(0);
        sprintf(message, "Factory all successfully");
    }
    else
    {
        paramDefaultInit(1);
        sprintf(message, "Factory Settings restored successfully");

    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

void doSmsreplyInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        sprintf(message, "SMS replies was %s", sysparam.smsRespon ? "enable" : "disable");
    }
    else
    {
        sysparam.smsRespon = atol(item->item_data[1]);
        sprintf(message, "%s sms replies", sysparam.smsRespon ? "Enable" : "Disable");
    }
    paramSaveSmsreply(sysparam.smsRespon);
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);

}

void doJTCYCLEInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    uint8_t value;
    char message[100];

    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        sprintf(message, "recording cycle %s runnig", resIsCycleRuning() ? "is" : "is not");
    }
    else
    {
        value = atoi(item->item_data[1]);

        if (value)
        {
            if (sysinfo.recordingflag || resIsCycleRuning())
            {
                sprintf(message, "The device is recording,please try again later");
            }
            else
            {
                sprintf(message, "%s", "Start recording cycle");
                recCycleStart();
            }
        }
        else
        {
            sprintf(message, "%s", "Stop recording cycle");
            recCycleStop();
        }
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);

}


void doAnswerInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        sprintf(message, "Answering phone function is %s", sysparam.autoAnswer ? "enable" : "disable");
    }
    else
    {
        sysparam.autoAnswer = atoi(item->item_data[1]);
        sprintf(message, "%s the answering phone function", sysparam.autoAnswer ? "Enable" : "Disable");
        paramSaveAutoAnswer(sysparam.autoAnswer);

    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}
void doTurnAlgInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        sprintf(message, "The turn algorithm is %s", sysparam.turnalg ? "enable" : "disable");
    }
    else
    {
        sysparam.turnalg = atoi(item->item_data[1]);
        sprintf(message, "%s the turn algorithm function", sysparam.turnalg ? "Enable" : "Disable");
        paramSaveTurnalg(sysparam.turnalg);

    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

void doAdccalInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    float v = 4.17;
    float voltage = 0;
    uint8_t i;
    if (item->item_data[1][0] != 0)
    {
        v = atoi(item->item_data[1]) / 100.0;
    }
    for (i = 0; i < 10; i++)
    {
        voltage += (((float)portGetAdc() / 4095) * 1.8);
        HAL_Delay(10);
    }
    voltage /= 10.0;
    sysparam.adccal = v / voltage ;
    paramSaveAdcCal(sysparam.adccal);
    sprintf(message, "RealVoltage:%.2f,Update new calibration parameter to %f", v, sysparam.adccal);
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

void doSetAgpsInstruction(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        sprintf(message, "Agps:%s,%d,%s,%s", sysparam.agpsServer, sysparam.agpsPort, sysparam.agpsUser, sysparam.agpsPswd);
    }
    else
    {
        if (item->item_data[1][0] != 0)
        {
            strcpy((char *)sysparam.agpsServer, item->item_data[1]);
            paramSaveAgpsServer();
        }
        if (item->item_data[2][0] != 0)
        {
            sysparam.agpsPort = atoi(item->item_data[2]);
            paramSaveAgpsPort();
        }
        if (item->item_data[3][0] != 0)
        {
            strcpy((char *)sysparam.agpsUser, item->item_data[3]);
            paramSaveAgpsUser();
        }
        if (item->item_data[4][0] != 0)
        {
            strcpy((char *)sysparam.agpsPswd, item->item_data[4]);
            paramSaveAgpsPswd();
        }
        sprintf(message, "Update Agps info:%s,%d,%s,%s", sysparam.agpsServer, sysparam.agpsPort, sysparam.agpsUser,
                sysparam.agpsPswd);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

void doAudioInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        sprintf(message, "%s", "Please enter your param");
    }
    else
    {
        if (sysinfo.audioPlayNow == 0)
        {
            sysinfo.audioInd = atoi(item->item_data[1]);
            sysinfo.playAudioCnt = atoi(item->item_data[2]);
            if (sysinfo.playAudioCnt == 0)
            {
                sysinfo.playAudioCnt = 1;
            }
            startTimer(1500, playAudio, 0);
            sprintf(message, "Play the Music%d.amr, %d times", sysinfo.audioInd, sysinfo.playAudioCnt);
        }
        else
        {
            strcpy(message, "Audio was play now, try again later");
        }
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

void doVolInstrucion(ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char message[100];
    uint8_t vol;
    if (item->item_data[1][0] == NULL || item->item_data[1][0] == '?')
    {
        sprintf(message, "%s", "Please enter your param");
    }
    else
    {
        vol = atoi(item->item_data[1]);
        if (vol > 100)
        {
            vol = 100;
        }
        setModuleVol(vol);
		sprintf(message, "Update Vol to %d", vol);
    }
    sendMessageWithDifMode((uint8_t *)message, strlen(message), mode, telnum);
}

