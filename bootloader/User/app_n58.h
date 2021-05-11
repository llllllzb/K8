#ifndef N58
#define N58
#include "main.h"
#include "app_uartfifo.h"

/*定义AT指令集*/
typedef enum
{
	N58_AT_CMD=1,
	N58_CPIN_CMD,
	N58_CGSN_CMD,
	N58_CIMI_CMD,
	N58_CSQ_CMD,
	N58_CREG_CMD,
	N58_CGATT_CMD,
	N58_MYSYSINFO_CMD,
	N58_CGDCONT_CMD,
	N58_XGAUTH_CMD,
	N58_XIIC_CMD,
	N58_TCPSETUP_CMD,
	N58_IPSTATUS_CMD,
	N58_TCPSEND_CMD,
	N58_TCPACK_CMD,
	N58_TCPCLOSE_CMD,
	N58_CCLK_CMD,
	N58_ATA_CMD,
	N58_ATH_CMD,	
	N58_CMGF_CMD,
	N58_CMGS_CMD,
	N58_CMGD_CMD,
	N58_CMGR_CMD,
	N58_CNMI_CMD,	
	N58_MYGPSPWR_CMD,
	N58_MYGPSPOS_CMD,
	N58_MYGPSSTATE_CMD,
	N58_MYPOWEROFF_CMD,
	N58_TTSPLAY_CMD,
	N58_PLAYTTS_CMD,
	N58_FSLIST_CMD,
	N58_FSWF_CMD,
	N58_FSRF_CMD,
	N58_FSDF_CMD,
	N58_FSFAT_CMD,
	N58_MAX_NUM,
}N58_CMD_TYPE;

/*定义系统运行状态*/
typedef enum
{
	N58_AT_STATUS,    //0
	N58_CPIN_STATUS,
	N58_IMSI_STATUS,
	N58_CSQ_STATUS,
	N58_CREG_STATUS,
	N58_CGATT_STATUS,//5
	N58_MYSYSINFO_STATUS,
	N58_CGDCONT_STATUS,
	N58_XGAUTH_STATUS,
	N58_XIIC_STATUS,
	N58_XIIC_QUERY_STATUS,//10
	N58_TCPCLOSE_STATUS,
	N58_TCPSETUP_STATUS,//12
	N58_NORMAL_STATUS,
} N58_WORK_STATUS;

/*指令集对应结构体*/
typedef struct
{
	N58_CMD_TYPE cmd_type;
	char cmd[50];
} N58_CMD_STRUCT;

/*记录运行的相关状态*/
typedef struct
{
	uint8_t invoke_status;
	uint64_t tick_time;
	uint64_t poweron_tick;
} N58_INVOKE_STATUS;

/*记录回复*/
typedef struct
{
	unsigned char at_respon;
	unsigned char cpin_respon;
	unsigned char cimi_respon;
	unsigned char csq_ok;
	unsigned char csq_rssi;
	unsigned char creg_state;
	unsigned char cgatt_ok;
	unsigned char mysysinfo_ok;
	unsigned char cgdcont_ok;
	unsigned char xgauth_ok;
	unsigned char xiic_ok;
	unsigned char xiic_queryok;
	unsigned char tcpsetup_ok;
	unsigned char tcpsetup_error;
	unsigned char cgps;
	unsigned char tcpclose_ok;
	unsigned char fslist_ok;
	/*reboot count*/
	unsigned char cgatt_bymen;//手动模式次数
	unsigned char rexiic_count;
	unsigned char retcpclose_count;
	unsigned char retcpsetup_count;
	/**/
	unsigned char current_cmd;
} N58_LTE_STATUS;

extern N58_LTE_STATUS 		n58_lte_status;

uint8_t  sendN58Cmd(uint8_t cmd,char *param);
void n58Init(void);
void clearSetupCount(void);
void n58NetworkConnectProcess(void);
void n58responparase(uint8_t * buf,uint16_t len);
void N58_SendDataToServer(uint8_t * data,uint16_t len);
void n58ModulePowerOn(void);
void n58ModuleReset(void);

uint8_t N58_GetInvokeStatus(void);
void n58TTSPlay(char * ttsdata);

#endif
