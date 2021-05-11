#ifndef APP_EEPROM_H
#define APP_EEPROM_H
#include <stdint.h>
#include "app_sys.h"
#define EEPROM_BASE_ADDRESS	0X08080000
#define EEPROM_MAX_SIZE			0x7FF

#define EEPROM_VER				0X2B
//设置地址时，需要保证地址对齐，单个字节无所谓，半字或字，需要注意对齐，勿跨界
//addr%4==0,字对齐
//addr%2==0,半字对齐
//bootloader使用区域
#define EEPROM_UPDATE_FLAG_ADDR					1   /*是否需要升级的标志位地址*/
#define EEPROM_UPDATEDOMAIN_ADDR				10 	/*升级地址，共50个字节*/
#define EEPROM_UPDATEDOMAIN_PORT_ADDR			60  /*升级端口，共2个字节*/
#define EEPROM_CODEVERSION_ADDR					62  /*版本号，共30个字节，到92*/




//字节区域，0~1023         共(1024)字节
#define EEPROM_VERSION_APP_ADDR					99
#define EEPROM_SN_ADDR							100	/*SN号区域20个字节*/
#define EEPROM_APN_NAME_ADDR					120 /*apn 50字节*/
#define EEPROM_APN_USER_ADDR					170 /*apn 50字节*/
#define EEPROM_APN_PSWD_ADDR					220 /*apn 50字节*/

#define EEPROM_MODE_ADDR						270 //模式
#define EEPROM_LEN_ADDR							271 //感光
#define EEPROM_SEN_ADDR							272 //按键开关
#define EEPROM_MODE1_GAP_ADDR					273 //模式一间隔日期
#define EEPROM_SLEEP_ADDR						274 //睡眠模式
#define EEPROM_DOMAIN_ADDR						275 //275+50=325
//#define EEPROM_HEARTBEATTIME_ADDR				325 //心跳间隔
#define EEPROM_LOWW_ADDR						326 //低电
#define EEPROM_LED_ADDR							327 //led
#define EEPROM_POITYPE_ADDR						328 //POITYPE
#define EEPROM_UTC_ADDR							329 //UTC
#define EEPROM_ACCCTLGNSS_ADDR					330 //ACCCTLGNSS
#define EEPROM_BLEMAC_ADDR						331 //BLEMAC 331+19=350
#define EEPROM_BF_ADDR							350
#define EEPROM_FENCE_ADDR						351
#define EEPROM_CM_ADDR							352
#define EEPROM_SMSREPLY_ADDR					353
#define EEPROM_AUTOANSWER_ADDR					354
#define EEPROM_TURNALG_ADDR						355



//半字区域，1024~1534        共(512)字节
#define EEPROM_ALARMTIEM_ADDR						1024
#define EEPROM_MODE3_GAP_ADDR						1034
#define EEPROM_mode2worktime_ADDR				1036
#define EEPROM_GPS_INTERVAL_ADDR				1038
#define EEPROM_MODE1TIMER_ADDR					1040
#define EEPROM_PDOP_ADDR								1042
#define EEPROM_HEARTBEATTIME_ADDR				1044 //心跳间隔

//字区域，1536~2044         共(512)字节

#define EEPROM_DOMAIN_PORT_ADDR					1536
#define EEPROM_ADCCAL_ADDR						1540

/*
设备类型
V
主版本     	架构变化较大时改变
次版本     	与之前版本不能兼容时改变
修订版本        细节修改
*/
#ifdef RI_ENABLE
    #define EEPROM_VERSION									"K8_RI_V1.0.16"
#else
    #define EEPROM_VERSION									"ZT09_V1.0.10"
#endif

/*EPROM中的数据*/
typedef struct
{
    uint8_t VERSION;        /*当前软件版本号*/
    uint8_t MODE;      /*系统工作模式*/
    uint8_t Light_Alarm_En; /*光感触发功能是否开启*/
    uint8_t Switch_Alarm_En;/*开关触发功能是否开启*/
    uint8_t MODE1_GAP_DAY;        /*模式一间隔天数*/
    uint8_t SLEEP;
    uint8_t updateStatus;
    uint8_t ledctrl;
    uint8_t poitype;
    uint8_t accctlgnss;
    uint8_t ring;
    uint8_t SN[20];
    uint8_t Server[50];
    uint8_t apn[50];
    uint8_t apnuser[50];
    uint8_t apnpassword[50];
    uint8_t blemac[19];//d7:5a:79:da:45:40
    uint8_t lowvoltage;
    uint8_t bf;
    uint8_t fence;
	uint8_t smsRespon;
	uint8_t autoAnswer;//自动接听
	uint8_t turnalg;
	
    int8_t utc;

    uint16_t gpsuploadgap;
    uint16_t  AlarmTime[5];  /*每日时间，0XFFFF，表示未定义，单位分钟*/
    uint16_t  interval_wakeup_minutes;    /*模式三间隔周期，单位分钟*/
    uint16_t  mode1startuptime;
    uint16_t  pdop;
    uint16_t heartbeatgap;

    uint32_t  mode2worktime;  /*模式二的工作时间，单位分钟*/
    uint32_t ServerPort;

    float  adccal;

} SYSTEM_FLASH_DEFINE;

extern char CODEVERSION[];
extern SYSTEM_FLASH_DEFINE sysparam;


void paramInit(void);
void paramDefaultInit(uint8_t level);
void paramSaveMode(uint8_t mode);
void paramSaveMode1GapDay(uint8_t day);
void paramSaveInterval(void);
void paramSaveSleepState(void);
void paramSaveAlarmTime(void);
void paramSaveSnNumber(uint8_t *sn, uint16_t len);
void paramSaveServer(uint8_t *server, uint32_t port);
void paramSaveHeartbeatInterval(uint16_t hbt);
void paramGetHeartbeatInterval(uint16_t *hbt);
void paramSaveGPSUploadInterval(uint16_t gpsupload);
void paramGetGPSUploadInterval(uint16_t *data);
void paramGetSnNumber(uint8_t *sn, uint16_t len);
void paramGetServer(uint8_t *server, uint32_t *port);
void paramSaveApnName(uint8_t *apnname);
void paramGetApnName(uint8_t *apnname);
void paramSaveApnUser(uint8_t *apnuser);
void paramGetApnUser(uint8_t *apnuser);
void paramSaveApnPswd(uint8_t *apnPSWD);
void paramGetApnPswd(uint8_t *apnPSWD);
void paramSaveUpdateStatus(uint8_t status);
void paramGetUpdateStatus(uint8_t *status);
void paramSaveUPSServer(uint8_t *server, uint16_t port);
void paramGetUPSServer(uint8_t *server, uint16_t *port);
void paramSaveCodeVersion(uint8_t *ver);
void paramGetCodeVersion(uint8_t *ver);
void paramSaveLoww(uint8_t loww);
void paramGetLoww(uint8_t *loww);
void paramSaveLedCtl(uint8_t LED);
void paramSavePoitype(uint8_t value);
void paramSaveMode1Timer(uint16_t count);
void paramSaveUTC(uint8_t UTC);
void paramSaveLightAlarmMode(uint8_t onoff);
void paramSaveACCCTLGNSS(uint8_t onoff);
void paramSavePdop(uint16_t pdop);
void paramSaveAdcCal(float cal);
float paramGetAdcCal(void);
void paramSaveBleMac(uint8_t *mac);
void paramGetBleMac(uint8_t *mac);
void paramSaveMode2cnt(uint32_t cnt);
void paramGetMode2cnt(void);
void paramSaveBF(uint8_t onoff);
void paramSaveFence(uint8_t distance);
void paramSaveCM(uint8_t value);
uint8_t paramGetCM(void);
void paramSaveSmsreply(uint8_t value);
void paramSaveAutoAnswer(uint8_t onoff);
void paramSaveTurnalg(uint8_t onoff);

#endif

