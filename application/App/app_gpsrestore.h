#ifndef APP_GPSRESTORE
#define APP_GPSRESTORE
#include <stdint.h>

#define FSM_GPSRESTORE_SEARCH	0
#define FSM_GPSRESTORE_READ		1

#define GPS_MAXREADRESTORESIZE	20*10
/*
1������ʱ����ѯ�ļ��������Ƿ���gpssave<x>.dat���ļ�
2������У�˵�����ļ�δ��������ϣ��½�һ���µ�gpssave<x+1>.dat�ļ�
3������ޣ��½�һ���µ�gspsave1.dat�ļ�
4������д���ݣ�����¼д�����ݴ�С
*/

/*
1������ָ�ʱ����ѯ�ļ��������Ƿ���gpssave<x>.dat���ļ�
2������У���¼���ļ���С������ʼ�����ļ��ṹ��
4��ÿ��3����ļ��ж�ȡ1000���ֽڵ�����
5���ļ���ȡ���ɾ���ļ�
*/


typedef struct{
	char filename[50];
	uint8_t  nofileflag;
	uint8_t  fsmstate;
	uint32_t totalsize;
	uint32_t havereaddata;
}GPSReadFromModule;


//�ܹ�17���ֽڣ��洢20���ֽ�
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
