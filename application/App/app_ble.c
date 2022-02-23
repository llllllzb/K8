#include "app_ble.h"
#include "app_net.h"
#include "app_sys.h"
#include "app_port.h"
#include "app_param.h"
#include "app_atcmd.h"
BLE_INFO ble_info;


void appBleInfoReset(void)
{
    memset(&ble_info, 0, sizeof(BLE_INFO));
}

void appBleChangeFsm(uint8_t fsm)
{
    ble_info.ble_fsm = fsm;
    ble_info.ble_tick = 0;
}

uint8_t appBleIsLinkNormal(void)
{
    if (ble_info.ble_fsm == BLE_FSM_SLAVER)
        return 1;
    return 0;
}

#if 1
/*配置为从机模式*/
void appBleTask(void)
{
    char param[20];
    if (netWorkModuleRunOk() == 0)
        return ;
    switch (ble_info.ble_fsm)
    {
        case BLE_FSM_POWER_CHK:
            if (ble_info.ble_power_Respon == 1)
            {
                ble_info.ble_power_Respon = 0;
                if (ble_info.ble_power == 1)
                {
                    //已开机
                    appBleChangeFsm(BLE_FSM_SLAVER);
                }
                else
                {
                    //未开机
                    appBleChangeFsm(BLE_FSM_CFG);
                }
            }
            else
            {

                if (ble_info.ble_tick % 2 == 0)
                {
                    ble_info.ble_power_Respon = 0;
                    sendModuleCmd(N58_NWBTBLEPWR_CMD, "?");
                }
            }
            break;
        case BLE_FSM_ROLE_CHK:
            if (ble_info.ble_role_Respon)
            {
                ble_info.ble_role_Respon = 0;
                if (ble_info.ble_role == 1)
                {
                    //主模式
                    appBleChangeFsm(BLE_FSM_POWEROFF);
                }
                else
                {
                    //从模式
                    appBleChangeFsm(BLE_FSM_POERON);
                }
            }
            else
            {
                if (ble_info.ble_tick % 2 == 0)
                {
                    ble_info.ble_role_Respon = 0;
                    sendModuleCmd(N58_NWBLEROLE_CMD, "?");
                }
            }
            break;
        case BLE_FSM_SLAVER:
            //LogMessage(DEBUG_ALL,"SLAVER MODE\n");
            break;
        case BLE_FSM_POERON:
            if (ble_info.ble_tick == 1)
            {
                sendModuleCmd(N58_NWBTBLEPWR_CMD, "1");
            }
            if (ble_info.ble_tick >= 3)
            {
                appBleChangeFsm(BLE_FSM_POWER_CHK);
            }
            break;
        case BLE_FSM_POWEROFF:
            if (ble_info.ble_tick == 1)
            {
                sendModuleCmd(N58_NWBTBLEPWR_CMD, "0");
            }
            if (ble_info.ble_tick >= 3)
            {
                appBleChangeFsm(BLE_FSM_POWER_CHK);
            }
            break;
        case BLE_FSM_CFG:
            if (ble_info.ble_name_cfg_Respon == 1)
            {
                ble_info.ble_name_cfg_Respon = 0;
                sendModuleCmd(N58_NWBLEROLE_CMD, "0");
                appBleChangeFsm(BLE_FSM_ROLE_CHK);
            }
            else
            {
                if (ble_info.ble_tick % 2 == 0)
                {
                    ble_info.ble_name_cfg_Respon = 0;
                    sprintf(param, "\"%s\"", sysparam.SN);
                    sendModuleCmd(N58_NWBTBLENAME_CMD, param);
                }
            }
            break;
        default:
            ble_info.ble_fsm = BLE_FSM_POWER_CHK;
            break;
    }
    ble_info.ble_tick++;
}


#else

static uint8_t appIfBleOn(void)
{
    if (strlen((char *)ble_info.ble_conmac) == 17)
        return 1;
    return 0;
}
/*配置为主机模式*/
void appBleTask(void)
{
    char param[20];
    static uint8_t reConnectCount;
    if (appIfBleOn() == 0)
        return ;
    if (netWorkModuleRunOk() == 0)
        return ;
    switch (ble_info.ble_fsm)
    {
        case BLE_FSM_POWER_CHK:
            if (ble_info.ble_power_Respon == 1)
            {
                ble_info.ble_power_Respon = 0;
                if (ble_info.ble_power == 1)
                {
                    //已开机
                    appBleChangeFsm(BLE_FSM_MASTER);
                    reConnectCount = 0;
                    strcpy((char *)ble_info.ble_conmac, "52:A6:4D:24:10:18");
                }
                else
                {
                    //未开机
                    appBleChangeFsm(BLE_FSM_CFG);
                }
            }
            else
            {

                if (ble_info.ble_tick % 2 == 0)
                {
                    ble_info.ble_power_Respon = 0;
                    sendModuleCmd(N58_NWBTBLEPWR_CMD, "?");
                }
            }
            break;
        case BLE_FSM_ROLE_CHK:
            if (ble_info.ble_role_Respon)
            {
                ble_info.ble_role_Respon = 0;
                if (ble_info.ble_role == 1)
                {
                    //主模式
                    appBleChangeFsm(BLE_FSM_POERON);
                }
                else
                {
                    //从模式
                    appBleChangeFsm(BLE_FSM_POWEROFF);
                }
            }
            else
            {
                if (ble_info.ble_tick % 2 == 0)
                {
                    ble_info.ble_role_Respon = 0;
                    sendModuleCmd(N58_NWBLEROLE_CMD, "?");
                }
            }
            break;
        case BLE_FSM_SLAVER:
            //LogMessage(DEBUG_ALL,"Slaver Mode\n");
            break;
        case BLE_FSM_MASTER: //连接对应设备
            LogPrintf(DEBUG_ALL, "Master Mode: Connecting %s\n", ble_info.ble_conmac);
            //连接超过5次，关机重来
            if (reConnectCount < 5)
            {
                sprintf(param, "0,\"%s\"", ble_info.ble_conmac);
                sendModuleCmd(N58_NWBLECCON_CMD, param);
                ble_info.ble_conncted = 0;
                appBleChangeFsm(BLE_FSM_CON_CHK);
            }
            else
            {
                appBleChangeFsm(BLE_FSM_POWEROFF);
            }
            reConnectCount++;
            break;
        case BLE_FSM_CON_CHK://连接查看

            if (ble_info.ble_conncted)
            {
                reConnectCount = 0;
                appBleChangeFsm(BLE_FSM_CONNECTED);
                break;
            }
            else if (ble_info.ble_ccon_Respon && ble_info.ble_tick > 5)
            {
                appBleChangeFsm(BLE_FSM_MASTER);
            }

            if (ble_info.ble_tick % 3 == 0)
            {
                sendModuleCmd(N58_NWBLECCON_CMD, "?");
            }
            break;
        //已连接
        case BLE_FSM_CONNECTED:
            if (ble_info.ble_conncted == 0)
            {
                //链接断开
                appBleChangeFsm(BLE_FSM_MASTER);
            }
            break;
        case BLE_FSM_POERON:
            if (ble_info.ble_tick == 1)
            {
                sendModuleCmd(N58_NWBTBLEPWR_CMD, "1");
            }
            if (ble_info.ble_tick >= 3)
            {
                appBleChangeFsm(BLE_FSM_POWER_CHK);
            }
            break;
        case BLE_FSM_POWEROFF:
            if (ble_info.ble_tick == 1)
            {
                sendModuleCmd(N58_NWBTBLEPWR_CMD, "0");
            }
            if (ble_info.ble_tick >= 3)
            {
                appBleChangeFsm(BLE_FSM_POWER_CHK);
            }
            break;
        case BLE_FSM_CFG:
            if (ble_info.ble_name_cfg_Respon == 1)
            {
                ble_info.ble_name_cfg_Respon = 0;
                //sendModuleCmd(N58_NWBLEROLE_CMD,"0");
                sendModuleCmd(N58_NWBLEROLE_CMD, "1");
                appBleChangeFsm(BLE_FSM_ROLE_CHK);
            }
            else
            {
                if (ble_info.ble_tick % 2 == 0)
                {
                    ble_info.ble_name_cfg_Respon = 0;
                    sprintf(param, "\"%s\"", sysparam.SN);
                    sendModuleCmd(N58_NWBTBLENAME_CMD, param);
                }
            }
            break;
        default:
            ble_info.ble_fsm = BLE_FSM_POWER_CHK;
            break;
    }
    ble_info.ble_tick++;
}
#endif
/*
+NWURCBLESTAT: 1,71:4A:CE:F4:B9:5A

+NWURCBLESTAT: 0

+NWBLEDISCON:
1,71:4A:CE:F4:B9:5A
OK

+NWURCBLEPRECV: 0,0,4,你好

*/

void appBlePRecvParser(uint8_t *buf, uint16_t len)
{
    int16_t index;
    uint8_t *rebuf;
    int16_t  relen;
    uint8_t   blerxlen;
    char content[30];
    rebuf = buf;
    relen = len;
    index = my_getstrindex((char *)rebuf, "+NWURCBLEPRECV:", relen);
    while (index >= 0)
    {
        rebuf = rebuf + index;
        relen = relen - index;
        index = getCharIndexWithNum(rebuf, relen, ',', 2);
		if(index<0)
			return ;
        rebuf = rebuf + index + 1;
        relen = relen - index - 1;
        index = getCharIndex(rebuf, relen, ',');
		if(index<0)
			return ;
        rebuf[index] = 0;
        blerxlen = atoi((char *)rebuf);
        rebuf = rebuf + index + 1;
        relen = relen - index - 1;
        if (relen >= blerxlen)
        {
            strncpy(content, (char *)rebuf, blerxlen);
            content[blerxlen] = NULL;
            instructionParase((uint8_t *)content, blerxlen, BLE_MODE, NULL);
        }
        index = my_getstrindex((char *)rebuf, "+NWURCBLEPRECV:", relen);
    }

}


void appBleCRecvParser(uint8_t *buf, uint16_t len)
{
    int16_t index;
    uint8_t *rebuf;
    int16_t  relen;
    uint8_t   blerxlen;
    char content[128];
    rebuf = buf;
    relen = len;
    index = my_getstrindex((char *)rebuf, "+NWURCBLECRECV:", relen);
    while (index >= 0)
    {
        rebuf = rebuf + index;
        relen = relen - index;
        index = getCharIndexWithNum(rebuf, relen, ',', 2);
		if(index<0)
			return ;
        rebuf = rebuf + index + 1;
        relen = relen - index - 1;
        index = getCharIndex(rebuf, relen, ',');
		if(index<0)
			return ;
        rebuf[index] = 0;
        blerxlen = atoi((char *)rebuf);
        rebuf = rebuf + index + 1;
        relen = relen - index - 1;
        if (relen >= blerxlen)
        {
            strncpy(content, (char *)rebuf, blerxlen);
            content[blerxlen] = NULL;
            LogPrintf(DEBUG_ALL, "CRecv:%s\n", content);
        }
        index = my_getstrindex((char *)rebuf, "+NWURCBLECRECV:", relen);
    }

}



void appBleSendData(uint8_t *buf, uint16_t len)
{
    char param[20];
    if (ble_info.ble_conncted)
    {
        sprintf(param, "0,0,0,1,%d", len);
        sendModuleCmd(N58_NWBLEPSEND_CMD, param);
        CreateNodeCmd((char *)buf, len, N58_NWBLEPSEND_CMD);
    }
}

