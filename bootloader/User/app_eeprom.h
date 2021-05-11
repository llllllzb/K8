#ifndef APP_EEPROM_H
#define APP_EEPROM_H
#include "app_common.h"

#define EEPROM_BASE_ADDRESS		0X08080000
#define EEPROM_MAX_SIZE			0x7FF

#define EEPROM_VER				0X2B
//设置地址时，需要保证地址对齐，单个字节无所谓，半字或字，需要注意对齐，勿跨界
//addr%4==0,字对齐
//addr%2==0,半字对齐
//分配100个字节给升级区

#define EEPROM_VERSION_ADDR						0	/*EEPROM 版本使用 1个字节*/
#define EEPROM_UPDATE_FLAG_ADDR					1   /*是否需要升级的标志位地址*/

#define EEPROM_UPDATEDOMAIN_ADDR						10 	/*升级地址，共50个字节*/
#define EEPROM_UPDATEDOMAIN_PORT_ADDR					60  /*升级端口，共2个字节*/
#define EEPROM_CODEVERSION_ADDR					62			/*版本号，共30个字节，到92*/
//与app层共用
#define EEPROM_VERSION_APP_ADDR					99
#define EEPROM_SN_ADDR							100	/*SN号区域20个字节*/
#define EEPROM_APN_NAME_ADDR					120 /*apn 50字节*/
#define EEPROM_APN_USER_ADDR					170 /*apn 50字节*/
#define EEPROM_APN_PSWD_ADDR					220 /*apn 50字节*/

/*存储在EEPROM中的数据*/
typedef struct
{
	uint8_t VERSION;        /*当前软件版本号*/
	uint8_t updateStatus;
	unsigned char SN[20];
	unsigned char Server[50];
	uint16_t ServerPort;
	uint8_t apn[50];
	uint8_t apnuser[50];
	uint8_t apnpassword[50];
} SYSTEM_FLASH_DEFINE;

extern char CODEVERSION[];
extern SYSTEM_FLASH_DEFINE eeprom_info;


void mcuEEpromInit(void);

void eepromParamSaveSnNumber(uint8_t * sn,uint16_t len);
void eepromParamSaveServer(uint8_t * server,uint16_t port);
void eepromParamGetSnNumber(uint8_t *sn,uint16_t len);
void eepromParamGetServer(uint8_t * server, uint16_t * port);
void eepromParamSaveApnName(uint8_t * apnname);
void eepromParamGetApnName(uint8_t *apnname);
void eepromParamSaveApnUser(uint8_t * apnuser);
void eepromParamGetApnUser(uint8_t *apnuser);
void eepromParamSaveApnPswd(uint8_t * apnPSWD);
void eepromParamGetApnPswd(uint8_t *apnPSWD);
void eepromParamSaveUpdateStatus(uint8_t status);
void eepromParamGetUpdateStatus(uint8_t *status);
void eepromParamSaveCodeVersion(uint8_t * ver);
void eepromParamGetCodeVersion(uint8_t *ver);


#endif

