#ifndef GPS_H
#define GPS_H

#include "app_sys.h"
#include "app_nmea.h"

#define GPSFIFOSIZE	7
#define ANALY_LEN	5


typedef struct{
	unsigned char currentindex;
	GPSINFO gpsinfohistory[GPSFIFOSIZE];
	GPSINFO currentgpsinfo;
	GPSINFO lastfixgpsinfo;
}GPSFIFO;

typedef struct
{
	float speed;
	unsigned long time;
	uint16_t angle;
} analysis_base_data;

typedef struct
{
	uint8_t index;
	analysis_base_data ayaly_fifo[ANALY_LEN];
} analysis_typedef;

void GPSFifoInit(void);
void addNewGPSInfo(GPSINFO *gpsinfo);
void gpsClearCurrentGPSInfo(void);
void updateLocalRTCTime(DATETIME * datetime);

GPSINFO *getCurrentGPSInfo(void);
GPSINFO *getLastFixedGPSInfo(void);
GPSFIFO *getGSPfifo(void);

#endif
