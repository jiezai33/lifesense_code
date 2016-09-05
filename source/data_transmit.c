/***********************************************************************************
 * �� �� ��   : data_transmit.c
 * �� �� ��   : LiuYuanBin
 * ��������   : 2016��8��22��
 * ��Ȩ˵��   : Copyright (c) 2016-2025   �㶫����ҽ�Ƶ��ӹɷ����޹�˾
 * �ļ�����   : ���ļ��ǽ���Flash�ĸ������ݽ��д�������������Դ���״̬���Ĳ�ѯά���Ȳ�������Flash������ǰ�Ĵ��临���ԣ�
 * �޸���ʷ   : 
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
 * �� �� �� : app_transmit_statue_init
 * �������� : 
 * ������� : ��
 * ������� : ��
 * �� �� ֵ : ��
 * �޸���ʷ : ��
 * ˵    �� : ���ݴ���״̬���ʼ��
*****************************************************************************/
void app_transmit_statue_init(void)
{
	g_sequences = 5;
	memset(&g_transmit,0,sizeof(g_transmit));
}

/*****************************************************************************
 * �� �� �� : app_get_transmit_statue
 * �������� : 
 * ������� : uint16_t sequences  �������к�
 * ������� : uint8_t *index      ����״̬�±�
 * �� �� ֵ : 	0:�ҵ�����
 				1:û���ҵ�����
 * �޸���ʷ : ��
 * ˵    �� : ��������Ǹ��ݴ����sequences�����ݴ���״̬�������ѯ������ҵ���
 				�᷵����Ӧ���±�ֵ����index
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
 * �� �� �� : app_data_retransmission
 * �������� : 
 * ������� : transmit_statue_st * transmit  ���ݷ���״ָ̬��
 * ������� : ��
 * �� �� ֵ : ��
 * �޸���ʷ : ��
 * ˵    �� : ���������������Ҫ���·��͵�ʱ�򣬵���������������ܸ��ݴ������
 				�ݴ���״̬����������ȥflash��ȡ���ݡ���������Ѿ��������ˣ���
 				�ߴ�����µ����ݣ�����Ϊ���ͳɹ���,���״̬����Ӧ��һ�У�����
 				��ȡ�������·���
*****************************************************************************/
uint8_t app_data_retransmission(transmit_statue_st * transmit)
{
	//if ��������Ѿ��������ˣ����ߴ�����µ����ݣ�����Ϊ���ͳɹ���,���״̬����Ӧ��һ��
		//app_data_transmit_ack(transmit->sequences);
	//else 
		//�ط����ݣ�����app_data_transmit����
	return 0;
}

/*****************************************************************************
 * �� �� �� : app_transmit_time_out_one_second
 * �������� : 
 * ������� : ��
 * ������� : ��
 * �� �� ֵ : ��
 * �޸���ʷ : ��
 * ˵    �� : ����������ж�1��������ã�������״̬�������ѷ��ͣ���δ�õ�Ӧ��
 				�ı����timeOutʱ����м�1�����������ݽ�������ط����߶Ͽ�BLE
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
				else if(g_transmit[i].time_out%DATA_TRANSMIT_TIME_OUT_EVERY_TIME == 0)//��Ҫ�ط�
				{
					app_data_retransmission(&g_transmit[i]);
				}
			}
		}
	}
}

/*****************************************************************************
 * �� �� �� : app_data_transmit
 * �������� : 
 * ������� :  transmit_statue_st * transmit   ���ݴ���״ָ̬��
               uint8_t *data       ����ָ��
               uint8_t length      ���ݳ���
 * ������� : ��
 * �� �� ֵ : 	0:�������ݲ������
 				1:���͵�����״̬��������ʱ���ܽ��з���
 * �޸���ʷ : ��
 * ˵    �� : ����������������������ݺ��ط����ݵĵ��ú��������ݴ����sequences
 				�����֣�������app_add_heap_send_data������������ݷ��ͳ�ȥ
*****************************************************************************/
uint8_t app_data_transmit(transmit_statue_st * transmit,uint8_t *data,uint8_t length)
{
	uint8_t index,error;
	uint16_t l_sequences;

	error = 0;
	
	if(transmit->sequences == 0)//������������
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
	else if(transmit->sequences >= 5)//�ط�����
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
 * �� �� �� : app_data_transmit_ack
 * �������� : 
 * ������� : uint16_t sequences  �������к�
 * ������� : ��
 * �� �� ֵ : 0:OK
 * �޸���ʷ : ��
 * ˵    �� : ��������Ƿ��������Ժ󣬽��յ���APP��Ӧ�𣬲�����Ӧ���е�sequences
 				��״̬������Ӧ��һ�����,�����flash��������ݣ���Ҫ�����������
 				����Ӧ�ķ��ͼ�������1
*****************************************************************************/
uint8_t app_data_transmit_ack(uint16_t sequences)
{
	uint8_t index;
	if(app_get_transmit_statue(&index,sequences) == 0)
	{
		if(g_transmit[index].group)//��ʾ�����ڴ����������
		{
			//�������ܣ�Ӧ��ɹ�һ��
			//g_transmit[index].data_type
		}
	
		//Ȼ�������״̬��
		memset(&g_transmit[index],0,sizeof(transmit_statue_st));		
	}
	return 0;
}


