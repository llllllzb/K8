#ifndef GPS_ARITHMETIC
#define GPS_ARITHMETIC

#include "app_gps.h"
typedef struct{
	DATETIME datetime;//����ʱ��
	double latitude;//γ��
	double longtitude;//����
	uint32_t gpsticksec;
	uint8_t init;
}LASTUPLOADGPSINFO;


void gpsUploadPointToServer(void);

#endif
