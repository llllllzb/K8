#include "app_update.h"

NetWorkConnectStruct netconnect;
static UndateInfoStruct uis;


/*生成协议头部
dest：数据存放区
Protocol_type：协议类型
*/
static int createProtocol7878Head(char *dest, unsigned char Protocol_type)
{
    int pdu_len;
    pdu_len = 0;
    if ( dest == NULL )
    {
        return 0;
    }
    dest[pdu_len++] = 0x78;
    dest[pdu_len++] = 0x78;
    dest[pdu_len++] =0;
    dest[pdu_len++] = Protocol_type;
    return pdu_len;
}

static int createProtocol7979Head(char *dest, unsigned char Protocol_type)
{
    int pdu_len;

    pdu_len = 0;

    if ( dest == NULL )
    {
        return 0;
    }

    dest[pdu_len++] = 0x79;
    dest[pdu_len++] = 0x79;
    /* len = 0 First */
    dest[pdu_len++] =0;
    dest[pdu_len++] =0;

    dest[pdu_len++] = Protocol_type;
    return pdu_len;
}

/*生成协议尾部
dest：数据存放区
h_b_len：前面数据长度
serial_no：序列号
*/
static int createProtocol7878Tail(char *dest, int h_b_len, int serial_no)
{
    int pdu_len;
    unsigned short crc_16;

    if ( dest == NULL )
    {
        return 0;
    }
    pdu_len = h_b_len;
    dest[pdu_len++] = (serial_no >> 8 ) & 0xff;
    dest[pdu_len++] = serial_no & 0xff;
    dest[2] = pdu_len-1;
    /* Caculate Crc */
    crc_16 = GetCrc16(dest+2, pdu_len-2);
    dest[pdu_len++] = (crc_16 >>8)& 0xff;
    dest[pdu_len++] = (crc_16 & 0xff);
    dest[pdu_len++] = 0x0d;
    dest[pdu_len++] = 0x0a;

    return pdu_len;
}

//长度是指长度后面开始到0D0A直接的值
static int createProtocol7979Tail(char *dest, int h_b_len, int serial_no)
{
    int pdu_len;
    unsigned short crc_16;

    if ( dest == NULL )
    {
        return -1;
    }
    pdu_len = h_b_len;
    dest[pdu_len++] = (serial_no >> 8 ) & 0xff;
    dest[pdu_len++] = serial_no & 0xff;
    dest[2] = ((pdu_len-2) >> 8 ) & 0xff;
    dest[3] = ((pdu_len-2) & 0xff);
    crc_16 = GetCrc16(dest+2, pdu_len-2);
    dest[pdu_len++] = (crc_16 >>8)& 0xff;
    dest[pdu_len++] = (crc_16 & 0xff);
    dest[pdu_len++] = 0x0d;
    dest[pdu_len++] = 0x0a;
    return pdu_len;
}

/*打包IMEI
IMEI:设备sn号
dest：数据存放区
*/
static int packIMEI(char *IMEI, char *dest)
{
    int imei_len;
    int i;
    if ( IMEI == NULL || dest == NULL )
    {
        return -1;
    }
    imei_len = strlen(IMEI);
    if ( imei_len == 0 )
    {
        return -2;
    }
    if ( imei_len % 2 == 0 )
    {
        for( i = 0; i < imei_len / 2; i++)
        {
            dest[i] = ((IMEI[i*2] - '0') << 4 ) | (IMEI[i*2+1] - '0');
        }
    }
    else
    {
        for( i = 0; i < imei_len / 2; i++)
        {
            dest[i+1] = ((IMEI[i*2+1] - '0') << 4 ) | (IMEI[i*2+2] - '0');
        }
        dest[0] = (IMEI[0] - '0');
    }
    return (imei_len + 1) / 2;
}

/*生成序列号
*/
static unsigned short createProtocolSerial(void)
{
    return netconnect.serial++;
}

/*生成登录协议01
IMEI：设备sn号
Serial：序列号
DestBuf：数据存放区
*/
static int createProtocol_01(char *IMEI, unsigned short Serial, char *DestBuf)
{
    int pdu_len;
    int ret;
    pdu_len = createProtocol7878Head(DestBuf,0x01);
    ret = packIMEI(IMEI, DestBuf + pdu_len);
    if ( ret < 0 )
    {
        return -1;
    }
    pdu_len += ret;
    ret = createProtocol7878Tail(DestBuf, pdu_len,  Serial);
    if ( ret < 0 )
    {
        return -2;
    }
    pdu_len = ret;
    return pdu_len;
}
void updateUIS(void)
{
    memset(&uis,0,sizeof(UndateInfoStruct));
    eepromParamGetCodeVersion((uint8_t *)CODEVERSION);
    strcpy(uis.curCODEVERSION,CODEVERSION);
    uis.file_len=1024;//务必保证该值能够被4整除
    LogPrintf("Current Version:%s\n",uis.curCODEVERSION);
}

uint32_t getUpgradeFileSize(void)
{
    return uis.file_totalsize;

}
static int createProtocol_F3(char *IMEI,unsigned short Serial,char *DestBuf,uint8_t cmd)
{
    int pdu_len;
    uint32_t readfilelen;
    pdu_len = createProtocol7979Head(DestBuf,0xF3);
    //命令
    DestBuf[pdu_len++]=cmd;
    if(cmd==0x01)
    {
        //SN号长度
        DestBuf[pdu_len++]=strlen(IMEI);
        //拷贝SN号
        memcpy(DestBuf+pdu_len,IMEI,strlen(IMEI));
        pdu_len+=strlen(IMEI);
        //版本号长度
        DestBuf[pdu_len++]=strlen(uis.curCODEVERSION);
        //拷贝SN号
        memcpy(DestBuf+pdu_len,uis.curCODEVERSION,strlen(uis.curCODEVERSION));
        pdu_len+=strlen(uis.curCODEVERSION);
    }
    else if(cmd==0x02)
    {
        DestBuf[pdu_len++]=(uis.file_id>>24)&0xFF;
        DestBuf[pdu_len++]=(uis.file_id>>16)&0xFF;
        DestBuf[pdu_len++]=(uis.file_id>>8)&0xFF;
        DestBuf[pdu_len++]=(uis.file_id)&0xFF;

        //文件偏移位置
        DestBuf[pdu_len++]=(uis.rxfileOffset>>24)&0xFF;
        DestBuf[pdu_len++]=(uis.rxfileOffset>>16)&0xFF;
        DestBuf[pdu_len++]=(uis.rxfileOffset>>8)&0xFF;
        DestBuf[pdu_len++]=(uis.rxfileOffset)&0xFF;

        readfilelen=uis.file_totalsize-uis.rxfileOffset;//得到剩余未接收大小
        if(readfilelen>uis.file_len)
        {
            readfilelen=uis.file_len;
        }
        //文件读取长度
        DestBuf[pdu_len++]=(readfilelen>>24)&0xFF;
        DestBuf[pdu_len++]=(readfilelen>>16)&0xFF;
        DestBuf[pdu_len++]=(readfilelen>>8)&0xFF;
        DestBuf[pdu_len++]=(readfilelen)&0xFF;
    }
    else if(cmd==0x03)
    {
        DestBuf[pdu_len++]=uis.updateOK;
        //SN号长度
        DestBuf[pdu_len++]=strlen(IMEI);
        //拷贝SN号
        memcpy(DestBuf+pdu_len,IMEI,strlen(IMEI));
        pdu_len+=strlen(IMEI);

        DestBuf[pdu_len++]=(uis.file_id>>24)&0xFF;
        DestBuf[pdu_len++]=(uis.file_id>>16)&0xFF;
        DestBuf[pdu_len++]=(uis.file_id>>8)&0xFF;
        DestBuf[pdu_len++]=(uis.file_id)&0xFF;

        //版本号长度
        DestBuf[pdu_len++]=strlen(uis.newCODEVERSION);
        //拷贝SN号
        memcpy(DestBuf+pdu_len,uis.newCODEVERSION,strlen(uis.newCODEVERSION));
        pdu_len+=strlen(uis.newCODEVERSION);
    }
    else
    {
        return 0;
    }
    pdu_len=createProtocol7979Tail(DestBuf,pdu_len,Serial);
    return pdu_len;
}

/*发送协议至服务器
protocol：协议类型
param：需要传送的参数
*/
void sendProtocolToServer(PROTOCOLTYPE protocol,void * param)
{
    char txdata[384];
    char senddata[768];
    int txlen;
    switch(protocol)
    {
    case PROTOCOL_01:
        txlen=createProtocol_01((char *)eeprom_info.SN,createProtocolSerial(),txdata);
        break;
    case PROTOCOL_F3:
        txlen=createProtocol_F3((char *)eeprom_info.SN,createProtocolSerial(),txdata,*(uint8_t*)param);
        break;
    default:
        break;
    }
    switch(protocol)
    {
    default:
        N58_SendDataToServer((uint8_t *)txdata,txlen);
        memset(senddata,0,sizeof(senddata));
        changeByteArrayToHexString((uint8_t *)txdata,(uint8_t *)senddata,txlen);
        LogMessage("TCP Send:");
        LogMessage(senddata);
        LogMessage("\n");
        break;
    }

}

/*切换连接状态
*/
void protocolFsmStateChange(NetWorkFsmState state)
{
    netconnect.fsmstate=state;
    netconnect.runtick=0;
    LogPrintf("protocolFsmStateChange:change state to %d\n",state);
}

/*重置状态机连接状态
*/
void netConnectReset(void)
{
    netconnect.fsmstate=NETWORK_LOGIN;
    netconnect.runtick=0;
}



/*200ms  2*n=xsec*/
void protocolRunFsm(void)
{
    uint8_t cmd;
    if(N58_GetInvokeStatus()!=N58_NORMAL_STATUS)
    {
        return ;
    }
    switch(netconnect.fsmstate)
    {
    case NETWORK_LOGIN:
        //发送登录,并进入等待状态
        sendProtocolToServer(PROTOCOL_01,NULL);
        protocolFsmStateChange(NETWORK_LOGIN_WAIT);
        break;
    case NETWORK_LOGIN_WAIT:
        //等待登录返回，45秒内任未连上，则继续重连
        if(netconnect.runtick>150)
        {
            protocolFsmStateChange(NETWORK_LOGIN);
        }
        break;
    case NETWORK_LOGIN_READY:
        //登录后获取新软件版本，未获取，每隔45秒重新获取
        if(netconnect.runtick%90==0)
        {
            cmd=1;
            sendProtocolToServer(PROTOCOL_F3,&cmd);
        }
        break;
    case NETWORK_DOWNLOAD_DOING:
        //发送下载协议,并进入等待状态
        cmd=2;
        sendProtocolToServer(PROTOCOL_F3,&cmd);
        protocolFsmStateChange(NETWORK_DOWNLOAD_WAIT);
        break;
    case NETWORK_DOWNLOAD_WAIT:
        //等下固件下载，超过18秒未收到数据，重新发送下载协议
        LogMessage("Waitting firmware data...\n");
        if(netconnect.runtick>90)
        {
            protocolFsmStateChange(NETWORK_DOWNLOAD_DOING);
        }
        break;
    case NETWORK_DOWNLOAD_DONE:
        //下载写入完成
        cmd=3;
        sendProtocolToServer(PROTOCOL_F3,&cmd);
        LogMessage("Download firmware complete!\n");
        protocolFsmStateChange(NETWORK_WAIT_JUMP);
        eepromParamSaveUpdateStatus(0);
        startTimer(1000,startJumpToApp,1);
        break;
    case NETWORK_DOWNLOAD_ERROR://下载错误，需要处理
        LogMessage("Writting firmware error,retry...\n");
		flashEarse(FLASH_USER_START_ADDR+uis.rxfileOffset,uis.file_len/128+1);
        protocolFsmStateChange(NETWORK_DOWNLOAD_DOING);
        break;
    case NETWORK_WAIT_JUMP:
        //18秒任未跳转，则重新下载
        if(netconnect.runtick>90)
        {
            protocolFsmStateChange(NETWORK_LOGIN);

        }
        break;
    }
    netconnect.runtick++;
}


/* 01 协议解析
登录包回复：78 78 05 01 00 00 C8 55 0D 0A
*/
static void protoclparase01(char * protocol,int size)
{
    protocolFsmStateChange(NETWORK_LOGIN_READY);
    LogMessage("登录成功\n");
}
/* F3 协议解析
命令01回复：79790042F3
01 命令
01 是否需要更新
00000174  文件ID
0004EB34  文件总大小
0F		  SN长度
363930323137303931323030303237   SN号
0B		  当前版本长度
5632303230373134303031
16		  新版本号长度
4137575F353030333030303330385F3131355F34374B
000D   序列号
AD3B   校验
0D0A   结束

命令02回复：79790073F3
02 命令
01 升级有效标志
00000000   文件偏移起始
00000064   文件大小
474D3930312D3031303030344542314333374330304331457F454C4601010100000000000000000002002800010000006C090300E4E9040004EA04000200000534002000010028000700060038009FE510402DE924108FE2090080E002A300EB24109FE5
0003  序列号
27C6  校验
0D0A  借宿
*/
//static uint8_t codepar[1024];
static void protoclparaseF3(char * protocol,int size)
{
    int8_t ret;
    uint8_t cmd,snlen,myversionlen,newversionlen;
    uint16_t index,filecrc,calculatecrc;
    uint32_t rxfileoffset,rxfilelen;
    char *codedata;
    cmd=protocol[5];
    if(cmd==0x01)
    {
        //判断是否有更新文件
        if(protocol[6]==0x01)
        {
            uis.file_id=(protocol[7]<<24|protocol[8]<<16|protocol[9]<<8|protocol[10]);
            uis.file_totalsize=(protocol[11]<<24|protocol[12]<<16|protocol[13]<<8|protocol[14]);
            snlen=protocol[15];
            index=16;
            if(snlen>(sizeof(uis.rxsn)-1))
            {
                LogPrintf("Sn too long %d\n", snlen);
                return ;
            }
            strncpy(uis.rxsn,(char *)&protocol[index],snlen);
            uis.rxsn[snlen]=0;
            index=16+snlen;
            myversionlen=protocol[index];
            index+=1;
            if(myversionlen>(sizeof(uis.rxcurCODEVERSION)-1))
            {
                LogPrintf("myversion too long %d\n", myversionlen);
                return ;
            }
            strncpy(uis.rxcurCODEVERSION,(char *)&protocol[index],myversionlen);
            uis.rxcurCODEVERSION[myversionlen]=0;
            index+=myversionlen;
            newversionlen=protocol[index];
            index+=1;
            if(newversionlen>(sizeof(uis.newCODEVERSION)-1))
            {
                LogPrintf("newversion too long %d\n", newversionlen);
                return ;
            }
            strncpy(uis.newCODEVERSION,(char *)&protocol[index],newversionlen);
            uis.newCODEVERSION[newversionlen]=0;
            LogPrintf("File %08X , Total size=%d Bytes\n",uis.file_id,uis.file_totalsize);
            LogPrintf("My SN:%s\nMy Ver:%s\nNew Ver:%s\n",uis.rxsn,uis.rxcurCODEVERSION,uis.newCODEVERSION);
            protocolFsmStateChange(NETWORK_DOWNLOAD_DOING);
            if(uis.rxfileOffset==0)
            {
                uis.rxfileOffset=0;
                flashEarseByFileSize(uis.file_totalsize);
            }
            else
            {
                LogMessage("Update firmware continute\n");
            }
        }
        else
        {
            LogMessage("No update file\n");
            eepromParamSaveUpdateStatus(0);
            startTimer(1000,startJumpToApp,1);
        }
    }
    else if(cmd==0x02)
    {
        if(protocol[6]==1)
        {
            rxfileoffset=(protocol[7]<<24|protocol[8]<<16|protocol[9]<<8|protocol[10]);//文件偏移
            rxfilelen=(protocol[11]<<24|protocol[12]<<16|protocol[13]<<8|protocol[14]);//文件大小
            calculatecrc=GetCrc16(protocol+2,size-6);//文件校验
            filecrc=(*(protocol+15+rxfilelen+2)<<8) |(*(protocol+15+rxfilelen+2+1));
			if(rxfileoffset<uis.rxfileOffset)
				{
					LogMessage("Receive the same firmware\n");
					protocolFsmStateChange(NETWORK_DOWNLOAD_DOING);
					return ;
				}
            if(calculatecrc==filecrc)
            {
                LogMessage("Data validation OK,Writting...\n");
                codedata=protocol+15;
                ret=flashWriteCode(FLASH_USER_START_ADDR+rxfileoffset,(uint8_t *)codedata,rxfilelen);
                if(ret==1)
                {
                    uis.rxfileOffset=rxfileoffset+rxfilelen;
                    LogPrintf("Current File Size=%d\n",uis.rxfileOffset);
					LogPrintf("FileOffset=0x%X\n",uis.rxfileOffset);
                    if(uis.rxfileOffset==uis.file_totalsize)
                    {
                        uis.updateOK=1;
                        protocolFsmStateChange(NETWORK_DOWNLOAD_DONE);
                    }
                    else if(uis.rxfileOffset>uis.file_totalsize)
                    {
                    	LogMessage("Recevie complete ,but total size is different,retry again\n");
                    	uis.rxfileOffset=0;
                        protocolFsmStateChange(NETWORK_LOGIN);
                    }
                    else
                    {
                        protocolFsmStateChange(NETWORK_DOWNLOAD_DOING);

                    }
                }
                else
                {
                	LogPrintf("Writing firmware error at 0x%X\n",FLASH_USER_START_ADDR+uis.rxfileOffset);
                    protocolFsmStateChange(NETWORK_DOWNLOAD_ERROR);
                }

            }
            else
            {
                LogMessage("Data validation Fail\n");
				protocolFsmStateChange(NETWORK_DOWNLOAD_DOING);
            }
        }
        else
        {
            LogMessage("未知\n");

        }
    }



}



/*解析接收到的服务器协议
*/
void protocolRxParase(char * protocol,int size)
{
    if(protocol[0]==0X78 && protocol[1]==0X78)
    {
        switch(protocol[3])
        {
        case (uint8_t)0x01:
            protoclparase01(protocol,size);
            break;
        }
    }
    else if(protocol[0]==0X79 && protocol[1]==0X79)
    {
        switch(protocol[4])
        {
        case (uint8_t)0xF3:
            protoclparaseF3(protocol,size);
            break;
        }
    }
    else
    {
        LogMessageNoBlock("protocolRxParase:Error\n");
    }
}


