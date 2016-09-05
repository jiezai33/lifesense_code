/***********************************************************************************
 * �� �� ��   : data_transfer.c
 * �� �� ��   : LiuYuanBin
 * ��������   : 2016��8��19��
 * ��Ȩ˵��   : Copyright (c) 2016-2025   �㶫����ҽ�Ƶ��ӹɷ����޹�˾
 * �ļ�����   : ���ļ��ǽ���Flash�ĸ������ݽ��д�������������Դ���״̬���Ĳ�ѯά���Ȳ�������Flash������ǰ�Ĵ��临���ԣ�
 * �޸���ʷ   : 
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
 * �� �� �� : get_data_falsh_addr
 * �������� : 
 * ������� : uint8_t data_type  ��������
               int8_t data_id     ����ID
 * ������� : ��
 * �� �� ֵ : �ο� flash_addr_struct �ṹ��
 * �޸���ʷ : ��
 * ˵    �� : ���ݴ�����������ͺ�ID��ȡ��Ӧ��flash��ַ��������
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
 * �� �� �� : data_transfer_statue_init
 * �������� : 
 * ������� : flash_data_enum data_type  ��������
 * ������� : ��
 * �� �� ֵ : ��
 * �޸���ʷ : ��
 * ˵    �� : �����������ÿ�η�����ǰ���Ѵ�������״̬��ĸ���ʼ��һ�Σ���ΪĿǰ����Ļ�����ÿ��ֻ�ܴ���һ�����͵�����
 				���Կ��Ը��ݴ��������������ͽ��г�ʼ������״̬��������Ӧ��flash�ĵ�ַ��ֵ��״̬��
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
 * �� �� �� : data_retransmission
 * �������� : 
 * ������� : 	uint8_t data_type        �������� 
 				uint8_t data_id          ����ID
                flash_addr_struct *addr  ��Ӧflash �ĵ�ַ�������� 
 * ������� : ��
 * �� �� ֵ : ��
 * �޸���ʷ : ��
 * ˵    �� : �����ظ����ƣ�������û�з��ͳɹ���������������������·�����Ӧ��������
*****************************************************************************/
uint8_t data_retransmission(uint8_t data_type,int8_t data_id,flash_addr_struct *addr)
{
	//��������Ѿ��������ˣ����ߴ�����µ����ݣ����� data_type �� data_id ��Ӧ�Ĳ���λ Ϊ 1
	//ata_transmit_ack(data_type,data_id);
	return 0;
}

/*****************************************************************************
 * �� �� �� : data_transmit
 * �������� : 
 * ������� :  uint8_t data_type  ��������
               uint8_t data_id    ����ID
               uint8_t *data      ����ָ��
               uint8_t length     ���ݳ���
 * ������� : ��
 * �� �� ֵ :   0:���ݴ�����ɻ����Ѿ��ѵ�ǰ�������ݴ��䵽����bufferȥ��
 				5:���������뵱ǰ��������״̬���� �������Ͳ�һ��
 				6:����ID������ǰ�������͵�ID�����ֵ
 				7:�ڳ�ʱ��ʱ�䷶Χ�ڣ��������ǰ��
 * �޸���ʷ : ��
 * ˵    �� : ���������flash��ѯ�Ƿ������ݷ��͸�APP������еĻ���������ĺ�����������Ӧ������
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

	//��������ǵ������ݷ��͵ĺ���������Ҫ�����WeChat����lifesense��APP�����ҷ��͵������Ǵ�flash���������
	//û�а������ݰ�ͷ������
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
 * �� �� �� : data_transmit_ack
 * �������� : 
 * ������� : uint8_t data_type  ��������
               uint8_t data_id    ����ID
 * ������� : ��
 * �� �� ֵ :   0:��ǰ���ݰ�������
 				5:��ǰ���͵��������Ͳ���
 				6:��ǰ���͵�����IDԽ��
 * �޸���ʷ : ��
 * ˵    �� : ͬ�������첽�������ݺ󣬵õ���������Ӧ���������Ӧ�İ�����λ��1
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
 * �� �� �� : data_transfer_statue_loop
 * �������� : 
 * ������� : void  ��
 * ������� : ��
 * �� �� ֵ :   0:˵����ǰ�������͵����а����Ѿ�������ɣ����Բ�����
 				1:�ظ��������ظ����������������������Ͽ�����
 				5:״̬�������ʹ���
 * �޸���ʷ : ��
 * ˵    �� : �������mainloop����ѭ������
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
			if(current_time - g_usr_data.data[i].time > SEND_DATA_TIME_OUT)//time out ʱ�䳬����
			{
				if(g_usr_data.data[i].time_out_count < SEND_DATA_TIME_OUT_COUNT)//time out ���������� 
				{
					error = data_retransmission(g_usr_data.statue.data_type,i,g_usr_data.data[i].addr);//������Ҫ�ط�
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


