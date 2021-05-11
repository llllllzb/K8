#include "app_atcmd.h"
#include "app_sys.h"
#include "app_kernal.h"
#include "app_task.h"
#include "app_net.h"
#include "app_mir3da.h"
#include "app_param.h"
#include "app_gps.h"
#include "app_port.h"
#include "app_sn.h"
#include "app_serverprotocol.h"
#include "app_instructioncmd.h"


const CMDTABLE atcmdtable[] =
{
    {AT_SMS_CMD, "SMS"},
    {AT_FMPC_NMEA_CMD, "FMPC_NMEA"},
    {AT_FMPC_BAT_CMD, "FMPC_BAT"},
    {AT_FMPC_GSENSOR_CMD, "FMPC_GSENSOR"},
    {AT_FMPC_ACC_CMD, "FMPC_ACC"},
    {AT_FMPC_GSM_CMD, "FMPC_GSM"},
    {AT_FMPC_RELAY_CMD, "FMPC_RELAY"},
    {AT_FMPC_CSQ_CMD, "FMPC_CSQ"},
    {AT_DEBUG_CMD, "DEBUG"},
    {AT_ADCCAL_CMD, "FMPC_ADCCAL"},
    {AT_FMPC_LDR_CMD, "FMPC_LDR"},
    {AT_NMEA_CMD, "NMEA"},
    {AT_FMPC_ALL_CMD, "FMPC_ALL"},
    {AT_FMPC_CM_CMD, "FMPC_CM"},

    {AT_ZTSN_CMD, "ZTSN"},
    {AT_IMEI_CMD, "IMEI"},
    {AT_FMPC_IMSI_CMD, "FMPC_IMSI"},
    {AT_FMPC_CHKP_CMD, "FMPC_CHKP"},
    {AT_FMCP_CMGET_CMD, "FMPC_CMGET"},
};

const CMDTABLE instructiontable[] =
{
    {PARAM_INS, "PARAM"},
    {STATUS_INS, "STATUS"},
    {VERSION_INS, "VERSION"},
    {SERVER_INS, "SERVER"},
    {HBT_INS, "HBT"},
    {MODE_INS, "MODE"},
    {TTS_INS, "TTS"},
    {JT_INS, "JT"},
    {POSITION_INS, "123"},
    {APN_INS, "APN"},
    {UPS_INS, "UPS"},
    {LOWW_INS, "LOWW"},
    {LED_INS, "LED"},
    {POITYPE_INS, "POITYPE"},
    {RESET_INS, "RESET"},
    {UTC_INS, "UTC"},
    {ALARMMODE_INS, "ALARMMODE"},
    {DEBUG_INS, "DEBUG"},
    {ACCCTLGNSS_INS, "ACCCTLGNSS"},
    {PDOP_INS, "PDOP"},
    {SETBLEMAC_INS, "SETBLEMAC"},
    {BF_INS, "BF"},
    {CF_INS, "CF"},
    {FACTORYTEST_INS, "FACTORYTEST"},
    {FENCE_INS, "FENCE"},
    {FACTORY_INS, "FACTORY"},
    {SMSREPLY_INS, "SMSREPLY"},
    {JTCYCLE_INS, "JTCYCLE"},
    {ANSWER_INS, "ANSWER"},
    {TURNALG_INS, "TURNALG"},
    {SN_INS, "*"},
};

static uint8_t mycmdPatch(uint8_t *cmd1, uint8_t *cmd2)
{
    uint8_t ilen1, ilen2;
    if (cmd1 == NULL || cmd2 == NULL)
        return 0;
    ilen1 = strlen((char *)cmd1);
    ilen2 = strlen((char *)cmd2);
    if (ilen1 != ilen2)
        return 0;
    for (ilen1 = 0; ilen1 < ilen2; ilen1++)
    {
        if (cmd1[ilen1] != cmd2[ilen1])
            return 0;
    }
    return 1;
}


static int16_t getatcmdid(uint8_t *cmdstr)
{
    uint16_t i = 0;
    for (i = 0; i < AT_MAX_CMD; i++)
    {
        if (mycmdPatch(cmdstr, (uint8_t *)atcmdtable[i].cmdstr) != NULL)
            return atcmdtable[i].cmdid;
    }
    return -1;
}

static int16_t getInstructionid(uint8_t *cmdstr)
{
    uint16_t i = 0;
    if (cmdstr == NULL)
    {
        LogMessage(DEBUG_ALL, "getInstructionid==>no cmd\n");
        return -1;
    }
    for (i = 0; i < MAX_INS; i++)
    {
        if (mycmdPatch(cmdstr, (uint8_t *)instructiontable[i].cmdstr) != NULL)
            return instructiontable[i].cmdid;
    }
    return -1;
}

void clearInstrucionRequest(void)
{
    sysinfo.instructionqequest = 0;
}


static void doinstruction(int16_t cmdid, ITEM *item, DOINSTRUCTIONMODE mode, char *telnum)
{
    char debug[50];
#ifdef RI_ENABLE
    sysinfo.instructionqequest = 1;
    startTimer(6000, clearInstrucionRequest, 0);
#endif
    switch (cmdid)
    {
        case PARAM_INS:
            doParamInstruction(item, mode, telnum);
            break;
        case STATUS_INS:
            doStatusInstruction(item, mode, telnum);
            break;
        case VERSION_INS:
            doVersionInstruction(item, mode, telnum);
            break;
        case SN_INS:
            doSNInstruction(item, mode, telnum);
            break;
        case SERVER_INS:
            doServerInstruction(item, mode, telnum);
            break;
        case HBT_INS:
            doHbtInstruction(item, mode, telnum);
            break;
        case MODE_INS:
            doModeInstruction(item, mode, telnum);
            break;
        case TTS_INS:
            doTTSInstruction(item, mode, telnum);
            break;
        case JT_INS:
            doJtInstruction(item, mode, telnum);
            break;
        case POSITION_INS:
            do123Instruction(item, mode, telnum);
            break;
        case APN_INS:
            doAPNInstruction(item, mode, telnum);
            break;
        case UPS_INS:
            doUPSInstruction(item, mode, telnum);
            break;
        case LOWW_INS:
            doLOWWInstruction(item, mode, telnum);
            break;
        case LED_INS:
            doLEDInstruction(item, mode, telnum);
            break;
        case POITYPE_INS:
            doPOITYPEInstruction(item, mode, telnum);
            break;
        case RESET_INS:
            doResetInstruction(item, mode, telnum);
            break;
        case UTC_INS:
            doUTCInstruction(item, mode, telnum);
            break;
        case ALARMMODE_INS:
            doAlarmModeInstrucion(item, mode, telnum);
            break;
        case DEBUG_INS:
            doDebugInstrucion(item, mode, telnum);
            break;
        case ACCCTLGNSS_INS:
            doACCCTLGNSSInstrucion(item, mode, telnum);
            break;
        case PDOP_INS:
            doPdopInstrucion(item, mode, telnum);
            break;
        case SETBLEMAC_INS:
            doSetblemacInstrucion(item, mode, telnum);
            break;
        case BF_INS:
            doBFInstruction(item, mode, telnum);
            break;
        case CF_INS:
            doCFInstruction(item, mode, telnum);
            break;
        case FACTORYTEST_INS:
            doFactoryTestInstruction(item, mode, telnum);
            break;
        case FENCE_INS:
            doFenceInstrucion(item, mode, telnum);
            break;
        case FACTORY_INS:
            doFactoryInstrucion(item, mode, telnum);
            break;
        case SMSREPLY_INS:
            doSmsreplyInstrucion(item, mode, telnum);
            break;
        case JTCYCLE_INS:
            doJTCYCLEInstrucion(item, mode, telnum);
            break;
        case ANSWER_INS:
            doAnswerInstrucion(item, mode, telnum);
            break;
        case TURNALG_INS:
            doTurnAlgInstrucion(item, mode, telnum);
            break;
        default:
            sprintf(debug, "%s==>%s\n", __FUNCTION__, "unknow cmd");
            LogMessage(DEBUG_ALL, debug);
            break;
    }
}

//SERVER,1,JZWZ.BASEGPS.COM,9998#
//PARAM#
//STATUS
void instructionParase(uint8_t *str, uint16_t len, DOINSTRUCTIONMODE mode, char *telnum)
{
    ITEM item;
    int16_t cmdid;
    paraseInstructionToItem(&item, str, len);
    cmdid = getInstructionid((uint8_t *)item.item_data[0]);
    doinstruction(cmdid, &item, mode, telnum);

}

static void atCmdNmeaParase(uint8_t *buf, uint16_t len)
{
    if (strstr((char *)buf, "ON") != NULL)
    {
        sysinfo.nmeaoutputonoff = 1;
        LogMessage(DEBUG_ALL, "NMEA OPEN\n");
    }
    else
    {
        sysinfo.nmeaoutputonoff = 0;
        LogMessage(DEBUG_ALL, "NMEA CLOSE\n");
    }
}

void atCmdDebugLevelParser(char *buf, uint16_t len)
{
    if (len <= 2)
    {
        if (buf[0] >= '0' && buf[0] <= '9')
        {
            sysinfo.logmessage = atoi(buf);
            LogPrintf(DEBUG_NONE, "Debug LEVEL:%d OK\r\n", sysinfo.logmessage);
        }
    }
}

static void atCmdDebugParase(uint8_t *buf, uint16_t len)
{
    ITEM item;
    paraseInstructionToItem(&item, buf, len);
    if (strstr(item.item_data[0], "SUSPEND") != NULL)
    {
        LogMessage(DEBUG_FACTORY, "Suspend all task\n");
        systemTaskSuspend(sysinfo.SystaskID);
    }
    else if (strstr(item.item_data[0], "RESUME") != NULL)
    {
        LogMessage(DEBUG_FACTORY, "Resume all task\n");
        systemTaskResume(sysinfo.SystaskID);
    }
    //**********************************
    else if (strstr(item.item_data[0], "POWERON") != NULL)
    {
        MODULE_POWER_ON;
        LogMessage(DEBUG_NET, "开启4G电源\n");
    }
    else if (strstr(item.item_data[0], "POWEROFF") != NULL)
    {
        MODULE_POWER_OFF;
        LogMessage(DEBUG_NET, "关闭4G电源\n");
    }
    else if (strstr(item.item_data[0], "POWERKEYH") != NULL)
    {
        MODULE_POWERKEY_HIGH;
        LogMessage(DEBUG_NET, "开机键高电平\n");
    }

    else if (strstr(item.item_data[0], "POWERKEYL") != NULL)
    {
        MODULE_POWERKEY_LOW;
        LogMessage(DEBUG_NET, "开机键低电平\n");
    }
    else if (strstr(item.item_data[0], "RSTKEYH") != NULL)
    {
        MODULE_RSTKEY_HIGH;
        LogMessage(DEBUG_NET, "复位键高电平\n");
    }

    else if (strstr(item.item_data[0], "RSTKEYL") != NULL)
    {
        MODULE_RSTKEY_LOW;
        LogMessage(DEBUG_NET, "复位键低电平\n");
    }
    else if (strstr(item.item_data[0], "DTRH") != NULL)
    {
        MODULE_DTR_HIGH;
        LogMessage(DEBUG_NET, "DTR 高电平\n");
    }
    else if (strstr(item.item_data[0], "DTRL") != NULL)
    {
        MODULE_DTR_LOW;
        LogMessage(DEBUG_NET, "DTR 低电平\n");
    }
    //**********************************
    else if (strstr(item.item_data[0], "ENTERSLEEP") != NULL)
    {
        LogMessage(DEBUG_FACTORY, "快速进入睡眠\n");
        sysinfo.runFsm = MODE_STOP;
        sysinfo.GPSRequest = 0;
        sysinfo.blerequest = 0;
        sysinfo.alarmrequest = 0;
    }
    else if (strstr(item.item_data[0], "RESET") != NULL)
    {
        HAL_NVIC_SystemReset();
    }
    else
    {
        atCmdDebugLevelParser(item.item_data[0], strlen(item.item_data[0]));
    }

}

static void atCmdFMPCnmeaParase(uint8_t *buf, uint16_t len)
{
    if (my_strstr((char *)buf, "ON", len))
    {
        LogMessage(DEBUG_FACTORY, "NMEA ON OK\n");
        GPSLNAON;
        sendModuleCmd(N58_AT_CMD, NULL);
        sendModuleCmd(N58_AT_CMD, NULL);
        openModuleGPS();
        sysinfo.nmeaoutputonoff = 1;
    }
    else
    {
        LogMessage(DEBUG_FACTORY, "NMEA OFF OK\n");
        GPSLNAOFF;
        closeModuleGPS();
        sysinfo.nmeaoutputonoff = 0;
    }

}

static void atCmdFMPCrelayParase(uint8_t *buf, uint16_t len)
{
    ITEM item;
    paraseInstructionToItem(&item, buf, len);
    if (strstr(item.item_data[0], "OFF") != NULL)
    {
        LogMessage(DEBUG_FACTORY, "Relay OFF OK\r\n");
    }
    else if (strstr(item.item_data[0], "ON") != NULL)
    {
        LogMessage(DEBUG_FACTORY, "Relay ON OK\r\n");
    }
}

static void atCmdFMPCbatParase(void)
{
    getBatVoltage();
    LogPrintf(DEBUG_FACTORY, "Vbat: %.3fv\r\n", sysinfo.outsidevoltage);
}

static void atCmdFMPCgsensorParase(void)
{
    read_gsensor_id();
}

static void atCmdFMPCaccParase(void)
{
    LogPrintf(DEBUG_FACTORY, "ACC is %s\r\n", getTerminalAccState() > 0 ? "ON" : "OFF");
}


static void atCmdFMPCgsmParase(void)
{
    if (isModuleRunNormal())
    {
        LogMessage(DEBUG_FACTORY, "GSM SERVICE OK\r\n");
    }
}

static void atCmdFmpcAdccalParase(void)
{
    sysparam.adccal = 4 / (((float)getVoltageAdcValue() / 4095) * 1.8);
    paramSaveAdcCal(sysparam.adccal);
    LogPrintf(DEBUG_FACTORY, "Update the voltage calibration parameter to %f\n", sysparam.adccal);

}
static void atCmdFMPCLdrParase(void)
{
    if (LDRDET)
    {
        LogMessage(DEBUG_FACTORY, "Light sensor detects darkness\n");
    }
    else
    {
        LogMessage(DEBUG_FACTORY, "Light sensor detects brightness\n");

    }
}

void atCmdFMPCCSQParse(void)
{
    csqRequest();
}

void atCmdFmpcAllParse(uint8_t *buf, uint16_t len)
{
    ITEM item;
    paraseInstructionToItem(&item, buf, len);
    csqRequest();
    doFactoryTestInstruction(&item, AT_SMS_MODE, NULL);
}

void atCmdFmpcCMParse(uint8_t *buf, uint16_t len)
{
    paramSaveCM(1);
    if (paramGetCM() == 1)
    {
        LogMessage(DEBUG_FACTORY, "CM OK\n");
    }
    else
    {
        LogMessage(DEBUG_FACTORY, "CM FAIL\n");
    }

}

void atCmdCmGetParser(void)
{
    if (paramGetCM() == 1)
    {
        LogMessage(DEBUG_FACTORY, "CM OK\n");
    }
    else
    {
        LogMessage(DEBUG_FACTORY, "CM FAIL\n");
    }

}

void atCmdFmpcChkpParse(void)
{
    LogPrintf(DEBUG_FACTORY, "+FMPC_CHKP:%s,%s:%d\r\n", sysparam.SN, sysparam.Server, sysparam.ServerPort);
}



void atCmdZTSNParse(uint8_t *buf, uint16_t len)
{
    char IMEI[15];
    uint8_t sndata[30];
    changeHexStringToByteArray(sndata, buf, len / 2);
    decryptSN(sndata, IMEI);
    LogPrintf(DEBUG_FACTORY, "Decrypt:%s\n", IMEI);
    paramSaveSnNumber((uint8_t *)IMEI, 15);
    paramGetSnNumber(sysparam.SN, 15);
    LogMessage(DEBUG_FACTORY, "Write Sn Ok\r\n");
}

static void atCmdIMEIParseRespon(void)
{
    LogPrintf(DEBUG_FACTORY, "ZTINFO:%s:%s:%s\r\n", sysparam.SN, (char *)getModuleIMEI(), CODEVERSION);

}
void atCmdIMEIParse(void)
{
    sendModuleCmd(N58_AT_CMD, NULL);
    sendModuleCmd(N58_CIMI_CMD, NULL);
    sendModuleCmd(N58_CGSN_CMD, NULL);
    sendModuleCmd(N58_CCID_CMD, NULL);
    startTimer(1200, atCmdIMEIParseRespon, 0);
}

static void atCmdFmpcIMSIParseRespon(void)
{
    LogPrintf(DEBUG_FACTORY, "FMPC_IMSI_RSP OK, IMSI=%s&&%s&&%s\r\n", sysparam.SN, (char *)getModuleIMSI(),
              (char *)getModuleICCID());

}
void atCmdFmpcIMSIParse(void)
{
    sendModuleCmd(N58_AT_CMD, NULL);
    sendModuleCmd(N58_CIMI_CMD, NULL);
    sendModuleCmd(N58_CGSN_CMD, NULL);
    sendModuleCmd(N58_CCID_CMD, NULL);
    startTimer(1200, atCmdFmpcIMSIParseRespon, 0);
}

//指令集处理
void atCmdParaseFunction(uint8_t *buf, uint16_t len)
{
    int ret, cmdlen, cmdid;
    char debug[100];
    char cmdbuf[51];
    LogMessageWL(DEBUG_FACTORY, (char *)buf, len);
    if (buf[0] == 'A' && buf[1] == 'T' && buf[2] == '^')
    {
        ret = getCharIndex(buf, len, '=');
        if (ret < 0)
        {
            ret = getCharIndex(buf, len, '\r');
        }
        if (ret >= 0)
        {
            cmdlen = ret - 3;
            if (cmdlen < 50)
            {
                strncpy(cmdbuf, (const char *)buf + 3, cmdlen);
                cmdbuf[cmdlen] = NULL;
                cmdid = getatcmdid((uint8_t *)cmdbuf);
                switch (cmdid)
                {
                    case AT_SMS_CMD:
                        instructionParase(buf + ret + 1, len - ret - 1, AT_SMS_MODE, NULL);
                        break;
                    case AT_FMPC_NMEA_CMD :
                        atCmdFMPCnmeaParase(buf + ret + 1, len - ret - 1);
                        break;
                    case AT_FMPC_BAT_CMD :
                        atCmdFMPCbatParase();
                        break;
                    case AT_FMPC_GSENSOR_CMD :
                        atCmdFMPCgsensorParase();
                        break;
                    case AT_FMPC_ACC_CMD :
                        atCmdFMPCaccParase();
                        break;
                    case AT_FMPC_GSM_CMD :
                        atCmdFMPCgsmParase();
                        break;
                    case AT_FMPC_CSQ_CMD :
                        atCmdFMPCCSQParse();
                        break;
                    case AT_FMPC_RELAY_CMD:
                        atCmdFMPCrelayParase(buf + ret + 1, len - ret - 1);
                        break;
                    case AT_DEBUG_CMD:
                        atCmdDebugParase(buf + ret + 1, len - ret - 1);
                        break;
                    case AT_NMEA_CMD:
                        atCmdNmeaParase(buf + ret + 1, len - ret - 1);
                        break;
                    case AT_ADCCAL_CMD:
                        atCmdFmpcAdccalParase();
                        break;
                    case AT_FMPC_LDR_CMD:
                        atCmdFMPCLdrParase();
                        break;
                    case AT_FMPC_ALL_CMD:
                        atCmdFmpcAllParse(buf + ret + 1, len - ret - 1);
                        break;
                    case AT_FMPC_CM_CMD:
                        atCmdFmpcCMParse(buf + ret + 1, len - ret - 1);
                        break;
                    case AT_ZTSN_CMD:
                        atCmdZTSNParse(buf + ret + 1, len - ret - 1);
                        break;
                    case AT_IMEI_CMD:
                        atCmdIMEIParse();
                        break;
                    case AT_FMPC_IMSI_CMD:
                        atCmdFmpcIMSIParse();
                        break;
                    case AT_FMPC_CHKP_CMD:
                        atCmdFmpcChkpParse();
                        break;
                    case AT_FMCP_CMGET_CMD:
                        atCmdCmGetParser();
                        break;
                    default:
                        sprintf(debug, "%s==>unknow cmd:%s\n", __FUNCTION__, cmdbuf);
                        LogMessage(DEBUG_FACTORY, debug);
                        break;
                }
            }
        }
        else
        {
            sprintf(debug, "%s==>%s\n", __FUNCTION__, "please check you param");
            LogMessage(DEBUG_FACTORY, debug);

        }
    }
    else
    {
        appUartSend(&usart2_ctl, buf, len);
    }

}




