#ifndef APP_BLE
#define APP_BLE

#include <stdint.h>

typedef struct
{
	uint8_t ble_power				:1; //蓝牙电源状态
	uint8_t ble_power_Respon		:1; //是否有回复
	uint8_t ble_name_cfg_Respon		:1; //修改名称是否回复
	uint8_t ble_role				:1;
	uint8_t ble_role_Respon			:1;
	uint8_t ble_conncted			:1;
	uint8_t ble_ccon_Respon			:1;
	uint8_t ble_fsm;
	uint8_t ble_tick;
	uint8_t ble_conmac[20];
}BLE_INFO;

enum
{
	BLE_FSM_POWER_CHK, 		//电源检测
	BLE_FSM_ROLE_CHK,  		//角色检测
	BLE_FSM_SLAVER,       //从机状态
	BLE_FSM_MASTER,       //主机状态
	BLE_FSM_CON_CHK,      //链接检测
	BLE_FSM_CONNECTED,		//已链接
	BLE_FSM_POERON,			//蓝牙上电
	BLE_FSM_POWEROFF,       //蓝牙断电
	BLE_FSM_CFG,		//配置名称
};

extern BLE_INFO ble_info;


void appBleInfoReset(void);
void appBleTask(void);
void appBlePRecv(uint8_t *buf,uint16_t len);
void appBleCRecv(uint8_t *buf,uint16_t len);
void appBleSendData(uint8_t *buf,uint16_t len);
uint8_t appBleIsLinkNormal(void);

#endif
