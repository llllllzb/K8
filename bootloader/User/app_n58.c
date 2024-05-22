#include "app_N58.h"
#include "app_common.h"
N58_INVOKE_STATUS	n58_invoke_status;
N58_LTE_STATUS 		n58_lte_status;


const N58_CMD_STRUCT n58_cmd_table[N58_MAX_NUM] =
{
    {N58_AT_CMD, "ATE0"},
    {N58_CPIN_CMD, "AT+CPIN?"},
    {N58_CGSN_CMD, "AT+CGSN"},
    {N58_CIMI_CMD, "AT+CIMI"},
    {N58_CSQ_CMD, "AT+CSQ"},
    {N58_CREG_CMD, "AT+CREG"},
    {N58_CGATT_CMD, "AT+CGATT"},
    {N58_MYSYSINFO_CMD, "AT$MYSYSINFO"},
    {N58_CGDCONT_CMD, "AT+CGDCONT"},
    {N58_XGAUTH_CMD, "AT+XGAUTH"},
    {N58_XIIC_CMD, "AT+XIIC"},
    {N58_TCPSETUP_CMD, "AT+TCPSETUP"},
    {N58_IPSTATUS_CMD, "AT+IPSTATUS"},
    {N58_TCPSEND_CMD, "AT+TCPSEND"},
    {N58_TCPACK_CMD, "AT+TCPACK"},
    {N58_TCPCLOSE_CMD, "AT+TCPCLOSE"},
    {N58_CMGF_CMD, "AT+CMGF"},
    {N58_CMGS_CMD, "AT+CMGS"},
    {N58_CMGD_CMD, "AT+CMGD"},
    {N58_CMGR_CMD, "AT+CMGR"},
    {N58_CNMI_CMD, "AT+CNMI"},
    {N58_ATA_CMD, "ATA"},
    {N58_ATH_CMD, "ATH"},
    {N58_CCLK_CMD, "AT+CCLK?"},
    {N58_MYGPSPWR_CMD, "AT$MYGPSPWR"},
    {N58_MYGPSPOS_CMD, "AT$MYGPSPOS"},
    {N58_MYGPSSTATE_CMD, "AT$MYGPSSTATE"},
    {N58_MYPOWEROFF_CMD, "AT$MYPOWEROFF"},
    {N58_TTSPLAY_CMD, "AT+TTSPLAY"},
    {N58_PLAYTTS_CMD, "AT+PLAYTTS"},
    {N58_FSLIST_CMD, "AT+FSLIST?"},
    {N58_FSWF_CMD, "AT+FSWF"},
    {N58_FSRF_CMD, "AT+FSRF"},
    {N58_FSDF_CMD, "AT+FSDF"},
    {N58_FSFAT_CMD, "AT+FSFAT"},
    {N58_MAX_NUM, NULL},
};

static void n58PowerKeyProcessB(void)
{
    //POWKEYOFMODULEON;//开机键高电平
    LogMessage("Power on complete\n");
    system_info.modulepowerstate = 1;
}

static void n58PowerKeyProcessA(void)
{
    POWKEYOFMODULEOFF;//开机键低电平
    LogMessage("Prepare to power on\n");
    startTimer(1000, n58PowerKeyProcessB, 0);
}

void n58ModulePowerOn(void)
{
    POWEROFMODULEON;//开电源
    POWKEYOFMODULEON;//开机键高电平
    RSTKEYOFMODULEON;//复位键高电平
    LogMessage("Open moudle power supply\n");
    startTimer(1500, n58PowerKeyProcessA, 0);
    n58Init();
    system_info.modulepowerstate = 0;

}

static void n58ResetKeyProcessA(void)
{
    RSTKEYOFMODULEON;//复位键高电平
    LogMessage("Reset Complete\n");
    system_info.modulepowerstate = 1;
}

void n58ModuleReset(void)
{
    RSTKEYOFMODULEOFF;//复位键低电平
    LogMessage("Reset Module\n");
    startTimer(100, n58ResetKeyProcessA, 0);
    n58Init();
    system_info.modulepowerstate = 0;
}

uint8_t  sendN58Cmd(uint8_t cmd, char *param)
{
    uint8_t i;
    int cmdtype = -1;
    char sendData[1024];
    if(cmd >= N58_MAX_NUM)
        return 0;
    for(i = 0; i < N58_MAX_NUM; i++)
    {
        if(cmd == n58_cmd_table[i].cmd_type)
        {
            cmdtype = i;
            break;
        }
    }
    if(cmdtype < 0)
    {
        sprintf(sendData, "sendN58Cmd==>No cmd\n");
        LogMessage(sendData);
        return 0;
    }
    if(param != NULL && strlen(param) <= 1000)
    {
        if(param[0] == '?')
        {
            sprintf(sendData, "%s?\r\n", n58_cmd_table[cmdtype].cmd);

        }
        else
        {
            sprintf(sendData, "%s=%s\r\n", n58_cmd_table[cmdtype].cmd, param);
        }
    }
    else if(param == NULL)
    {
        sprintf(sendData, "%s\r\n", n58_cmd_table[cmdtype].cmd);
    }
    else
    {
        return 0;
    }
    CreateNodeCmd(sendData, strlen(sendData), cmd);
    return 1;
}

void n58setAPN(void)
{
    char param[40];
    sprintf(param, "1,\"IP\",\"%s\"", eeprom_info.apn);
    sendN58Cmd(N58_CGDCONT_CMD, param);
}
void n58setXGAUTH(void)
{
    char param[40];
    sprintf(param, "1,1,\"%s\",\"%s\"", eeprom_info.apnuser, eeprom_info.apnpassword);
    sendN58Cmd(N58_XGAUTH_CMD, param);
}
void n58Init(void)
{
    memset(&n58_invoke_status, 0, sizeof(N58_INVOKE_STATUS));
    memset(&n58_lte_status, 0, sizeof(N58_LTE_STATUS));
}
void N58_ChangeInvokeStatus(N58_WORK_STATUS status)
{
    n58_invoke_status.invoke_status = status;
    n58_invoke_status.tick_time = 0;
}

uint8_t N58_GetInvokeStatus(void)
{
    return n58_invoke_status.invoke_status;
}

void N58_CreateSocket(uint8_t link, char * server, uint16_t port)
{
    char param[100];
    sprintf(param, "%d,%s,%d", link, server, port);
    sendN58Cmd(N58_TCPSETUP_CMD, param);
}
void N58_SendDataToServer(uint8_t * data, uint16_t len)
{
    char param[10];
    sprintf(param, "0,%d", len);
    sendN58Cmd(N58_TCPSEND_CMD, param);
    CreateNodeCmd((char *)data, len, N58_TCPSETUP_CMD);
}

void clearSetupCount(void)
{
	
	n58_lte_status.retcpsetup_count = 0;

}
void n58NetworkConnectProcess(void)
{
    //char debug[100];
    if(system_info.modulepowerstate == 0)
    {
        return ;
    }
    n58_invoke_status.poweron_tick++;
    //sprintf(debug,"n58NetworkConnectProcess==> run %d\n",n58_invoke_status.invoke_status);
    //LogMessage(debug);
    switch(n58_invoke_status.invoke_status)
    {
    case N58_AT_STATUS:
        if(n58_lte_status.at_respon == 0)
        {
            n58_invoke_status.poweron_tick = 0;
            sendN58Cmd(N58_AT_CMD, NULL);
            break;
        }
        else
        {
            n58_lte_status.at_respon = 0;
            //n58TTSPlay("开机成功");
            N58_ChangeInvokeStatus(N58_CPIN_STATUS);
        }
    case N58_CPIN_STATUS:
        if(n58_lte_status.cpin_respon == 0)
        {
            sendN58Cmd(N58_CPIN_CMD, NULL);
            if(n58_invoke_status.tick_time > 16)
            {
                n58ModuleReset();//复位设备
            }
            break;
        }
        else
        {
            n58_lte_status.cpin_respon = 0;
            sendN58Cmd(N58_CNMI_CMD, "0,1");
            N58_ChangeInvokeStatus(N58_IMSI_STATUS);
        }
    case N58_IMSI_STATUS:
        if(n58_lte_status.cimi_respon == 0)
        {
            if(n58_invoke_status.tick_time % 2 == 0)
                sendN58Cmd(N58_CIMI_CMD, NULL);
            if(n58_invoke_status.tick_time > 7)
            {
                N58_ChangeInvokeStatus(N58_CSQ_STATUS);
            }
            else
            {
                break;
            }
        }
        else
        {
            n58_lte_status.cimi_respon = 0;
            N58_ChangeInvokeStatus(N58_CSQ_STATUS);
        }
    case N58_CSQ_STATUS:
        if(n58_lte_status.csq_ok == 0)
        {
            sendN58Cmd(N58_CSQ_CMD, NULL);
            if(n58_invoke_status.tick_time > 40)
            {
                n58ModuleReset();//复位设备
            }
            break;
        }
        else
        {
            n58_lte_status.csq_ok = 0;
            sendN58Cmd(N58_CREG_CMD, "2");
            N58_ChangeInvokeStatus(N58_CREG_STATUS);
        }
    case N58_CREG_STATUS:
        if(!(n58_lte_status.creg_state == 1 || n58_lte_status.creg_state == 5))
        {
            sendN58Cmd(N58_CREG_CMD, "?");
            if(n58_invoke_status.tick_time > 40)
            {
                n58ModuleReset();//复位设备
            }
            break;
        }
        else
        {
            n58_lte_status.creg_state = 0;
            N58_ChangeInvokeStatus(N58_CGATT_STATUS);
        }
    case N58_CGATT_STATUS:
        if(n58_lte_status.cgatt_ok == 0)
        {
            sendN58Cmd(N58_CGATT_CMD, "?");
            if(n58_invoke_status.tick_time > 30)
            {
                if(n58_lte_status.cgatt_bymen == 1)
                {
                    n58ModuleReset();//复位设备
                    n58_lte_status.cgatt_bymen = 0;
                }
                else
                {
                    sendN58Cmd(N58_CGATT_CMD, "1");
                    n58_lte_status.cgatt_bymen = 1;
                    n58_invoke_status.tick_time = 0;
                }
            }
            break;
        }
        else
        {
            n58_lte_status.cgatt_ok = 0;
            n58_lte_status.cgatt_bymen = 0;
            N58_ChangeInvokeStatus(N58_MYSYSINFO_STATUS);
        }
    case N58_MYSYSINFO_STATUS:
        if(n58_lte_status.mysysinfo_ok == 0)
        {
            sendN58Cmd(N58_MYSYSINFO_CMD, NULL);
            break;
        }
        else
        {
            n58_lte_status.mysysinfo_ok = 0;
            N58_ChangeInvokeStatus(N58_CGDCONT_STATUS);
        }
    case N58_CGDCONT_STATUS:
        if(n58_lte_status.cgdcont_ok == 0)
        {
            n58setAPN();
            if(n58_invoke_status.tick_time > 4)
            {
                n58ModuleReset();//复位设备
            }
            break;
        }
        else
        {
            n58_lte_status.cgdcont_ok = 0;
            N58_ChangeInvokeStatus(N58_XGAUTH_STATUS);
        }
    case N58_XGAUTH_STATUS:
        if(n58_lte_status.xgauth_ok == 0)
        {
            n58setXGAUTH();
            if(n58_invoke_status.tick_time > 4)
            {
                n58ModuleReset();//复位设备
            }
            break;
        }
        else
        {
            N58_ChangeInvokeStatus(N58_XIIC_STATUS);
            n58_lte_status.rexiic_count = 0;
            n58_lte_status.xgauth_ok = 0;
        }
    case N58_XIIC_STATUS:
        if(n58_lte_status.xiic_ok == 0)
        {
            sendN58Cmd(N58_XIIC_CMD, "1");
            if(n58_invoke_status.tick_time > 4)
            {
                n58ModuleReset();//复位设备
            }
            break;
        }
        else
        {
            n58_lte_status.xiic_ok = 0;
            N58_ChangeInvokeStatus(N58_XIIC_QUERY_STATUS);
        }
    case N58_XIIC_QUERY_STATUS:
        if(n58_lte_status.xiic_queryok == 0)
        {
            sendN58Cmd(N58_XIIC_CMD, "?");
            if(n58_invoke_status.tick_time > 30)
            {
                N58_ChangeInvokeStatus(N58_XIIC_STATUS);
                sendN58Cmd(N58_CGATT_CMD, "1");
                n58_lte_status.rexiic_count++;
                if(n58_lte_status.rexiic_count > 3)
                {
                    n58ModuleReset();//复位设备
                }
            }
            break;
        }
        else
        {
            n58_lte_status.retcpclose_count = 0;
            n58_lte_status.xiic_queryok = 0;
            n58_lte_status.rexiic_count = 0;
            N58_ChangeInvokeStatus(N58_TCPCLOSE_STATUS);
        }
    case N58_TCPCLOSE_STATUS:
        if(n58_lte_status.tcpclose_ok == 0)
        {
            if(n58_invoke_status.tick_time % 30 == 0)
            {
                sendN58Cmd(N58_TCPCLOSE_CMD, "1");
                n58_lte_status.retcpclose_count++;
                if(n58_lte_status.retcpclose_count >= 3)
                {
                    n58ModuleReset();//复位设备
                }
            }
            break;
        }
        else
        {
            n58_lte_status.tcpclose_ok = 0;
            n58_lte_status.tcpsetup_error = 0;
            n58_lte_status.retcpclose_count = 0;
            N58_ChangeInvokeStatus(N58_TCPSETUP_STATUS);
        }
    case N58_TCPSETUP_STATUS:
        if(n58_lte_status.tcpsetup_ok == 0)
        {
            if(n58_invoke_status.tick_time % 30 == 0 || n58_lte_status.tcpsetup_error == 1)
            {
                n58_lte_status.tcpsetup_error = 0;
                n58_invoke_status.tick_time = 0;
                n58_lte_status.retcpsetup_count++;
                if(n58_lte_status.retcpsetup_count > 3)
                {
                    if(getUpgradeFileSize() == 0)
                    {
                        LogMessage("Upgrade firmware fail,system restart\n");
                        eepromParamSaveUpdateStatus(0);
                        HAL_NVIC_SystemReset();
                    }
                    else
                    {
                        n58ModuleReset();//复位设备
                    }
                }
                else
                {
                    //创建链接
                    N58_CreateSocket(0, (char *)eeprom_info.Server, eeprom_info.ServerPort);
                }
            }
            break;
        }
        else
        {
            n58_lte_status.tcpsetup_ok = 0;
            n58_lte_status.tcpsetup_error = 0;
            netConnectReset();
            //n58TTSPlay("升级中");
            N58_ChangeInvokeStatus(N58_NORMAL_STATUS);
        }
    case N58_NORMAL_STATUS:

        break;
    default :
        n58_invoke_status.invoke_status = N58_AT_STATUS;
        break;
    }
    n58_invoke_status.tick_time++;
}

/**************************协议解析*************************************/
void N58_ClearCmd(void)
{
    n58_lte_status.current_cmd = 0;
}

/*
+CSQ: 25,99
OK
*/
void n58CSQparase(uint8_t * buf, uint16_t len)
{
    int index, indexa, datalen;
    uint8_t * rebuf;
    uint16_t  relen;
    char restore[5];
    char debug[20];
    index = my_getstrindex((char *)buf, "+CSQ:", len);
    if(index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        indexa = getCharIndex(rebuf, relen, ',');
        datalen = indexa - 6;
        if(datalen > 5)
            return;
        memset(restore, 0, 5);
        strncpy(restore, (char *)rebuf + 6, datalen);
        n58_lte_status.csq_rssi = atoi(restore);
        sprintf(debug, "CSQ State=%d\n", n58_lte_status.csq_rssi);
        LogMessage(debug);
        if(n58_lte_status.csq_rssi >= 6 && n58_lte_status.csq_rssi <= 31)
            n58_lte_status.csq_ok = 1;
        N58_ClearCmd();
    }
}

/*
+CREG: 2,3,"0000","029cc501",0
OK

*/
void n58CREGparase(uint8_t * buf, uint16_t len)
{
    int index, datalen;
    uint8_t * rebuf;
    uint16_t  relen, i;
    char restore[50];
    char debug[100];
    uint8_t   cnt;
    index = my_getstrindex((char *)buf, "+CREG:", len);
    if(index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        datalen = 0;
        cnt = 0;
        restore[0] = 0;
        for(i = 0; i < relen; i++)
        {
            if(rebuf[i] == ',' || rebuf[i] == '\r' || rebuf[i] == '\n')
            {
                if(restore[0] != 0)
                {
                    restore[datalen] = 0;
                    cnt++;
                    datalen = 0;
                    sprintf(debug, "n58CREGparase==>%s,cnt=%d\n", restore, cnt);
                    LogMessage( debug);
                    switch(cnt)
                    {
                    case 2:
                        n58_lte_status.creg_state = atoi(restore);
                        sprintf(debug, "CREG state=%d\n", n58_lte_status.creg_state);
                        LogMessage( debug);
                        break;

                    }
                }
            }
            else
            {
                restore[datalen] = rebuf[i];
                datalen++;
                if(datalen >= 50)
                {
                    return ;
                }

            }
        }
        N58_ClearCmd();
    }
}
/*
+CGATT: 0
OK
*/
void n58cgattparase(uint8_t * buf, uint16_t len)
{
    int16_t index;
    uint8_t  value;
    uint8_t  *rebuf;
    char debug[30];
    index = my_getstrindex((char *)buf, "+CGATT:", len);
    if(index >= 0)
    {
        rebuf = buf + index;
        value = rebuf[8] - '0';
        n58_lte_status.cgatt_ok = value;
        sprintf(debug, "CGATT=%d\n", n58_lte_status.cgatt_ok);
        LogMessage( debug);
    }
}

void n58mysysteminfoparase(uint8_t * buf, uint16_t len)
{
    if(my_getstrindex((char *)buf, "OK", len) >= 0 || my_getstrindex((char *)buf, "ERROR", len) >= 0)
    {
        n58_lte_status.mysysinfo_ok = 1;

    }
}
void n58cgdcontparase(uint8_t * buf, uint16_t len)
{
    if(distinguishOK((char *) buf))
    {
        n58_lte_status.cgdcont_ok = 1;
    }
}
void n58xgauthparase(uint8_t * buf, uint16_t len)
{
    if(distinguishOK((char *) buf))
    {
        n58_lte_status.xgauth_ok = 1;
    }
}
void n58xiicparase(uint8_t * buf, uint16_t len)
{
    if(distinguishOK((char *) buf))
    {
        n58_lte_status.xiic_ok = 1;
    }
}
void n58xiicqueryparase(uint8_t * buf, uint16_t len)
{

    int index, datalen;
    uint8_t * rebuf;
    uint16_t  relen, i;
    char restore[50];
    char debug[100];
    uint8_t   cnt, value;
    index = my_getstrindex((char *)buf, "+XIIC:", len);
    if(index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        datalen = 0;
        cnt = 0;
        restore[0] = 0;
        for(i = 0; i < relen; i++)
        {
            if(rebuf[i] == ',' || rebuf[i] == '\r' || rebuf[i] == '\n')
            {
                if(restore[0] != 0)
                {
                    restore[datalen] = 0;
                    cnt++;
                    datalen = 0;
                    sprintf(debug, "n58CREGparase==>%s,cnt=%d\n", restore, cnt);
                    LogMessage( debug);
                    switch(cnt)
                    {
                    case 1:
                        value = restore[10];
                        n58_lte_status.xiic_queryok = value - '0';
                        sprintf(debug, "XIIC state=%d\n", n58_lte_status.xiic_queryok);
                        LogMessage( debug);
                        break;
                    }
                }
            }
            else
            {
                restore[datalen] = rebuf[i];
                datalen++;
                if(datalen >= 50)
                {
                    return ;
                }

            }
        }
        N58_ClearCmd();
    }
}
void n58tcpcloseparase(uint8_t * buf, uint16_t len)
{
    if(strstr((char *)buf, "+TCPCLOSE:") != NULL)
    {
        n58_lte_status.tcpclose_ok = 1;
    }
}
/*
+CIMI: 460045562404799
OK

*/
void n58CIMIParase(uint8_t * buf, uint16_t len)
{
    int16_t index, indexa;
    uint8_t * rebuf;
    uint16_t  relen;
    uint8_t snlen, i;
    char imsirestore[20];
    char debug[50];
    index = my_getstrindex((char *)buf, "+CIMI:", len);
    if(index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        indexa = getCharIndex(rebuf, relen, '\r');
        snlen = indexa - 6;
        if(snlen < 20)
        {
            memset(imsirestore, 0, 20);
            for(i = 0; i < snlen; i++)
            {
                imsirestore[i] = rebuf[i + 6];
            }
            sprintf(debug, "Get IMSI:%s\n", imsirestore);
            LogMessage(debug);
            n58_lte_status.cimi_respon = 1;
        }

    }

}
void n58TCPsetupparase(uint8_t * buf, uint16_t len)
{

    int index, datalen;
    uint8_t * rebuf;
    uint16_t  relen, i;
    char restore[50];
    uint8_t   cnt;
    index = my_getstrindex((char *)buf, "+TCPSETUP:", len);
    if(index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        datalen = 0;
        cnt = 0;
        restore[0] = 0;
        for(i = 0; i < relen; i++)
        {
            if(rebuf[i] == ',' || rebuf[i] == '\r' || rebuf[i] == '\n')
            {
                if(restore[0] != 0)
                {
                    restore[datalen] = 0;
                    cnt++;
                    datalen = 0;
                    LogPrintf("n58CREGparase==>%s,cnt=%d\n", restore, cnt);
                    switch(cnt)
                    {
                    case 1:
                        if(strstr(restore, "ERROR") != NULL)
                        {
                            LogMessage("TCP socket Error\n");
                        }
                    case 2:
                        if(distinguishOK((char *) restore))
                        {
                            n58_lte_status.tcpsetup_ok = 1;
                            LogMessage("TCP socket connect OK\n");
                        }
                        else if(strstr(restore, "FAIL") != NULL)
                        {
                            LogMessage("TCP socket connect Fail\n");
                        }
                        else if(strstr(restore, "ERROR1") != NULL)
                        {
                            n58_lte_status.tcpsetup_ok = 1;
                            LogMessage("TCP socket had build\n");
                        }
                        break;
                    }
                }
            }
            else
            {
                restore[datalen] = rebuf[i];
                datalen++;
                if(datalen >= 50)
                {
                    return ;
                }

            }
        }
        N58_ClearCmd();
    }
}


void n58TCPRecvparase(uint8_t * buf, uint16_t len)
{
    int16_t index, datalen;
    uint8_t * rebuf;
    uint16_t  relen, i, tcpdata;
    uint8_t   cnt;
    char restore[512];
    index = my_getstrindex((char *)buf, "+TCPRECV:", len);
    if(index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        datalen = 0;
        cnt = 0;
        restore[0] = 0;
        for(i = 0; i < relen; i++)
        {
            if(rebuf[i] == ',' || rebuf[i] == '\r' || rebuf[i] == '\n')
            {
                if(restore[0] != 0)
                {
                    restore[datalen] = 0;
                    cnt++;
                    datalen = 0;
                    if(cnt == 2)
                    {
                        tcpdata = atoi(restore);
                        break;

                    }
                }
            }
            else
            {
                restore[datalen] = rebuf[i];
                datalen++;
                if(datalen >= 50)
                {
                    return ;
                }

            }
        }
        index = getCharIndexWithNum(rebuf, relen, ',', 2);
        LogPrintf("TCP data=%d Bytes\n", tcpdata);
        if(tcpdata < 256)
        {
            changeByteArrayToHexString(rebuf + index + 1, (uint8_t *)restore, tcpdata);
            restore[tcpdata * 2] = 0;
            LogMessage("TCP Recv:");
            LogMessage( restore);
            LogMessage("\n");
        }
		clearSetupCount();
        protocolRxParase((char *)rebuf + index + 1, tcpdata);
        N58_ClearCmd();
    }
}
static void n58TcpsendParase(uint8_t * buf, uint16_t len)
{
    if(my_strstr((char *)buf, "ERROR", len) || my_strstr((char *)buf, "+TCPSEND: SOCKET ID OPEN FAILED", len))
    {
        N58_ChangeInvokeStatus(N58_TCPSETUP_STATUS);
        N58_ClearCmd();
    }
}
/*
$MYGPSSTATE: gps opened
$MYGPSSTATE: gps closed


*/



/*

GPRS DISCONNECTION

+TCPCLOSE: 0,Link Closed

*/
void n58responparase(uint8_t * buf, uint16_t len)
{
    uint8_t *restore = malloc(len);
    if(restore == NULL)
    {
        LogMessage("n58responparase==>Malloc Error\n");
        return ;
    }
    memcpy(restore, buf, len);
    LogMessage("--->>>---\n");
    if(len < 512)
    {

        LogMessageWL((char *)restore, len);
    }
    else
    {
        LogMessage("The received data is too long to be displayed\n");
    }
    LogMessage("---<<<---\n");
    /*不区分指令区域*/
    n58TCPRecvparase(restore, len);
    n58TCPsetupparase(restore, len);
    n58tcpcloseparase(restore, len);
    n58CIMIParase(restore, len);

    switch(n58_lte_status.current_cmd)
    {
    case N58_AT_CMD:
        if(distinguishOK((char *)restore))
        {
            n58_lte_status.at_respon = 1;
            N58_ClearCmd();
        }
        break;
    case N58_CPIN_CMD:
        if(my_strstr((char *)restore, "+CPIN: READY", len))
        {
            n58_lte_status.cpin_respon = 1;
            N58_ClearCmd();
        }
        break;
    case N58_CGSN_CMD:
        break;
    case N58_CIMI_CMD:
        break;
    case N58_CSQ_CMD:
        n58CSQparase(restore, len);
        break;
    case N58_CREG_CMD:
        n58CREGparase(restore, len);
        break;
    case N58_CGATT_CMD:
        n58cgattparase(restore, len);
        break;
    case N58_MYSYSINFO_CMD:
        n58mysysteminfoparase(restore, len);
        break;
    case N58_CGDCONT_CMD:
        n58cgdcontparase(restore, len);
        break;
    case N58_XGAUTH_CMD:
        n58xgauthparase(restore, len);
        break;
    case N58_XIIC_CMD:
        n58xiicparase(restore, len);
        n58xiicqueryparase(restore, len);
        break;
    case N58_TCPSETUP_CMD:

        break;
    case N58_IPSTATUS_CMD:
        break;
    case N58_TCPACK_CMD:
        break;
    case N58_TCPCLOSE_CMD:
        break;
    case N58_TCPSEND_CMD:
        n58TcpsendParase(restore, len);
        break;
    case N58_CCLK_CMD:
        break;
    case N58_ATA_CMD:
        break;
    case N58_ATH_CMD:
        break;
    case N58_CMGF_CMD:
        break;
    case N58_CMGS_CMD:
        break;
    case N58_CMGD_CMD:
        break;
    case N58_CMGR_CMD:
        break;
    case N58_CNMI_CMD:
        break;
    case N58_MYGPSPWR_CMD:
        break;
    case N58_MYGPSPOS_CMD:
        break;
    case N58_MYGPSSTATE_CMD:
        break;
    case N58_MYPOWEROFF_CMD:
        break;
    case N58_TTSPLAY_CMD:
        break;
    case N58_PLAYTTS_CMD:
        break;
    case N58_FSLIST_CMD:
        break;
    case N58_FSWF_CMD:
        break;
    case N58_FSRF_CMD:
        break;
    case N58_FSDF_CMD:
        break;
    case N58_FSFAT_CMD:
        break;
    }
    free(restore);
}

void n58TTSPlay(char * ttsdata)
{
    uint16_t decodelen;
    uint8_t decode[300];
    char param[5];
    decodelen = strlen(ttsdata);
    if(decodelen > 140)
    {
        return ;
    }
    else
    {
        LogMessage("Play:");
        LogMessage(ttsdata);
        LogMessage("\n");
        changeByteArrayToHexString((uint8_t *)ttsdata, decode, decodelen);
        decode[decodelen * 2] = 0;
        //播放长度
        sprintf(param, "%d", decodelen);
        //发送指令
        sendN58Cmd(N58_TTSPLAY_CMD, param);
        //发送TTS内容
        CreateNodeCmd((char *)decode, decodelen * 2, N58_TTSPLAY_CMD);
    }
}
