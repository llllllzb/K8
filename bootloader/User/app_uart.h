#ifndef APP_UART_H
#define APP_UART_H
#include "app_common.h"


#define USART1_RX_DMA_SIZE  512

#define USART2_RX_DMA_SIZE  2048

typedef struct
{
    UART_HandleTypeDef *uart_handle;
    uint8_t receiveComplete;
    uint8_t * dmarxbuf;
    uint16_t  dmarxlen;

    void (*rxhandlefun)(uint8_t *,uint16_t len);
} UART_RXTX_CTL;


extern UART_RXTX_CTL usart1_ctl,usart2_ctl;

void UsartBufInit(void);
void uartBufPush(UART_RXTX_CTL *uartctl, uint8_t * buf,uint16_t len);
void uartBufPost(UART_RXTX_CTL *uartctl);
void UsartInterruptHandler(UART_HandleTypeDef *huart);
void uartReceiveCompleteCheck(void);

#endif
