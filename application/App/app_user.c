#include "app_user.h"
#include "app_sys.h"
#include "app_kernal.h"
#include "app_port.h"
#include "app_param.h"
#include "app_net.h"
#include "app_task.h"
#include "app_rec.h"
#include "app_atcmd.h"
#include "adc.h"
/**************************/
void myAppConfig(void)
{
    portUartCfg(APPUSART1, 1, atCmdParserFunction);
    paramInit();
    portLowPowerCfg();
    createSystemTask(ledRunTask, 100);
    createSystemTask(outPutNodeCmd, 200);
    sysinfo.SystaskID = createSystemTask(taskRunInOneSecond, 1000);
    HAL_ADCEx_Calibration_Start(&hadc, ADC_SINGLE_ENDED);
}

void myAppRun(void)
{
    kernalRun();
    portPollUart();
    systemRequestTask();
    alarmUploadRequest();
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GSINT_Pin)
    {
        portGsensorInt();
    }
    else if (GPIO_Pin == LDR_Pin)
    {
        portLdrInt();
    }
    else if (GPIO_Pin == RI_Pin)
    {
        portRingSignal();
    }
}


