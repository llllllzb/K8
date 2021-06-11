#ifndef APP_SYS
#define APP_SYS
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "app_nmea.h"

#define RI_ENABLE

#define ITEMCNTMAX	8
#define ITEMSIZEMAX	60

#define DEBUG_NONE			0
#define DEBUG_LOW			1
#define DEBUG_NET			2
#define DEBUG_GPS			3
#define DEBUG_FACTORY		4
#define DEBUG_ALL			9
#define DEBUG_BLE			10


/******************声明结构体**********************/
/*系统相关信息*/
typedef struct
{
    __IO uint8_t nmeaoutputonoff   	:1;
    __IO uint8_t lbsrequest			:1;
    __IO uint8_t wifirequest		:1;
    __IO uint8_t netModuleState		:1; /*MTK电源状态*/
    __IO uint8_t localrtchadupdate	:1;
    __IO uint8_t debuguartctrl		:1;
	__IO uint8_t flag123			:1;
	uint8_t blerequest				:1;
	uint8_t gsensoronoff			:1;
	uint8_t gsensorerror			:1;
	uint8_t recordingflag			:1;
	uint8_t hearbeatrequest			:1;
	uint8_t instructionqequest		:1;
	uint8_t smsReplyUpload			:1;
	uint8_t noNetworkFlag			:1;
	uint8_t netCtrlStop				:1;
    __IO uint8_t runFsm;
    __IO uint8_t GPSRequestFsm;
	
    uint8_t GPSStatus;
    uint8_t gsensortapcount;
    uint8_t terminalStatus;
    uint8_t modulepowerstate;
    uint8_t systempowerlow;
    uint8_t systemledstatus;
	uint8_t SystaskID;
    uint8_t onetaprecord[15];
	uint8_t logmessage;
	
    uint8_t mnc;
	uint8_t moduleGMR[40];
	
	
    __IO uint16_t systemRequest;
    uint16_t gpsuploadgap;
    uint16_t gpsuploadonepositiontime;
    uint16_t alarmrequest;
    uint16_t mcc;
    uint16_t lac;
	uint16_t mode4checktick;
	uint32_t netCtlTick;
		
    __IO uint32_t GPSRequest;	  /*GPS 开关请求*/
    uint32_t cid;
		uint32_t csqSearchTime;
    uint32_t System_Tick;    /*系统节拍*/
    uint32_t runStartTick;  /*开机节拍*/
    uint32_t gpsUpdatetick;
    uint32_t agpsOpenTick;
	
    float outsidevoltage;
    float lowvoltage;
} SystemInfoTypedef;

typedef struct
{
    unsigned char item_cnt;
    char item_data[ITEMCNTMAX][ITEMSIZEMAX];
} ITEM;


typedef enum
{
    TERMINAL_WARNNING_NORMAL = 0, /*    0 		*/
    TERMINAL_WARNNING_SHUTTLE,    /*    1：震动报警       */
    TERMINAL_WARNNING_LOSTV,      /*    2：断电报警       */
    TERMINAL_WARNNING_LOWV,       /*    3：低电报警       */
    TERMINAL_WARNNING_SOS,        /*    4：SOS求救      */
    TERMINAL_WARNNING_CARDOOR,        /*    5：车门求救      */
    TERMINAL_WARNNING_SWITCH,        /*    6：开关      */
    TERMINAL_WARNNING_LIGHT,      /*    7：感光报警       */
} TERMINAL_WARNNING_TYPE;


/***************共享变量***********************/

extern SystemInfoTypedef sysinfo;
/***************共享函数***********************/

void LogMessage(uint8_t level, char *debug);
void LogMessageWL(uint8_t level, char *buf,uint16_t len);
void LogPrintf(uint8_t level,const char * debug,...);


unsigned short GetCrc16(const char *pData, int nLength);


void terminalDefense(void);
void terminalDisarm(void);
uint8_t getTerminalAccState(void);
void terminalAccon(void);
void terminalAccoff(void);
void terminalCharge(void);
void terminalunCharge(void);
uint8_t getTerminalChargeState(void);
void terminalGPSFixed(void);
void terminalGPSUnFixed(void);
void terminalAlarmSet(TERMINAL_WARNNING_TYPE alarm);


int getCharIndex(uint8_t * src, int src_len, char ch);
int my_strpach(char *str1, const char *str2);
int my_strstr(char * str1, const char * str2, int len);
int my_getstrindex(char *str1, const char *str2, int len);
int distinguishOK(char *buf);

void changeByteArrayToHexString(uint8_t *src, uint8_t *dest,uint16_t srclen);
int16_t getCharIndexWithNum(uint8_t * src,uint16_t src_len,uint8_t ch,uint8_t num);
int16_t changeHexStringToByteArray(uint8_t *dest,uint8_t *src,uint16_t size);
int16_t changeHexStringToByteArray_10in(uint8_t *dest,uint8_t *src,uint16_t size);

void paraseInstructionToItem(ITEM *item,uint8_t * str,uint16_t len);

DATETIME changeUTCTimeToLocalTime(DATETIME utctime,int8_t localtimezone);

void updateRTCtimeRequest(void);
void updateSystemStartTime(void);

#endif
