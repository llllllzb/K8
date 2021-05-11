#include "app_flash.h"
#include "main.h"
#include "app_common.h"
/*对用户代码区进行擦除*/
int8_t flashEarse(uint32_t flashaddr, uint16_t numofpage)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t errorPageaddr;
    int8_t ret=0;
    if(numofpage>APP_FLASH_PAGE_SIZE)
    {
        return ret;
    }
	LogPrintf("flashEarse==>Addr=0x%X,PageCnt=%d\n",flashaddr,numofpage);
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = flashaddr;
    EraseInitStruct.NbPages     = numofpage;
    ret=1;
    __disable_irq();
    HAL_FLASH_Unlock();
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &errorPageaddr) != HAL_OK)
    {
        ret =-1;
    }
    if(errorPageaddr!=0xFFFFFFFFU)
    {
        ret= -2;
    }
    HAL_FLASH_Lock();
    __enable_irq();
    LogPrintf("flashEarse==>Earse %s\n",ret>0?"Success":"Fail");
    return ret;

}


/*filesize :文件字节大小*/
int8_t flashEarseByFileSize(uint32_t filesize)
{
    uint16_t totalpagenum;
    uint8_t i;

    totalpagenum=filesize/128+1;
    LogPrintf("%s==>%d\n",__FUNCTION__,totalpagenum);
    for(i=0; i<20; i++)
    {
        if(flashEarse(FLASH_USER_START_ADDR,totalpagenum)==1)
        {
            LogMessage("Earse complete\n");
            return 1;
        }
        else
        {
            LogPrintf("Earse Fail ,Re-earse %d\n",i+1);
        }
    }
    return 0;
}

int8_t flashWriteCode(uint32_t addr, uint8_t *code,uint16_t codelen)
{
    uint32_t writedata;
    uint32_t writeaddr;
    uint32_t readdata;
    uint32_t errorcount=0;
    uint16_t i,j;
    //用户起始地址
    writeaddr=addr;
    LogPrintf("FlashWriteCode==>Addr=0x%08X,Size=%d\n",addr,codelen);
    __disable_irq();
    HAL_FLASH_Unlock();

    for(i=0; i<codelen; i++)
    {
        writedata=0;
        writedata=(code[i])|(code[i+1]<<8)|(code[i+2]<<16)|(code[i+3]<<24);
        for(j=0; j<8; j++)
        {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, writeaddr, writedata) == HAL_OK)
            {
                writeaddr = writeaddr + 4;
                i+=3;
                break;
            }
            else
            {
                errorcount++;
            }
        }
        if(j>=8)
        {
            HAL_FLASH_Lock();
            __enable_irq();
			LogPrintf("Write code TimeOut Error,error cnt=%d\n",errorcount);            
            return 0;
        }
    }
    HAL_FLASH_Lock();
    writeaddr=addr;
    for(i=0; i<codelen; i++)
    {
        writedata=0;
        writedata=(code[i])|(code[i+1]<<8)|(code[i+2]<<16)|(code[i+3]<<24);
        i+=3;
        readdata = *(__IO uint32_t *)writeaddr;
        writeaddr = writeaddr + 4;
        if(readdata!=writedata)
        {
            __enable_irq();
            LogMessage("Firmware data valid error\n");
            return 0;
        }
    }
    __enable_irq();
	LogMessage("Firmware writting OK\n");
    return 1;
}




