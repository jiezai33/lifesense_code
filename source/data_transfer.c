/***********************************************************************************
 * 文 件 名   : data_transfer.c
 * 创 建 者   : LiuYuanBin
 * 创建日期   : 2016年8月19日
 * 版权说明   : Copyright (c) 2016-2025   广东乐心医疗电子股份有限公司
 * 文件描述   : 本文件是进行Flash的各种数据进行传输给服务器，对传输状态机的查询维护等操作，简化Flash数据以前的传输复杂性！
 * 修改历史   : 
***********************************************************************************/

#include "data_transfer.h"
#include <string.h>
#include "channel_select.h"

#define STEP_DATA_ID_MAX			(10)
#define SLEEP_DATA_ID_MAX			(10)
#define HOUR_DATA_ID_MAX			(10)

#define SEND_DATA_TIME_OUT			(4)
#define SEND_DATA_TIME_OUT_COUNT	(3)

data_transfer_table_struct g_usr_data;

/*
	data_tye		data_id		earse_flag		time		time_out_count		flash_addr		flash_group_count
		1				0			0			0				3					0x10000				20
		1				1			0			0				3					0x20000				20
		1				2			0			0				3					0x30000				20
		1				3			0			0				3					0x40000				20

*/

flash_addr_struct g_step_data_type_addr[STEP_DATA_ID_MAX] = {
		{0x0000,10},
		{0x0010,10},
		{0x0020,10},
		{0x0030,10},
		{0x0040,10},
		{0x0050,10},
		{0x0060,10},
		{0x0070,10},
		{0x0080,10},
		{0x0090,10}
	};

flash_addr_struct g_sleep_data_type_addr[SLEEP_DATA_ID_MAX] = {
		{0x0000,10},
		{0x0010,10},
		{0x0020,10},
		{0x0030,10},
		{0x0040,10},
		{0x0050,10},
		{0x0060,10},
		{0x0070,10},
		{0x0080,10},
		{0x0090,10}
	};

flash_addr_struct g_hour_data_type_addr[SLEEP_DATA_ID_MAX] = {
		{0x0000,10},
		{0x0010,10},
		{0x0020,10},
		{0x0030,10},
		{0x0040,10},
		{0x0050,10},
		{0x0060,10},
		{0x0070,10},
		{0x0080,10},
		{0x0090,10}
	};

/*****************************************************************************
 * 函 数 名 : get_data_falsh_addr
 * 函数功能 : 
 * 输入参数 : uint8_t data_type  数据类型
               int8_t data_id     数据ID
 * 输出参数 : 无
 * 返 回 值 : 参考 flash_addr_struct 结构体
 * 修改历史 : 无
 * 说    明 : 根据传入的数据类型和ID获取对应的flash地址和组数量
*****************************************************************************/
flash_addr_struct *get_data_falsh_addr(uint8_t data_type,int8_t data_id)
{
	if(data_type != g_usr_data.statue.data_type)
		return NULL;

	if(data_id >= g_usr_data.statue.data_id_max)
		return NULL;

	return g_usr_data.data[data_id].addr;
}


/*****************************************************************************
 * 函 数 名 : data_transfer_statue_init
 * 函数功能 : 
 * 输入参数 : flash_data_enum data_type  数据类型
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 这个函数是在每次发数据前，把传输数据状态表的给初始化一次，因为目前传输的机制是每次只能传输一种类型的数据
 				所以可以根据传进来的数据类型进行初始化传输状态机，把相应的flash的地址赋值到状态机
*****************************************************************************/
void data_transfer_statue_init(flash_data_enum data_type)
{
	uint8_t size,i;
	flash_addr_struct *fash_addr;

	memset(&g_usr_data,0,sizeof(data_transfer_table_struct));
	
	if(data_type >= FLASH_DATA_MAX)
		return;
	
	g_usr_data.statue.data_type = data_type;
	
	switch(data_type)
	{
		case STEP_DATA:
			size = STEP_DATA_ID_MAX;
			fash_addr = &g_step_data_type_addr[0];
			break;
			
		case SLEEP_DATA:
			size = SLEEP_DATA_ID_MAX;
			fash_addr = &g_sleep_data_type_addr[0];
			break;
			
		case HOUR_DATA:
			size = HOUR_DATA_ID_MAX;
			fash_addr = &g_hour_data_type_addr[0];
			break;

		default:break;
	}

	g_usr_data.statue.data_id_max= size;
	
	for(i=0;i<size;i++)
	{
		g_usr_data.data[i].data_id 	= i;
		g_usr_data.data[i].addr 	= &(fash_addr[i]);
	}
}

/*****************************************************************************
 * 函 数 名 : data_retransmission
 * 函数功能 : 
 * 输入参数 : 	uint8_t data_type        数据类型 
 				uint8_t data_id          数据ID
                flash_addr_struct *addr  对应flash 的地址和组数量 
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 数据重复机制，当数据没有发送成功，调用这个函数进行重新发送相应包的数据
*****************************************************************************/
uint8_t data_retransmission(uint8_t data_type,int8_t data_id,flash_addr_struct *addr)
{
	//如果数据已经被擦除了，或者存放了新的数据，则置 data_type 和 data_id 对应的擦除位 为 1
	//ata_transmit_ack(data_type,data_id);
	return 0;
}

/*****************************************************************************
 * 函 数 名 : data_transmit
 * 函数功能 : 
 * 输入参数 :  uint8_t data_type  数据类型
               uint8_t data_id    数据ID
               uint8_t *data      数据指针
               uint8_t length     数据长度
 * 输出参数 : 无
 * 返 回 值 :   0:数据传输完成或者已经把当前包的数据传输到发送buffer去了
 				5:数据类型与当前传输数据状态机的 数据类型不一致
 				6:数据ID超过当前数据类型的ID的最大值
 				7:在超时的时间范围内，传输过当前包
 * 修改历史 : 无
 * 说    明 : 这个函数是flash查询是否有数据发送给APP，如果有的话调用下面的函数，发送相应的数据
 *****************************************************************************/
uint32_t data_transmit(uint8_t data_type,uint8_t data_id,uint8_t *data,uint8_t length)
{
	uint32_t current_time,error;
	if(data_type != g_usr_data.statue.data_type)
		return 5;

	if(data_id >= g_usr_data.statue.data_id_max)
		return 6;

	if(current_time - g_usr_data.data[data_id].time < SEND_DATA_TIME_OUT)
		return 7;
	
	if(g_usr_data.data[data_id].earse_flag == 1)
		return 0;

	//这个函数是调用数据发送的函数，不需要理会是WeChat或者lifesense的APP，而且发送的数据是纯flash里面的数据
	//没有包含数据包头的数据
	error = app_add_heap_send_data(data_type,data_id,data,length);

	if(error == 0)
	{	
		g_usr_data.data[data_id].data_id 	= data_id;
		g_usr_data.data[data_id].time 		= current_time;
		return 0;
	}
	else
	{
		return error;
	}
}

/*****************************************************************************
 * 函 数 名 : data_transmit_ack
 * 函数功能 : 
 * 输入参数 : uint8_t data_type  数据类型
               uint8_t data_id    数据ID
 * 输出参数 : 无
 * 返 回 值 :   0:当前数据包发送完
 				5:当前发送的数据类型不对
 				6:当前发送的数据ID越界
 * 修改历史 : 无
 * 说    明 : 同步或者异步发送数据后，得到服务器的应答包，在相应的包擦除位置1
*****************************************************************************/
uint8_t data_transmit_ack(uint8_t data_type,uint8_t data_id)
{
	if(data_type != g_usr_data.statue.data_type)
		return 5;

	if(data_id >= g_usr_data.statue.data_id_max)
		return 6;

	g_usr_data.data[data_id].earse_flag = 1;
	
	return 0;
}

/*****************************************************************************
 * 函 数 名 : data_transfer_statue_loop
 * 函数功能 : 
 * 输入参数 : void  无
 * 输出参数 : 无
 * 返 回 值 :   0:说明当前数据类型的所有包都已经发送完成，可以擦除了
 				1:重复或者是重复超过最大次数，进行蓝牙断开操作
 				5:状态或者类型错误
 * 修改历史 : 无
 * 说    明 : 这个函数mainloop里面循环调用
*****************************************************************************/
uint8_t data_transfer_statue_mainloop(void)
{
	uint8_t i,error;
	uint32_t current_time;

	if(g_usr_data.statue.data_type == 0 || g_usr_data.statue.data_type >= FLASH_DATA_MAX || g_usr_data.statue.data_id_max == 0)
		return 5;
	
	for(i=0;i<g_usr_data.statue.data_id_max;i++)
	{
		if(g_usr_data.data[i].earse_flag == 0)
			break;
	}

	if(i >= g_usr_data.statue.data_id_max)
		return 0;	//enable earse the data type section flash

	for(i=0;i<g_usr_data.statue.data_id_max;i++)
	{
		if(g_usr_data.data[i].time != 0 && g_usr_data.data[i].earse_flag == 0)
		{
			if(current_time - g_usr_data.data[i].time > SEND_DATA_TIME_OUT)//time out 时间超过了
			{
				if(g_usr_data.data[i].time_out_count < SEND_DATA_TIME_OUT_COUNT)//time out 次数超过了 
				{
					error = data_retransmission(g_usr_data.statue.data_type,i,g_usr_data.data[i].addr);//数据需要重发
					if(!error)
					{
						//g_usr_data[i].time = current_time;
						g_usr_data.data[i].time_out_count++;
					}
				}
				else
				{
					//ble_disconnect();
				}
				break;
			}
		}
	}

	return 1;
}


