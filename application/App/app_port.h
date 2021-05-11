#ifndef APP_PORT
#define APP_PORT

#include "main.h"
#include "usart.h"


/*串口驱动配置*/
#define USART1_RX_BUF_SIZE	256
#define USART2_RX_BUF_SIZE	2560
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
/*充电*/
#define CHARGEDET	HAL_GPIO_ReadPin(CHARGE_GPIO_Port,CHARGE_Pin)
/**********************************************************/
/*GPS*/
#define GPSLNAON	HAL_GPIO_WritePin(GPSLNA_GPIO_Port,GPSLNA_Pin,GPIO_PIN_SET)
#define GPSLNAOFF	HAL_GPIO_WritePin(GPSLNA_GPIO_Port,GPSLNA_Pin,GPIO_PIN_RESET)

/***********************/
extern UART_RXTX_CTL usart1_ctl;
extern UART_RXTX_CTL usart2_ctl;
extern UART_RXTX_CTL usart3_ctl;

/*******FUNCTION********/
/*串口功能*/
void appUartConfig(UARTTYPE type, uint8_t onoff, void (*rxhandlefun)(uint8_t *, uint16_t len));
int8_t appUartSend(UART_RXTX_CTL *uartctl, uint8_t * buf, uint16_t len);
void UsartInterruptHandler(UART_HandleTypeDef *huart);
void pollUartData(void);
/*gsensor功能*/
void gsensorConfig(uint8_t onoff);
void gsensorInterrupt(void);
/*rtc功能*/
void getRtcDateTime(uint16_t * year,uint8_t * month,uint8_t * date,uint8_t * hour,uint8_t * minute,uint8_t * second);
uint32_t getCurrentDateTimeOfSec(void);
void disPlaySystemTime(void);
void updateRTCdatetime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second);
int setNextAlarmTime( void );
void setNextWakeUpTime(void);
/*感光功能*/
void ldrInterrupt(void);
/*adc功能*/
uint32_t getVoltageAdcValue(void);
/*模组功能*/
void modulePinConfig(uint8_t onoff);
/*低功耗功能*/
void lowPowerConfig(void);
/*看门狗*/
void feedWdt(void);
/*RI 信号*/
void moduleRiSignal(void);
#endif
