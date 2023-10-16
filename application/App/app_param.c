#include "app_param.h"
#include "main.h"
#include "app_sys.h"
SYSTEM_FLASH_DEFINE sysparam;

char CODEVERSION[30];

/**
  * @brief  读取EEprom 中一个字节的数据
  * @param  None
  * @retval None
  */
static uint8_t eepromReadOneByte(uint32_t address)
{
    return *(__I uint8_t *)(EEPROM_BASE_ADDRESS + address);
}
/**
  * @brief  读取EEprom 中两个字节的数据
  * @param  None
  * @retval None
  */
static uint16_t eepromReadTwoBytes(uint32_t address)
{
    return *(__I uint16_t *)(EEPROM_BASE_ADDRESS + address);
}

/**
  * @brief  读取EEprom 中四个字节的数据
  * @param  None
  * @retval None
  */

static uint32_t eepromReadWord(uint32_t address)
{
    return *(__I uint32_t *)(EEPROM_BASE_ADDRESS + address);
}

/**
  * @brief  往EEPROM中写入一个字节数据
  * @param  None
  * @retval None
  */
static void eepromWriteByte(uint32_t addr, uint32_t data)
{
    uint8_t i;
    __disable_irq();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    for (i = 0; i < 20; i++)
    {
        if (HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EEPROM_BASE_ADDRESS + addr, data) == HAL_OK)
        {
            break;
        }
    }
    HAL_FLASHEx_DATAEEPROM_Lock();
    __enable_irq();
}
/**
  * @brief  往EEPROM中写入两个字节数据
  * @param  None
  * @retval None
  */
static void eepromWriteTwoBytes(uint32_t addr, uint32_t data)
{
    uint8_t i;
    __disable_irq();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    for (i = 0; i < 20; i++)
    {
        if (HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_HALFWORD, EEPROM_BASE_ADDRESS + addr, data) == HAL_OK)
        {
            break;
        }
    }
    HAL_FLASHEx_DATAEEPROM_Lock();
    __enable_irq();
}
/**
  * @brief  往EEPROM中写入四个字节数据
  * @param  None
  * @retval None
  */
static void eepromWriteWord(uint32_t addr, uint32_t data)
{
    uint8_t i;
    __disable_irq();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    for (i = 0; i < 20; i++)
    {
        if (HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, EEPROM_BASE_ADDRESS + addr, data) == HAL_OK)
        {
            break;
        }
    }
    HAL_FLASHEx_DATAEEPROM_Lock();
    __enable_irq();
}


/**
  * @brief  复位原始值EEPROM
  * @param  None
  * @retval None
  */
void paramDefaultInit(uint8_t level)
{
    uint8_t i;

    if (level == 0)
    {
        paramSaveSnNumber((uint8_t *)"888888887777777", 15);
        paramSaveServer((uint8_t *)"jzwz.basegps.com", 9998);
        paramSaveApnName((uint8_t *)"cmnet");
        paramSaveApnUser((uint8_t *)"");
        paramSaveApnPswd((uint8_t *)"");

        sysparam.MODE = 1;
        eepromWriteByte(EEPROM_MODE_ADDR, sysparam.MODE);

        sysparam.MODE1_GAP_DAY = 1;
        eepromWriteByte(EEPROM_MODE1_GAP_ADDR, sysparam.MODE1_GAP_DAY);

        sysparam.gapMinutes = 0;
        eepromWriteTwoBytes(EEPROM_MODE3_GAP_ADDR, sysparam.gapMinutes);

        sysparam.utc = 8;
        paramSaveUTC(sysparam.utc);

        sysparam.AlarmTime[0] = 720;
        for (i = 1; i < 5; i++)
        {
            sysparam.AlarmTime[i] = 0XFFFF;
            eepromWriteTwoBytes(EEPROM_ALARMTIEM_ADDR + (2 * i), sysparam.AlarmTime[i]);
        }
        paramSaveAdcCal(2.860535);
    }
    sysparam.VERSION = EEPROM_VER;
    sysparam.Light_Alarm_En = 1;
    sysparam.Switch_Alarm_En = 1;
    sysparam.heartbeatgap = 180;
    sysparam.startUpCnt = 0;
    sysparam.gpsuploadgap = 30;
   	sysparam.gapMinutes = 0;
    sysparam.ledctrl = 0;
    sysparam.poitype = 2;
    sysparam.smsRespon = 0;
    sysparam.hardfault = 0;
    sysparam.mallocfault = 0;
    strcpy((char *)sysparam.agpsServer, "121.41.40.95");
    strcpy((char *)sysparam.agpsUser, "isimact@189.cn");
    strcpy((char *)sysparam.agpsPswd, "tinfo_13310886056");
    sysparam.agpsPort = 2621;
    paramSaveServer((uint8_t *)"39.105.31.47", 7700);
    paramSaveApnName((uint8_t *)"cmiot");
    sysparam.step = 0;
    paramSaveStep();
    sysparam.vibRange = 0;
    paramSaveVibrange();
    sysparam.staticTime = 0;
    paramSaveStaticTime();

    paramSaveAgpsServer();
    paramSaveAgpsPort();
    paramSaveAgpsUser();
    paramSaveAgpsPswd();

    paramSaveSmsreply(sysparam.smsRespon);

    eepromWriteByte(EEPROM_POITYPE_ADDR, sysparam.poitype);
    eepromWriteByte(EEPROM_LED_ADDR, sysparam.ledctrl);
    eepromWriteByte(EEPROM_LEN_ADDR, sysparam.Light_Alarm_En);
    eepromWriteByte(EEPROM_SEN_ADDR, sysparam.Switch_Alarm_En);
    eepromWriteByte(EEPROM_SLEEP_ADDR, sysparam.SLEEP);
    paramSaveHeartbeatInterval(sysparam.heartbeatgap);
    paramSaveFence(30);
    paramSaveAutoAnswer(0);
    paramSaveTurnalg(0);
    paramSaveNoNetWakeUpMinutes(60);
    paramSaveHardFault();
    paramSaveMallocFault();
    //2个字节区域
    paramSaveGPSUploadInterval(sysparam.gpsuploadgap);
    paramSaveACCCTLGNSS(1);
    paramSavePdop(600);
    paramSaveMode1Timer(0);
    paramSaveMode2cnt(0);
	eepromWriteTwoBytes(EEPROM_MODE3_GAP_ADDR, sysparam.gapMinutes);

    paramSaveLoww(36);
    paramSaveBF(0);

    paramSaveAutoParam(PARAM_AUTO_UPDATE_FLAG);
    eepromWriteByte(EEPROM_VERSION_APP_ADDR, sysparam.VERSION);
}

/**
  * @brief  初始化EEPROM
  * @param  None
  * @retval None
  */
void paramInit(void)
{
    uint8_t i;
    sysparam.VERSION = eepromReadOneByte(EEPROM_VERSION_APP_ADDR);
    if (sysparam.VERSION != EEPROM_VER)
    {
        paramDefaultInit(0);
    }
    sysparam.poitype = eepromReadOneByte(EEPROM_POITYPE_ADDR);
    sysparam.ledctrl = eepromReadOneByte(EEPROM_LED_ADDR);
    sysparam.MODE = eepromReadOneByte(EEPROM_MODE_ADDR);
    sysparam.Light_Alarm_En = eepromReadOneByte(EEPROM_LEN_ADDR);
    sysparam.Switch_Alarm_En = eepromReadOneByte(EEPROM_SEN_ADDR);
    sysparam.MODE1_GAP_DAY = eepromReadOneByte(EEPROM_MODE1_GAP_ADDR);
    sysparam.SLEEP = eepromReadOneByte(EEPROM_SLEEP_ADDR);
    sysparam.gapMinutes = eepromReadTwoBytes(EEPROM_MODE3_GAP_ADDR);
    sysparam.bf = eepromReadOneByte(EEPROM_BF_ADDR);
    sysparam.pdop = eepromReadTwoBytes(EEPROM_PDOP_ADDR);
    sysparam.startUpCnt = eepromReadTwoBytes(EEPROM_MODE1TIMER_ADDR);
    sysparam.utc = (int8_t)eepromReadOneByte(EEPROM_UTC_ADDR);
    sysparam.accctlgnss = (int8_t)eepromReadOneByte(EEPROM_ACCCTLGNSS_ADDR);
    sysparam.fence = eepromReadOneByte(EEPROM_FENCE_ADDR);
    sysparam.smsRespon = eepromReadOneByte(EEPROM_SMSREPLY_ADDR);
    sysparam.adccal = paramGetAdcCal();
    sysparam.autoAnswer = eepromReadOneByte(EEPROM_AUTOANSWER_ADDR);
    sysparam.turnalg = eepromReadOneByte(EEPROM_TURNALG_ADDR);
    sysparam.noNetWakeUpMinutes = eepromReadOneByte(EEPROM_NONETWAKEUP_ADDR);
    sysparam.hardfault = eepromReadOneByte(EEPROM_HEADFAULT_ADDR);
    sysparam.mallocfault = eepromReadOneByte(EEPROM_MALLOC_ADDR);
    for (i = 0; i < 5; i++)
    {
        sysparam.AlarmTime[i] = eepromReadTwoBytes(EEPROM_ALARMTIEM_ADDR + (2 * i));
    }

    paramGetSnNumber(sysparam.SN, 15);
    paramGetBleMac(sysparam.blemac);
    paramGetServer(sysparam.Server, &sysparam.ServerPort);
    paramGetGPSUploadInterval(&sysparam.gpsuploadgap);
    paramGetHeartbeatInterval(&sysparam.heartbeatgap);
    paramGetApnName(sysparam.apn);
    paramGetApnUser(sysparam.apnuser);
    paramGetApnPswd(sysparam.apnpassword);
    paramGetCodeVersion((uint8_t *)CODEVERSION);
    paramGetMode2cnt();
    if (strstr(CODEVERSION, EEPROM_VERSION) == NULL)
    {
        paramSaveCodeVersion((uint8_t *)EEPROM_VERSION);
        paramGetCodeVersion((uint8_t *)CODEVERSION);
    }
    paramGetLoww(&sysparam.lowvoltage);
    sysinfo.lowvoltage = sysparam.lowvoltage / 10.0;

    if (sysparam.heartbeatgap == 0)
    {
        sysparam.heartbeatgap = 180;
        paramSaveHeartbeatInterval(180);
    }
    paramGetAgpsServer();
    paramGetAgpsPort();
    paramGetAgpsUser();
    paramGetAgpsPswd();
    paramGetAutoParam();
    paramGetStep();
    paramGetVibrange();
    paramGetStaticTime();
//    if (sysparam.staticTime == 0)
//    {
//        sysparam.staticTime = 5;
//    }
    /*--------------------------------------------------*/
    //远程升级时，参数自动更新
    if (sysparam.autoParamUpdate != PARAM_AUTO_UPDATE_FLAG)
    {
        paramSaveAutoParam(PARAM_AUTO_UPDATE_FLAG);
        strcpy((char *)sysparam.agpsServer, "121.41.40.95");
        strcpy((char *)sysparam.agpsUser, "isimact@189.cn");
        strcpy((char *)sysparam.agpsPswd, "tinfo_13310886056");
        sysparam.agpsPort = 2621;

        paramSaveAgpsServer();
        paramSaveAgpsPort();
        paramSaveAgpsUser();
        paramSaveAgpsPswd();

		sysparam.vibRange=0;
		sysparam.staticTime=0;
		paramSaveVibrange();
		paramSaveStaticTime();

		sysparam.heartbeatgap=180;
		paramSaveHeartbeatInterval(sysparam.heartbeatgap);

		sysparam.smsRespon = 0;
		sysparam.MODE = 2;
		sysparam.gpsuploadgap = 30;
   		sysparam.gapMinutes = 0;
   		eepromWriteByte(EEPROM_MODE_ADDR, sysparam.MODE);
   		eepromWriteTwoBytes(EEPROM_MODE3_GAP_ADDR, sysparam.gapMinutes);
   		paramSaveGPSUploadInterval(sysparam.gpsuploadgap);
		paramSaveServer((uint8_t *)"39.105.31.47", 7700);
    	paramSaveApnName((uint8_t *)"cmiot");
    	paramSaveSmsreply(sysparam.smsRespon);
    	paramGetServer(sysparam.Server, &sysparam.ServerPort);
    	paramGetApnName(sysparam.apn);
    }
}


void paramSaveMode(uint8_t mode)
{
    sysparam.MODE = mode;
    eepromWriteByte(EEPROM_MODE_ADDR, sysparam.MODE);
}

void paramSaveMode1GapDay(uint8_t day)
{
    sysparam.MODE1_GAP_DAY = day;
    eepromWriteByte(EEPROM_MODE1_GAP_ADDR, sysparam.MODE1_GAP_DAY);
}

void paramSaveAlarmTime(void)
{
    uint8_t i;
    for (i = 0; i < 5; i++)
    {
        eepromWriteTwoBytes(EEPROM_ALARMTIEM_ADDR + (2 * i), sysparam.AlarmTime[i]);
    }
}
void paramSaveInterval(void)
{
    eepromWriteTwoBytes(EEPROM_MODE3_GAP_ADDR, sysparam.gapMinutes);
}
void paramSaveSleepState(void)
{
    eepromWriteByte(EEPROM_SLEEP_ADDR, sysparam.SLEEP);
}

void paramSaveSnNumber(uint8_t *sn, uint16_t len)
{
    uint8_t i;
    for (i = 0; i < len; i++)
    {
        eepromWriteByte(EEPROM_SN_ADDR + i, sn[i]);
    }
}
void paramGetSnNumber(uint8_t *sn, uint16_t len)
{
    uint8_t i;
    for (i = 0; i < len; i++)
    {
        sn[i] = eepromReadOneByte(EEPROM_SN_ADDR + i);
    }
}
void paramSaveServer(uint8_t *server, uint32_t port)
{
    uint8_t i, serverlen;
    uint8_t newserver[50];
    memset(newserver, 0, 50);
    serverlen = strlen((const char *)server);
    if (serverlen > 50)
        return;
    strncpy((char *)newserver, (char *)server, serverlen);
    for (i = 0; i < 50; i++)
    {
        eepromWriteByte(EEPROM_DOMAIN_ADDR + i, newserver[i]);
    }
    eepromWriteWord(EEPROM_DOMAIN_PORT_ADDR, port);
}
void paramGetServer(uint8_t *server, uint32_t *port)
{
    uint8_t i;
    for (i = 0; i < 50; i++)
    {
        server[i] = eepromReadOneByte(EEPROM_DOMAIN_ADDR + i);
    }
    *port = eepromReadWord(EEPROM_DOMAIN_PORT_ADDR);
}

void paramSaveHeartbeatInterval(uint16_t hbt)
{
    eepromWriteTwoBytes(EEPROM_HEARTBEATTIME_ADDR, hbt);
}

void paramGetHeartbeatInterval(uint16_t *hbt)
{
    *hbt = eepromReadTwoBytes(EEPROM_HEARTBEATTIME_ADDR);
}

void paramSaveGPSUploadInterval(uint16_t gpsupload)
{
    eepromWriteTwoBytes(EEPROM_GPS_INTERVAL_ADDR, gpsupload);
}
void paramGetGPSUploadInterval(uint16_t *data)
{
    *data = eepromReadTwoBytes(EEPROM_GPS_INTERVAL_ADDR);
}

void paramSaveApnName(uint8_t *apnname)
{
    uint8_t i;
    uint8_t nbuf[50];
    memset(nbuf, 0, 50);
    if (apnname != NULL)
    {
        strncpy((char *)nbuf, (char *)apnname, strlen((char *)apnname));
    }
    for (i = 0; i < 50; i++)
    {
        eepromWriteByte(EEPROM_APN_NAME_ADDR + i, nbuf[i]);
    }
}
void paramGetApnName(uint8_t *apnname)
{
    uint8_t i;
    for (i = 0; i < 50; i++)
    {
        apnname[i] = eepromReadOneByte(EEPROM_APN_NAME_ADDR + i);
    }
}

void paramSaveApnUser(uint8_t *apnuser)
{
    uint8_t i;
    uint8_t nbuf[50];
    memset(nbuf, 0, 50);
    if (apnuser != NULL)
    {
        strncpy((char *)nbuf, (char *)apnuser, strlen((char *)apnuser));
    }
    for (i = 0; i < 50; i++)
    {
        eepromWriteByte(EEPROM_APN_USER_ADDR + i, nbuf[i]);
    }
}
void paramGetApnUser(uint8_t *apnuser)
{
    uint8_t i;
    for (i = 0; i < 50; i++)
    {
        apnuser[i] = eepromReadOneByte(EEPROM_APN_USER_ADDR + i);
    }
}


void paramSaveApnPswd(uint8_t *apnPSWD)
{
    uint8_t i;
    uint8_t nbuf[50];
    memset(nbuf, 0, 50);
    if (apnPSWD != NULL)
    {
        strncpy((char *)nbuf, (char *)apnPSWD, strlen((char *)apnPSWD));
    }
    for (i = 0; i < 50; i++)
    {
        eepromWriteByte(EEPROM_APN_PSWD_ADDR + i, nbuf[i]);
    }
}
void paramGetApnPswd(uint8_t *apnPSWD)
{
    uint8_t i;
    for (i = 0; i < 50; i++)
    {
        apnPSWD[i] = eepromReadOneByte(EEPROM_APN_PSWD_ADDR + i);
    }
}

void paramSaveUpdateStatus(uint8_t status)
{
    sysparam.updateStatus = status;
    eepromWriteByte(EEPROM_UPDATE_FLAG_ADDR, sysparam.updateStatus);
}

void paramGetUpdateStatus(uint8_t *status)
{
    *status = eepromReadOneByte(EEPROM_UPDATE_FLAG_ADDR);
}


void paramSaveUPSServer(uint8_t *server, uint16_t port)
{
    uint8_t i, serverlen;
    uint8_t newserver[50];
    memset(newserver, 0, 50);
    serverlen = strlen((const char *)server);
    if (serverlen > 50)
        return;
    strncpy((char *)newserver, (char *)server, serverlen);
    for (i = 0; i < 50; i++)
    {
        eepromWriteByte(EEPROM_UPDATEDOMAIN_ADDR + i, newserver[i]);
    }
    eepromWriteTwoBytes(EEPROM_UPDATEDOMAIN_PORT_ADDR, port);
}
void paramGetUPSServer(uint8_t *server, uint16_t *port)
{
    uint8_t i;
    for (i = 0; i < 50; i++)
    {
        server[i] = eepromReadOneByte(EEPROM_UPDATEDOMAIN_ADDR + i);
    }
    *port = eepromReadWord(EEPROM_UPDATEDOMAIN_PORT_ADDR);
}

void paramSaveCodeVersion(uint8_t *ver)
{
    uint8_t i;
    uint8_t version[30];
    strcpy((char *)version, (char *)ver);
    for (i = 0; i < 30; i++)
    {
        eepromWriteByte(EEPROM_CODEVERSION_ADDR + i, version[i]);
    }
}
void paramGetCodeVersion(uint8_t *ver)
{
    uint8_t i;
    for (i = 0; i < 30; i++)
    {
        ver[i] = eepromReadOneByte(EEPROM_CODEVERSION_ADDR + i);
    }
}

void paramSaveLoww(uint8_t loww)
{
    eepromWriteByte(EEPROM_LOWW_ADDR, loww);
}


void paramSaveLedCtl(uint8_t LED)
{
    sysparam.ledctrl = LED;
    eepromWriteByte(EEPROM_LED_ADDR, LED);
}


void paramSavePoitype(uint8_t value)
{
    sysparam.poitype = value;
    eepromWriteByte(EEPROM_POITYPE_ADDR, sysparam.poitype);
}


void paramGetLoww(uint8_t *loww)
{
    *loww = eepromReadOneByte(EEPROM_LOWW_ADDR);
}


void paramSaveMode1Timer(uint16_t count)
{
    sysparam.startUpCnt = count;
    eepromWriteTwoBytes(EEPROM_MODE1TIMER_ADDR, sysparam.startUpCnt);
}

void paramSaveUTC(uint8_t UTC)
{
    sysparam.utc = UTC;
    eepromWriteByte(EEPROM_UTC_ADDR, sysparam.utc);
}

void paramSaveLightAlarmMode(uint8_t onoff)
{
    sysparam.Light_Alarm_En = onoff;
    eepromWriteByte(EEPROM_LEN_ADDR, sysparam.Light_Alarm_En);
}


void paramSaveACCCTLGNSS(uint8_t onoff)
{
    sysparam.accctlgnss = onoff;
    eepromWriteByte(EEPROM_ACCCTLGNSS_ADDR, sysparam.accctlgnss);
}

void paramSavePdop(uint16_t pdop)
{
    sysparam.pdop = pdop;
    eepromWriteTwoBytes(EEPROM_PDOP_ADDR, pdop);
}
//float 4byte
void paramSaveAdcCal(float cal)
{
    uint32_t value = cal * 100000;
    eepromWriteWord(EEPROM_ADCCAL_ADDR, value);
}
float paramGetAdcCal(void)
{
    return (float)(eepromReadWord(EEPROM_ADCCAL_ADDR) / 100000.0);
}


void paramSaveBleMac(uint8_t *mac)
{
    uint8_t i;
    uint8_t nbuf[19];
    memset(nbuf, 0, 19);
    if (mac != NULL)
    {
        strncpy((char *)nbuf, (char *)mac, strlen((char *)mac));
    }
    for (i = 0; i < 19; i++)
    {
        eepromWriteByte(EEPROM_BLEMAC_ADDR + i, nbuf[i]);
    }
}
void paramGetBleMac(uint8_t *mac)
{
    uint8_t i;
    for (i = 0; i < 19; i++)
    {
        mac[i] = eepromReadOneByte(EEPROM_BLEMAC_ADDR + i);
    }
}

void paramSaveMode2cnt(uint32_t cnt)
{
    sysparam.runTime = cnt;
    eepromWriteTwoBytes(EEPROM_mode2worktime_ADDR, sysparam.runTime);
}

void paramGetMode2cnt(void)
{
    sysparam.runTime = eepromReadTwoBytes(EEPROM_mode2worktime_ADDR);
}

void paramSaveBF(uint8_t onoff)
{
    sysparam.bf = onoff;
    eepromWriteByte(EEPROM_BF_ADDR, onoff);
}

void paramSaveFence(uint8_t distance)
{
    sysparam.fence = distance;
    eepromWriteByte(EEPROM_FENCE_ADDR, distance);
}

void paramSaveCM(uint8_t value)
{
    eepromWriteByte(EEPROM_CM_ADDR, value);
}

uint8_t paramGetCM(void)
{
    return eepromReadOneByte(EEPROM_CM_ADDR);
}

void paramSaveSmsreply(uint8_t value)
{
    eepromWriteByte(EEPROM_SMSREPLY_ADDR, value);
}
void paramSaveAutoAnswer(uint8_t onoff)
{
    sysparam.autoAnswer = onoff;
    eepromWriteByte(EEPROM_AUTOANSWER_ADDR, onoff);
}
void paramSaveTurnalg(uint8_t onoff)
{
    sysparam.turnalg = onoff;
    eepromWriteByte(EEPROM_TURNALG_ADDR, onoff);
}

void paramSaveNoNetWakeUpMinutes(uint8_t mis)
{
    sysparam.noNetWakeUpMinutes = mis;
    eepromWriteByte(EEPROM_NONETWAKEUP_ADDR, mis);
}

void paramSaveHardFault(void)
{
    eepromWriteByte(EEPROM_HEADFAULT_ADDR, sysparam.hardfault);
}
void paramSaveMallocFault(void)
{
    eepromWriteByte(EEPROM_MALLOC_ADDR, sysparam.mallocfault);
}

void paramSaveAgpsServer(void)
{
    uint8_t i;
    for (i = 0; i < 50; i++)
    {
        eepromWriteByte(EEPROM_AGPSSERVER_ADDR + i, sysparam.agpsServer[i]);
    }
}
void paramGetAgpsServer(void)
{
    uint8_t i;
    for (i = 0; i < 50; i++)
    {
        sysparam.agpsServer[i] = eepromReadOneByte(EEPROM_AGPSSERVER_ADDR + i);
    }
}

void paramSaveAgpsUser(void)
{
    uint8_t i;
    for (i = 0; i < 50; i++)
    {
        eepromWriteByte(EEPROM_AGPSUSER_ADDR + i, sysparam.agpsUser[i]);
    }
}
void paramGetAgpsUser(void)
{
    uint8_t i;
    for (i = 0; i < 50; i++)
    {
        sysparam.agpsUser[i] = eepromReadOneByte(EEPROM_AGPSUSER_ADDR + i);
    }
}


void paramSaveAgpsPswd(void)
{
    uint8_t i;
    for (i = 0; i < 50; i++)
    {
        eepromWriteByte(EEPROM_AGPSPSWD_ADDR + i, sysparam.agpsPswd[i]);
    }
}

void paramGetAgpsPswd(void)
{
    uint8_t i;
    for (i = 0; i < 50; i++)
    {
        sysparam.agpsPswd[i] = eepromReadOneByte(EEPROM_AGPSPSWD_ADDR + i);
    }
}

void paramSaveAgpsPort(void)
{
    eepromWriteTwoBytes(EEPROM_AGPSPORT_ADDR, sysparam.agpsPort);
}
void paramGetAgpsPort(void)
{
    sysparam.agpsPort = eepromReadTwoBytes(EEPROM_AGPSPORT_ADDR);
}


void paramSaveAutoParam(uint8_t flag)
{
    eepromWriteByte(EEPROM_PARAM_AUTOUPDATE_ADDR, flag);
}

void paramGetAutoParam(void)
{
    sysparam.autoParamUpdate = eepromReadOneByte(EEPROM_PARAM_AUTOUPDATE_ADDR);
}

void paramSaveStep(void)
{
    eepromWriteTwoBytes(EEPROM_STEP_ADDR, sysparam.step);
}
void paramGetStep(void)
{
    sysparam.step = eepromReadTwoBytes(EEPROM_STEP_ADDR);
}

void paramSaveVibrange(void)
{
    eepromWriteByte(EEPROM_VIBRANGE_ADDR, sysparam.vibRange);
}
void paramGetVibrange(void)
{
    sysparam.vibRange = eepromReadOneByte(EEPROM_VIBRANGE_ADDR);
}

void paramSaveStaticTime(void)
{
    eepromWriteByte(EEPROM_STATICTIME_ADDR, sysparam.staticTime);
}
void paramGetStaticTime(void)
{
    sysparam.staticTime = eepromReadOneByte(EEPROM_STATICTIME_ADDR);
}


