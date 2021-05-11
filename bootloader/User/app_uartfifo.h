#ifndef APP_UARTFIFO_H
#define APP_UARTFIFO_H

#include "app_common.h"
//���Ͷ��нṹ��
typedef struct NODE
{
	char * data;
	uint16_t datalen;
	uint8_t currentcmd;
	struct NODE *nextnode;
}NODEDATA;

uint8_t CreateNodeCmd(char *data,uint16_t datalen,uint8_t currentcmd);
void outPutNodeCmd(void);
#endif

