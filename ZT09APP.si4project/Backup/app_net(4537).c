#include "app_net.h"
#include "app_port.h"
#include "app_kernal.h"
#include "app_sys.h"
#include "app_param.h"
#include "app_task.h"
#include "app_serverprotocol.h"
#include "app_rec.h"
#include "app_instructioncmd.h"
#include "app_ble.h"

N58_INVOKE_STATUS	n58_invoke_status;
N58_LTE_STATUS 		n58_lte_status;
static NODEDATA *headNode = NULL;


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
    {N58_MYGNSSSEL_CMD, "AT$MYGNSSSEL"},
    {N58_TTSPLAY_CMD, "AT+TTSPLAY"},
    {N58_PLAYTTS_CMD, "AT+PLAYTTS"},
    {N58_FSLIST_CMD, "AT+FSLIST?"},
    {N58_FSWF_CMD, "AT+FSWF"},
    {N58_FSRF_CMD, "AT+FSRF"},
    {N58_FSDF_CMD, "AT+FSDF"},
    {N58_FSFAT_CMD, "AT+FSFAT"},
    {N58_RECMODE_CMD, "AT+RECMODE"},
    {N58_RECF_CMD, "AT+RECF"},
    {N58_WIFIAPSCAN_CMD, "AT+WIFIAPSCAN"},
    {N58_CCID_CMD, "AT+CCID"},
    {N58_SETSERVER_CMD, "AT+SETSERVER"},
    {N58_ENPWRSAVE_CMD, "AT+ENPWRSAVE"},
    {N58_CFUN_CMD, "AT+CFUN"},
    {N58_MICL_CMD, "AT+MICL"},
    {N58_MYLACID_CMD, "AT$MYLACID"},
    {N58_NWBTBLEPWR_CMD, "AT+NWBTBLEPWR"},
    {N58_NWBTBLENAME_CMD, "AT+NWBTBLENAME"},
    {N58_NWBLEROLE_CMD, "AT+NWBLEROLE"},
    {N58_NWBLEPSEND_CMD, "AT+NWBLEPSEND"},
    {N58_NWBLEDISCON_CMD, "AT+NWBLEDISCON"},
    {N58_NWBLEMAC_CMD, "AT+NWBLEMAC"},
    {N58_NWBLECCON_CMD, "AT+NWBLECCON"},
    {N58_MAX_NUM, NULL},
};

uint8_t CreateNodeCmd(char *data, uint16_t datalen, uint8_t currentcmd)
{
    NODEDATA *nextnode;
    NODEDATA *currentnode;
    //如果链表头未创建，则创建链表头。
    MODULE_DTR_HIGH;
    if (headNode == NULL)
    {
        headNode = malloc(sizeof(NODEDATA));
        if (headNode != NULL)
        {
            headNode->currentcmd = currentcmd;
            headNode->data = NULL;
            headNode->data = malloc(datalen);
            if (headNode->data != NULL)
            {
                memcpy(headNode->data, data, datalen);
                headNode->datalen = datalen;
                headNode->nextnode = NULL;
                //LogPrintf(DEBUG_ALL,"CreateNodeCmd==>Head 0x%08X ,data 0x%08X\n",headNode,headNode->data);
                return 1;
            }
            else
            {
                free(headNode);
                headNode = NULL;
                LogMessage(DEBUG_ALL, "CreateNodeCmd==>Malloc memory error\n");
                HAL_NVIC_SystemReset();
                return 0;
            }
        }
        else
        {
            LogMessage(DEBUG_ALL, "CreateNodeCmd==>Malloc memory error\n");
            HAL_NVIC_SystemReset();
            return 0;
        }
    }
    currentnode = headNode;
    do
    {
        nextnode = currentnode->nextnode;
        if (nextnode == NULL)
        {
            nextnode = malloc(sizeof(NODEDATA));
            if (nextnode != NULL)
            {
                nextnode->currentcmd = currentcmd;
                nextnode->data = NULL;
                nextnode->data = malloc(datalen);
                if (nextnode->data != NULL)
                {
                    //LogPrintf(DEBUG_ALL,"CreateNodeCmd==>Node 0x%08X ,data 0x%08X\n",nextnode,nextnode->data);
                    memcpy(nextnode->data, data, datalen);
                    nextnode->datalen = datalen;
                    nextnode->nextnode = NULL;
                    currentnode->nextnode = nextnode;
                    nextnode = nextnode->nextnode;
                }
                else
                {
                    free(nextnode);
                    nextnode = NULL;
                    LogMessage(DEBUG_ALL, "CreateNodeCmd==>Malloc memory error\n");
                    HAL_NVIC_SystemReset();
                    return 0;
                }
            }
            else
            {
                LogMessage(DEBUG_ALL, "CreateNodeCmd==>Malloc memory error\n");
                HAL_NVIC_SystemReset();
                return 0;
            }
        }

        currentnode = nextnode;
    }
    while (nextnode != NULL);

    return 1;
}

/*输出队列*/

void outPutNodeCmd(void)
{
    NODEDATA *nextnode;
    NODEDATA *currentnode;
    if (headNode == NULL)
        return ;
    currentnode = headNode;
    if (currentnode != NULL)
    {
        nextnode = currentnode->nextnode;
        n58_lte_status.current_cmd = currentnode->currentcmd;
        //数据发送
        appUartSend(&usart2_ctl, (uint8_t *)currentnode->data, currentnode->datalen);
        if (currentnode->data[0] != 0X78 && currentnode->data[0] != 0x79)
        {
            LogMessageWL(DEBUG_ALL, currentnode->data, currentnode->datalen);
        }
        //LogPrintf(DEBUG_ALL,"outPutNodeCmd==>Free 0x%08X,Free Data 0x%08X\n",currentnode,currentnode->data);
        free(currentnode->data);
        free(currentnode);

    }
    headNode = nextnode;
    if (headNode == NULL)
        MODULE_DTR_LOW;
}

/*********************************************************/
/*发送模组指令*/
uint8_t  sendModuleCmd(uint8_t cmd, char *param)
{
    uint8_t i;
    int cmdtype = -1;
    char sendData[1024];
    if (cmd >= N58_MAX_NUM)
        return 0;
    for (i = 0; i < N58_MAX_NUM; i++)
    {
        if (cmd == n58_cmd_table[i].cmd_type)
        {
            cmdtype = i;
            break;
        }
    }
    if (cmdtype < 0)
    {
        sprintf(sendData, "sendModuleCmd==>No cmd\n");
        LogMessage(DEBUG_ALL, sendData);
        return 0;
    }
    if (param != NULL && strlen(param) <= 1000)
    {
        if (param[0] == '?')
        {
            sprintf(sendData, "%s?\r\n", n58_cmd_table[cmdtype].cmd);

        }
        else
        {
            sprintf(sendData, "%s=%s\r\n", n58_cmd_table[cmdtype].cmd, param);
        }
    }
    else if (param == NULL)
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
/*********************************************************/
static void moduleInfoInit(void)
{
    memset(&n58_invoke_status, 0, sizeof(N58_INVOKE_STATUS));
    memset(&n58_lte_status, 0, sizeof(N58_LTE_STATUS));
}
/*模组开机*/
void modulePowerOn(void)
{
    MODULE_RSTKEY_HIGH;
    MODULE_DTR_HIGH;
    MODULE_POWER_ON;
    moduleInfoInit();
    n58_invoke_status.modulepowerstate = 1;
    LogMessage(DEBUG_ALL, "Open moudle power supply\n");
    appBleInfoReset();
}
/*模组关机*/
void modulePowerOff(void)
{
    MODULE_POWER_OFF;
    moduleInfoInit();
    netConnectReset();
    n58_invoke_status.modulepowerstate = 0;
    LogMessage(DEBUG_ALL, "Close moudle power supply\n");
    appBleInfoReset();
}

static void moduleResetProcessA(void)
{
    MODULE_DTR_HIGH;
    MODULE_RSTKEY_HIGH;
    n58_invoke_status.modulepowerstate = 1;
    LogMessage(DEBUG_ALL, "Reset complete\n");
}
/*模组复位*/
void moduleReset(void)
{
    MODULE_DTR_HIGH;
    MODULE_RSTKEY_LOW;
    LogMessage(DEBUG_ALL, "Reset module\n");
    moduleInfoInit();
    netConnectReset();
    updateSystemLedStatus(SYSTEM_LED_NETOK, 0);
    startTimer(800, moduleResetProcessA, 0);
    appBleInfoReset();
}

/*********************************************************/
/*模组联网*/

void N58_ChangeInvokeStatus(N58_WORK_STATUS status)
{
    n58_invoke_status.invoke_status = status;
    n58_invoke_status.tick_time = 0;
    if (n58_invoke_status.invoke_status != N58_NORMAL_STATUS)
    {
        updateSystemLedStatus(SYSTEM_LED_NETOK, 0);
    }
}

static void N58enterFlightMode(void)
{
    LogMessage(DEBUG_ALL, "Module enter flight mode\n");
    sendModuleCmd(N58_CFUN_CMD, "4,0");
}
static void N58enterNormalMode(void)
{
    sendModuleCmd(N58_CFUN_CMD, "1,0");
    LogMessage(DEBUG_ALL, "Module enter normal mode\n");
}

void N58_CreateSocket(uint8_t link, char *server, uint16_t port)
{
    char param[100];
    sprintf(param, "%d,%s,%d", link, server, port);
    sendModuleCmd(N58_TCPSETUP_CMD, param);
}
void sendDataToServer(uint8_t *data, uint16_t len)
{
    char param[10];
    sprintf(param, "0,%d", len);
    sendModuleCmd(N58_TCPSEND_CMD, param);
    CreateNodeCmd((char *)data, len, N58_TCPSETUP_CMD);
}

void n58setAPN(void)
{
    char param[40];
    sprintf(param, "1,\"IP\",\"%s\"", sysparam.apn);
    sendModuleCmd(N58_CGDCONT_CMD, param);
}
void n58setXGAUTH(void)
{
    char param[40];
    sprintf(param, "1,1,\"%s\",\"%s\"", sysparam.apnuser, sysparam.apnpassword);
    sendModuleCmd(N58_XGAUTH_CMD, param);
}

void reConnectServer(void)
{
    N58_ChangeInvokeStatus(N58_TCPCLOSE_STATUS);
}

uint8_t netWorkModuleRunOk(void)
{
    if (n58_invoke_status.poweron_tick > 20)
        return 1;
    return 0;
}

void networkConnectProcess(void)
{
    if (n58_invoke_status.modulepowerstate == 0)
    {
        return ;
    }
    n58_invoke_status.poweron_tick++;
    switch (n58_invoke_status.invoke_status)
    {
        case N58_AT_STATUS:
            if (n58_invoke_status.poweron_tick < 3)
            {
                break;
            }
            if (n58_lte_status.at_respon == 0)
            {
                n58_invoke_status.poweron_tick = 0;
                sendModuleCmd(N58_AT_CMD, NULL);
                if (n58_invoke_status.tick_time > 90)
                {
                    moduleReset();//复位设备
                }
                break;
            }
            else
            {
                n58_lte_status.at_respon = 0;
                N58_ChangeInvokeStatus(N58_CPIN_STATUS);
            }
        case N58_CPIN_STATUS:
            if (n58_lte_status.cpin_respon == 0)
            {
                sendModuleCmd(N58_CPIN_CMD, NULL);
                if (n58_invoke_status.tick_time > 30)
                {
                    moduleReset();//复位设备
                }
                break;
            }
            else
            {
                n58_lte_status.cpin_respon = 0;
                sendModuleCmd(N58_CIMI_CMD, NULL);
                N58_ChangeInvokeStatus(N58_IMSI_STATUS);
            }
        case N58_IMSI_STATUS:
            if (n58_lte_status.cimi_respon == 0)
            {
                if (n58_invoke_status.tick_time % 3 == 0)
                    sendModuleCmd(N58_CIMI_CMD, NULL);
                if (n58_invoke_status.tick_time > 7)
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
                sendModuleCmd(N58_ENPWRSAVE_CMD, "1");
                n58_lte_status.cimi_respon = 0;
                N58_ChangeInvokeStatus(N58_CSQ_STATUS);
            }
        case N58_CSQ_STATUS:
            if (n58_lte_status.csq_ok == 0)
            {
                sendModuleCmd(N58_CSQ_CMD, NULL);
                if (n58_invoke_status.tick_time >= sysinfo.csqSearchTime) //
                {
                    sysinfo.csqSearchTime *= 2;
                    n58_invoke_status.tick_time = 0;
                    n58_lte_status.reCSQ_count++;

                    if (sysparam.MODE == MODE4 && n58_lte_status.reCSQ_count = 2)
                    {
                        modulePowerOff();
                        return;
                    }
                    if (n58_lte_status.reCSQ_count > = 3) //超过3次，6分钟，重启设备
                    {
                        moduleReset();//复位设备
                        break;
                    }
                    N58_ChangeInvokeStatus(N58_AT_STATUS);
                    N58enterFlightMode();
                    startTimer(15000, N58enterNormalMode, 0);

                }
                break;
            }
            else
            {
                n58_lte_status.csq_ok = 0;
                n58_lte_status.reCSQ_count = 0;
                sysinfo.csqSearchTime = 30;
                sendModuleCmd(N58_CMGF_CMD, "1");
                sendModuleCmd(N58_CNMI_CMD, "0,1");
                sendModuleCmd(N58_ENPWRSAVE_CMD, "1");
                sendModuleCmd(N58_AT_CMD, NULL);
                N58_ChangeInvokeStatus(N58_CREG_STATUS);
            }
        case N58_CREG_STATUS:
            if (!(n58_lte_status.creg_state == 1 || n58_lte_status.creg_state == 5))
            {
                sendModuleCmd(N58_CREG_CMD, "?"); //注册网络
                if (n58_invoke_status.tick_time >= 90) //搜索超时1.5minutes
                {
                    n58_invoke_status.tick_time = 0;
                    n58_lte_status.reCreg_Count++;
                    if (n58_lte_status.reCreg_Count >= 4) //超过4次，6分钟，重启设备
                    {
                        moduleReset();//复位设备
                        break;
                    }
                    N58_ChangeInvokeStatus(N58_AT_STATUS);
                    N58enterFlightMode();
                    startTimer(15000, N58enterNormalMode, 0);

                }
                break;
            }
            else
            {
                n58_lte_status.reCreg_Count = 0;
                n58_lte_status.creg_state = 0;
                n58_lte_status.ICCID[0] = 0;
                N58_ChangeInvokeStatus(N58_CCID_STATUS);
                if (sysparam.MODE == MODE4 && sysinfo.GPSRequest == 0 && sysinfo.lbsrequest == 0 && sysinfo.alarmrequest == 0 &&
                        sysinfo.wifirequest == 0)
                {
                    N58_ChangeInvokeStatus(N58_NORMAL_STATUS);
                    break;
                }

            }
        case N58_CCID_STATUS:
            if (n58_lte_status.ICCID[0] == 0)
            {
                sendModuleCmd(N58_CCID_CMD, NULL);
                if (n58_invoke_status.tick_time > 40)
                {
                    moduleReset();//复位设备
                }
                break;
            }
            else
            {
                N58_ChangeInvokeStatus(N58_CGATT_STATUS);
            }
        case N58_CGATT_STATUS:
            if (n58_lte_status.cgatt_ok == 0)
            {
                if (n58_invoke_status.tick_time % 3 == 0)
                    sendModuleCmd(N58_CGATT_CMD, "?");
                if (n58_invoke_status.tick_time > 30)
                {
                    if (n58_lte_status.cgatt_bymen == 1)
                    {
                        moduleReset();//复位设备
                        n58_lte_status.cgatt_bymen = 0;
                    }
                    else
                    {
                        sendModuleCmd(N58_CGATT_CMD, "1");
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
            if (n58_lte_status.mysysinfo_ok == 0)
            {
                sendModuleCmd(N58_MYSYSINFO_CMD, NULL);
                break;
            }
            else
            {
                n58_lte_status.mysysinfo_ok = 0;
                N58_ChangeInvokeStatus(N58_CGDCONT_STATUS);
            }
        case N58_CGDCONT_STATUS:
            if (n58_lte_status.cgdcont_ok == 0)
            {
                n58setAPN();
                if (n58_invoke_status.tick_time > 4)
                {
                    moduleReset();//复位设备
                }
                break;
            }
            else
            {
                n58_lte_status.cgdcont_ok = 0;
                N58_ChangeInvokeStatus(N58_XGAUTH_STATUS);
            }
        case N58_XGAUTH_STATUS:
            if (n58_lte_status.xgauth_ok == 0)
            {
                n58setXGAUTH();
                if (n58_invoke_status.tick_time > 5)
                {
                    moduleReset();//复位设备
                }
                break;
            }
            else
            {
                N58_ChangeInvokeStatus(N58_XIIC_STATUS);
                n58_lte_status.rexiic_count = 0;
                n58_lte_status.xgauth_ok = 0;
                n58_lte_status.xiic_ok = 0;
            }
        case N58_XIIC_STATUS:
            if (n58_lte_status.xiic_ok == 0)
            {
                sendModuleCmd(N58_XIIC_CMD, "1");
                if (n58_invoke_status.tick_time > 8)
                {
                    moduleReset();//复位设备
                }
                break;
            }
            else
            {
                n58_lte_status.xiic_ok = 0;
                N58_ChangeInvokeStatus(N58_XIIC_QUERY_STATUS);
            }
        case N58_XIIC_QUERY_STATUS:
            if (n58_lte_status.xiic_queryok == 0)
            {
                sendModuleCmd(N58_XIIC_CMD, "?");
                if (n58_invoke_status.tick_time > 30)
                {
                    N58_ChangeInvokeStatus(N58_XIIC_QUERY_STATUS);
                    sendModuleCmd(N58_XIIC_CMD, "1");
                    n58_lte_status.rexiic_count++;
                    n58_lte_status.xiic_ok = 0;
                    if (n58_lte_status.rexiic_count > 3)
                    {
                        moduleReset();//复位设备
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
            if (n58_lte_status.tcpclose_ok == 0)
            {
                if (n58_invoke_status.tick_time % 30 == 0)
                {
                    sendModuleCmd(N58_TCPCLOSE_CMD, "0");
                    n58_lte_status.retcpclose_count++;
                    if (n58_lte_status.retcpclose_count >= 3)
                    {
                        moduleReset();//复位设备
                    }
                }
                break;
            }
            else
            {
                n58_lte_status.tcpclose_ok = 0;
                n58_lte_status.tcpsetup_error = 0;
                //n58_lte_status.retcpsetup_count=0;
                n58_lte_status.retcpclose_count = 0;
                N58_ChangeInvokeStatus(N58_TCPSETUP_STATUS);
                if (sysparam.MODE == MODE4 && sysinfo.GPSRequest == 0 && sysinfo.lbsrequest == 0 && sysinfo.alarmrequest == 0 &&
                        sysinfo.wifirequest == 0)
                {
                    N58_ChangeInvokeStatus(N58_NORMAL_STATUS);
                    break;
                }
            }
        case N58_TCPSETUP_STATUS:
            LogMessage(DEBUG_ALL, "Waitting for socket connect\n");
            if (n58_lte_status.tcpsetup_ok == 0)
            {

                if (n58_invoke_status.tick_time % 30 == 0 || n58_lte_status.tcpsetup_error == 1)
                {
                    n58_lte_status.tcpsetup_error = 0;
                    n58_invoke_status.tick_time = 0;
                    n58_lte_status.retcpsetup_count++;

                    if (n58_lte_status.retcpsetup_count == 2)
                    {
                        n58_lte_status.csq_ok = 0;
                        N58_ChangeInvokeStatus(N58_CSQ_STATUS);
                        break;
                    }
                    else if (n58_lte_status.retcpsetup_count >= 4)
                    {
                        moduleReset();//复位设备
                        break;
                    }
                    //创建链接
                    N58_CreateSocket(0, (char *)sysparam.Server, sysparam.ServerPort);
                }
                break;
            }
            else
            {
                n58_lte_status.tcpsetup_ok = 0;
                n58_lte_status.retcpsetup_count = 0;
                n58_lte_status.tcpsetup_error = 0;
                netConnectReset();
                N58_ChangeInvokeStatus(N58_NORMAL_STATUS);
            }
        case N58_NORMAL_STATUS:
            if (sysparam.MODE == MODE4)
            {
                if (sysinfo.GPSRequest != 0 || sysinfo.lbsrequest != 0 || sysinfo.wifirequest != 0 || sysinfo.alarmrequest != 0)
                {
                    protocolRunFsm();
                }
            }
            else
            {
                protocolRunFsm();
            }
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
void n58CSQparase(uint8_t *buf, uint16_t len)
{
    int index, indexa, datalen;
    uint8_t *rebuf;
    uint16_t  relen;
    char restore[5];
    char debug[20];
    index = my_getstrindex((char *)buf, "+CSQ:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        indexa = getCharIndex(rebuf, relen, ',');
        datalen = indexa - 6;
        if (datalen > 5)
            return;
        memset(restore, 0, 5);
        strncpy(restore, (char *)rebuf + 6, datalen);
        n58_lte_status.csq_rssi = atoi(restore);
        sprintf(debug, "CSQ State=%d\n", n58_lte_status.csq_rssi);
        LogMessage(DEBUG_ALL, debug);
        if (n58_lte_status.csq_rssi >= 6 && n58_lte_status.csq_rssi <= 31)
            n58_lte_status.csq_ok = 1;
        N58_ClearCmd();
        clearHbtTimer();
    }
}

/*
+CREG: 2,3,"0000","029cc501",0
OK

*/
void n58CREGparase(uint8_t *buf, uint16_t len)
{
    int index, datalen;
    uint8_t *rebuf;
    uint16_t  relen, i;
    char restore[50];
    char debug[100];
    uint8_t   cnt;
    index = my_getstrindex((char *)buf, "+CREG:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        datalen = 0;
        cnt = 0;
        restore[0] = 0;
        for (i = 0; i < relen; i++)
        {
            if (rebuf[i] == ',' || rebuf[i] == '\r' || rebuf[i] == '\n')
            {
                if (restore[0] != 0)
                {
                    restore[datalen] = 0;
                    cnt++;
                    datalen = 0;
                    //sprintf(debug,"n58CREGparase==>%s,cnt=%d\n",restore,cnt);
                    //LogMessage(DEBUG_ALL, debug);
                    switch (cnt)
                    {
                        case 2:
                            n58_lte_status.creg_state = atoi(restore);
                            sprintf(debug, "CREG state=%d\n", n58_lte_status.creg_state);
                            LogMessage(DEBUG_ALL, debug);
                            return;
                    }
                    restore[0] = 0;
                }
            }
            else
            {
                restore[datalen] = rebuf[i];
                datalen++;
                if (datalen >= 50)
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
void n58cgattparase(uint8_t *buf, uint16_t len)
{
    int16_t index;
    uint8_t  value;
    uint8_t  *rebuf;
    char debug[30];
    index = my_getstrindex((char *)buf, "+CGATT:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        value = rebuf[8] - '0';
        n58_lte_status.cgatt_ok = value;
        sprintf(debug, "CGATT=%d\n", n58_lte_status.cgatt_ok);
        LogMessage(DEBUG_ALL, debug);
    }
}

void n58mysysteminfoparase(uint8_t *buf, uint16_t len)
{
    if (my_getstrindex((char *)buf, "OK", len) >= 0 || my_getstrindex((char *)buf, "ERROR", len) >= 0)
    {
        n58_lte_status.mysysinfo_ok = 1;

    }
}
void n58cgdcontparase(uint8_t *buf, uint16_t len)
{
    if (distinguishOK((char *) buf))
    {
        n58_lte_status.cgdcont_ok = 1;
    }
}
void n58xgauthparase(uint8_t *buf, uint16_t len)
{
    if (distinguishOK((char *) buf))
    {
        n58_lte_status.xgauth_ok = 1;
    }
}
void n58xiicparase(uint8_t *buf, uint16_t len)
{
    if (distinguishOK((char *) buf))
    {
        n58_lte_status.xiic_ok = 1;
    }
}
void n58xiicqueryparase(uint8_t *buf, uint16_t len)
{

    int index, datalen;
    uint8_t *rebuf;
    uint16_t  relen, i;
    char restore[50];
    char debug[100];
    uint8_t   cnt, value;
    index = my_getstrindex((char *)buf, "+XIIC:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        datalen = 0;
        cnt = 0;
        restore[0] = 0;
        for (i = 0; i < relen; i++)
        {
            if (rebuf[i] == ',' || rebuf[i] == '\r' || rebuf[i] == '\n')
            {
                if (restore[0] != 0)
                {
                    restore[datalen] = 0;
                    cnt++;
                    datalen = 0;
                    //sprintf(debug,"n58CREGparase==>%s,cnt=%d\n",restore,cnt);
                    //LogMessage(DEBUG_ALL, debug);
                    switch (cnt)
                    {
                        case 1:
                            value = restore[10];
                            n58_lte_status.xiic_queryok = value - '0';
                            sprintf(debug, "XIIC state=%d\n", n58_lte_status.xiic_queryok);
                            LogMessage(DEBUG_ALL, debug);
                            break;
                    }
                    restore[0] = 0;
                }
            }
            else
            {
                restore[datalen] = rebuf[i];
                datalen++;
                if (datalen >= 50)
                {
                    return ;
                }

            }
        }
        N58_ClearCmd();
    }
}
/*
+TCPCLOSE: 0,Link Closed

*/
void n58TCPcloseparase(uint8_t *buf, uint16_t len)
{
    int index;
    uint8_t *rebuf;
    //uint16_t relen;
    index = my_getstrindex((char *)buf, "+TCPCLOSE:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        //relen=len-index;
        n58_lte_status.tcpclose_ok = 1;
        if (strstr((char *)rebuf, "Closed") != NULL)
        {
            if (sysparam.MODE == MODE4)
            {
                if (sysinfo.GPSRequest != 0 || sysinfo.lbsrequest != 0 || sysinfo.wifirequest != 0 || sysinfo.alarmrequest != 0)
                {
                    N58_ChangeInvokeStatus(N58_TCPSETUP_STATUS);
                }
            }
            else
            {
                N58_ChangeInvokeStatus(N58_TCPSETUP_STATUS);
            }
            LogMessage(DEBUG_ALL, "Socket link close\n");
        }
    }
}
/*
+CIMI: 460045562404799
OK

*/
void n58CIMIParase(uint8_t *buf, uint16_t len)
{
    int16_t index, indexa;
    uint8_t *rebuf;
    uint16_t  relen;
    uint8_t snlen, i;
    char debug[50];
    index = my_getstrindex((char *)buf, "+CIMI:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        indexa = getCharIndex(rebuf, relen, '\r');
        snlen = indexa - 7;
        if (snlen < 30)
        {
            for (i = 0; i < snlen; i++)
            {
                n58_lte_status.IMSI[i] = rebuf[i + 7];
            }
            n58_lte_status.IMSI[snlen] = 0;
            sprintf(debug, "Get IMSI:%s\n", n58_lte_status.IMSI);
            LogMessage(DEBUG_ALL, debug);
            n58_lte_status.cimi_respon = 1;
            sysinfo.mcc = (n58_lte_status.IMSI[0] - '0') * 100 + (n58_lte_status.IMSI[1] - '0') * 10 + n58_lte_status.IMSI[2] - '0';
            sysinfo.mnc = (n58_lte_status.IMSI[3] - '0') * 10 + n58_lte_status.IMSI[4] - '0';
            LogPrintf(DEBUG_ALL, "MCC=%d,MNC=%d\n", sysinfo.mcc, sysinfo.mnc);
        }

    }

}

void n58CGSNParase(uint8_t *buf, uint16_t len)
{
    int16_t index, indexa;
    uint8_t *rebuf;
    uint16_t  relen;
    uint8_t snlen, i;
    index = my_getstrindex((char *)buf, "+CGSN:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        indexa = getCharIndex(rebuf, relen, '\r');
        snlen = indexa - 7;
        if (snlen < 20)
        {
            for (i = 0; i < snlen; i++)
            {
                n58_lte_status.IMEI[i] = rebuf[i + 7];
            }
            n58_lte_status.IMEI[snlen] = 0;
        }
    }
}

/*
+CCID: 89860403101872377455
OK

*/
void n58CCIDParase(uint8_t *buf, uint16_t len)
{
    int16_t index, indexa;
    uint8_t *rebuf;
    uint16_t  relen;
    uint8_t snlen, i;
    char debug[70];
    index = my_getstrindex((char *)buf, "+CCID:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        indexa = getCharIndex(rebuf, relen, '\r');
        snlen = indexa - 7;
        if (snlen < sizeof(n58_lte_status.ICCID))
        {
            for (i = 0; i < snlen; i++)
            {
                n58_lte_status.ICCID[i] = rebuf[i + 7];
            }
            n58_lte_status.ICCID[snlen] = 0;
            sprintf(debug, "Get ICCID:%s\n", n58_lte_status.ICCID);
            LogMessage(DEBUG_ALL, debug);
        }

    }

}

/*+TCPSETUP: 0,ERROR1*/
void n58TCPsetupparase(uint8_t *buf, uint16_t len)
{

    int index, datalen;
    uint8_t *rebuf;
    uint16_t  relen, i;
    char restore[50];
    char debug[100];
    uint8_t   cnt;
    index = my_getstrindex((char *)buf, "+TCPSETUP:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        datalen = 0;
        cnt = 0;
        restore[0] = 0;
        for (i = 0; i < relen; i++)
        {
            if (rebuf[i] == ',' || rebuf[i] == '\r' || rebuf[i] == '\n')
            {
                if (restore[0] != 0)
                {
                    restore[datalen] = 0;
                    cnt++;
                    datalen = 0;
                    //sprintf(debug,"n58CREGparase==>%s,cnt=%d\n",restore,cnt);
                    //LogMessage(DEBUG_ALL,debug);
                    switch (cnt)
                    {
                        case 1:
                            if (strstr(restore, "ERROR") != NULL)
                            {
                                n58_lte_status.tcpsetup_error = 1;
                                sprintf(debug, "TCP socket Error\n");
                                LogMessage(DEBUG_ALL, debug);
                            }
                        case 2:
                            if (distinguishOK((char *) restore))
                            {
                                n58_lte_status.tcpsetup_ok = 1;
                                sprintf(debug, "TCP socket connect OK\n");
                                LogMessage(DEBUG_ALL, debug);
                            }
                            else if (strstr(restore, "FAIL") != NULL)
                            {
                                n58_lte_status.tcpsetup_error = 1;
                                sprintf(debug, "TCP socket connect Fail\n");
                                LogMessage(DEBUG_ALL, debug);
                            }
                            else if (strstr(restore, "ERROR1") != NULL)
                            {
                                n58_lte_status.tcpsetup_ok = 1;
                                sprintf(debug, "TCP socket already alive\n");
                                LogMessage(DEBUG_ALL, debug);
                            }
                            break;
                    }
                }
            }
            else
            {
                restore[datalen] = rebuf[i];
                datalen++;
                if (datalen >= 50)
                {
                    return ;
                }

            }
        }
        N58_ClearCmd();
    }
}


void n58TCPRecvparase(uint8_t *buf, uint16_t len)
{
    int16_t index;
    uint8_t *rebuf;
    int32_t relen, datalen;
    char restore[512];
    rebuf = buf;
    relen = len;
    index = my_getstrindex((char *)rebuf, "+TCPRECV:", relen);
    while (index >= 0)
    {
        rebuf += index;
        relen -= index;
        index = getCharIndex(rebuf, relen, ',');
        rebuf += (index + 1);
        relen -= (index + 1);
        index = getCharIndex(rebuf, relen, ',');
        strncpy(restore, (char *)rebuf, index);
        restore[index] = 0;
        datalen = atoi(restore);
        rebuf += (index + 1);
        relen -= (index + 1);
        if (datalen < 256)
        {
            changeByteArrayToHexString(rebuf, (uint8_t *)restore, datalen);
            restore[datalen * 2] = 0;
            LogMessage(DEBUG_ALL, "TCP Recv:");
            LogMessage(DEBUG_ALL, restore);
            LogMessage(DEBUG_ALL, "\n");
        }
        protocolRxParase((char *)rebuf, datalen);
        rebuf += (datalen);
        relen -= (datalen);
        index = my_getstrindex((char *)rebuf, "+TCPRECV:", relen);
    }

}



static void n58TcpsendParase(uint8_t *buf, uint16_t len)
{
    if (my_strstr((char *)buf, "ERROR", len) || my_strstr((char *)buf, "+TCPSEND: SOCKET ID OPEN FAILED", len))
    {
        if (sysparam.MODE == MODE4)
        {
            if (sysinfo.GPSRequest != 0 || sysinfo.lbsrequest != 0 || sysinfo.wifirequest != 0 || sysinfo.alarmrequest != 0)
            {
                N58_ChangeInvokeStatus(N58_TCPSETUP_STATUS);
            }
        }
        else
        {
            N58_ChangeInvokeStatus(N58_TCPSETUP_STATUS);
        }
        N58_ClearCmd();
    }
}
/*
$MYGPSSTATE: gps opened
$MYGPSSTATE: gps closed


*/
void n58MygpsstateParase(uint8_t *buf, uint16_t len)
{
    uint8_t *rebuf;
    uint16_t  relen;
    int16_t index;
    index = my_getstrindex((char *)buf, "$MYGPSSTATE:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        if (my_strstr((char *)rebuf, "opened", relen))
        {
            sysinfo.GPSStatus = 1;
            gpsChangeFsmState(GPSOPENSTATUS);
        }
        else if (my_strstr((char *)rebuf, "closed", relen))
        {
            sysinfo.GPSStatus = 0;
            gpsChangeFsmState(GPSCLOSESTATUS);
            gpsClearCurrentGPSInfo();
            updateSystemLedStatus(SYSTEM_LED_GPSOK, 0);
        }
    }
}

/*

gpssave1.dat,3580
test.txt,20
test1.txt,20
test2.txt,20
OK

*/

void n58FSLISTparase(uint8_t *buf, uint16_t len)
{
    uint16_t i, count = 0, index;
    char fileinfobuf[70];
    char restore[70];
    char debug[100];
    if (distinguishOK((char *) buf))
    {
        n58_lte_status.fslist_ok = 1;
    }
    for (i = 0; i < len; i++)
    {
        if (buf[i] == '\r' || buf[i] == '\n')
        {
            fileinfobuf[count] = 0;
            if (fileinfobuf[0] != 0)
            {
                if (my_strpach(fileinfobuf, "gpssave"))
                {
                    index = getCharIndex((uint8_t *)fileinfobuf, count, ',');
                    strncpy(restore, fileinfobuf, index);
                    restore[index] = 0;
                    gpsUpdateRestoreFileNameAndTotalSize(restore, 0);
                    sprintf(debug, "GPSFile:%s", restore);
                    strncpy(restore, fileinfobuf + index + 1, count - index - 1);
                    restore[count - index - 1] = 0;
                    gpsUpdateRestoreFileNameAndTotalSize(NULL, atoi(restore));
                    sprintf(debug + strlen(debug), " ,Size:%d\n", atoi(restore));
                    LogMessage(DEBUG_ALL, debug);
                    N58_ClearCmd();
                    return;
                }
                else
                {
                    gpsNoRestoreFileToRead();
                }

                if (my_strpach(fileinfobuf, "REC"))
                {
                    index = getCharIndex((uint8_t *)fileinfobuf, count, ',');
                    strncpy(restore, fileinfobuf, index);
                    restore[index] = 0;
                    recUpdateRestoreFileNameAndTotalSize(restore, 0);
                    sprintf(debug, "RECFile:%s", restore);
                    strncpy(restore, fileinfobuf + index + 1, count - index - 1);
                    restore[count - index - 1] = 0;
                    recUpdateRestoreFileNameAndTotalSize(NULL, atoi(restore));
                    sprintf(debug + strlen(debug), " ,Size:%d\n", atoi(restore));
                    LogMessage(DEBUG_ALL, debug);
                    N58_ClearCmd();
                    return;
                }
                else
                {
                    recNoFileToread();
                }
            }
            count = 0;

        }
        else
        {
            if (count >= 70)
            {
                count = 0;
            }
            fileinfobuf[count++] = buf[i];
        }
    }
    N58_ClearCmd();
}

/*
+FSRF: 20,4Vx4Vx4Vx4Vx4Vx
OK

*/
void n58FswrParase(uint8_t *buf, uint16_t len)
{
    int16_t index;
    uint8_t gpscount, i;
    uint16_t relen, datalen, destlen, packnum;
    uint8_t *rebuf;
    uint8_t readsize[10];
    GPSRestoreStruct *gpsinfo;
    RECORDFILE *rffile;
    char  dest[1152];
    index = my_getstrindex((char *)buf, "+FSRF:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        index = getCharIndex(rebuf, len, ',');
        if ((index - 7) > 8)
            return ;
        memcpy(readsize, rebuf + 7, index - 7);
        readsize[index - 7] = 0;
        //解析内容长度
        datalen = atoi((char *)readsize);
        //数据在数组中的位置和长度
        rebuf = rebuf + index + 1;
        relen = relen - index - 1;
        //避免只接收了一段数据
        if (relen > datalen)
        {
            if (gpsIsRun() == 1)
            {
                gpscount = datalen / sizeof(GPSRestoreStruct);
                LogPrintf(DEBUG_ALL, "GPS totoal count=%d\n", gpscount);
                destlen = 0;
                for (i = 0; i < gpscount; i++)
                {
                    gpsinfo = (GPSRestoreStruct *)(rebuf + (sizeof(GPSRestoreStruct) * i));
                    gpsRestoreDataSend(gpsinfo, dest + destlen, &datalen);
                    destlen += datalen;
                }
                sendDataToServer((uint8_t *)dest, destlen);
                return;
            }
            if (recIsRun() == 1)
            {
                LogPrintf(DEBUG_ALL, "REC data len=%d\n", datalen);
                rffile = recGetRecStruct();
                packnum = rffile->havereaddata / RECORD_UPLOADONEPACKSIZE;
                LogPrintf(DEBUG_ALL, "Upload Record Pack Num %d\n", packnum);
                createProtocol62(dest, rffile->filename + 3, packnum, rebuf, datalen);
                recReadFileOk();
            }

        }
    }
}

/*
+CMTI: "SM",33

*/
void n58CMTIparase(uint8_t *buf, uint16_t len)
{
    uint8_t i;
    int16_t index;
    uint8_t *rebuf;
    char restore[5];
    index = my_getstrindex((char *)buf, "+CMTI:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        index = getCharIndex(rebuf, len, ',');
        rebuf = rebuf + index + 1;
        index = getCharIndex(rebuf, len, '\r');
        if (index > 5)
            return ;
        for (i = 0; i < index; i++)
        {
            restore[i] = rebuf[i];
        }
        restore[index] = 0;
        LogPrintf(DEBUG_ALL, "Message index=%d\n", atoi(restore));
        sendModuleCmd(N58_CMGR_CMD, restore);
    }
}
/*
+CMGR: "REC UNREAD","8613106937034",,"2020/08/18,16:40:29+32"
PARAM
OK


*/
void n58CMGRparse(uint8_t *buf, uint16_t len)
{
    int index;
    uint8_t *rebuf;
    uint8_t *numbuf;
    uint16_t  relen, i, renumlen;
    char restore[100];
    //找到特定字符串在buf的位置
    index = my_getstrindex((char *)buf, "+CMGR:", len);
    if (index >= 0)
    {
        //得到特定字符串的开始位置和剩余长度
        rebuf = buf + index;
        relen = len - index;
        //识别手机号码
        index = getCharIndexWithNum(rebuf, relen, '"', 3);
        numbuf = rebuf + index + 1;
        renumlen = relen - index - 1;
        index = getCharIndex(numbuf, renumlen, '"');
        if (index > 100)
            return ;
        for (i = 0; i < index; i++)
        {
            restore[i] = numbuf[i];
        }
        restore[index] = 0;

        if (index > sizeof(n58_lte_status.messagePhone))
            return ;
        strcpy((char *)n58_lte_status.messagePhone, restore);
        LogPrintf(DEBUG_ALL, "Tel:%s\n", n58_lte_status.messagePhone);
        //得到第一个\n的位置
        index = getCharIndex(rebuf, len, '\n');
        //偏移到内容处
        rebuf = rebuf + index + 1;
        //得到从内容处开始的第一个\n，测试index就是内容长度
        index = getCharIndex(rebuf, len, '\n');
        if (index > 100)
            return ;
        for (i = 0; i < index; i++)
        {
            restore[i] = rebuf[i];
        }
        restore[index] = 0;
        LogPrintf(DEBUG_ALL, "Message:%s\n", restore);
        instructionParase((uint8_t *)restore, index, SHORTMESSAGE_MODE, (char *)n58_lte_status.messagePhone);
    }
}




/*
+WIFIAPSCAN: ec41180c8209,-88,4
+WIFIAPSCAN: 8cf228971824,-71,1
+WIFIAPSCAN: 882593728700,-76,5
+WIFIAPSCAN: dc9fdb1c1d76,-93,13
+WIFIAPSCAN: 3891d5604d68,-99,4
OK

*/
void n58WIFISCANParse(uint8_t *buf, uint16_t len)
{
    int16_t index;
    uint8_t *rebuf;
    uint16_t relen;
    char restore[50];
    char txdata[256];
    N58_WIFIAPSCAN wap;
    GPSINFO *gpsinfo;
    rebuf = buf;
    relen = len;
    index = my_getstrindex((char *)rebuf, "+WIFIAPSCAN:", relen);
    wap.apcount = 0;
    while (index >= 0)
    {
        rebuf = rebuf + index;
        relen = relen - index;
        index = getCharIndex(rebuf, relen, ',');
        if (index >= 25)
        {
            strncpy(restore, (char *)rebuf + 13, 12);
            restore[12] = 0;
            changeHexStringToByteArray(wap.ap[wap.apcount].ssid, (uint8_t *)restore, 6);
            wap.apcount++;
            LogPrintf(DEBUG_ALL, "Ap:%d,MAC:%s\n", wap.apcount, restore);
            if (wap.apcount >= N58MAXWIFISCAN)
            {
                break;
            }
        }
        rebuf += 11;
        relen -= 11;
        index = my_getstrindex((char *)rebuf, "+WIFIAPSCAN:", relen);
    }

    if (wap.apcount > 0)
    {
        //WIFI 结束，判断GPS是否已定位，如果未定位，则上报WIFI，已定位，上报GPS
        gpsinfo = getCurrentGPSInfo();
        if (gpsinfo->fixstatus == 1)
        {
            sendProtocolToServer(PROTOCOL_12, gpsinfo);
        }
        else
        {
            createProtocolF3(txdata, &wap);
        }
    }

}


void n58MygpsposParaser(uint8_t *buf, uint16_t len)
{
    int16_t index;
    uint8_t *rebuf;
    uint16_t relen;
    index = my_getstrindex((char *)buf, "$MYGPSPOS:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        nmeaParse(rebuf, relen);
    }
}

/*

GPRS DISCONNECTION

+TCPCLOSE: 0,Link Closed

*/

static int8_t autoatatimerid = -1;

void n58ATA(void)
{
    sendModuleCmd(N58_ATA_CMD, NULL);
    autoatatimerid = -1;
}
void n58AutoATA(uint8_t *buf, uint16_t len)
{
    if (sysparam.autoAnswer == 0)
        return ;
    if (my_strstr((char *)buf, "RING", len))
    {
        if (autoatatimerid == -1)
        {
            autoatatimerid = startTimer(2500, n58ATA, 0);
        }
    }
}

/*
$MYLACID: 24D3,029CC501
OK
*/
void n58MylacidParse(uint8_t *buf, uint16_t len)
{
    int index, datalen;
    uint8_t *rebuf;
    uint16_t  relen, i;
    char restore[50];
    char debug[100];
    uint8_t   cnt;
    index = my_getstrindex((char *)buf, "$MYLACID:", len);
    if (index >= 0)
    {
        rebuf = buf + index + 10;
        relen = len - index - 10;
        datalen = 0;
        cnt = 0;
        restore[0] = 0;
        for (i = 0; i < relen; i++)
        {
            if (rebuf[i] == ',' || rebuf[i] == '\r' || rebuf[i] == '\n')
            {
                if (restore[0] != 0)
                {
                    restore[datalen] = 0;
                    cnt++;
                    datalen = 0;
                    switch (cnt)
                    {
                        case 1:
                            sysinfo.lac = strtoul(restore, NULL, 16);
                            sprintf(debug, "LAC=%s,0x%X\n", restore, sysinfo.lac);
                            LogMessage(DEBUG_ALL, debug);

                            break;
                        case 2:
                            sysinfo.cid = strtoul(restore, NULL, 16);
                            sprintf(debug, "CID=%s,0x%X\n", restore, sysinfo.cid);
                            LogMessage(DEBUG_ALL, debug);
                            break;
                    }
                    restore[0] = 0;
                }
            }
            else
            {
                restore[datalen] = rebuf[i];
                datalen++;
                if (datalen >= 50)
                {
                    return ;
                }

            }
        }
        N58_ClearCmd();
    }
}

void n58NwbtblepwrParser(uint8_t *buf, uint16_t len)
{
    int index;
    uint8_t *rebuf;
    uint16_t  relen;
    index = my_getstrindex((char *)buf, "+NWBTBLEPWR:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        if (relen > 13)
        {
            ble_info.ble_power_Respon = 1;
            ble_info.ble_power = rebuf[13] - '0';
            LogPrintf(DEBUG_ALL, "蓝牙：%s\n", ble_info.ble_power ? "开" : "关");
            N58_ClearCmd();
        }
    }
}
void n58NwbleroleParser(uint8_t *buf, uint16_t len)
{
    int index;
    uint8_t *rebuf;
    uint16_t  relen;
    index = my_getstrindex((char *)buf, "+NWBLEROLE:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        if (relen > 12)
        {
            ble_info.ble_role_Respon = 1;
            ble_info.ble_role = rebuf[12] - '0';
            LogPrintf(DEBUG_ALL, "蓝牙：%s\n", ble_info.ble_role ? "Master" : "Slaver");
            N58_ClearCmd();
        }
    }
}


//+NWURCBLESTAT: 0
void n58NwurcblestatParser(uint8_t *buf, uint16_t len)
{
    int index;
    uint8_t *rebuf;
    uint16_t  relen;
    index = my_getstrindex((char *)buf, "+NWURCBLESTAT:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        if (relen >= 15)
        {
            ble_info.ble_conncted = rebuf[15] - '0';
            LogPrintf(DEBUG_ALL, "蓝牙：%s\n", ble_info.ble_conncted ? "已连接" : "已断开");
        }
    }
}

/*
+NWBLECCON:
1,52:A6:4D:24:10:18
OK
*/
void n58NwblecconParser(uint8_t *buf, uint16_t len)
{
    int index;
    uint8_t *rebuf;
    uint16_t  relen;
    index = my_getstrindex((char *)buf, "+NWBLECCON:", len);
    if (index >= 0)
    {
        rebuf = buf + index;
        relen = len - index;
        ble_info.ble_ccon_Respon = 1;
        if (my_getstrindex((char *)rebuf, (char *)ble_info.ble_conmac, relen))
        {
            ble_info.ble_conncted = 1;
            LogMessage(DEBUG_ALL, "蓝牙：已连接\n");
        }
    }
}

void moduleResponParaser(uint8_t *buf, uint16_t len)
{
    uint8_t n58DataRestore[USART2_RX_BUF_SIZE];
    memcpy(n58DataRestore, buf, len);
    n58DataRestore[len] = NULL;
    LogMessage(DEBUG_ALL, "--->>>---\n");
    LogMessageWL(DEBUG_FACTORY, (char *)n58DataRestore, len);
    LogMessage(DEBUG_ALL, "---<<<---\n");
    /*不区分指令区域*/
    n58MygpsposParaser(n58DataRestore, len);
    n58TCPRecvparase(n58DataRestore, len);
    n58TCPsetupparase(n58DataRestore, len);
    n58TCPcloseparase(n58DataRestore, len);
    n58CMTIparase(n58DataRestore, len);
    n58CMGRparse(n58DataRestore, len);
    n58WIFISCANParse(n58DataRestore, len);
    n58AutoATA(n58DataRestore, len);
    n58MylacidParse(n58DataRestore, len);
    appBlePRecv(n58DataRestore, len);
    appBleCRecv(n58DataRestore, len);
    n58NwurcblestatParser(n58DataRestore, len);
    n58NwblecconParser(n58DataRestore, len);
    /*区分指令区域*/
    switch (n58_lte_status.current_cmd)
    {
        case N58_AT_CMD:
            if (distinguishOK((char *)n58DataRestore))
            {
                n58_lte_status.at_respon = 1;
                N58_ClearCmd();
            }
            break;
        case N58_CPIN_CMD:
            if (my_strstr((char *)n58DataRestore, "+CPIN: READY", len))
            {
                n58_lte_status.cpin_respon = 1;
                N58_ClearCmd();
            }
            break;
        case N58_CGSN_CMD:
            n58CGSNParase(n58DataRestore, len);
            break;
        case N58_CIMI_CMD:
            n58CIMIParase(n58DataRestore, len);
            break;
        case N58_CCID_CMD:
            n58CCIDParase(n58DataRestore, len);
            break;
        case N58_CSQ_CMD:
            n58CSQparase(n58DataRestore, len);
            break;
        case N58_CREG_CMD:
            n58CREGparase(n58DataRestore, len);
            break;
        case N58_CGATT_CMD:
            n58cgattparase(n58DataRestore, len);
            break;
        case N58_MYSYSINFO_CMD:
            n58mysysteminfoparase(n58DataRestore, len);
            break;
        case N58_CGDCONT_CMD:
            n58cgdcontparase(n58DataRestore, len);
            break;
        case N58_XGAUTH_CMD:
            n58xgauthparase(n58DataRestore, len);
            break;
        case N58_XIIC_CMD:
            n58xiicparase(n58DataRestore, len);
            n58xiicqueryparase(n58DataRestore, len);
            break;
        case N58_TCPSEND_CMD:
            n58TcpsendParase(n58DataRestore, len);
            break;
        case N58_MYGPSSTATE_CMD:
            n58MygpsstateParase(n58DataRestore, len);
            break;
        case N58_FSLIST_CMD:
            n58FSLISTparase(n58DataRestore, len);
            break;
        case N58_FSRF_CMD:
            n58FswrParase(n58DataRestore, len);
            break;
        case N58_NWBTBLEPWR_CMD:
            n58NwbtblepwrParser(n58DataRestore, len);
            break;
        case N58_NWBTBLENAME_CMD:
            if (distinguishOK((char *)n58DataRestore))
            {
                ble_info.ble_name_cfg_Respon = 1;
                N58_ClearCmd();
            }
            break;
        case N58_NWBLEROLE_CMD:
            n58NwbleroleParser(n58DataRestore, len);
            break;
    }
}

/*****************************************************/
uint8_t getModuleRssi(void)
{
    return n58_lte_status.csq_rssi;
}
uint8_t *getModuleIMSI(void)
{
    return n58_lte_status.IMSI;
}
uint8_t *getModuleICCID(void)
{
    return n58_lte_status.ICCID;
}

uint8_t *getModuleIMEI(void)
{
    return n58_lte_status.IMEI;
}

uint8_t isModuleRunNormal(void)
{
    if (n58_invoke_status.invoke_status == N58_NORMAL_STATUS)
        return 1;
    return 0;
}

void csqRequest(void)
{
    sendModuleCmd(N58_CSQ_CMD, NULL);
}

void sendMessage(uint8_t *buf, uint16_t len, char *telnum)
{
    char param[60];
    sprintf(param, "\"%s\"", telnum);
    sendModuleCmd(N58_CMGF_CMD, "1");
    sendModuleCmd(N58_CMGS_CMD, param);
    buf[len] = 0x1A;
    CreateNodeCmd((char *)buf, len + 1, N58_MAX_NUM);
    LogMessage(DEBUG_ALL, "\n");
}

void deleteMessage(void)
{
    sendModuleCmd(N58_CMGD_CMD, "0,4");
}

void openModuleGPS(void)
{
    sendModuleCmd(N58_MYGPSPWR_CMD, "1");
    sendModuleCmd(N58_MYGPSPOS_CMD, "6,1");
}
void closeModuleGPS(void)
{
    sendModuleCmd(N58_MYGPSPWR_CMD, "0");
}
