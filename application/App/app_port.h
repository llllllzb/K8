#ifndef APP_PORT
#define APP_PORT

#include "main.h"
#include "usart.h"


/*������������*/
#define USART1_RX_BUF_SIZE	256
#define USART2_RX_BUF_SIZE	2048
#define USART3_RX_BUF_SIZE	128


typedef enum {
    APPUSART1,
    APPUSART2,
    APPUSART3,
} UARTTYPE;

typedef struct {
    uint8_t   rxcompleteflag;
    uint8_t * rxbuf;
    uint16_t  rxlen;
    UART_HandleTypeDef *uart_handle;
    void (*rxhandlefun)(uint8_t *, uint16_t len);
} UART_RXTX_CTL;


/**********************************************************/
/*LED*/
#define LED1ON      HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET)
#define LED1OFF     HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET)
#define LED1TOGGLE  HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin)
/**********************************************************/
/*4G*/
#define PWRENON     HAL_GPIO_WritePin(PWREN_GPIO_Port,PWREN_Pin,GPIO_PIN_SET)
#define PWRENOFF    HAL_GPIO_WritePin(PWREN_GPIO_Port,PWREN_Pin,GPIO_PIN_RESET)

#define PWRKEYHIGH	HAL_GPIO_WritePin(PWRKEY_GPIO_Port, PWRKEY_Pin, GPIO_PIN_SET)
#define PWRKEYLOW	HAL_GPIO_WritePin(PWRKEY_GPIO_Port, PWRKEY_Pin, GPIO_PIN_RESET)

#define RSTKEYHIGH  HAL_GPIO_WritePin(RSTKEY_GPIO_Port,RSTKEY_Pin,GPIO_PIN_SET)
#define RSTKEYLOW   HAL_GPIO_WritePin(RSTKEY_GPIO_Port,RSTKEY_Pin,GPIO_PIN_RESET)

#define DTRHIGH     HAL_GPIO_WritePin(DTR_GPIO_Port,DTR_Pin,GPIO_PIN_SET)
#define DTRLOW      HAL_GPIO_WritePin(DTR_GPIO_Port,DTR_Pin,GPIO_PIN_RESET)
/**********************************************************/
/*GSENSOR*/
#define GSENSORON	HAL_GPIO_WritePin(GSPWR_GPIO_Port, GSPWR_Pin, GPIO_PIN_RESET)
#define GSENSOROFF	HAL_GPIO_WritePin(GSPWR_GPIO_Port, GSPWR_Pin, GPIO_PIN_SET)
/**********************************************************/
/*LDR*/
#define LDRDET		HAL_GPIO_ReadPin(LDR_GPIO_Port,LDR_Pin)
/**********************************************************/
/*���*/
#define CHARGEDET	HAL_GPIO_ReadPin(CHARGE_GPIO_Port,CHARGE_Pin)
/**********************************************************/
/*GPS*/
#define GPSLNAON	HAL_GPIO_WritePin(GPSLNA_GPIO_Port,GPSLNA_Pin,GPIO_PIN_SET)
#define GPSLNAOFF	HAL_GPIO_WritePin(GPSLNA_GPIO_Port,GPSLNA_Pin,GPIO_PIN_RESET)

//��λ������SCL�ܽ�
#define	SET_SCL	HAL_GPIO_WritePin(GPIOB,SCL_Pin,GPIO_PIN_SET) 
#define	CLR_SCL	HAL_GPIO_WritePin(GPIOB,SCL_Pin,GPIO_PIN_RESET)
//��λ������SDA�ܽ�
#define	SET_SDA	HAL_GPIO_WritePin(GPIOB,SDA_Pin,GPIO_PIN_SET)
#define	CLR_SDA	HAL_GPIO_WritePin(GPIOB,SDA_Pin,GPIO_PIN_RESET)
//��SDA�ܽ�״̬
#define READ_SDA	HAL_GPIO_ReadPin(GPIOB,SDA_Pin)


#define IIC_SCL_PIN SCL_Pin
#define IIC_SCL_PORT GPIOB

#define IIC_SDA_PIN SDA_Pin
#define IIC_SDA_PORT GPIOB

#define IIC_DELAY_TIME 10




#define IIC_SCL_H()     HAL_GPIO_WritePin(GPIOB,SCL_Pin,GPIO_PIN_SET)
#define IIC_SCL_L()     HAL_GPIO_WritePin(GPIOB,SCL_Pin,GPIO_PIN_RESET)
#define IIC_SDA_H()     HAL_GPIO_WritePin(GPIOB,SDA_Pin,GPIO_PIN_SET)
#define IIC_SDA_L()     HAL_GPIO_WritePin(GPIOB,SDA_Pin,GPIO_PIN_RESET)


/***********************/
extern UART_RXTX_CTL usart1_ctl;
extern UART_RXTX_CTL usart2_ctl;
extern UART_RXTX_CTL usart3_ctl;
/* USER CODE BEGIN Includes */
//����ö������ -> BITλ
typedef enum
{
	BIT0 = (uint8_t)(0x01 << 0),  
	BIT1 = (uint8_t)(0x01 << 1),  
	BIT2 = (uint8_t)(0x01 << 2),  
	BIT3 = (uint8_t)(0x01 << 3),  
	BIT4 = (uint8_t)(0x01 << 4),
	BIT5 = (uint8_t)(0x01 << 5),
	BIT6 = (uint8_t)(0x01 << 6),
	BIT7 = (uint8_t)(0x01 << 7),
}BIT_t;

//�궨��
//����ö������
typedef enum
{
	ACK	 = GPIO_PIN_SET,
	NACK = GPIO_PIN_RESET,
}ACK_Value_t;

//����ṹ������
typedef struct
{
	void (*Init)(void);  //I2C��ʼ��
	void (*Start)(void); //I2C��ʼ�ź�
	void (*Stop)(void);  //I2Cֹͣ�ź�
	ACK_Value_t (*Write_Byte)(uint8_t);      //I2Cд�ֽ�
	uint8_t     (*Read_Byte) (ACK_Value_t);  //I2C���ֽ�
}I2C_Soft_t;

/* extern variables-----------------------------------------------------------*/
extern I2C_Soft_t  I2C_Soft;

/* USER CODE END Includes */

extern I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */


/* USER CODE BEGIN Prototypes */
uint8_t iicWriteData(uint8_t addr, uint8_t reg, uint8_t data);
uint8_t iicReadData(uint8_t addr, uint8_t regaddr, uint8_t *data, uint8_t len);

void Init(void);  //I2C��ʼ��
void portGsensorSdaSclCtl(uint8_t onoff);


/*******FUNCTION********/
/*���ڹ���*/
void portUartCfg(UARTTYPE type, uint8_t onoff, void (*rxhandlefun)(uint8_t *, uint16_t len));
int8_t portUartSend(UART_RXTX_CTL *uartctl, uint8_t * buf, uint16_t len);
void halRecvInIdle(UART_HandleTypeDef *huart);
void portPollUart(void);
/*gsensor����*/
void portGsensorCfg(uint8_t onoff);

void portGsensorInt(void);
/*rtc����*/
void portGetSystemDateTime(uint16_t * year,uint8_t * month,uint8_t * date,uint8_t * hour,uint8_t * minute,uint8_t * second);
uint32_t portGetDateTimeOfSeconds(void);
void disPlaySystemTime(void);
void portSetSystemDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second);
int portSetNextAlarmTime( void );
void portSetNextWakeUpTime(void);
/*�й⹦��*/
void portLdrInt(void);
/*adc����*/
uint32_t portGetAdc(void);
/*ģ�鹦��*/
void portModuleCfg(uint8_t onoff);
/*�͹��Ĺ���*/
void portLowPowerCfg(void);
/*���Ź�*/
void portWdtFeed(void);
/*RI �ź�*/
void portRingSignal(void);
void portUpdateStep(void);
void portClearStep(void);
void portSaveStep(void);
void portSystemReset(void);

#endif
