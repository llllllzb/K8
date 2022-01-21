#ifndef APPSERVERPROTOCOL
#define APPSERVERPROTOCOL
#include <stdint.h>
#include "app_gps.h"
#include "app_gpsrestore.h"
#include "app_net.h"
typedef enum{
	PROTOCOL_01,//登录
	PROTOCOL_12,//定位
	PROTOCOL_13,//心跳
	PROTOCOL_16,//定位
	PROTOCOL_19,//多基站
	PROTOCOL_21,//蓝牙
	PROTOCOL_8A,
	PROTOCOL_F1,//ICCID
}PROTOCOLTYPE;
typedef enum{
	NETWORK_LOGIN,
	NETWORK_LOGIN_WAIT,
	NETWORK_LOGIN_READY,
}NetWorkFsmState;
typedef struct
{
	NetWorkFsmState fsmstate;
	unsigned int heartbeattick;
	unsigned short serial;
	uint8_t logintick;
	uint8_t loginCount;
	uint8_t uploadFlag;
}NetWorkConnectStruct;

typedef struct
{
	int id;
	char message[30];
}Message;


void sendProtocolToServer(PROTOCOLTYPE protocol,void * param);
void netConnectReset(void);
void protocolFsmStateChange(NetWorkFsmState state);
void protocolRunFsm(void);
void protocolRxParase(char * protocol,int size);
uint8_t isProtocolReday(void);
void createProtocol61(char *dest,char *datetime,uint32_t totalsize,uint8_t filetye,uint16_t packsize);
void createProtocol62(char *dest,char *datetime,uint16_t packnum,uint8_t * recdata,uint16_t reclen);
void createProtocolF3(char *dest,N58_WIFIAPSCAN *wap);
void save123InstructionId(void);
void reCover123InstructionId(void);
void clearHbtTimer(void);

#endif
