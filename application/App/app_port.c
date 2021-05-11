#include "app_port.h"
#include "app_sys.h"
#include "app_mir3da.h"
#include "app_param.h"
#include "rtc.h"
#include <time.h>
#include "adc.h"
#include "app_task.h"
#include "i2c.h"
#include "iwdg.h"

UART_RXTX_CTL usart1_ctl;
UART_RXTX_CTL usart2_ctl;
UART_RXTX_CTL usart3_ctl;


uint8_t usart1rxbuf[USART1_RX_BUF_SIZE];
uint8_t usart2rxbuf[USART2_RX_BUF_SIZE];
uint8_t usart3rxbuf[USART3_RX_BUF_SIZE];


/***********************************************************************************************/
//UART
void appUartConfig(UARTTYPE type, uint8_t onoff, void (*rxhandlefun)(uint8_t *, uint16_t len))
{
    switch(type)
    {
    case APPUSART1:
        if(onoff)
        {
            MX_USART1_UART_Init();
            usart1_ctl.rxbuf = usart1rxbuf;
            usart1_ctl.uart_handle = &huart1;
            usart1_ctl.rxhandlefun = rxhandlefun;
            HAL_UART_Receive_DMA(&huart1, usart1_ctl.rxbuf, USART1_RX_BUF_SIZE);
            __HAL_UART_CLEAR_IDLEFLAG(&huart1);
            __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
            LogMessage(DEBUG_ALL,"Open Usart1\n");
        }
        else
        {
            LogMessage(DEBUG_ALL,"Close Usart1\n");
            HAL_UART_DeInit(usart1_ctl.uart_handle);
            usart1_ctl.uart_handle = NULL;
            usart1_ctl.rxhandlefun = NULL;
            usart1_ctl.rxcompleteflag = 0;
            usart1_ctl.rxlen = 0;
        }
        break;
    case APPUSART2:
        if(onoff)
        {
            MX_USART2_UART_Init();
            usart2_ctl.rxbuf = usart2rxbuf;
            usart2_ctl.uart_handle = &huart2;
            usart2_ctl.rxhandlefun = rxhandlefun;
            HAL_UART_Receive_DMA(&huart2, usart2_ctl.rxbuf, USART2_RX_BUF_SIZE);
            __HAL_UART_CLEAR_IDLEFLAG(&huart2);
            __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);
            LogMessage(DEBUG_ALL,"Open Usart2\n");
        }
        else
        {
            LogMessage(DEBUG_ALL,"Close Usart2\n");
            HAL_UART_DeInit(usart2_ctl.uart_handle);
            usart2_ctl.uart_handle = NULL;
            usart2_ctl.rxhandlefun = NULL;
            usart2_ctl.rxcompleteflag = 0;
            usart2_ctl.rxlen = 0;
        }
        break;
    case APPUSART3:
        if(onoff)
        {
            MX_LPUART1_UART_Init();
            usart3_ctl.rxbuf = usart3rxbuf;
            usart3_ctl.uart_handle = &hlpuart1;
            usart3_ctl.rxhandlefun = rxhandlefun;
            HAL_UART_Receive_DMA(&hlpuart1, usart3_ctl.rxbuf, USART3_RX_BUF_SIZE);
            __HAL_UART_CLEAR_IDLEFLAG(&hlpuart1);
            __HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_IDLE);
            LogMessage(DEBUG_ALL,"Open Lpusart1\n");
        }
        else
        {
            LogMessage(DEBUG_ALL,"Close Lpusart1\n");
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
int8_t appUartSend(UART_RXTX_CTL *uartctl, uint8_t * buf, uint16_t len)
{
    if(uartctl->uart_handle!=NULL)
    {
        HAL_UART_Transmit(uartctl->uart_handle, buf, len, 1000);
    }
    return 0;
}

//将接收的数据塞入接收环形队列中
static int8_t UsartReceiveDataPush(UART_RXTX_CTL *uartctl, uint8_t * buf, uint16_t len)
{
    uartctl->rxcompleteflag = 1;
    uartctl->rxlen = len;
    return 0;
}

//查找循环队列中是否有接收的数据
static void UsartReceiveDataPost(UART_RXTX_CTL *uartctl)
{
    if(uartctl->rxcompleteflag == 1)
    {
        uartctl->rxcompleteflag = 0;
        if(uartctl->rxhandlefun != NULL)
        {
            uartctl->rxhandlefun(uartctl->rxbuf, uartctl->rxlen);
        }
    }
}

//在IDLE中断中调用，读取DMA已接收数据大小，并重置DMA数据接收
static void usartGetDataFromDma(UART_HandleTypeDef *huart)
{
    uint16_t rxcount;
    if(huart->Instance == USART1)
    {
        rxcount = USART1_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
        if(rxcount > 0)
        {
            //LogPrintf(DEBUG_ALL,"Usart1 Rx %d Bytes\n", rxcount);
            UsartReceiveDataPush(&usart1_ctl, usart1_ctl.rxbuf, rxcount);
            HAL_UART_DMAStop(huart);
            HAL_UART_Receive_DMA(huart, usart1_ctl.rxbuf, USART1_RX_BUF_SIZE);
        }
    }
    else if(huart->Instance == USART2)
    {
        rxcount = USART2_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
        if(rxcount > 0)
        {
            //LogPrintf(DEBUG_ALL,"Usart2 Rx %d Bytes\n", rxcount);
            UsartReceiveDataPush(&usart2_ctl, usart2_ctl.rxbuf, rxcount);
            HAL_UART_DMAStop(huart);
            HAL_UART_Receive_DMA(huart, usart2_ctl.rxbuf, USART2_RX_BUF_SIZE);
        }
    }
    else if(huart->Instance == LPUART1)
    {
        rxcount = USART3_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
        if(rxcount > 0)
        {
            //LogPrintf(DEBUG_ALL,"LPUsart1 Rx %d Bytes\n", rxcount);
            UsartReceiveDataPush(&usart3_ctl, usart3_ctl.rxbuf, rxcount);
            HAL_UART_DMAStop(huart);
            HAL_UART_Receive_DMA(huart, usart3_ctl.rxbuf, USART3_RX_BUF_SIZE);
        }
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        LogPrintf(DEBUG_ALL,"USART1 Error Code=0x%X\n", huart->ErrorCode);
        HAL_UART_Receive_DMA(&huart1, usart1_ctl.rxbuf, USART1_RX_BUF_SIZE);
    }
    else if(huart->Instance == USART2)
    {
        LogPrintf(DEBUG_ALL,"USART2 Error Code=0x%X\n", huart->ErrorCode);
        HAL_UART_Receive_DMA(&huart2, usart2_ctl.rxbuf, USART2_RX_BUF_SIZE);
    }
    else if(huart->Instance == LPUART1)
    {
        LogPrintf(DEBUG_ALL,"LPUSART1 Error Code=0x%X\n", huart->ErrorCode);
        HAL_UART_Receive_DMA(&hlpuart1, usart3_ctl.rxbuf, USART3_RX_BUF_SIZE);
    }
}
//串口处理中断
void UsartInterruptHandler(UART_HandleTypeDef *huart)
{
    if(__HAL_UART_GET_FLAG(huart, UART_FLAG_IDLE))
    {
        __HAL_UART_CLEAR_IDLEFLAG(huart);
        usartGetDataFromDma(huart);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance==USART1)
    {
        LogMessage(DEBUG_ALL,"USART1 DMARX complete\n");
    }
    else if(huart->Instance==USART2)
    {
        LogMessage(DEBUG_ALL,"USART2 DMARX complete\n");
    }
    else if(huart->Instance==LPUART1)
    {
        LogMessage(DEBUG_ALL,"LPUSART1 DMARX complete\n");
    }
}

//处理串口接收数据
void pollUartData(void)
{
    UsartReceiveDataPost(&usart1_ctl);
    UsartReceiveDataPost(&usart2_ctl);
    UsartReceiveDataPost(&usart3_ctl);
}
/***********************************************************************************************/
/*Gsensor 配置*/
void gsensorConfig(uint8_t onoff)
{
    if(onoff)
    {
        sysinfo.gsensoronoff=1;
        MX_I2C1_Init();
        GSENSORON;
        mir3da_init();
        mir3da_open_interrupt(10);
        mir3da_set_enable(1);
        LogMessage(DEBUG_ALL,"gsensorConfig==>on\n");
    }
    else
    {
        sysinfo.gsensoronoff=0;
        GSENSOROFF;
        HAL_I2C_DeInit(&hi2c1);
        LogMessage(DEBUG_ALL,"gsensorConfig==>off\n");
    }
}

void gsensorInterrupt(void)
{
    sysinfo.gsensortapcount++;
}

/***********************************************************************************************/
/*RTC 配置*/

void getRtcDateTime(uint16_t * year,uint8_t * month,uint8_t * date,uint8_t * hour,uint8_t * minute,uint8_t * second)
{
    RTC_DateTypeDef sdatestructureget;
    RTC_TimeTypeDef stimestructureget;
    HAL_RTC_GetTime(&hrtc, &stimestructureget, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sdatestructureget, RTC_FORMAT_BIN);
    *year=sdatestructureget.Year+2000;
    *month=sdatestructureget.Month;
    *date=sdatestructureget.Date;

    *hour=stimestructureget.Hours;
    *minute=stimestructureget.Minutes;
    *second=stimestructureget.Seconds;
}

uint32_t getCurrentDateTimeOfSec(void)
{
    uint32_t sec;
    uint16_t year;
    uint8_t month,date,hour,minute,second;
    struct tm datetime;
    getRtcDateTime(&year,&month,&date,&hour,&minute,&second);
    datetime.tm_year=year;
    datetime.tm_mon=month;
    datetime.tm_mday=date;
    datetime.tm_hour=hour;
    datetime.tm_min=minute;
    datetime.tm_sec=second;
    sec=mktime(&datetime);
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
    if(sysinfo.logmessage == DEBUG_ALL)
    {
        getRtcDateTime(&year,&month,&date,&hour,&minute,&second);
        sprintf((char*)showtime, "<%.2d-%.2d-%.2d %.2d:%.2d:%.2d>\n", year,month,date,hour,minute,second);
        LogMessage(DEBUG_ALL,showtime);
    }
}

void updateRTCdatetime(uint8_t year, uint8_t month, uint8_t date, uint8_t hour, uint8_t minute, uint8_t second)
{
    char debug[100];
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
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
    sprintf(debug, "updateRTCdatetime==>%02d/%02d/%02d-%02d:%02d:%02d\n", year + 2000, month, date, hour, minute, second);
    LogMessage(DEBUG_ALL,debug);
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
    LogMessage(DEBUG_ALL,debug);
}

/**
  * @brief  设置下一次时间点
  * @param  None
  * @retval None
  */

int setNextAlarmTime( void )
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

    getRtcDateTime(&year,&month,&date,&hour,&minute,&second);

    //1、读取当前时间点的总分钟数
    HAL_RTC_GetAlarm(&hrtc, &rtc_a, RTC_ALARM_A, RTC_FORMAT_BIN);
    rtc_mins = (hour & 0x1F) * 60;
    rtc_mins += (minute & 0x3f);
    //2、读取当前年月
    YearToday = year; //计算当前年，从2000年开始算起
    MonthToday = month;
    DateToday = date;
    //3、根据当前月，计算下个月日期
    if(MonthToday == 4 || MonthToday == 6 || MonthToday == 9 || MonthToday == 11)
    {
        next_date = (DateToday + sysparam.MODE1_GAP_DAY) % 30; //当前日期加上间隔日，计算下一次的时间点
        if(next_date == 0)
            next_date = 30;
    }
    else if(MonthToday == 2)
    {
        //4、如果是2月，判断是不是闰年
        if (((YearToday % 100 != 0) && (YearToday % 4 == 0)) || ( YearToday % 400 == 0)) //闰年
        {
            next_date = (DateToday + sysparam.MODE1_GAP_DAY) % 29;
            if(next_date == 0)
                next_date = 29;
        }
        else
        {
            next_date = (DateToday + sysparam.MODE1_GAP_DAY) % 28;
            if(next_date == 0)
                next_date = 28;
        }
    }
    else
    {
        next_date = (DateToday + sysparam.MODE1_GAP_DAY) % 31;
        if(next_date == 0)
            next_date = 31;
    }
    next_ones = 0xFFFF;
    //5、查找闹铃表里面是否有在当前时间点之后的时间
    for ( i = 0; i < 5; i++)
    {
        if ( sysparam.AlarmTime[i] == 0xFFFF )
            continue;
        if ( sysparam.AlarmTime[i] > rtc_mins ) //跟当前时间比对
        {
            next_ones = sysparam.AlarmTime[i];  //得到新的时间
            set_nextdate=0;
            break;
        }
    }


    if ( next_ones == 0xFFFF )//没有配置时间
    {
        //Set Current Alarm Time
        next_ones = sysparam.AlarmTime[0];
        if ( next_ones == 0xFFFF)
        {
            next_ones = 720; //默认12:00
        }
    }
    //6、设置下次上报的日期和时间
    if(set_nextdate)
        updateAlarmA(next_date, (next_ones / 60) % 24, next_ones % 60);
    else
        updateAlarmA(date, (next_ones / 60) % 24, next_ones % 60);
    return 0;
}


void setNextWakeUpTime(void)
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

    getRtcDateTime(&year,&month,&day,&hour,&minute,&second);
    YearToday = year; //计算当前年，从2000年开始算起
    MonthToday = month;
    DateToday = day;
    totalminutes = hour * 60 + minute;
    totalminutes += sysparam.interval_wakeup_minutes;
    if(totalminutes >= 1440)
    {
        date = 1;
        totalminutes -= 1440;
    }
    //3、根据当前月，计算下个月日期
    if(MonthToday == 4 || MonthToday == 6 || MonthToday == 9 || MonthToday == 11)
    {
        next_date = (DateToday + date) % 30; //当前日期加上间隔日，计算下一次的时间点
        if(next_date == 0)
            next_date = 30;
    }
    else if(MonthToday == 2)
    {
        //4、如果是2月，判断是不是闰年
        if (((YearToday % 100 != 0) && (YearToday % 4 == 0)) || ( YearToday % 400 == 0)) //闰年
        {
            next_date = (DateToday + date) % 29;
            if(next_date == 0)
                next_date = 29;
        }
        else
        {
            next_date = (DateToday + date) % 28;
            if(next_date == 0)
                next_date = 28;
        }
    }
    else
    {
        next_date = (DateToday + date) % 31;
        if(next_date == 0)
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
    char buf[100];
    sprintf(buf, "%s", "HAL_RTC_AlarmAEventCallback==>Alarm Complete!\n");
    LogMessage(DEBUG_ALL,buf);

    if(sysparam.MODE != MODE2 && sysparam.MODE!=MODE5 && sysparam.MODE!=MODE4)
    {
        if(sysparam.MODE == MODE3|| sysparam.MODE == MODE23)
        {
            setNextWakeUpTime();
        }
        else
        {
            setNextAlarmTime();
        }
        systemModeRunStart();
    }

}

/***********************************************************************************************/
/*感光*/
void ldrInterrupt(void)
{
    static uint32_t lastTick=0;
    static uint8_t  ldrLastState = 1;
    uint8_t curldr;
    uint32_t currenttick;
    curldr = LDRDET;
    LogPrintf(DEBUG_ALL,"LDR = %s\n",curldr?"暗":"亮");
    currenttick=getCurrentDateTimeOfSec();
    //LogPrintf(DEBUG_ALL,"alarm check,current tick=%ld,lasttick=%ld,%d\n",currenttick,lastTick,currenttick-lastTick);
    if(curldr == 0 && ldrLastState == 1 && sysparam.Light_Alarm_En == 1)
    {
        if( currenttick- lastTick >= 60)
        {
            LogMessage(DEBUG_ALL,"Light Alarm\n");
            alarmRequestSet(ALARM_LIGHT_REQUEST);
            systemModeRunStart();
        }
    }
    ldrLastState = curldr;
    lastTick = currenttick;
}


/***********************************************************************************************/
/*adc引脚配置*/
uint32_t getVoltageAdcValue(void)
{
    uint32_t adcvalue;
    //MX_ADC_Init();

    if(HAL_ADC_Start(&hadc) != HAL_OK)
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
void modulePinConfig(uint8_t onoff)
{

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    if(onoff)
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
        LogMessage(DEBUG_ALL,"modulePinConfig==>on\n");

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
        LogMessage(DEBUG_ALL,"modulePinConfig==>off\n");
    }
}

/***********************************************************************************************/
/*低功耗配置*/
void lowPowerConfig(void)
{
    HAL_PWREx_EnableUltraLowPower();
    HAL_PWREx_EnableFastWakeUp();
    __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);
}
/***********************************************************************************************/
/*WDT喂狗*/
void feedWdt(void)
{
    HAL_IWDG_Refresh(&hiwdg);
}

/***********************************************************************************************/
void moduleRiSignal(void)
{
    LogPrintf(DEBUG_ALL,"RI 信号唤醒\n");
}


