#ifndef APP_COMMON_H
#define APP_COMMON_H


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "main.h"
#include "usart.h"
#include "app_uart.h"
#include "app_timer.h"
#include "app_flash.h"
#include "app_n58.h"
#include "app_update.h"
#include "app_eeprom.h"


typedef enum{
	GPSCLOSESTATUS,
	GPSOPENSTATUS

}GPSREQUESTFSMSTATUS;

/*系统相关信息*/
typedef struct{
  uint8_t modulepowerstate;
  uint8_t logmessagectl;
}SystemInfoTypedef;


#define APPLICATION_ADDRESS     (uint32_t)0x08007800
typedef  void (*pFunction)(void);

extern SystemInfoTypedef   system_info;

unsigned short GetCrc16(const char *pData, int nLength);
int16_t changeHexStringToByteArray(uint8_t *dest,uint8_t *src,uint16_t size);

int getCharIndex(uint8_t * src,int src_len,char ch);
int distinguishOK(char *buf);
int my_strpach(char *str1,const char *str2);
int my_strstr(char * str1,const char * str2,int len);
int my_getstrindex(char *str1,const char *str2,int len);
int16_t getCharIndexWithNum(uint8_t * src,uint16_t src_len,uint8_t ch,uint8_t num);
void changeByteArrayToHexString(uint8_t *src, uint8_t *dest,uint16_t srclen);


/*************************************************/
void LogMessage(char *debug);
void LogMessageWL(char *debug,uint16_t len);
void LogMessageNoBlock(char * buf);
void LogPrintf(const char * debug,...);
uint8_t JumpToApp(uint32_t appaddress);
void startJumpToApp(void);

#endif

