#ifndef APP_REC
#define APP_REC

#include <stdint.h>

#define FSM_RECORD_SEARCH	0
#define FSM_RECORD_READ		1
#define FSM_RECORD_WAIT		2

#define RECORD_MAXREADRESTORESIZE  1024
#define RECORD_UPLOADONEPACKSIZE   RECORD_MAXREADRESTORESIZE
typedef struct{
	uint8_t nofileflag;
	uint8_t fsmstate;
	uint8_t readfileok;
	uint8_t waittick;
	uint8_t reccycle;
	uint8_t reccycletick;
	char filename[50];
	uint32_t totalsize;
	uint32_t havereaddata;
	uint32_t needreadsize;
	
}RECORDFILE;

void recStart(void);
void recStop(void);
void recStopAndUpload(void);
void recNoFileToread(void);
uint8_t recIsRun(void);
void recordUploadRun(void);
void recUpdateRestoreFileNameAndTotalSize(char *filename,uint32_t totalsize);
void recUpdateFileName(char *filename);
void recUpdateFileSize(uint32_t totalsize);

void recReadFileOk(void);
void recCycleStart(void);
void recCycleStop(void);
uint8_t resIsCycleRuning(void);
void recCycleTask(void);

RECORDFILE * recGetRecStruct(void);
void recRecordUploadTask(void);

#endif



