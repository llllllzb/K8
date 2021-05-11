#ifndef APP_GPSRESTORE
#define APP_GPSRESTORE
#include <stdint.h>

#define FSM_GPSRESTORE_SEARCH	0
#define FSM_GPSRESTORE_READ		1

#define GPS_MAXREADRESTORESIZE	20*10
/*
1、掉网时，查询文件盘里面是否有gpssave<x>.dat的文件
2、如果有，说明该文件未被发送完毕，新建一个新的gpssave<x+1>.dat文件
3、如果无，新建一个新的gspsave1.dat文件
4、往里写数据，并记录写的数据大小
*/

/*
1、网络恢复时，查询文件盘里面是否有gpssave<x>.dat的文件
2、如果有，记录该文件大小，并初始化读文件结构体
4、每隔3秒从文件中读取1000个字节的数据
5、文件读取完后，删除文件
*/


typedef struct{
	char filename[50];
	uint8_t  nofileflag;
	uint8_t  fsmstate;
	uint32_t totalsize;
	uint32_t havereaddata;
}GPSReadFromModule;


//总过17个字节，存储20个字节
typedef struct{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t latititude[4];
	uint8_t longtitude[4];
	uint8_t speed;
	uint8_t coordinate[2];
	uint8_t temp[3];
}GPSRestoreStruct;

void gpsRestoreTest(void);
void gpsRestoreWriteData(GPSRestoreStruct *gpsres);

void gpsReadRestoreReset(void);
void gpsUpdateRestoreFileNameAndTotalSize(char *filename,uint32_t totalsize);
void gpsNoRestoreFileToRead(void);
void gpsReadFromModuleAndSendtoServe(void);

void gpsRestoreDataSend(GPSRestoreStruct *grs ,char *dest	,uint16_t * len);
uint8_t gpsIsRun(void);


#endif
