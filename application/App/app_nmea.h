#ifndef NMEA_PARSER_H
#define NMEA_PARSER_H

#include "main.h"

#define GPSFIXED		1
#define GPSINVALID	0


#define GPSITEMCNTMAX		24
#define GPSITEMSIZEMAX	20


typedef struct{
	unsigned char item_cnt;
	char item_data[GPSITEMCNTMAX][GPSITEMSIZEMAX];
}GPSITEM;


typedef struct{
	uint8_t year;  
	uint8_t month; 
	uint8_t day;
	int8_t hour;
	uint8_t minute;
	uint8_t second;
}DATETIME;

typedef struct{
	DATETIME datetime;//����ʱ��
	char fixstatus; //��λ״̬
	double latitude;//γ��
	double longtitude;//����
	double speed;		//�ٶ�
	uint16_t course;//�����
	float pdop;
	uint32_t gpsticksec;
	
	char NS; //�ϱ�γ
	char EW; //������
	char fixmode;
	unsigned char used_star;//��������
	unsigned char gpsviewstart;//�ɼ�����
	unsigned char beidouviewstart;
	unsigned char glonassviewstart;
	uint8_t hadupload;
	uint8_t beidouCn[30];
	uint8_t gpsCn[30];
}GPSINFO;

typedef enum{
	NMEA_RMC=1,
	NMEA_GSA,
	NMEA_GGA,
	NMEA_GSV,
}NMEATYPE;

void parseGPS(uint8_t *str,uint16_t len);
void nmeaParse(uint8_t * buf,uint16_t len);
unsigned int charstrToHexValue(char *value);
double latitude_to_double( double latitude, char Direction);
double longitude_to_double(double longitude, char Direction);
#endif
