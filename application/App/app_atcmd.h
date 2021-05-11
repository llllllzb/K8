#ifndef ATCMD_H
#define ATCMD_H

#include <stdint.h>
typedef struct
{
	uint8_t cmdid;
	int8_t   cmdstr[16];
}CMDTABLE;

typedef enum{
	AT_SMS_CMD,
	AT_FMPC_NMEA_CMD,
	AT_FMPC_BAT_CMD,
	AT_FMPC_GSENSOR_CMD,
	AT_FMPC_ACC_CMD,
	AT_FMPC_GSM_CMD,
	AT_FMPC_RELAY_CMD,
	AT_FMPC_CSQ_CMD,
	AT_DEBUG_CMD,
	AT_NMEA_CMD,
	AT_ADCCAL_CMD,
	AT_FMPC_LDR_CMD,
	AT_FMPC_ALL_CMD,
	AT_FMPC_CM_CMD,
	
	AT_ZTSN_CMD,
	AT_IMEI_CMD,
	AT_FMPC_IMSI_CMD,
	AT_FMPC_CHKP_CMD,
	AT_FMCP_CMGET_CMD,
	AT_MAX_CMD
}ATCMDID;

typedef enum{
	PARAM_INS,
	STATUS_INS,
	VERSION_INS,
	SN_INS,
	SERVER_INS,
	MODE_INS,
	HBT_INS,
	TTS_INS,
	JT_INS,
	POSITION_INS,
	APN_INS,
	UPS_INS,
	LOWW_INS,
	LED_INS,
	POITYPE_INS,
	RESET_INS,
	UTC_INS,
	ALARMMODE_INS,
	DEBUG_INS,
	ACCCTLGNSS_INS,
	PDOP_INS,
	SETBLEMAC_INS,
	BF_INS,
	CF_INS,
	FACTORYTEST_INS,
	FENCE_INS,
	FACTORY_INS,
	SMSREPLY_INS,
	JTCYCLE_INS,
	ANSWER_INS,
	TURNALG_INS,
	MAX_INS
}INSTRUCTIONID;

typedef enum{
	AT_SMS_MODE,
	SHORTMESSAGE_MODE,
	NETWORK_MODE,
	BLE_MODE
}DOINSTRUCTIONMODE;


void atCmdParaseFunction(uint8_t *buf, uint16_t len);
void instructionParase(uint8_t * str,uint16_t len,DOINSTRUCTIONMODE mode,char * telnum);


#endif
