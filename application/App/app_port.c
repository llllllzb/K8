#include "app_port.h"
#include "app_sys.h"
#include "app_mir3da.h"
#include "app_param.h"
#include "rtc.h"
#include <time.h>
#include "adc.h"
#include "app_task.h"

#include "iwdg.h"
#include <time.h>
#include "app_kernal.h"

UART_RXTX_CTL usart1_ctl;
UART_RXTX_CTL usart2_ctl;
UART_RXTX_CTL usart3_ctl;


uint8_t usart1rxbuf[USART1_RX_BUF_SIZE];
uint8_t usart2rxbuf[USART2_RX_BUF_SIZE];
uint8_t usart3rxbuf[USART3_RX_BUF_SIZE];


/***********************************************************************************************/
//UART
void portUartCfg(UARTTYPE type, uint8_t onoff, void (*rxhandlefun)(uint8_t *, uint16_t))
{
    switch (type)
    {
        case APPUSART1:
            if (onoff)
            {
                MX_USART1_UART_Init();
                usart1_ctl.rxbuf = usart1rxbuf;
                usart1_ctl.uart_handle = &huart1;
                usart1_ctl.rxhandlefun = rxhandlefun;
                HAL_UART_Receive_DMA(&huart1, usart1_ctl.rxbuf, USART1_RX_BUF_SIZE);
                __HAL_UART_CLEAR_IDLEFLAG(&huart1);
                __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
                LogMessage(DEBUG_ALL, "Open Usart1\n");
            }
            else
            {
                LogMessage(DEBUG_ALL, "Close Usart1\n");
                HAL_UART_DeInit(usart1_ctl.uart_handle);
                usart1_ctl.uart_handle = NULL;
                usart1_ctl.rxhandlefun = NULL;
                usart1_ctl.rxcompleteflag = 0;
                usart1_ctl.rxlen = 0;
            }
            break;
        case APPUSART2:
            if (onoff)
            {
                MX_USART2_UART_Init();
                usart2_ctl.rxbuf = usart2rxbuf;
                usart2_ctl.uart_handle = &huart2;
                usart2_ctl.rxhandlefun = rxhandlefun;
                HAL_UART_Receive_DMA(&huart2, usart2_ctl.rxbuf, USART2_RX_BUF_SIZE);
                __HAL_UART_CLEAR_IDLEFLAG(&huart2);
                __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
                LogMessage(DEBUG_ALL, "Open Usart2\n");
            }
            else
            {
                LogMessage(DEBUG_ALL, "Close Usart2\n");
                HAL_UART_DeInit(usart2_ctl.uart_handle);
                usart2_ctl.uart_handle = NULL;
                usart2_ctl.rxhandlefun = NULL;
                usart2_ctl.rxcompleteflag = 0;
                usart2_ctl.rxlen = 0;
            }
            break;
        case APPUSART3:
            if (onoff)
            {
                MX_LPUART1_UART_Init();
                usart3_ctl.rxbuf = usart3rxbuf;
                usart3_ctl.uart_handle = &hlpuart1;
                usart3_ctl.rxhandlefun = rxhandlefun;
                HAL_UART_Receive_DMA(&hlpuart1, usart3_ctl.rxbuf, USART3_RX_BUF_SIZE);
                __HAL_UART_CLEAR_IDLEFLAG(&hlpuart1);
                __HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_IDLE);
                LogMessage(DEBUG_ALL, "Open Lpusart1\n");
            }
            else
            {
                LogMessage(DEBUG_ALL, "Close Lpusart1\n");
                HAL_UART_DeInit(usart3_ctl.uart_handle);
                usart3_ctl.uart_handle = NULL;
                usart3_ctl.rxhandlefun = NULL;
                usart3_ctl.rxcompleteflag = 0;
                usart3_ctl.rxlen = 0;
            }
            break;
    }
}


//将需要发送的数据塞入发送环形队列中
int8_t portUartSend(UART_RXTX_CTL *uartctl, uint8_t *buf, uint16_t len)
{
    if (uartctl->uart_handle != NULL)
    {
        HAL_UART_Transmit(uartctl->uart_handle, buf, len, 1000);
    }
    return 0;
}

//将接收的数据塞入接收环形队列中
static int8_t halUartRecvPushIn(UART_RXTX_CTL *uartctl, uint8_t *buf, uint16_t len)
{
    uartctl->rxcompleteflag = 1;
    uartctl->rxlen = len;
    return 0;
}

//查找循环队列中是否有接收的数据
static void halUartRecvPushOut(UART_RXTX_CTL *uartctl)
{
    if (uartctl->rxcompleteflag == 1)
    {
        uartctl->rxcompleteflag = 0;
        if (uartctl->rxhandlefun != NULL)
        {
            uartctl->rxhandlefun(uartctl->rxbuf, uartctl->rxlen);
        }
    }
}

//在IDLE中断中调用，读取DMA已接收数据大小，并重置DMA数据接收
static void halGetInDma(UART_HandleTypeDef *huart)
{
    uint16_t rxcount;
    if (huart->Instance == USART1)
    {
        rxcount = USART1_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
        if (rxcount > 0)
        {
            //LogPrintf(DEBUG_ALL,"Usart1 Rx %d Bytes\n", rxcount);
            halUartRecvPushIn(&usart1_ctl, usart1_ctl.rxbuf, rxcount);
            HAL_UART_DMAStop(huart);
            HAL_UART_Receive_DMA(huart, usart1_ctl.rxbuf, USART1_RX_BUF_SIZE);
        }
    }
    else if (huart->Instance == USART2)
    {
        rxcount = USART2_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
        if (rxcount > 0)
        {
            //LogPrintf(DEBUG_ALL,"Usart2 Rx %d Bytes\n", rxcount);
            halUartRecvPushIn(&usart2_ctl, usart2_ctl.rxbuf, rxcount);
            HAL_UART_DMAStop(huart);
            HAL_UART_Receive_DMA(huart, usart2_ctl.rxbuf, USART2_RX_BUF_SIZE);
        }
    }
    else if (huart->Instance == LPUART1)
    {
        rxcount = USART3_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
        if (rxcount > 0)
        {
            //LogPrintf(DEBUG_ALL,"LPUsart1 Rx %d Bytes\n", rxcount);
            halUartRecvPushIn(&usart3_ctl, usart3_ctl.rxbuf, rxcount);
            HAL_UART_DMAStop(huart);
            HAL_UART_Receive_DMA(huart, usart3_ctl.rxbuf, USART3_RX_BUF_SIZE);
        }
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        LogPrintf(DEBUG_ALL, "USART1 Error Code=0x%X\n", huart->ErrorCode);
        HAL_UART_Receive_DMA(&huart1, usart1_ctl.rxbuf, USART1_RX_BUF_SIZE);
    }
    else if (huart->Instance == USART2)
    {
        LogPrintf(DEBUG_ALL, "USART2 Error Code=0x%X\n", huart->ErrorCode);
        HAL_UART_Receive_DMA(&huart2, usart2_ctl.rxbuf, USART2_RX_BUF_SIZE);
    }
    else if (huart->Instance == LPUART1)
    {
        LogPrintf(DEBUG_ALL, "LPUSART1 Error Code=0x%X\n", huart->ErrorCode);
        HAL_UART_Receive_DMA(&hlpuart1, usart3_ctl.rxbuf, USART3_RX_BUF_SIZE);
    }
}
//串口处理中断
void halRecvInIdle(UART_HandleTypeDef *huart)
{
    if (__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE))
    {
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        halGetInDma(huart);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        LogMessage(DEBUG_ALL, "USART1 DMARX complete\n");
    }
    else if (huart->Instance == USART2)
    {
        halUartRecvPushIn(&usart2_ctl, usart2_ctl.rxbuf, USART2_RX_BUF_SIZE);
        LogMessage(DEBUG_ALL, "USART2 DMARX complete\n");
    }
    else if (huart->Instance == LPUART1)
    {
        LogMessage(DEBUG_ALL, "LPUSART1 DMARX complete\n");
    }
}

//处理串口接收数据
void portPollUart(void)
{
    halUartRecvPushOut(&usart1_ctl);
    halUartRecvPushOut(&usart2_ctl);
    halUartRecvPushOut(&usart3_ctl);
}
/***********************************************************************************************/
void portGsensorSdaSclCtl(uint8_t onoff)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if (onoff)
	{
		HAL_GPIO_WritePin(GPIOB, SCL_Pin|SDA_Pin, GPIO_PIN_SET);
		GPIO_InitStruct.Pin = SCL_Pin|SDA_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOB, SCL_Pin|SDA_Pin, GPIO_PIN_SET);
		GPIO_InitStruct.Pin = SCL_Pin|SDA_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
}

/*Gsensor 配置*/
void portGsensorCfg(uint8_t onoff)
{
    if (onoff)
    {
        sysinfo.gsensoronoff = 1;
        portGsensorSdaSclCtl(1);
        HAL_Delay(50);
        GSENSORON;
        HAL_Delay(50);
        portGsensorSdaSclCtl(0);
        mir3da_init();
        mir3da_open_interrupt(10);
        mir3da_set_enable(1);
        updateRange(sysparam.vibRange);
        startStep();
        LogMessage(DEBUG_ALL, "portGsensorCfg==>on\n");
    }
    else
    {
        sysinfo.gsensoronoff = 0;
        GSENSOROFF;
        LogMessage(DEBUG_ALL, "portGsensorCfg==>off\n");
    }
}



void portGsensorInt(void)
{
    sysinfo.gsensortapcount++;
}

/***********************************************************************************************/
/*RTC 配置*/

void portGetSystemDateTime(uint16_t *year, uint8_t *month, uint8_t *date, uint8_t *hour, uint8_t *minute,
                           uint8_t *second)
{
    RTC_DateTypeDef sdatestructureget;
    RTC_TimeTypeDef stimestructureget;
    HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN);
    *year = sdatestructureget.Year + 2000;
    *month = sdatestructureget.Month;
    *date = sdatestructureget.Date;

    *hour = stimestructureget.Hours;
    *minute = stimestructureget.Minutes;
    *second = stimestructureget.Seconds;
}

uint32_t portGetDateTimeOfSeconds(void)
{
    uint32_t sec;
    uint16_t year;
    uint8_t month, date, hour, minute, second;
    portGetSystemDateTime(&year, &month, &date, &hour, &minute, &second);
    sec = hour * 3600 + minute * 60 + second;
    return sec;
}

void disPlaySystemTime(void)
{
    char showtime[30];
    uint16_t year;
    uint8_t  month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    portGetSystemDateTime(&year, &month, &date, &hour, &minute, &second);
    sprintf((char *)showtime, "<%.2d-%.2d-%.2d %.2d:%.2d:%.2d>\n", year, month, date, hour, minute, second);
    LogMessage(DEBUG_ALL, showtime);
    if (hour == 0 && minute == 0 && second > 0 && second <= 20 && year >= 2022)
    {
        portClearStep();
    }
}

void portSetSystemDateTime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second)
{
    char debug[100];
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
	uint32_t sec1, sec2;
    uint16_t cyear;
    uint8_t cmonth, cdate, chour, cminute, csecond;
    struct tm datetime;
    portGetSystemDateTime(&cyear, &cmonth, &cdate, &chour, &cminute, &csecond);
    datetime.tm_year = cyear % 100;
    datetime.tm_mon = cmonth;
    datetime.tm_mday = cdate;
    datetime.tm_hour = chour;
    datetime.tm_min = cminute;
    datetime.tm_sec = csecond;
    sec1 = mktime(&datetime);
    //LogPrintf(DEBUG_ALL, "SEC1:%02d/%02d/%02d-%02d:%02d:%02d==>%u\r\n", cyear%100,cmonth,cdate,chour,cminute,csecond ,sec1);
    datetime.tm_year = year % 100;
    datetime.tm_mon = month;
    datetime.tm_mday = date;
    datetime.tm_hour = hour;
    datetime.tm_min = minute;
    datetime.tm_sec = second + 1;
    sec2 = mktime(&datetime);
    //LogPrintf(DEBUG_ALL, "SEC2:%02d/%02d/%02d-%02d:%02d:%02d==>%u\r\n",year%100,month,date,hour,minute,second,sec2);

    if (abs(sec1 - sec2) <= 300)
    {
    	LogMessage(DEBUG_ALL,"no need to update rtc\r\n");
        return;
    }
    sTime.Hours = hour;
    sTime.Minutes =	minute;
    sTime.Seconds = second;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
    sDate.WeekDay = RTC_WEEKDAY_MONDAY;
    sDate.Month = month;
    sDate.Date = date;
    sDate.Year = year;

    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
    sprintf(debug, "portSetSystemDateTime==>%02d/%02d/%02d-%02d:%02d:%02d\n", year + 2000, month, date, hour, minute,
            second);
    LogMessage(DEBUG_ALL, debug);
}

static void updateAlarmA(uint8_t date, uint8_t hour, uint8_t minutes)
{
    RTC_AlarmTypeDef rtc_a;
    char debug[100];
    rtc_a.AlarmTime.Hours = hour;
    rtc_a.AlarmTime.Minutes = minutes;
    rtc_a.AlarmTime.Seconds = 0x0;
    rtc_a.AlarmTime.SubSeconds = 0x0;
    rtc_a.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    rtc_a.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
    rtc_a.AlarmMask = RTC_ALARMMASK_NONE;
    rtc_a.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
    rtc_a.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    rtc_a.AlarmDateWeekDay = date;
    rtc_a.Alarm = RTC_ALARM_A;
    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
    if (HAL_RTC_SetAlarm_IT(&hrtc, &rtc_a, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }
    sprintf(debug, "UpdateAlarmA==>Date:%02d ,Time:%02d:%02d\n", date, hour, minutes);
    LogMessage(DEBUG_ALL, debug);
}

/**
  * @brief  设置下一次时间点
  * @param  None
  * @retval None
  */

int portSetNextAlarmTime(void)
{
    unsigned short  rtc_mins, next_ones;
    unsigned char next_date, set_nextdate = 1;
    uint16_t  YearToday;      /*当前年*/
    uint8_t  MonthToday;     /*当前月*/
    uint8_t  DateToday;      /*当前日*/
    int i;
    RTC_AlarmTypeDef rtc_a;
    uint16_t year;
    uint8_t  month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    portGetSystemDateTime(&year, &month, &date, &hour, &minute, &second);

    //1、读取当前时间点的总分钟数
    HAL_RTC_GetAlarm(&hrtc, &rtc_a, RTC_ALARM_A, RTC_FORMAT_BIN);
    rtc_mins = (hour & 0x1F) * 60;
    rtc_mins += (minute & 0x3f);
    //2、读取当前年月
    YearToday = year; //计算当前年，从2000年开始算起
    MonthToday = month;
    DateToday = date;
    //3、根据当前月，计算下个月日期
    if (MonthToday == 4 || MonthToday == 6 || MonthToday == 9 || MonthToday == 11)
    {
        next_date = (DateToday + sysparam.MODE1_GAP_DAY) % 30; //当前日期加上间隔日，计算下一次的时间点
        if (next_date == 0)
            next_date = 30;
    }
    else if (MonthToday == 2)
    {
        //4、如果是2月，判断是不是闰年
        if (((YearToday % 100 != 0) && (YearToday % 4 == 0)) || (YearToday % 400 == 0))  //闰年
        {
            next_date = (DateToday + sysparam.MODE1_GAP_DAY) % 29;
            if (next_date == 0)
                next_date = 29;
        }
        else
        {
            next_date = (DateToday + sysparam.MODE1_GAP_DAY) % 28;
            if (next_date == 0)
                next_date = 28;
        }
    }
    else
    {
        next_date = (DateToday + sysparam.MODE1_GAP_DAY) % 31;
        if (next_date == 0)
            next_date = 31;
    }
    next_ones = 0xFFFF;
    //5、查找闹铃表里面是否有在当前时间点之后的时间
    for (i = 0; i < 5; i++)
    {
        if (sysparam.AlarmTime[i] == 0xFFFF)
            continue;
        if (sysparam.AlarmTime[i] > rtc_mins)   //跟当前时间比对
        {
            next_ones = sysparam.AlarmTime[i];  //得到新的时间
            set_nextdate = 0;
            break;
        }
    }


    if (next_ones == 0xFFFF)  //没有配置时间
    {
        //Set Current Alarm Time
        next_ones = sysparam.AlarmTime[0];
        if (next_ones == 0xFFFF)
        {
            next_ones = 720; //默认12:00
            sysparam.AlarmTime[0] = 720;
        }
    }
    //6、设置下次上报的日期和时间
    if (set_nextdate)
        updateAlarmA(next_date, (next_ones / 60) % 24, next_ones % 60);
    else
        updateAlarmA(date, (next_ones / 60) % 24, next_ones % 60);
    return 0;
}


void portSetNextWakeUpTime(void)
{
    uint16_t  YearToday;      /*当前年*/
    uint8_t  MonthToday;     /*当前月*/
    uint8_t  DateToday;      /*当前日*/
    uint8_t  date = 0, next_date;
    uint16_t totalminutes;
    uint16_t year;
    uint8_t  month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    portGetSystemDateTime(&year, &month, &day, &hour, &minute, &second);
    YearToday = year; //计算当前年，从2000年开始算起
    MonthToday = month;
    DateToday = day;
    totalminutes = hour * 60 + minute;
    totalminutes += sysparam.gapMinutes;
    if (totalminutes >= 1440)
    {
        date = 1;
        totalminutes -= 1440;
    }
    //3、根据当前月，计算下个月日期
    if (MonthToday == 4 || MonthToday == 6 || MonthToday == 9 || MonthToday == 11)
    {
        next_date = (DateToday + date) % 30; //当前日期加上间隔日，计算下一次的时间点
        if (next_date == 0)
            next_date = 30;
    }
    else if (MonthToday == 2)
    {
        //4、如果是2月，判断是不是闰年
        if (((YearToday % 100 != 0) && (YearToday % 4 == 0)) || (YearToday % 400 == 0))  //闰年
        {
            next_date = (DateToday + date) % 29;
            if (next_date == 0)
                next_date = 29;
        }
        else
        {
            next_date = (DateToday + date) % 28;
            if (next_date == 0)
                next_date = 28;
        }
    }
    else
    {
        next_date = (DateToday + date) % 31;
        if (next_date == 0)
            next_date = 31;
    }
    updateAlarmA(next_date, (totalminutes / 60) % 24, totalminutes % 60);
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
    systemRequestSet(SYSTEM_WDT_REQUEST);
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
    LogMessage(DEBUG_ALL, "Rtc WakeUp==>Alarm Complete!\n");
    if (sysparam.MODE == MODE2 || sysparam.MODE == MODE4)
    {
        return;
    }


    if (sysparam.MODE == MODE3 || sysparam.MODE == MODE23)
    {
        portSetNextWakeUpTime();
    }
    else
    {
        portSetNextAlarmTime();
    }
    systemModeRunStart();

}

/***********************************************************************************************/
/*感光*/
void portLdrInt(void)
{
    static uint32_t lastTick = 0;
    static uint8_t  ldrLastState = 1;
    uint8_t curldr;
    uint32_t currenttick;
    curldr = LDRDET;
    LogPrintf(DEBUG_ALL, "LDR = %s\n", curldr ? "暗" : "亮");
    currenttick = portGetDateTimeOfSeconds();
    if (curldr == 0 && ldrLastState == 1 && sysparam.Light_Alarm_En == 1)
    {
        if (currenttick - lastTick >= 60)
        {
            LogMessage(DEBUG_ALL, "Light Alarm\n");
            alarmRequestSet(ALARM_LIGHT_REQUEST);
            systemModeRunStart();
        }
    }
    ldrLastState = curldr;
    lastTick = currenttick;
}


/***********************************************************************************************/
/*adc引脚配置*/
uint32_t portGetAdc(void)
{
    uint32_t adcvalue;
    //MX_ADC_Init();

    if (HAL_ADC_Start(&hadc) != HAL_OK)
    {
        Error_Handler();
    }
    HAL_ADC_PollForConversion(&hadc, 10);
    if ((HAL_ADC_GetState(&hadc) & HAL_ADC_STATE_REG_EOC) == HAL_ADC_STATE_REG_EOC)
    {
        adcvalue = HAL_ADC_GetValue(&hadc);
    }
    HAL_ADC_Stop(&hadc);
    //HAL_ADC_DeInit(&hadc);
    return adcvalue;
}

/***********************************************************************************************/
/*模组引脚配置*/
void portModuleCfg(uint8_t onoff)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    if (onoff)
    {
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;

        //推挽输出
        GPIO_InitStruct.Pin = PWREN_Pin;
        HAL_GPIO_Init(PWREN_GPIO_Port, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = DTR_Pin;
        HAL_GPIO_Init(DTR_GPIO_Port, &GPIO_InitStruct);

        //RST 开漏输出
        GPIO_InitStruct.Pin = RSTKEY_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        HAL_GPIO_Init(RSTKEY_GPIO_Port, &GPIO_InitStruct);
        LogMessage(DEBUG_ALL, "portModuleCfg==>on\n");

    }
    else
    {
        //设为输入
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pin = DTR_Pin;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(DTR_GPIO_Port, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = RSTKEY_Pin;
        HAL_GPIO_Init(RSTKEY_GPIO_Port, &GPIO_InitStruct);
        LogMessage(DEBUG_ALL, "portModuleCfg==>off\n");
    }
}

/***********************************************************************************************/
/*低功耗配置*/
void portLowPowerCfg(void)
{
    HAL_PWREx_EnableUltraLowPower();
    HAL_PWREx_EnableFastWakeUp();
    __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);
}
/***********************************************************************************************/
/*WDT喂狗*/
void portWdtFeed(void)
{
    HAL_IWDG_Refresh(&hiwdg);
}

/***********************************************************************************************/

int8_t ringTimerId = -1;

static void clearRingWakeUp(void)
{
    ringTimerId = -1;
    sysinfo.ringWakeUp = 0;
}


void portRingSignal(void)
{
    sysinfo.ringWakeUp = 1;
    if (ringTimerId > 0)
    {
        stopTimer(ringTimerId);
    }
    ringTimerId = startTimer(6000, clearRingWakeUp, 0);
    LogPrintf(DEBUG_ALL, "Ring 信号唤醒\r\n");
}

void portUpdateStep(void)
{
    sysinfo.step = getStep() + sysparam.step;
}

void portClearStep(void)
{
    if (sysinfo.gsensoronoff == 0)
        return;
    if (sysparam.step != 0 || sysinfo.step != 0)
    {
        sysinfo.step = 0;
        sysparam.step = 0;
        paramSaveStep();
        clearStep();
    }
}

void portSaveStep(void)
{
    portUpdateStep();
    sysparam.step = sysinfo.step;
    paramSaveStep();
    LogPrintf(DEBUG_ALL, "Save last step %d\r\n", sysparam.step);
}

void portSystemReset(void)
{
    portSaveStep();
    HAL_NVIC_SystemReset();
    LogMessage(DEBUG_ALL, "portSystemReset\r\n");
}



/* Private variables----------------------------------------------------------*/
void Init(void);  //I2C初始化
void Start(void); //I2C起始信号
void Stop(void);  //I2C停止信号
ACK_Value_t Write_Byte(uint8_t);      //I2C写字节
uint8_t     Read_Byte (ACK_Value_t);  //I2C读字节

/* Public variables-----------------------------------------------------------*/
I2C_Soft_t I2C_Soft = 
{
	Init,
	Start,
	Stop,
	Write_Byte,
	Read_Byte
};

/* Private function prototypes------------------------------------------------*/      
static void I2C_Delay_us(uint8_t);

/*
	* @name   Init
	* @brief  I2C初始化
	* @param  None
	* @retval None      
*/
void Init(void)
{
	SET_SCL;
	SET_SDA;
}

/*
	* @name   Start
	* @brief  I2C起始信号
	* @param  None
	* @retval None      
*/
static void Start(void)
{
	//SCL为高电平，SDA的下降沿为I2C起始信号
	SET_SDA;
	SET_SCL;
	I2C_Delay_us(1);
	
	CLR_SDA;
	I2C_Delay_us(10);
	
	CLR_SCL;
	I2C_Delay_us(1);
}

/*
	* @name   Stop
	* @brief  I2C停止信号
	* @param  None
	* @retval None      
*/
static void Stop(void)
{
	//SCL为高电平，SDA的上升沿为I2C停止信号
	CLR_SDA;
	SET_SCL;
	I2C_Delay_us(1);
		
	I2C_Delay_us(10);
	SET_SDA;
}

/*
	* @name   Write_Byte
	* @brief  I2C写字节
	* @param  WR_Byte -> 待写入数据
	* @retval ACK_Value_t -> 从机应答值      
*/
static ACK_Value_t Write_Byte(uint8_t WR_Byte)
{
	uint8_t i;
	ACK_Value_t  ACK_Rspond;
	
	//SCL为低电平时，SDA准备数据,接着SCL为高电平，读取SDA数据
	//数据按8位传输，高位在前，利用for循环逐个接收
	for(i=0;i<8;i++)
	{
		//SCL清零，主机SDA准备数据
		CLR_SCL;
		I2C_Delay_us(1);
		if((WR_Byte&BIT7) == BIT7)
		{
			SET_SDA;
		}
		else
		{
			CLR_SDA;
		}
		I2C_Delay_us(1);
		//SCL置高，传输数据
		SET_SCL;
		I2C_Delay_us(10);
		
		//准备发送下一比特位
		WR_Byte <<= 1;
	}
	
	CLR_SCL;	
	//释放SDA，等待从机应答
	SET_SDA;
	I2C_Delay_us(1);
	
	SET_SCL;
	I2C_Delay_us(10);
	
	ACK_Rspond = (ACK_Value_t)READ_SDA;
	
	CLR_SCL;
	I2C_Delay_us(1);
	
	//返回从机的应答信号
	return ACK_Rspond;
}

/*
	* @name   Write_Byte
	* @brief  I2C写字节
	* @param  ACK_Value -> 主机回应值
	* @retval 从机返回值      
*/
static uint8_t Read_Byte(ACK_Value_t ACK_Value)
{
	uint8_t RD_Byte = 0,i;
		
	////接收数据
	//SCL为低电平时，SDA准备数据,接着SCL为高电平，读取SDA数据
	//数据按8位传输，高位在前，利用for循环逐个接收
	for(i=0;i<8;i++)
	{
		//准备接收下一比特位
		RD_Byte <<= 1;
		
		//SCL清零，从机SDA准备数据
		CLR_SCL;
		I2C_Delay_us(10);
		
		//SCL置高，获取数据
		SET_SCL;
		I2C_Delay_us(10);	

		RD_Byte |= READ_SDA;		
	}
	
	
	//SCL清零，主机准备应答信号
	CLR_SCL;
	I2C_Delay_us(1);
	
	//主机发送应答信号	
	if(ACK_Value == ACK)
	{
		CLR_SDA;
	}
	else
	{
		SET_SDA;	
  }	
	I2C_Delay_us(1);
	
	
	SET_SCL; 	
	I2C_Delay_us(10);
	
	//Note:
  //释放SDA数据线
	//SCL先清零，再释放SDA，防止连续传输数据时，从机错将SDA释放信号当成NACk信号
	CLR_SCL;
  SET_SDA; 	
	I2C_Delay_us(1);

	//返回数据
	return RD_Byte;
}

/**
 * @brief   IIC 读取多个字节
 * @param
 * @return
 */
uint8_t iicReadData(uint8_t addr, uint8_t regaddr, uint8_t *data, uint8_t len)
{
    uint8_t i, ret = 0;
    if (data == NULL)
        return ret;
    Start();
    addr &= ~0x01;
    ret = Write_Byte(addr);
    Write_Byte(regaddr);
    Start();
    addr |= 0x01;
    Write_Byte(addr);
    for (i = 0; i < len; i++)
    {
        if (i == (len - 1))
        {
            data[i] = Read_Byte(ACK);
        }
        else
        {
            data[i] = Read_Byte(NACK);
        }
    }
    Stop();
    return ret;
}
/**
 * @brief   IIC 写多个字节
 * @param
 * @return
 */
uint8_t iicWriteData(uint8_t addr, uint8_t reg, uint8_t data)
{
    uint8_t ret = 0;
    Start();
    addr &= ~0x01;
    ret = Write_Byte(addr);
    Write_Byte(reg);
    Write_Byte(data);
    Stop();
    return ret;
}


/*
	* @name   I2C_Delay
	* @brief  I2C延时
	* @param  None
	* @retval None      
*/
static void I2C_Delay_us(uint8_t us)
{
    uint32_t Delay = us * 168/4;
    do
    {
        __NOP();
    }
    while (Delay --);

}

