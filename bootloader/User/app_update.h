#ifndef APP_UPDATE
#define APP_UPDATE

#include "app_common.h"
typedef enum{
	NETWORK_LOGIN,
	NETWORK_LOGIN_WAIT,
	NETWORK_LOGIN_READY,
	NETWORK_DOWNLOAD_DOING,
	NETWORK_DOWNLOAD_WAIT,
	NETWORK_DOWNLOAD_DONE,
	NETWORK_DOWNLOAD_ERROR,
	NETWORK_WAIT_JUMP
}NetWorkFsmState;


typedef struct
{
	NetWorkFsmState fsmstate;
	unsigned short serial;
	uint16_t runtick;
}NetWorkConnectStruct;

typedef enum{
	PROTOCOL_01,//登录
	PROTOCOL_F3
}PROTOCOLTYPE;


typedef struct
{
	char curCODEVERSION[50];
	char newCODEVERSION[50];
	char rxsn[50];
	char rxcurCODEVERSION[50];
	uint32_t file_id;
	uint32_t file_offset;
	uint32_t file_len;
	uint32_t file_totalsize;

	uint32_t rxfileOffset;//已接收文件长度
	uint8_t updateOK;
}UndateInfoStruct;

void netConnectReset(void);
void protocolRxParase(char * protocol,int size);
void protocolRunFsm(void);
void updateUIS(void);
uint32_t getUpgradeFileSize(void);


#endif

