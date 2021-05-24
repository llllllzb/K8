#ifndef APP_NET
#define APP_NET

#include <stdint.h>

#define N58MAXWIFISCAN	10

//4G电源
#define MODULE_POWER_ON			  PWRENON
#define MODULE_POWER_OFF		  PWRENOFF
//4G开机键
#define MODULE_POWERKEY_HIGH
#define MODULE_POWERKEY_LOW
//4G复位键
#define MODULE_RSTKEY_HIGH		RSTKEYHIGH
#define MODULE_RSTKEY_LOW		  RSTKEYLOW
//4GDTR唤醒
#define MODULE_DTR_HIGH			  DTRHIGH
#define MODULE_DTR_LOW        DTRLOW


//发送队列结构体
typedef struct NODE
{
    char * data;
    uint16_t datalen;
    uint8_t currentcmd;
    struct NODE *nextnode;
} NODEDATA;

/*定义AT指令集*/
typedef enum
{
    N58_AT_CMD = 1,
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
    N58_MYGNSSSEL_CMD,
    N58_MYPOWEROFF_CMD,
    N58_TTSPLAY_CMD,
    N58_PLAYTTS_CMD,
    N58_FSLIST_CMD,
    N58_FSWF_CMD,
    N58_FSRF_CMD,
    N58_FSDF_CMD,
    N58_FSFAT_CMD,
    N58_RECMODE_CMD,
    N58_RECF_CMD,
    N58_WIFIAPSCAN_CMD,
    N58_CCID_CMD,
    N58_SETSERVER_CMD,
    N58_ENPWRSAVE_CMD,
    N58_CFUN_CMD,
    N58_MICL_CMD,
    N58_MYLACID_CMD,
    N58_NWBTBLEPWR_CMD,
    N58_NWBTBLENAME_CMD,
    N58_NWBLEROLE_CMD,
    N58_NWBLEPSEND_CMD,
    N58_NWBLEDISCON_CMD,
    N58_NWBLEMAC_CMD,
    N58_NWBLECCON_CMD,
    N58_MAX_NUM,
} N58_CMD_TYPE;


/*定义系统运行状态*/
typedef enum
{
    N58_AT_STATUS,    //0
    N58_CPIN_STATUS,
    N58_IMSI_STATUS,
    N58_CSQ_STATUS,
    N58_CREG_STATUS,
    N58_CCID_STATUS,
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
    uint8_t modulepowerstate;
    uint32_t tick_time;
    uint32_t poweron_tick;
} N58_INVOKE_STATUS;

/*记录回复*/
typedef struct
{
    uint8_t at_respon   	: 1;
    uint8_t cpin_respon   	: 1;
    uint8_t cimi_respon   	: 1;
    uint8_t csq_ok   		: 1;
    uint8_t mysysinfo_ok   	: 1;
    uint8_t cgdcont_ok   	: 1;
    uint8_t xgauth_ok   	: 1;
    uint8_t xiic_ok   		: 1;
    uint8_t tcpsetup_ok   	: 1;
    uint8_t tcpclose_ok   	: 1;
    uint8_t fslist_ok   	: 1;
    uint8_t btcommstate;
    uint8_t btconnection;
    uint8_t btok;
    uint8_t xiic_queryok;
    uint8_t cgatt_ok;
    uint8_t csq_rssi;
    uint8_t creg_state;
    uint8_t tcpsetup_error;
    uint8_t cgps;
    /*reboot count*/
    uint8_t cgatt_bymen;//手动模式次数
    uint8_t rexiic_count;
    uint8_t retcpclose_count;
    uint8_t retcpsetup_count;
    uint8_t reCSQ_count;
    uint8_t reCreg_Count;
    /**/
    uint8_t current_cmd;

    uint8_t IMEI[30];
    uint8_t IMSI[30];
    uint8_t ICCID[30];
    uint8_t messagePhone[40];
    uint8_t messageindex;
} N58_LTE_STATUS;

typedef struct
{
    uint8_t ssid[6];
    int8_t signal;
} N58_WIFIone;

typedef struct
{
    N58_WIFIone ap[N58MAXWIFISCAN];
    uint8_t apcount;
} N58_WIFIAPSCAN;

uint8_t CreateNodeCmd(char *data, uint16_t datalen, uint8_t currentcmd);
void outPutNodeCmd(void);
uint8_t  sendModuleCmd(uint8_t cmd,char *param);

void modulePowerOn(void);
void modulePowerOff(void);
void moduleReset(void);


void sendDataToServer(uint8_t * data,uint16_t len);
void reConnectServer(void);
uint8_t netWorkModuleRunOk(void);
void networkConnectProcess(void);
void moduleResponParaser(uint8_t * buf,uint16_t len);
void N58_ChangeInvokeStatus(N58_WORK_STATUS status);


uint8_t getModuleRssi(void);
uint8_t *getModuleIMSI(void);
uint8_t *getModuleICCID(void);
uint8_t *getModuleIMEI(void);

uint8_t isModuleRunNormal(void);
void csqRequest(void);
void sendMessage(uint8_t *buf,uint16_t len,char * telnum);
void deleteMessage(void);
void openModuleGPS(void);
void closeModuleGPS(void);



#endif
