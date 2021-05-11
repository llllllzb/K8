#ifndef GPS_ARITHMETIC
#define GPS_ARITHMETIC

#include "app_gps.h"
typedef struct{
	DATETIME datetime;//日期时间
	double latitude;//纬度
	double longtitude;//经度
	uint32_t gpsticksec;
	uint8_t init;
}LASTUPLOADGPSINFO;


void gpsUploadPointToServer(void);

#endif
