#ifndef APP_BLE
#define APP_BLE

#include <stdint.h>

typedef struct
{
	uint8_t ble_power				:1; //������Դ״̬
	uint8_t ble_power_Respon		:1; //�Ƿ��лظ�
	uint8_t ble_name_cfg_Respon		:1; //�޸������Ƿ�ظ�
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
	BLE_FSM_POWER_CHK, 		//��Դ���
	BLE_FSM_ROLE_CHK,  		//��ɫ���
	BLE_FSM_SLAVER,       //�ӻ�״̬
	BLE_FSM_MASTER,       //����״̬
	BLE_FSM_CON_CHK,      //���Ӽ��
	BLE_FSM_CONNECTED,		//������
	BLE_FSM_POERON,			//�����ϵ�
	BLE_FSM_POWEROFF,       //�����ϵ�
	BLE_FSM_CFG,		//��������
};

extern BLE_INFO ble_info;


void appBleInfoReset(void);
void appBleTask(void);
void appBlePRecv(uint8_t *buf,uint16_t len);
void appBleCRecv(uint8_t *buf,uint16_t len);
void appBleSendData(uint8_t *buf,uint16_t len);
uint8_t appBleIsLinkNormal(void);

#endif
