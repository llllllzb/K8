/******************** (C) COPYRIGHT 2018 MiraMEMS *****************************
* File Name     : mir3da.c
* Author        : ycwang@miramems.com
* Version       : V1.0
* Date          : 05/18/2018
* Description   : Demo for configuring mir3da
*******************************************************************************/
#include "app_mir3da.h"
#include "stm32l0xx_hal.h"
#include "i2c.h"
#include <stdio.h>
#include "app_sys.h"
u8_m i2c_addr = 0x26;

s8_m mir3da_register_read(u8_m addr, u8_m *data_m, u8_m len)
{
	//To do i2c read api
	//HAL_I2C_Master_Receive(&hi2c1,i2c_addr,data_m,len,0xfffff);
	HAL_I2C_Mem_Read(&hi2c1,i2c_addr<<1,addr,1,data_m,len,50);
	return 0;

}

s8_m mir3da_register_write(u8_m addr, u8_m data_m)
{
	//To do i2c write api
	HAL_I2C_Mem_Write(&hi2c1,i2c_addr<<1,addr,1,&data_m,1,50);
	return 0;

}

//Initialization
s8_m mir3da_init(void)
{
	s8_m res = 0;
	u8_m data_m = 0;
	//Retry 3 times
	res = mir3da_register_read(NSA_REG_WHO_AM_I,&data_m,1);
	if(data_m != 0x13)
	{
		res = mir3da_register_read(NSA_REG_WHO_AM_I,&data_m,1);
		if(data_m != 0x13)
		{
			res = mir3da_register_read(NSA_REG_WHO_AM_I,&data_m,1);
			if(data_m != 0x13)
			{
				LogPrintf(DEBUG_ALL,"mir3da_init==>Read chip error=%X\r\n\r\n",data_m);
				return -1;
			}
		}
	}

	LogPrintf(DEBUG_ALL,"mir3da_init==>Read chip id=%X\r\n",data_m);

	res |= mir3da_register_write(NSA_REG_SPI_I2C, 0x24);
	HAL_Delay(20);

	res |= mir3da_register_write(NSA_REG_G_RANGE, 0x00);               //+/-2G,14bit
	res |= mir3da_register_write(NSA_REG_POWERMODE_BW, 0x34);          //normal mode
	res |= mir3da_register_write(NSA_REG_ODR_AXIS_DISABLE, 0x06);      //ODR = 62.5hz

	//Engineering mode
	res |= mir3da_register_write(NSA_REG_ENGINEERING_MODE, 0x83);
	res |= mir3da_register_write(NSA_REG_ENGINEERING_MODE, 0x69);
	res |= mir3da_register_write(NSA_REG_ENGINEERING_MODE, 0xBD);

	HAL_Delay(50);

	//Reduce power consumption
	if(i2c_addr == 0x26)
	{
		mir3da_register_write(NSA_REG_SENS_COMP, 0x00);
	}

	return res;
}

//enable/disable the chip
s8_m mir3da_set_enable(u8_m enable)
{
	s8_m res = 0;
	if(enable)
		res = mir3da_register_write(NSA_REG_POWERMODE_BW,0x30);
	else
		res = mir3da_register_write(NSA_REG_POWERMODE_BW,0x80);

	return res;
}

//Read three axis data, 1024 LSB = 1 g
s8_m mir3da_read_data(s16_m *x, s16_m *y, s16_m *z)
{
	u8_m    tmp_data[6] = {0};

	if (mir3da_register_read(NSA_REG_ACC_X_LSB, tmp_data,6) != 0)
	{
		return -1;
	}

	*x = ((s16_m)(tmp_data[1] << 8 | tmp_data[0]))>> 4;
	*y = ((s16_m)(tmp_data[3] << 8 | tmp_data[2]))>> 4;
	*z = ((s16_m)(tmp_data[5] << 8 | tmp_data[4]))>> 4;

	return 0;
}

//open active interrupt
s8_m mir3da_open_interrupt(u8_m th)
{
	s8_m   res = 0;

	res = mir3da_register_write(NSA_REG_INTERRUPT_SETTINGS1,0x87);
	res = mir3da_register_write(NSA_REG_ACTIVE_DURATION,0x00 );
	res = mir3da_register_write(NSA_REG_ACTIVE_THRESHOLD,th);
	res = mir3da_register_write(NSA_REG_INTERRUPT_MAPPING1,0x04 );

	return res;
}

//close active interrupt
s8_m mir3da_close_interrupt(void)
{
	s8_m   res = 0;

	res = mir3da_register_write(NSA_REG_INTERRUPT_SETTINGS1,0x00 );
	res = mir3da_register_write(NSA_REG_INTERRUPT_MAPPING1,0x00 );

	return res;
}

s8_m read_gsensor_id(void)
{
	s8_m res = 0;
	u8_m data_m = 0;
	//Retry 3 times
	res = mir3da_register_read(NSA_REG_WHO_AM_I,&data_m,1);
	if(data_m != 0x13)
	{
		res = mir3da_register_read(NSA_REG_WHO_AM_I,&data_m,1);
		if(data_m != 0x13)
		{
			res = mir3da_register_read(NSA_REG_WHO_AM_I,&data_m,1);
			if(data_m != 0x13)
			{
				LogPrintf(DEBUG_ALL,"Read gsensor chip id error =%x\r\n",data_m);
				return -1;
			}
		}
	}
	LogPrintf(DEBUG_FACTORY,"GSENSOR Chk. ID=0x%X\r\nGSENSOR CHK OK\r\n",data_m);
	return res;
}

s8_m readInterruptConfig(void)
{
    uint8_t data_m;
    mir3da_register_read(NSA_REG_INTERRUPT_SETTINGS1,&data_m,1);
    if(data_m !=0x87)
    {
        mir3da_register_read(NSA_REG_INTERRUPT_SETTINGS1,&data_m,1);
        if(data_m !=0x87)
        {
           return -1;
        }
    }
	LogPrintf(DEBUG_ALL,"Gsensor OK\n",data_m);
    return 0;
}


