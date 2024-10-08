#include "app_uart.h"



uint8_t usart1rxdmabuf[USART1_RX_DMA_SIZE];
uint8_t usart2rxdmabuf[USART2_RX_DMA_SIZE];

UART_RXTX_CTL usart1_ctl, usart2_ctl;

void debuguartctrl(uint8_t *buf, uint16_t len)
{
    if(strstr((char *)buf, "AT^DEBUG=9") != NULL)
    {
        system_info.logmessagectl = 9;
        LogMessage("DEBUG OK\n");
    }
}

/*初始化串口缓冲区*/
void UsartBufInit(void)
{
    usart1_ctl.dmarxbuf = usart1rxdmabuf;
    usart1_ctl.dmarxlen = 0;
    usart1_ctl.rxhandlefun = debuguartctrl;
    usart1_ctl.uart_handle = &huart1;
    usart1_ctl.receiveComplete = 0;

    usart2_ctl.dmarxbuf = usart2rxdmabuf;
    usart2_ctl.dmarxlen = 0;
    usart2_ctl.rxhandlefun = n58responparase;
    usart2_ctl.uart_handle = &huart2;
    usart2_ctl.receiveComplete = 0;
}
void uartReceiveCompleteCheck(void)
{
    if(usart1_ctl.receiveComplete == 1)
    {
        usart1_ctl.receiveComplete = 0;

        if(usart1_ctl.rxhandlefun != NULL)
        {
            LogMessageWL((char *)usart1_ctl.dmarxbuf, usart1_ctl.dmarxlen);
            usart1_ctl.rxhandlefun(usart1_ctl.dmarxbuf, usart1_ctl.dmarxlen);
        }
    }
    if(usart2_ctl.receiveComplete == 1)
    {
        usart2_ctl.receiveComplete = 0;
        if(usart2_ctl.rxhandlefun != NULL)
        {
            usart2_ctl.rxhandlefun(usart2_ctl.dmarxbuf, usart2_ctl.dmarxlen);
        }
    }
}
//在IDLE中断中调用，读取DMA已接收数据大小，并重置DMA数据接收
static void UsartGetDataFromDMA(UART_HandleTypeDef *huart)
{
    uint16_t rxcount;
    if(huart->Instance == USART1)
    {
        rxcount = USART1_RX_DMA_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
        if(rxcount > 0)
        {
            usart1_ctl.receiveComplete = 1;
            usart1_ctl.dmarxlen = rxcount;
            HAL_UART_DMAStop(huart);
            HAL_UART_Receive_DMA(huart, usart1_ctl.dmarxbuf, USART1_RX_DMA_SIZE);
        }
    }
    else if(huart->Instance == USART2)
    {
        rxcount = USART2_RX_DMA_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
        if(rxcount > 0)
        {
            usart2_ctl.receiveComplete = 1;
            usart2_ctl.dmarxlen = rxcount;

            HAL_UART_DMAStop(huart);
            HAL_UART_Receive_DMA(huart, usart2_ctl.dmarxbuf, USART2_RX_DMA_SIZE);
        }
    }
}

/*串口中断调用，处理其他事件*/
void UsartInterruptHandler(UART_HandleTypeDef *huart)
{
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE))
    {
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        UsartGetDataFromDMA(huart);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        //LogMessageNoBlock("USART1 Tx Complete\n");
    }
    else if(huart->Instance == USART2)
    {
        //LogMessageNoBlock("USART2 Tx Complete\n");
    }
    else if(huart->Instance == LPUART1)
    {
        //LogMessageNoBlock("LPUSART1 Tx Complete\n");
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    char debug[50];
    if(huart->Instance == USART1)
    {
        HAL_UART_Receive_DMA(&huart1, usart1_ctl.dmarxbuf, USART1_RX_DMA_SIZE);
        sprintf(debug, "USART1 ERROR CODE =%d\n", huart->ErrorCode);
        LogMessageNoBlock(debug);
    }
    else if(huart->Instance == USART2)
    {
        HAL_UART_Receive_DMA(&huart2, usart2_ctl.dmarxbuf, USART2_RX_DMA_SIZE);
        sprintf(debug, "USART2 ERROR CODE =%d\n", huart->ErrorCode);
        LogMessageNoBlock(debug);
    }

}
