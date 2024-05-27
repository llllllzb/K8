#ifndef APP_TASK
#define APP_TASK

#include <stdint.h>
#include "app_port.h"


#define GPSLEDON
#define GPSLEDOFF

#define SIGNALLEDON					LED1ON
#define SIGNALLEDOFF				LED1OFF

#define SYSTEM_LED_RUN				0X01
#define SYSTEM_LED_NETOK			0X02
#define SYSTEM_LED_GPSOK			0X04

#define SIGNALLED					0
#define GPSLED						1



#define ALARM_LIGHT_REQUEST			0x0001 //感光
#define ALARM_LOSTV_REQUEST			0x0002 //断电
#define ALARM_LOWV_REQUEST			0x0004 //低电
#define ALARM_SHUTTLE_REQUEST		0x0008//震动报警
#define ALARM_ACCLERATE_REQUEST		0X0010
#define ALARM_DECELERATE_REQUEST	0X0020
#define ALARM_RAPIDRIGHT_REQUEST	0X0040
#define ALARM_RAPIDLEFT_REQUEST		0X0080
#define ALARM_GUARD_REQUEST			0X0100
#define ALARM_BLE_REQUEST			0X0200

#define GPS_REQUEST_UPLOAD_ONE			0X00000001
#define GPS_REQUEST_ACC_CTL				0X00000002
#define GPS_REQUEST_GPSKEEPOPEN_CTL		0X00000004

#define SYSTEM_MODULE_STARTUP_REQUEST	0X0001
#define SYSTEM_MODULE_SHUTDOWN_REQUEST	0X0002
#define SYSTEM_ENTERSLEEP_REQUEST		0X0004
#define SYSTEM_POWERLOW_REQUEST			0X0008
#define SYSTEM_CLOSEUART1_REQUEST		0X0010
#define SYSTEM_OPENUART1_REQUEST		0X0020
#define SYSTEM_WDT_REQUEST				0X0040



#define MODE1									1
#define MODE2									2
#define MODE3									3
#define MODE4									4
#define MODE5									5
#define MODE21									6  //模式2的扩展，类似模式1
#define MODE23									7  //模式2的扩展，类似模式3


#define NET_MODULE_OFF	0
#define NET_MODULE_ON	1

#define MODE_START		0
#define MODE_RUNING		1
#define MODE_STOP		2
#define MODE_DONE		3

//GPS_UPLOAD_GAP_MAX 以下，gps常开，以上(包含GPS_UPLOAD_GAP_MAX),周期开启
#define GPS_UPLOAD_GAP_MAX				60


typedef struct
{
    uint32_t sys_tick;		//记录系统运行时间
    uint8_t	sys_led_onoff;
    uint8_t	sys_gps_led_onoff;
    uint8_t sys_led_on_time;
    uint8_t sys_led_off_time;
    uint8_t sys_gps_led_on_time;
    uint8_t sys_gps_led_off_time;
} SystemLEDInfo;

typedef enum{
	GPSCLOSESTATUS,
	GPSOPENWAITSTATUS,
	GPSOPENSTATUS,

}GPSREQUESTFSMSTATUS;



void ledRunTask(void);
void updateSystemLedStatus(uint8_t status,uint8_t onoff);


void alarmRequestSet(uint16_t request);
void alarmRequestClear(uint16_t request);
void alarmUploadRequest(void);


void gpsRequestSet(uint32_t flag);
void gpsRequestClear(uint32_t flag);
uint8_t gpsRequestGet(uint32_t flag);
void gpsChangeFsmState(uint8_t state);

void systemRequestSet(uint16_t request);
void systemRequestClear(uint16_t request);
void systemRequestTask(void);
void SystemClock_Config(void);


void systemModeRunStart(void);
void getBatVoltage(void);

void noNetWakeUpGapInit(void);
void noNetWakeUpGapUpdate(void);

void taskRunInOneSecond(void);


#endif
