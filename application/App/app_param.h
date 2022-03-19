#ifndef APP_EEPROM_H
#define APP_EEPROM_H
#include <stdint.h>
#include "app_sys.h"
#define EEPROM_BASE_ADDRESS	0X08080000
#define EEPROM_MAX_SIZE			0x7FF

#define EEPROM_VER				0X2B


//Զ������������Ҫ���Ĳ���ʱ�����ֵ��һ��
#define PARAM_AUTO_UPDATE_FLAG							0xA1

//���õ�ַʱ����Ҫ��֤��ַ���룬�����ֽ�����ν�����ֻ��֣���Ҫע����룬����
//addr%4==0,�ֶ���
//addr%2==0,���ֶ���
//bootloaderʹ������
#define EEPROM_UPDATE_FLAG_ADDR					1   /*�Ƿ���Ҫ�����ı�־λ��ַ*/
#define EEPROM_UPDATEDOMAIN_ADDR				10 	/*������ַ����50���ֽ�*/
#define EEPROM_UPDATEDOMAIN_PORT_ADDR			60  /*�����˿ڣ���2���ֽ�*/
#define EEPROM_CODEVERSION_ADDR					62  /*�汾�ţ���30���ֽڣ���92*/




//�ֽ�����0~1023         ��(1024)�ֽ�
#define EEPROM_VERSION_APP_ADDR					99
#define EEPROM_SN_ADDR							100	/*SN������20���ֽ�*/
#define EEPROM_APN_NAME_ADDR					120 /*apn 50�ֽ�*/
#define EEPROM_APN_USER_ADDR					170 /*apn 50�ֽ�*/
#define EEPROM_APN_PSWD_ADDR					220 /*apn 50�ֽ�*/

#define EEPROM_MODE_ADDR						270 //ģʽ
#define EEPROM_LEN_ADDR							271 //�й�
#define EEPROM_SEN_ADDR							272 //��������
#define EEPROM_MODE1_GAP_ADDR					273 //ģʽһ�������
#define EEPROM_SLEEP_ADDR						274 //˯��ģʽ
#define EEPROM_DOMAIN_ADDR						275 //275+50=325
//#define EEPROM_HEARTBEATTIME_ADDR				325 //�������
#define EEPROM_LOWW_ADDR						326 //�͵�
#define EEPROM_LED_ADDR							327 //led
#define EEPROM_POITYPE_ADDR						328 //POITYPE
#define EEPROM_UTC_ADDR							329 //UTC
#define EEPROM_ACCCTLGNSS_ADDR					330 //ACCCTLGNSS
#define EEPROM_BLEMAC_ADDR						331 //BLEMAC 331+19=350
#define EEPROM_BF_ADDR							350
#define EEPROM_FENCE_ADDR						351
#define EEPROM_CM_ADDR							352
#define EEPROM_SMSREPLY_ADDR					353
#define EEPROM_AUTOANSWER_ADDR					354
#define EEPROM_TURNALG_ADDR						355
#define EEPROM_NONETWAKEUP_ADDR					356
#define EEPROM_HEADFAULT_ADDR					357
#define EEPROM_MALLOC_ADDR						358
#define EEPROM_AGPSSERVER_ADDR					359 //359+50=409
#define EEPROM_AGPSUSER_ADDR					409 //409+50=459
#define EEPROM_AGPSPSWD_ADDR					459 //459+50=509
#define EEPROM_PARAM_AUTOUPDATE_ADDR			509
#define EEPROM_VIBRANGE_ADDR					510


//��������1024~1534        ��(512)�ֽ�
#define EEPROM_ALARMTIEM_ADDR					1024
#define EEPROM_MODE3_GAP_ADDR					1034
#define EEPROM_mode2worktime_ADDR				1036
#define EEPROM_GPS_INTERVAL_ADDR				1038
#define EEPROM_MODE1TIMER_ADDR					1040
#define EEPROM_PDOP_ADDR						1042
#define EEPROM_HEARTBEATTIME_ADDR				1044 //�������
#define EEPROM_AGPSPORT_ADDR					1046
#define EEPROM_STEP_ADDR						1048

//������1536~2044         ��(512)�ֽ�

#define EEPROM_DOMAIN_PORT_ADDR					1536
#define EEPROM_ADCCAL_ADDR						1540

/*
�豸����
V
���汾     	�ܹ��仯�ϴ�ʱ�ı�
�ΰ汾     	��֮ǰ�汾���ܼ���ʱ�ı�
�޶��汾        ϸ���޸�
*/
#define EEPROM_VERSION									"K8_RI_V1.1.29"

/*EPROM�е�����*/
typedef struct
{
    uint8_t VERSION;   /*��ǰ����汾��*/
    uint8_t MODE;      /*ϵͳ����ģʽ*/
    uint8_t Light_Alarm_En; /*��д��������Ƿ���*/
    uint8_t Switch_Alarm_En;/*���ش��������Ƿ���*/
    uint8_t MODE1_GAP_DAY;  /*ģʽһ�������*/
    uint8_t SLEEP;
    uint8_t updateStatus;
    uint8_t ledctrl;
    uint8_t poitype;
    uint8_t accctlgnss;
    uint8_t ring;
    uint8_t SN[20];
    uint8_t Server[50];
    uint8_t apn[50];
    uint8_t apnuser[50];
    uint8_t apnpassword[50];
    uint8_t blemac[19];//d7:5a:79:da:45:40
    uint8_t lowvoltage;
    uint8_t bf;
    uint8_t fence;
    uint8_t smsRespon;
    uint8_t autoAnswer;//�Զ�����
    uint8_t turnalg;
    uint8_t noNetWakeUpMinutes;
    uint8_t hardfault;
    uint8_t mallocfault;
    uint8_t agpsServer[50];
    uint8_t agpsUser[50];
    uint8_t agpsPswd[50];
    uint8_t autoParamUpdate;
	uint8_t vibRange;
    int8_t utc;

    uint16_t gpsuploadgap;
    uint16_t  AlarmTime[5];  /*ÿ��ʱ�䣬0XFFFF����ʾδ���壬��λ����*/
    uint16_t  gapMinutes;    /*ģʽ��������ڣ���λ����*/
    uint16_t  startUpCnt;
    uint16_t  pdop;
    uint16_t heartbeatgap;
    uint16_t agpsPort;
	uint16_t step;

    uint32_t  runTime;  /*ģʽ���Ĺ���ʱ�䣬��λ����*/
    uint32_t ServerPort;

    float  adccal;

} SYSTEM_FLASH_DEFINE;

extern char CODEVERSION[];
extern SYSTEM_FLASH_DEFINE sysparam;


void paramInit(void);
void paramDefaultInit(uint8_t level);
void paramSaveMode(uint8_t mode);
void paramSaveMode1GapDay(uint8_t day);
void paramSaveInterval(void);
void paramSaveSleepState(void);
void paramSaveAlarmTime(void);
void paramSaveSnNumber(uint8_t *sn, uint16_t len);
void paramSaveServer(uint8_t *server, uint32_t port);
void paramSaveHeartbeatInterval(uint16_t hbt);
void paramGetHeartbeatInterval(uint16_t *hbt);
void paramSaveGPSUploadInterval(uint16_t gpsupload);
void paramGetGPSUploadInterval(uint16_t *data);
void paramGetSnNumber(uint8_t *sn, uint16_t len);
void paramGetServer(uint8_t *server, uint32_t *port);
void paramSaveApnName(uint8_t *apnname);
void paramGetApnName(uint8_t *apnname);
void paramSaveApnUser(uint8_t *apnuser);
void paramGetApnUser(uint8_t *apnuser);
void paramSaveApnPswd(uint8_t *apnPSWD);
void paramGetApnPswd(uint8_t *apnPSWD);
void paramSaveUpdateStatus(uint8_t status);
void paramGetUpdateStatus(uint8_t *status);
void paramSaveUPSServer(uint8_t *server, uint16_t port);
void paramGetUPSServer(uint8_t *server, uint16_t *port);
void paramSaveCodeVersion(uint8_t *ver);
void paramGetCodeVersion(uint8_t *ver);
void paramSaveLoww(uint8_t loww);
void paramGetLoww(uint8_t *loww);
void paramSaveLedCtl(uint8_t LED);
void paramSavePoitype(uint8_t value);
void paramSaveMode1Timer(uint16_t count);
void paramSaveUTC(uint8_t UTC);
void paramSaveLightAlarmMode(uint8_t onoff);
void paramSaveACCCTLGNSS(uint8_t onoff);
void paramSavePdop(uint16_t pdop);
void paramSaveAdcCal(float cal);
float paramGetAdcCal(void);
void paramSaveBleMac(uint8_t *mac);
void paramGetBleMac(uint8_t *mac);
void paramSaveMode2cnt(uint32_t cnt);
void paramGetMode2cnt(void);
void paramSaveBF(uint8_t onoff);
void paramSaveFence(uint8_t distance);
void paramSaveCM(uint8_t value);
uint8_t paramGetCM(void);
void paramSaveSmsreply(uint8_t value);
void paramSaveAutoAnswer(uint8_t onoff);
void paramSaveTurnalg(uint8_t onoff);
void paramSaveNoNetWakeUpMinutes(uint8_t mis);
void paramSaveHardFault(void);
void paramSaveMallocFault(void);

void paramSaveAgpsServer(void);
void paramGetAgpsServer(void);
void paramSaveAgpsUser(void);
void paramGetAgpsUser(void);
void paramSaveAgpsPswd(void);
void paramGetAgpsPswd(void);
void paramSaveAgpsPort(void);
void paramGetAgpsPort(void);

void paramSaveAutoParam(uint8_t flag);
void paramGetAutoParam(void);

void paramSaveStep(void);
void paramGetStep(void);

void paramSaveVibrange(void);
void paramGetVibrange(void);

#endif

