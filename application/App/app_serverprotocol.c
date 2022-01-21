#include "app_serverprotocol.h"
#include "app_sys.h"
#include "app_param.h"
#include "app_net.h"
#include "app_task.h"
#include "app_instructioncmd.h"
#include "app_gps.h"
#include "app_kernal.h"
//联网状态变量
NetWorkConnectStruct netconnect;

static uint8_t instructionid[4];
static uint8_t instructionid123[4];
static uint16_t instructionserier = 0;
static GPSRestoreStruct gpsres;
static int8_t hbtTimerid = -1;
Message protocolmessage[] =
{
    {NETWORK_LOGIN, "NETWORK_LOGIN"},
    {NETWORK_LOGIN_WAIT, "NETWORK_LOGIN_WAIT"},
    {NETWORK_LOGIN_READY, "NETWORK_LOGIN_READY"},
};


unsigned short createProtocolSerial(void);
/*生成协议头部
dest：数据存放区
Protocol_type：协议类型
*/
static int createProtocolHead(char *dest, unsigned char Protocol_type)
{
    int pdu_len;
    pdu_len = 0;
    if (dest == NULL)
    {
        return 0;
    }
    dest[pdu_len++] = 0x78;
    dest[pdu_len++] = 0x78;
    dest[pdu_len++] = 0;
    dest[pdu_len++] = Protocol_type;
    return pdu_len;
}
/*生成协议尾部
dest：数据存放区
h_b_len：前面数据长度
serial_no：序列号
*/
static int createProtocolTail(char *dest, int h_b_len, int serial_no)
{
    int pdu_len;
    unsigned short crc_16;

    if (dest == NULL)
    {
        return 0;
    }
    pdu_len = h_b_len;
    dest[pdu_len++] = (serial_no >> 8) & 0xff;
    dest[pdu_len++] = serial_no & 0xff;
    dest[2] = pdu_len - 1;
    /* Caculate Crc */
    crc_16 = GetCrc16(dest + 2, pdu_len - 2);
    dest[pdu_len++] = (crc_16 >> 8) & 0xff;
    dest[pdu_len++] = (crc_16 & 0xff);
    dest[pdu_len++] = 0x0d;
    dest[pdu_len++] = 0x0a;

    return pdu_len;
}


static int createProtocolTail_7979(char *dest, int h_b_len, int serial_no)
{
    int pdu_len;
    unsigned short crc_16;

    if (dest == NULL)
    {
        return -1;
    }
    pdu_len = h_b_len;
    dest[pdu_len++] = (serial_no >> 8) & 0xff;
    dest[pdu_len++] = serial_no & 0xff;
    dest[2] = ((pdu_len - 2) >> 8) & 0xff;
    dest[3] = ((pdu_len - 2) & 0xff);
    crc_16 = GetCrc16(dest + 2, pdu_len - 2);
    dest[pdu_len++] = (crc_16 >> 8) & 0xff;
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
    if (IMEI == NULL || dest == NULL)
    {
        return -1;
    }
    imei_len = strlen(IMEI);
    if (imei_len == 0)
    {
        return -2;
    }
    if (imei_len % 2 == 0)
    {
        for (i = 0; i < imei_len / 2; i++)
        {
            dest[i] = ((IMEI[i * 2] - '0') << 4) | (IMEI[i * 2 + 1] - '0');
        }
    }
    else
    {
        for (i = 0; i < imei_len / 2; i++)
        {
            dest[i + 1] = ((IMEI[i * 2 + 1] - '0') << 4) | (IMEI[i * 2 + 2] - '0');
        }
        dest[0] = (IMEI[0] - '0');
    }
    return (imei_len + 1) / 2;
}



static int protocolGPSpack(GPSINFO *gpsinfo, char *dest, int protocol, GPSRestoreStruct *gpsres)
{
    int pdu_len;
    unsigned long la;
    unsigned long lo;
    double f_la, f_lo;
    unsigned char speed, gps_viewstar, beidou_viewstar;
    int direc;

    DATETIME datetimenew;
    pdu_len = 0;
    la = lo = 0;
    /* UTC日期，DDMMYY格式 */
    datetimenew = changeUTCTimeToLocalTime(gpsinfo->datetime, sysparam.utc);

    dest[pdu_len++] = datetimenew.year % 100 ;
    dest[pdu_len++] = datetimenew.month;
    dest[pdu_len++] = datetimenew.day;
    dest[pdu_len++] = datetimenew.hour;
    dest[pdu_len++] = datetimenew.minute;
    dest[pdu_len++] = datetimenew.second;

    gps_viewstar = (unsigned char)gpsinfo->gpsviewstart;
    beidou_viewstar = (unsigned char)gpsinfo->beidouviewstart;
    if (gps_viewstar > 15)
    {
        gps_viewstar = 15;
    }
    if (beidou_viewstar > 15)
    {
        beidou_viewstar = 15;
    }
    if (protocol == PROTOCOL_12)
    {
        dest[pdu_len++] = (beidou_viewstar & 0x0f) << 4 | (gps_viewstar & 0x0f);
    }
    else
    {
        /* 前 4Bit为 GPS信息长度，后 4Bit为参与定位的卫星数 */
        if (gpsinfo->used_star   > 0)
        {
            dest[pdu_len++] = (12 << 4) | (gpsinfo->used_star & 0x0f);
        }
        else
        {
            /* 前 4Bit为 GPS信息长度，后 4Bit为参与定位的卫星数 */
            dest[pdu_len++] = (12 << 4) | (gpsinfo->gpsviewstart & 0x0f);
        }
    }
    /*
    纬度: 占用4个字节，表示定位数据的纬度值。数值范围0至162000000，表示0度到90度的范围，单位：1/500秒，转换方法如下：
    把GPS模块输出的经纬度值转化成以分为单位的小数；然后再把转化后的小数乘以30000，把相乘的结果转换成16进制数即可。
    如 ， ，然后转换成十六进制数为0x02 0x6B 0x3F 0x3E。
     */
    f_la  = gpsinfo->latitude * 60 * 30000;
    if (f_la < 0)
    {
        f_la = f_la * (-1);
    }
    la = (unsigned long)f_la;

    dest[pdu_len++] = (la >> 24) & 0xff;
    dest[pdu_len++] = (la >> 16) & 0xff;
    dest[pdu_len++] = (la >> 8) & 0xff;
    dest[pdu_len++] = (la) & 0xff;
    /*
    经度:占用4个字节，表示定位数据的经度值。数值范围0至324000000，表示0度到180度的范围，单位：1/500秒，转换方法
    和纬度的转换方法一致。
     */

    f_lo  = gpsinfo->longtitude * 60 * 30000;
    if (f_lo < 0)
    {
        f_lo = f_lo * (-1);
    }
    lo = (unsigned long)f_lo;
    dest[pdu_len++] = (lo >> 24) & 0xff;
    dest[pdu_len++] = (lo >> 16) & 0xff;
    dest[pdu_len++] = (lo >> 8) & 0xff;
    dest[pdu_len++] = (lo) & 0xff;


    /*
    速度:占用1个字节，表示GPS的运行速度，值范围为0x00～0xFF表示范围0～255公里/小时。
     */
    speed = (unsigned char)(gpsinfo->speed);
    dest[pdu_len++] = speed;
    /*
    航向:占用2个字节，表示GPS的运行方向，表示范围0～360，单位：度，以正北为0度，顺时针。
     */
    if (gpsres != NULL)
    {

        gpsres->year = datetimenew.year % 100 ;
        gpsres->month = datetimenew.month;
        gpsres->day = datetimenew.day;
        gpsres->hour = datetimenew.hour;
        gpsres->minute = datetimenew.minute;
        gpsres->second = datetimenew.second;

        gpsres->latititude[0] = (la >> 24) & 0xff;
        gpsres->latititude[1] = (la >> 16) & 0xff;
        gpsres->latititude[2] = (la >> 8) & 0xff;
        gpsres->latititude[3] = (la) & 0xff;


        gpsres->longtitude[0] = (lo >> 24) & 0xff;
        gpsres->longtitude[1] = (lo >> 16) & 0xff;
        gpsres->longtitude[2] = (lo >> 8) & 0xff;
        gpsres->longtitude[3] = (lo) & 0xff;

        gpsres->speed = speed;
    }

    direc = (int)gpsinfo->course;
    dest[pdu_len] = (direc >> 8) & 0x03;
    //GPS FIXED:
    dest[pdu_len] |= 0x10 ; //0000 1000
    /*0：南纬 1：北纬 */
    if (gpsinfo->NS == 'N')
    {
        dest[pdu_len] |= 0x04 ; //0000 0100
    }
    /*0：东经 1：西经*/
    if (gpsinfo->EW == 'W')
    {
        dest[pdu_len] |= 0x08 ; //0000 1000
    }
    if (gpsres != NULL)
    {
        gpsres->coordinate[0] = dest[pdu_len];
    }
    pdu_len++;
    dest[pdu_len] = (direc) & 0xff;
    if (gpsres != NULL)
    {
        gpsres->coordinate[1] = dest[pdu_len];
    }
    pdu_len++;
    return pdu_len;
}

static int protocolLBSPack(char *dest)
{
    int pdu_len;
    if (dest == NULL)
    {
        return -1;
    }
    pdu_len = 0;
    /*mcc*/
    dest[pdu_len++] = 0; //(lai.MCC >> 8) & 0xff;
    dest[pdu_len++] = 0; //(lai.MCC) & 0xff;
    /*mnc*/
    dest[pdu_len++] = 0; ///(lai.MNC) & 0xff;
    /*lac*/
    dest[pdu_len++] = 0; //(lai.LAC>> 8) & 0xff;
    dest[pdu_len++] = 0; //(lai.LAC) & 0xff;
    /*cid*/
    dest[pdu_len++] = 0; //(lai.CELL_ID>> 16) & 0xff;
    dest[pdu_len++] = 0; //(lai.CELL_ID >> 8) & 0xff;
    dest[pdu_len++] = 0; //(lai.CELL_ID) & 0xff;
    return pdu_len;
}



/*生成登录协议01
IMEI：设备sn号
Serial：序列号
DestBuf：数据存放区
*/
int createProtocol_01(char *IMEI, unsigned short Serial, char *DestBuf)
{
    int pdu_len;
    int ret;
    pdu_len = createProtocolHead(DestBuf, 0x01);
    ret = packIMEI(IMEI, DestBuf + pdu_len);
    if (ret < 0)
    {
        return -1;
    }
    pdu_len += ret;
    ret = createProtocolTail(DestBuf, pdu_len,  Serial);
    if (ret < 0)
    {
        return -2;
    }
    pdu_len = ret;
    return pdu_len;
}

/*生成定位协议12
gpsinfo：gps数据信息
Serial：序列号
DestBuf：数据存放区
*/

int createProtocol_12(GPSINFO *gpsinfo,  unsigned short Serial, char *DestBuf)
{
    int pdu_len;
    int ret;
    unsigned char gsm_level_value;

    if (gpsinfo == NULL)
        return -1;
    pdu_len = createProtocolHead(DestBuf, 0x12);
    /* Pack GPS */
    ret = protocolGPSpack(gpsinfo,  DestBuf + pdu_len, PROTOCOL_12, &gpsres);
    if (ret < 0)
    {
        return -1;
    }
    pdu_len += ret;
    /* Pack LBS */
    ret = protocolLBSPack(DestBuf + pdu_len);
    if (ret < 0)
    {
        return -2;
    }
    pdu_len += ret;
    /* Add Language Reserved */
    gsm_level_value = getModuleRssi();
    gsm_level_value |= 0x80;
    DestBuf[pdu_len++] = gsm_level_value;
    DestBuf[pdu_len++] = 0;
    /* Pack Tail */
    ret = createProtocolTail(DestBuf, pdu_len,  Serial);
    if (ret <=  0)
    {
        return -3;
    }
    gpsinfo->hadupload = 1;
    pdu_len = ret;
    return pdu_len;
}

void gpsRestoreDataSend(GPSRestoreStruct *grs, char *dest	, uint16_t *len)
{
    char debug[200];
    int pdu_len;
    pdu_len = createProtocolHead(dest, 0x12);
    //时间戳
    dest[pdu_len++] = grs->year ;
    dest[pdu_len++] = grs->month;
    dest[pdu_len++] = grs->day;
    dest[pdu_len++] = grs->hour;
    dest[pdu_len++] = grs->minute;
    dest[pdu_len++] = grs->second;
    //数量
    dest[pdu_len++] = 0;
    //维度
    dest[pdu_len++] = grs->latititude[0];
    dest[pdu_len++] = grs->latititude[1];
    dest[pdu_len++] = grs->latititude[2];
    dest[pdu_len++] = grs->latititude[3];
    //经度
    dest[pdu_len++] = grs->longtitude[0];
    dest[pdu_len++] = grs->longtitude[1];
    dest[pdu_len++] = grs->longtitude[2];
    dest[pdu_len++] = grs->longtitude[3];
    //速度
    dest[pdu_len++] = grs->speed;
    //定位信息
    dest[pdu_len++] = grs->coordinate[0];
    dest[pdu_len++] = grs->coordinate[1];
    //mmc
    dest[pdu_len++] = 0;
    dest[pdu_len++] = 0;
    //mnc
    dest[pdu_len++] = 0;
    //lac
    dest[pdu_len++] = 0;
    dest[pdu_len++] = 0;
    //cellid
    dest[pdu_len++] = 0;
    dest[pdu_len++] = 0;
    dest[pdu_len++] = 0;
    //signal
    dest[pdu_len++] = 0;
    //语言位
    dest[pdu_len++] = 1;
    pdu_len = createProtocolTail(dest, pdu_len,	createProtocolSerial());
    *len = pdu_len;
    changeByteArrayToHexString((uint8_t *)dest, (uint8_t *)debug, pdu_len);
    debug[pdu_len * 2] = 0;
    LogMessage(DEBUG_ALL, "TCP Send:");
    LogMessage(DEBUG_ALL, debug);
    LogMessage(DEBUG_ALL, "\n");

}

static uint8_t getBatteryLevel(void)
{
    uint8_t level = 0;
    if (sysinfo.outsidevoltage > 4.15)
    {
        level = 100;

    }
    else if (sysinfo.outsidevoltage < 3.45)
    {
        level = 0;
    }
    else
    {

        level = (uint8_t)((sysinfo.outsidevoltage - 3.45) / 0.7 * 100);
    }
    return level;
}
/*生成心跳协议
Serial：序列号
DestBuf：数据存放区
*/
int createProtocol_13(unsigned short Serial, char *DestBuf)
{
    int pdu_len;
    int ret;
    uint16_t value;
    GPSINFO *gpsinfo;
    uint8_t gpsvewstar, beidouviewstar;
    pdu_len = createProtocolHead(DestBuf, 0x13);
    DestBuf[pdu_len++] = sysinfo.terminalStatus;
    gpsinfo = getCurrentGPSInfo();

    value  = 0;
    gpsvewstar = gpsinfo->gpsviewstart;
    beidouviewstar = gpsinfo->beidouviewstart;
    if (sysinfo.GPSStatus == 0)
    {
        gpsvewstar = 0;
        beidouviewstar = 0;
    }
    value |= ((beidouviewstar & 0x1F) << 10);
    value |= ((gpsvewstar & 0x1F) << 5);
    value |= ((getModuleRssi() & 0x1F));
    value |= 0x8000;

    DestBuf[pdu_len++] = (value >> 8) & 0xff; //卫星数(BDGPV>>8)&0XFF;
    DestBuf[pdu_len++] = value & 0xff; // BDGPV&0XFF;
    DestBuf[pdu_len++ ] = 0;
    DestBuf[pdu_len++ ] = 0;//language
    DestBuf[pdu_len++ ] = getBatteryLevel();//电量
    DestBuf[pdu_len ] = 2 | (0x01 << 6); //模式
    pdu_len++;
    value = (uint16_t)(sysinfo.outsidevoltage * 100);
    DestBuf[pdu_len++ ] = (value >> 8) & 0xff; //电压(vol >>8 ) & 0xff;
    DestBuf[pdu_len++ ] = value & 0xff; //vol & 0xff;
    DestBuf[pdu_len++ ] = 0;//感光
    DestBuf[pdu_len++ ] = (sysparam.mode1startuptime >> 8) & 0xff; //模式一次数
    DestBuf[pdu_len++ ] = sysparam.mode1startuptime & 0xff;
    DestBuf[pdu_len++ ] = (sysparam.mode2worktime >> 8) & 0xff; //模式二次数
    DestBuf[pdu_len++ ] = sysparam.mode2worktime & 0xff;
    ret = createProtocolTail(DestBuf, pdu_len,  Serial);
    if (ret < 0)
    {
        return -1;
    }
    pdu_len = ret;
    return pdu_len;

}
//78 78 14 80 0C 00 1D DA 4B 56 45 52 53 49 4F 4E 23 00 00 00 00 00 61 0D 0A
//79 79 00 2a 21 00 1d da 4b 01 56 45 52 53 49 4f 4e 3a 41 37 57 5f 35 30 30 33 30 30 30 33 0d 0a 00 00 3c 62 0d 0a
int createProtocol_21(char *DestBuf, char *data, uint16_t datalen)
{
    int pdu_len = 0, i;
    DestBuf[pdu_len++] = 0x79; //协议头
    DestBuf[pdu_len++] = 0x79;
    DestBuf[pdu_len++] = 0x00; //指令长度待定
    DestBuf[pdu_len++] = 0x00;
    DestBuf[pdu_len++] = 0x21; //协议号
    DestBuf[pdu_len++] = instructionid[0]; //指令ID
    DestBuf[pdu_len++] = instructionid[1];
    DestBuf[pdu_len++] = instructionid[2];
    DestBuf[pdu_len++] = instructionid[3];
    DestBuf[pdu_len++] = 1; //内容编码
    for (i = 0; i < datalen; i++) //返回内容
    {
        DestBuf[pdu_len++] = data[i];
    }
    pdu_len = createProtocolTail_7979(DestBuf, pdu_len, instructionserier);
    return pdu_len;
}


void createProtocol61(char *dest, char *datetime, uint32_t totalsize, uint8_t filetye, uint16_t packsize)
{
    uint16_t pdu_len;
    char debug[200];
    uint16_t packnum;
    pdu_len = createProtocolHead(dest, 0x61);
    changeHexStringToByteArray_10in((uint8_t *)dest + 4, (uint8_t *)datetime, 6);
    pdu_len += 6;
    dest[pdu_len++] = filetye;
    packnum = totalsize / packsize;
    if (totalsize % packsize != 0)
    {
        packnum += 1;
    }
    dest[pdu_len++] = (packnum >> 8) & 0xff;
    dest[pdu_len++] = packnum & 0xff;
    dest[pdu_len++] = (totalsize >> 24) & 0xff;
    dest[pdu_len++] = (totalsize >> 16) & 0xff;
    dest[pdu_len++] = (totalsize >> 8) & 0xff;
    dest[pdu_len++] = totalsize & 0xff;
    pdu_len = createProtocolTail(dest, pdu_len,  createProtocolSerial());
    changeByteArrayToHexString((uint8_t *)dest, (uint8_t *)debug, pdu_len);
    debug[pdu_len * 2] = 0;
    LogMessage(DEBUG_ALL, "TCP Send:");
    LogMessage(DEBUG_ALL, debug);
    LogMessage(DEBUG_ALL, "\n");
    sendDataToServer((uint8_t *)dest, pdu_len);
}

void createProtocol62(char *dest, char *datetime, uint16_t packnum, uint8_t *recdata, uint16_t reclen)
{
    char debug[50];
    uint16_t pdu_len, i;
    pdu_len = 0;
    dest[pdu_len++] = 0x79; //协议头
    dest[pdu_len++] = 0x79;
    dest[pdu_len++] = 0x00; //指令长度待定
    dest[pdu_len++] = 0x00;
    dest[pdu_len++] = 0x62; //协议号
    changeHexStringToByteArray_10in((uint8_t *)dest + 5, (uint8_t *)datetime, 6);
    pdu_len += 6;
    dest[pdu_len++] = (packnum >> 8) & 0xff;
    dest[pdu_len++] = packnum & 0xff;
    for (i = 0; i < reclen; i++)
    {
        dest[pdu_len++] = recdata[i];
    }
    pdu_len = createProtocolTail_7979(dest, pdu_len,  createProtocolSerial());
    changeByteArrayToHexString((uint8_t *)dest, (uint8_t *)debug, 15);
    debug[30] = 0;
    LogMessage(DEBUG_ALL, "TCP Send:");
    LogMessage(DEBUG_ALL, debug);
    LogMessage(DEBUG_ALL, "\n");
    sendDataToServer((uint8_t *)dest, pdu_len);
}

void createProtocolF3(char *dest, N58_WIFIAPSCAN *wap)
{
    uint8_t i, j;

    char debug[256];
    uint16_t year;
    uint8_t  month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    getRtcDateTime(&year, &month, &date, &hour, &minute, &second);
    uint16_t pdu_len;
    pdu_len = createProtocolHead(dest, 0xF3);
    dest[pdu_len++] = year % 100;
    dest[pdu_len++] = month;
    dest[pdu_len++] = date;
    dest[pdu_len++] = hour;
    dest[pdu_len++] = minute;
    dest[pdu_len++] = second;
    dest[pdu_len++] = wap->apcount;
    for (i = 0; i < wap->apcount; i++)
    {
        dest[pdu_len++] = 0;
        dest[pdu_len++] = 0;
        for (j = 0; j < 6; j++)
        {
            dest[pdu_len++] = wap->ap[i].ssid[j];
        }
    }
    pdu_len = createProtocolTail(dest, pdu_len,  createProtocolSerial());
    changeByteArrayToHexString((uint8_t *)dest, (uint8_t *)debug, pdu_len);
    debug[pdu_len * 2] = 0;
    LogMessage(DEBUG_ALL, "TCP Send:");
    LogMessage(DEBUG_ALL, debug);
    LogMessage(DEBUG_ALL, "\n");
    sendDataToServer((uint8_t *)dest, pdu_len);
}


int createProtocolF1(unsigned short Serial, char *DestBuf)
{
    int pdu_len;
    pdu_len = createProtocolHead(DestBuf, 0xF1);
    sprintf(DestBuf + pdu_len, "%s&&%s&&%s", sysparam.SN, getModuleIMSI(), getModuleICCID());
    pdu_len += strlen(DestBuf + pdu_len);
    pdu_len = createProtocolTail(DestBuf, pdu_len,  createProtocolSerial());
    return pdu_len;
}

int createProtocol16(unsigned short Serial, char *DestBuf, uint8_t event)
{
    int pdu_len, ret, i;
    GPSINFO *gpsinfo;
    pdu_len = createProtocolHead(DestBuf, 0x16);
    gpsinfo = getLastFixedGPSInfo();
    ret = protocolGPSpack(gpsinfo, DestBuf + pdu_len, PROTOCOL_16, NULL);
    pdu_len += ret;

    /**********************/

    DestBuf[pdu_len++] = 0xFF;
    DestBuf[pdu_len++] = (sysinfo.mcc >> 8) & 0xff;
    DestBuf[pdu_len++] = sysinfo.mcc & 0xff;
    DestBuf[pdu_len++] = sysinfo.mnc;
    DestBuf[pdu_len++] = (sysinfo.lac >> 8) & 0xff;
    DestBuf[pdu_len++] = sysinfo.lac & 0xff;
    DestBuf[pdu_len++] = (sysinfo.cid >> 24) & 0xff;
    DestBuf[pdu_len++] = (sysinfo.cid >> 16) & 0xff;
    DestBuf[pdu_len++] = (sysinfo.cid >> 8) & 0xff;
    DestBuf[pdu_len++] = (sysinfo.cid) & 0xff;
    for (i = 0; i < 36; i++)
        DestBuf[pdu_len++] = 0;

    /**********************/


    DestBuf[pdu_len++] = sysinfo.terminalStatus;
    sysinfo.terminalStatus &= ~0x38;
    DestBuf[pdu_len++] = 0;
    DestBuf[pdu_len++] = 0;
    DestBuf[pdu_len++] = event;
    if (event == 0)
    {
        DestBuf[pdu_len++] = 0x01;
    }
    else
    {
        DestBuf[pdu_len++] = 0x81;
    }
    pdu_len = createProtocolTail(DestBuf, pdu_len,  Serial);
    return pdu_len;

}


/*
78 78 17 19 14 0c 0e 13 20 10
01 cc 		 // mcc 460
08 			 // mnc 8
01
64 f0        // lac 0x64F0
0d 11 fa 48  //cid 0x0d11fa48
00 00
16 0f
8b ef
0d 0a

*/
int createProtocol19(unsigned short Serial, char *DestBuf)
{
    int pdu_len;
    uint16_t year;
    uint8_t  month;
    uint8_t date;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    getRtcDateTime(&year, &month, &date, &hour, &minute, &second);
    pdu_len = createProtocolHead(DestBuf, 0x19);
    DestBuf[pdu_len++] = year % 100;
    DestBuf[pdu_len++] = month;
    DestBuf[pdu_len++] = date;
    DestBuf[pdu_len++] = hour;
    DestBuf[pdu_len++] = minute;
    DestBuf[pdu_len++] = second;
    DestBuf[pdu_len++] = sysinfo.mcc >> 8;
    DestBuf[pdu_len++] = sysinfo.mcc;
    DestBuf[pdu_len++] = sysinfo.mnc;
    DestBuf[pdu_len++] = 1;
    DestBuf[pdu_len++] = sysinfo.lac >> 8;
    DestBuf[pdu_len++] = sysinfo.lac;
    DestBuf[pdu_len++] = sysinfo.cid >> 24;
    DestBuf[pdu_len++] = sysinfo.cid >> 16;
    DestBuf[pdu_len++] = sysinfo.cid >> 8;
    DestBuf[pdu_len++] = sysinfo.cid;
    DestBuf[pdu_len++] = 0;
    DestBuf[pdu_len++] = 0;
    pdu_len = createProtocolTail(DestBuf, pdu_len,  Serial);
    return pdu_len;
}

int createProtocol_8A(unsigned short Serial, char *DestBuf)
{
    int pdu_len;
    int ret;
    pdu_len = createProtocolHead(DestBuf, 0x8A);
    ret = createProtocolTail(DestBuf, pdu_len,  Serial);
    if (ret < 0)
    {
        return -2;
    }
    pdu_len = ret;
    return pdu_len;
}


/*生成序列号
*/
static unsigned short createProtocolSerial(void)
{
    return netconnect.serial++;
}
/*发送协议至服务器
protocol：协议类型
param：需要传送的参数
*/
void sendProtocolToServer(PROTOCOLTYPE protocol, void *param)
{
    GPSINFO *gpsinfo;
    char txdata[400];
    char senddata[202];
    int txlen;
    switch (protocol)
    {
        case PROTOCOL_01:
            txlen = createProtocol_01((char *)sysparam.SN, createProtocolSerial(), txdata);
            break;
        case PROTOCOL_12:
            gpsinfo = (GPSINFO *)param;
            if (gpsinfo->hadupload == 1 || gpsinfo->fixstatus == 0)
            {
                return ;
            }
            txlen = createProtocol_12((GPSINFO *)param, createProtocolSerial(), txdata);
            break;
        case PROTOCOL_13:
            if (isProtocolReday() == 0)
            {
                return ;
            }
            txlen = createProtocol_13(createProtocolSerial(), txdata);
            break;
        case PROTOCOL_16:
            txlen = createProtocol16(createProtocolSerial(), txdata, *(uint8_t *)param);
            break;
        case PROTOCOL_19:
            txlen = createProtocol19(createProtocolSerial(), txdata);
            break;
        case PROTOCOL_21:
            txlen = createProtocol_21(txdata, (char *)param, strlen((char *)param));
            break;
        case PROTOCOL_F1:
            txlen = createProtocolF1(createProtocolSerial(), txdata);
            break;
        case PROTOCOL_8A:
            txlen = createProtocol_8A(createProtocolSerial(), txdata);
            break;
    }
    switch (protocol)
    {
        case PROTOCOL_12:
            if (isProtocolReday())
            {
                sendDataToServer((uint8_t *)txdata, txlen);
                if (txlen > 100)
                {
                    txlen = 100;
                }
                //memset(senddata, 0, sizeof(senddata));
                changeByteArrayToHexString((uint8_t *)txdata, (uint8_t *)senddata, txlen);
                senddata[txlen * 2] = 0;
                LogMessage(DEBUG_ALL, "TCP Send:");
                LogMessage(DEBUG_ALL, senddata);
                LogMessage(DEBUG_ALL, "\n");
            }
            else
            {
                gpsRestoreWriteData(&gpsres);
                LogMessage(DEBUG_ALL, "Network error\n");
            }

            break;
        default:
            sendDataToServer((uint8_t *)txdata, txlen);
            if (txlen > 100)
            {
                txlen = 100;
            }
            //memset(senddata, 0, sizeof(senddata));
            changeByteArrayToHexString((uint8_t *)txdata, (uint8_t *)senddata, txlen);
            senddata[txlen * 2] = 0;
            LogMessage(DEBUG_ALL, "TCP Send:");
            LogMessage(DEBUG_ALL, senddata);
            LogMessage(DEBUG_ALL, "\n");
            break;
    }

}
/*重置状态机连接状态
*/
void netConnectReset(void)
{
    netconnect.fsmstate = NETWORK_LOGIN;
    netconnect.heartbeattick = 0;
}
/*切换连接状态
*/
void protocolFsmStateChange(NetWorkFsmState state)
{
    netconnect.fsmstate = state;
    LogPrintf(DEBUG_ALL, "protocolFsmStateChange:change state to %d\n", state);
}

void clearHbtTimer(void)
{
    if (hbtTimerid != -1)
    {
        stopTimer(hbtTimerid);
        hbtTimerid = -1;
    }
}

void moduleResetProcess(void)
{
    hbtTimerid = -1;
    moduleReset();
}

void uploadGpsRestoreData(void)
{
    static uint8_t waitTick = 0;
    if (netconnect.uploadFlag == 0)
    {

        if (++waitTick >= 10)
        {
            waitTick = 0;
            netconnect.uploadFlag = 1;
        }
        return ;
    }
    waitTick++;
    if (waitTick % 4 == 0)
    {
        gpsReadFromModuleAndSendtoServe();
    }
}


/*运行网络连接状态机
执行登录信息，心跳信息的发送，维持链接稳定
*/

void protocolRunFsm(void)
{
    switch (netconnect.fsmstate)
    {
        case NETWORK_LOGIN:
            sendProtocolToServer(PROTOCOL_01, NULL);
            sendProtocolToServer(PROTOCOL_F1, NULL);
            sendProtocolToServer(PROTOCOL_8A, NULL);
            protocolFsmStateChange(NETWORK_LOGIN_WAIT);
            gpsReadRestoreReset();
            netconnect.logintick = 0;
            netconnect.loginCount++;
            netconnect.uploadFlag = 0;
            if (sysparam.MODE == MODE2 || sysparam.MODE == MODE5)
            {
                sysinfo.hearbeatrequest = 1;
            }
            break;
        case NETWORK_LOGIN_WAIT:
            netconnect.logintick++;
            if (netconnect.logintick >= 60)
            {
                if (netconnect.loginCount >= 3)
                {
                    netconnect.loginCount = 0;
                    N58_ChangeInvokeStatus(N58_CPIN_STATUS);
                }
                else
                {
                    protocolFsmStateChange(NETWORK_LOGIN);
                }

            }
            break;
        case NETWORK_LOGIN_READY:
            if (sysparam.MODE == MODE2 || sysparam.MODE == MODE5)
            {
                if (sysinfo.hearbeatrequest == 1)
                {
                    if (netconnect.heartbeattick == 0)
                    {
                        csqRequest();
                        csqRequest();
                        csqRequest();
                        if (hbtTimerid == -1)
                        {
                            hbtTimerid = startTimer(8000, moduleResetProcess, 0);
                        }
                        sendProtocolToServer(PROTOCOL_13, NULL);

                    }
                    else if (netconnect.heartbeattick >= 10)
                    {
                        sysinfo.hearbeatrequest = 0;

                    }
                    netconnect.heartbeattick++;
                }
                else
                {
                    netconnect.heartbeattick = 0;
                }
            }
            else
            {
                if (netconnect.heartbeattick % (sysparam.heartbeatgap - 2) == 0)
                {
                    if (hbtTimerid == -1)
                    {
                        hbtTimerid = startTimer(10000, moduleResetProcess, 0);
                    }
                    csqRequest();
                }
                if (netconnect.heartbeattick % sysparam.heartbeatgap == 0)
                {
                    netconnect.heartbeattick = 0;
                    sendProtocolToServer(PROTOCOL_13, NULL);
                }
                netconnect.heartbeattick++;
            }
            uploadGpsRestoreData();
            break;
        default:
            netconnect.fsmstate = NETWORK_LOGIN;
            netconnect.heartbeattick = 0;
            break;
    }
}

uint8_t isProtocolReday(void)
{
    if (isModuleRunNormal() && netconnect.fsmstate == NETWORK_LOGIN_READY)
        return 1;
    return 0;
}
/* 01 协议解析
登录包回复：78 78 05 01 00 00 C8 55 0D 0A
*/
static void protoclparase01(char *protocol, int size)
{
    //unsigned short serialno=0;
    //serialno=protocol[4]<<8|protocol[5];
    protocolFsmStateChange(NETWORK_LOGIN_READY);
    updateSystemLedStatus(SYSTEM_LED_NETOK, 1);
	netconnect.loginCount=0;
    LogMessage(DEBUG_ALL, "登录成功\n");
}
/* 13 协议解析
登录包回复：787805130001E9F10D0A
*/
static void protoclparase13(char *protocol, int size)
{
    //unsigned short serialno=0;
    //serialno=protocol[4]<<8|protocol[5];
    protocolFsmStateChange(NETWORK_LOGIN_READY);
    LogMessage(DEBUG_ALL, "心跳回复\n");
    clearHbtTimer();
}

/* 80 协议解析
登录包回复：78 78 13 80 0B 00 1D D9 E6 53 54 41 54 55 53 23 00 00 00 00 A7 79 0D 0A
*/
static void protoclparase80(char *protocol, int size)
{

    uint8_t instructionlen;
    char debug[128];
    instructionid[0] = protocol[5];
    instructionid[1] = protocol[6];
    instructionid[2] = protocol[7];
    instructionid[3] = protocol[8];
    instructionlen = protocol[4] - 4;
    instructionserier = (protocol[instructionlen + 11] << 8) | (protocol[instructionlen + 12]);
    memset(debug, 0, sizeof(debug));
    strncpy(debug, protocol + 9, instructionlen);
    instructionParase((uint8_t *)debug, instructionlen, NETWORK_MODE, NULL);
}

/*78780B8A140C10053B2E00036B5B0D0A*/
static void protoclParase8A(char *protocol, int size)
{
    DATETIME datetime;
    if (sysinfo.localrtchadupdate == 1)
        return ;
    sysinfo.localrtchadupdate = 1;
    datetime.year = protocol[4];
    datetime.month = protocol[5];
    datetime.day = protocol[6];
    datetime.hour = protocol[7];
    datetime.minute = protocol[8];
    datetime.second = protocol[9];
    updateLocalRTCTime(&datetime);
}


/*解析接收到的服务器协议
*/
void protocolRxParase(char *protocol, int size)
{
    if (protocol[0] == 0X78 && protocol[1] == 0X78)
    {
        switch (protocol[3])
        {
            case (uint8_t)0x01:
                protoclparase01(protocol, size);
                break;
            case (uint8_t)0x13:
                protoclparase13(protocol, size);
                break;
            case (uint8_t)0x80:
                protoclparase80(protocol, size);
                break;
            case (uint8_t)0x8A:
                protoclParase8A(protocol, size);
                break;
        }
    }
    else
    {
        LogMessage(DEBUG_ALL, "protocolRxParase:Error\n");
    }
}

void save123InstructionId(void)
{
    instructionid123[0] = instructionid[0];
    instructionid123[1] = instructionid[1];
    instructionid123[2] = instructionid[2];
    instructionid123[3] = instructionid[3];
}
void reCover123InstructionId(void)
{
    instructionid[0] = instructionid123[0];
    instructionid[1] = instructionid123[1];
    instructionid[2] = instructionid123[2];
    instructionid[3] = instructionid123[3];
}
