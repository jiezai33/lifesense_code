/***********************************************************************************
 * 文 件 名   : data_transmit.c
 * 创 建 者   : LiuYuanBin
 * 创建日期   : 2016年8月22日
 * 版权说明   : Copyright (c) 2016-2025   广东乐心医疗电子股份有限公司
 * 文件描述   : 本文件是进行Flash的各种数据进行传输给服务器，对传输状态机的查询维护等操作，简化Flash数据以前的传输复杂性！
 * 修改历史   : 
***********************************************************************************/

#include "data_transmit.h"
#include <string.h>
static transmit_statue_st g_transmit[DATA_TRANSMIT_GROUP_SIZE];
static uint16_t g_sequences = 5;

/*
	data_tye		sequences		flash_addr		flash_group_count		time_out
		1			0x0000			0x10000				12						20
		1			0x0001			0x20000				11						20
		1			0x0002			0x30000				21						20
		1			0x0003			0x40000				32						20
		.				.			.					.						.
		.				.			.					.						.
		.				.			.					.						.
*/


static uint32_t app_add_heap_send_data(uint16_t secquences,uint8_t *data,uint8_t length)
{
	return 0;
}

/*****************************************************************************
 * 函 数 名 : app_transmit_statue_init
 * 函数功能 : 
 * 输入参数 : 无
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 数据传输状态表初始化
*****************************************************************************/
void app_transmit_statue_init(void)
{
	g_sequences = 5;
	memset(&g_transmit,0,sizeof(g_transmit));
}

/*****************************************************************************
 * 函 数 名 : app_get_transmit_statue
 * 函数功能 : 
 * 输入参数 : uint16_t sequences  发送序列号
 * 输出参数 : uint8_t *index      数据状态下标
 * 返 回 值 : 	0:找到数据
 				1:没有找到数据
 * 修改历史 : 无
 * 说    明 : 这个函数是根据传入的sequences，数据传输状态表里面查询，如果找到了
 				会返回相应的下标值房子index
*****************************************************************************/
uint8_t app_get_transmit_statue(uint8_t *index,uint16_t sequences)
{
	uint8_t i,find;

	find = 255;
	for(i=0;i<DATA_TRANSMIT_GROUP_SIZE;i++)
	{
		if(g_transmit[i].sequences == sequences)
		{
			find = 0;
			*index = i;
		}
	}

	return find;
}

/*****************************************************************************
 * 函 数 名 : app_data_retransmission
 * 函数功能 : 
 * 输入参数 : transmit_statue_st * transmit  数据发送状态指针
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 这个函数当数据需要重新发送的时候，调用这个函数，邝总根据传入的数
 				据传输状态参数，进行去flash读取数据。如果数据已经被擦除了，或
 				者存放了新的数据，则认为发送成功了,清除状态表相应的一行，否则
 				读取数据重新发送
*****************************************************************************/
uint8_t app_data_retransmission(transmit_statue_st * transmit)
{
	//if 如果数据已经被擦除了，或者存放了新的数据，则认为发送成功了,清除状态表相应的一行
		//app_data_transmit_ack(transmit->sequences);
	//else 
		//重发数据，调用app_data_transmit函数
	return 0;
}

/*****************************************************************************
 * 函 数 名 : app_transmit_time_out_one_second
 * 函数功能 : 
 * 输入参数 : 无
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 这个函数在中断1秒里面调用，把数据状态表里面已发送，但未得到应答
 				的表项的timeOut时间进行减1操作，并根据结果进行重发或者断开BLE
*****************************************************************************/
void app_transmit_time_out_one_second(void)
{
	uint8_t i;
	for(i=0;i<DATA_TRANSMIT_GROUP_SIZE;i++)
	{
		if(g_transmit[i].sequences >= 5)
		{
			if(g_transmit[i].time_out > 0)
			{
				g_transmit[i].time_out--;
				if(g_transmit[i].time_out == 0)
				{
					//ble_disconnection();
				}
				else if(g_transmit[i].time_out%DATA_TRANSMIT_TIME_OUT_EVERY_TIME == 0)//需要重发
				{
					app_data_retransmission(&g_transmit[i]);
				}
			}
		}
	}
}

/*****************************************************************************
 * 函 数 名 : app_data_transmit
 * 函数功能 : 
 * 输入参数 :  transmit_statue_st * transmit   数据传输状态指针
               uint8_t *data       数据指针
               uint8_t length      数据长度
 * 输出参数 : 无
 * 返 回 值 : 	0:发送数据操作完成
 				1:发送的数据状态表满，暂时不能进行发送
 * 修改历史 : 无
 * 说    明 : 这个函数邝总主动发送数据和重发数据的调用函数，根据传入的sequences
 				来区分，并调用app_add_heap_send_data这个函数把数据发送出去
*****************************************************************************/
uint8_t app_data_transmit(transmit_statue_st * transmit,uint8_t *data,uint8_t length)
{
	uint8_t index,error;
	uint16_t l_sequences;

	error = 0;
	
	if(transmit->sequences == 0)//主动发送数据
	{		
		if(app_get_transmit_statue(&index,0) == 0)
		{
			g_transmit[index].data_type = transmit->data_type;
			g_transmit[index].sequences = g_sequences++;
			g_transmit[index].addr 		= transmit->addr;
			g_transmit[index].group		= transmit->group;
			g_transmit[index].time_out	= DATA_TRANSMIT_TIME_OUT_TOTAL_TIME;

			l_sequences = g_transmit[index].sequences;
			
			error = 0;
		}
		else
		{
			error = 1;
		}
	}
	else if(transmit->sequences >= 5)//重发数据
	{
		l_sequences = transmit->sequences;
		error = 0;
	}

	if(error == 0)
	{
		error = app_add_heap_send_data(l_sequences,data,length);
	}

	return error;
}

/*****************************************************************************
 * 函 数 名 : app_data_transmit_ack
 * 函数功能 : 
 * 输入参数 : uint16_t sequences  发送序列号
 * 输出参数 : 无
 * 返 回 值 : 0:OK
 * 修改历史 : 无
 * 说    明 : 这个函数是发送数据以后，接收到了APP的应答，并根据应答中的sequences
 				把状态表中相应的一项清除,如果是flash里面的数据，则要告诉邝总这边
 				的相应的发送计数器减1
*****************************************************************************/
uint8_t app_data_transmit_ack(uint16_t sequences)
{
	uint8_t index;
	if(app_get_transmit_statue(&index,sequences) == 0)
	{
		if(g_transmit[index].group)//表示不是内存里面的数据
		{
			//告诉邝总，应答成功一次
			//g_transmit[index].data_type
		}
	
		//然后再清除状态表
		memset(&g_transmit[index],0,sizeof(transmit_statue_st));		
	}
	return 0;
}


