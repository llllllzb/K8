#include "app_eeprom.h"
SYSTEM_FLASH_DEFINE eeprom_info;

char CODEVERSION[30];

/**
  * @brief  读取EEprom 中一个字节的数据
  * @param  None
  * @retval None
  */
static uint8_t eepromReadOneByte(uint32_t address)
{
    return *(__I uint8_t *)(EEPROM_BASE_ADDRESS+address);
}
/**
  * @brief  读取EEprom 中两个字节的数据
  * @param  None
  * @retval None
  */
static uint16_t eepromReadTwoBytes(uint32_t address)
{
    return *(__I uint16_t *)(EEPROM_BASE_ADDRESS+address);
}

/**
  * @brief  读取EEprom 中四个字节的数据
  * @param  None
  * @retval None
  */

//static uint32_t eepromReadWord(uint32_t address)
//{
//    return *(__I uint32_t *)(EEPROM_BASE_ADDRESS+address);
//}

/**
  * @brief  往EEPROM中写入一个字节数据
  * @param  None
  * @retval None
  */
static void eepromWriteByte(uint32_t addr,uint32_t data)
{
    uint8_t i;
    __disable_irq();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    for(i=0; i<20; i++)
    {
        if(HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE,EEPROM_BASE_ADDRESS+addr,data)==HAL_OK)
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
static void eepromWriteTwoBytes(uint32_t addr,uint32_t data)
{
    uint8_t i;
    __disable_irq();
    HAL_FLASHEx_DATAEEPROM_Unlock();
    for(i=0; i<20; i++)
    {
        if(HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_HALFWORD,EEPROM_BASE_ADDRESS+addr,data)==HAL_OK)
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
//static void eepromWriteWord(uint32_t addr,uint32_t data)
//{
//    uint8_t i;
//    __disable_irq();
//    HAL_FLASHEx_DATAEEPROM_Unlock();
//    for(i=0; i<20; i++)
//    {
//        if(HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD,EEPROM_BASE_ADDRESS+addr,data)==HAL_OK)
//        {
//            break;
//        }
//    }
//    HAL_FLASHEx_DATAEEPROM_Lock();
//    __enable_irq();
//}


/**
  * @brief  复位原始值EEPROM
  * @param  None
  * @retval None
  */
void mcuEEpromResetInit(void)
{
    //1个字节区域
    eeprom_info.VERSION=EEPROM_VER;
    eepromWriteByte(EEPROM_VERSION_ADDR,eeprom_info.VERSION);
    eepromParamSaveSnNumber((uint8_t *)"8888888877777777",15);
    eepromParamSaveServer((uint8_t *)"agps.basegps.com",9999);
    eepromParamSaveApnName((uint8_t *)"cmnet");
    eepromParamSaveApnUser((uint8_t *)"card");
    eepromParamSaveApnPswd((uint8_t *)"card");
	eepromParamSaveCodeVersion("boot");
    eepromParamSaveUpdateStatus(0);
    LogMessage("Reset EEPROM\n");
}

/**
  * @brief  初始化EEPROM
  * @param  None
  * @retval None
  */
void mcuEEpromInit(void)
{
    eeprom_info.VERSION=eepromReadOneByte(EEPROM_VERSION_ADDR);
    if(eeprom_info.VERSION!=EEPROM_VER)
    {
        mcuEEpromResetInit();
    }
    eepromParamGetSnNumber(eeprom_info.SN,15);
    eepromParamGetServer(eeprom_info.Server, &eeprom_info.ServerPort);
    eepromParamGetApnName(eeprom_info.apn);
    eepromParamGetApnUser(eeprom_info.apnuser);
    eepromParamGetApnPswd(eeprom_info.apnpassword);
    eepromParamGetUpdateStatus(&eeprom_info.updateStatus);

}

void eepromParamSaveSnNumber(uint8_t * sn,uint16_t len)
{
    uint8_t i;
    for(i=0; i<len; i++)
    {
        eepromWriteByte(EEPROM_SN_ADDR+i,sn[i]);
    }
}
void eepromParamGetSnNumber(uint8_t *sn,uint16_t len)
{
    uint8_t i;
    for(i=0; i<len; i++)
    {
        sn[i]=eepromReadOneByte(EEPROM_SN_ADDR+i);
    }
}

void eepromParamSaveCodeVersion(uint8_t * ver)
{
    uint8_t i;
    uint8_t version[30];
    strcpy((char *)version,(char *)ver);
    for(i=0; i<30; i++)
    {
        eepromWriteByte(EEPROM_CODEVERSION_ADDR+i,version[i]);
    }
}
void eepromParamGetCodeVersion(uint8_t *ver)
{
    uint8_t i;
    for(i=0; i<30; i++)
    {
        ver[i]=eepromReadOneByte(EEPROM_CODEVERSION_ADDR+i);
    }
}

void eepromParamSaveServer(uint8_t * server,uint16_t port)
{
    uint8_t i,serverlen;
    uint8_t newserver[50];
    memset(newserver,0,50);
    serverlen=strlen((const char *)server);
    if(serverlen>50)
        return;
    strncpy((char *)newserver,(char *)server,serverlen);
    for(i=0; i<50; i++)
    {
        eepromWriteByte(EEPROM_UPDATEDOMAIN_ADDR+i,newserver[i]);
    }
    eepromWriteTwoBytes(EEPROM_UPDATEDOMAIN_PORT_ADDR, port);
}
void eepromParamGetServer(uint8_t * server, uint16_t * port)
{
    uint8_t i;
    for(i=0; i<50; i++)
    {
        server[i]=eepromReadOneByte(EEPROM_UPDATEDOMAIN_ADDR+i);
    }
    *port=eepromReadTwoBytes(EEPROM_UPDATEDOMAIN_PORT_ADDR);
}

void eepromParamSaveApnName(uint8_t * apnname)
{
    uint8_t i;
    uint8_t nbuf[50];
    memset(nbuf,0,50);
    strncpy((char *)nbuf,(char *)apnname,strlen((char *)apnname));
    for(i=0; i<50; i++)
    {
        eepromWriteByte(EEPROM_APN_NAME_ADDR+i,nbuf[i]);
    }
}
void eepromParamGetApnName(uint8_t *apnname)
{
    uint8_t i;
    for(i=0; i<50; i++)
    {
        apnname[i]=eepromReadOneByte(EEPROM_APN_NAME_ADDR+i);
    }
}

void eepromParamSaveApnUser(uint8_t * apnuser)
{
    uint8_t i;
    uint8_t nbuf[50];
    memset(nbuf,0,50);
    strncpy((char *)nbuf,(char *)apnuser,strlen((char *)apnuser));
    for(i=0; i<50; i++)
    {
        eepromWriteByte(EEPROM_APN_USER_ADDR+i,nbuf[i]);
    }
}
void eepromParamGetApnUser(uint8_t *apnuser)
{
    uint8_t i;
    for(i=0; i<50; i++)
    {
        apnuser[i]=eepromReadOneByte(EEPROM_APN_USER_ADDR+i);
    }
}


void eepromParamSaveApnPswd(uint8_t * apnPSWD)
{
    uint8_t i;
    uint8_t nbuf[50];
    memset(nbuf,0,50);
    strncpy((char *)nbuf,(char *)apnPSWD,strlen((char *)apnPSWD));
    for(i=0; i<50; i++)
    {
        eepromWriteByte(EEPROM_APN_PSWD_ADDR+i,nbuf[i]);
    }
}
void eepromParamGetApnPswd(uint8_t *apnPSWD)
{
    uint8_t i;
    for(i=0; i<50; i++)
    {
        apnPSWD[i]=eepromReadOneByte(EEPROM_APN_PSWD_ADDR+i);
    }
}

void eepromParamSaveUpdateStatus(uint8_t status)
{
    eeprom_info.updateStatus=status;
    eepromWriteByte(EEPROM_UPDATE_FLAG_ADDR,eeprom_info.updateStatus);
}

void eepromParamGetUpdateStatus(uint8_t *status)
{
    *status=eepromReadOneByte(EEPROM_UPDATE_FLAG_ADDR);
}

