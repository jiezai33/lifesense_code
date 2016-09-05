/***********************************************************************************
 * �� �� ��   : transfer_usrdesign.c
 * �� �� ��   : LiuYuanBin
 * ��������   : 2016��8��16��
 * ��Ȩ˵��   : Copyright (c) 2016-2025   �㶫����ҽ�Ƶ��ӹɷ����޹�˾
 * �ļ�����   : ����ble_transfer.c������ͨ������A500�������շ����
 * �޸���ʷ   : 
***********************************************************************************/
#include "transfer_usrdesign.h"
#include <string.h>
#include "app_trans.h"
#include "debug.h"
#include "usr_design.h"

#define TRANS_PACKAGE_HEAP_SIZE		(4)

#if DATA_TYPE == DATA_POINTER_TYPE
uint8_t g_trans_common_tx_buffer[TRANS_TX_SIZE];
uint8_t g_trans_common_rx_buffer[TRANS_RX_SIZE];
#endif

static trans_receive_pack_st g_receive_st = {0};
static trans_send_pack_st g_send_st = {0};

static uint32_t transfer_indicate_send(uint8_t *data,uint16_t length)
{
	return ble_trans_indicate_send(&m_trans, data);
}

static uint32_t transfer_notify_send(uint8_t *data,uint16_t length,uint8_t chnl)
{
	return ble_trans_notify_send(&m_trans, chnl, data);
}

/*****************************************************************************
 * �� �� �� : transfer_receive_parse
 * �������� : 
 * ������� :  uint8_t *data    ����ble_transferͨ��������
               uint16_t length  �������ݵĳ���
 * ������� : ��
 * �� �� ֵ : 	0:���ݽ������
 				1:���ݽ���δ���
 				2:֡��ų���һ���������֡��
 				3:���ݰ�ͷ��������
 * �޸���ʷ : ��
 * ˵    �� : ��
*****************************************************************************/
static uint8_t transfer_receive_parse(uint8_t *data,uint16_t length)
{
	uint8_t framelen = 0,offset = 0;
	
	if(g_receive_st.rec_flg == TRANS_RECEIVE_ED)
	{
		if(*data & 0x80)
		{
			g_receive_st.package_i 		= (*data & 0x7F);
			g_receive_st.package_i 		<<= 8;
			g_receive_st.package_i 		+= *(data+1);
			g_receive_st.package_len 	= *(data+2);
			g_receive_st.frame_i 		= *(data+3);

			g_receive_st.data_len		= length - 4;

			if(g_receive_st.data_len > g_receive_st.package_len)
				g_receive_st.data_len = g_receive_st.package_len;
			
			if(g_receive_st.package_len > TRANS_RECEIVE_DATA_SIZE)
				g_receive_st.package_len = TRANS_RECEIVE_DATA_SIZE;
			
			#if DATA_TYPE == DATA_BUFFER_TYPE			
			memset(g_receive_st.data, 0, sizeof(g_receive_st.data)); 
        	memcpy(g_receive_st.data, data + TRANS_PACKAGE_HEAP_SIZE, g_receive_st.data_len); 
        	#elif DATA_TYPE == DATA_POINTER_TYPE
			if(g_receive_st.data_len > TRANS_RX_SIZE)
				g_receive_st.data_len = TRANS_RX_SIZE;
			memset(&g_trans_common_rx_buffer[TRANS_RX_START_ADDR], 0, TRANS_RX_SIZE); 
        	memcpy(&g_trans_common_rx_buffer[TRANS_RX_START_ADDR], data + TRANS_PACKAGE_HEAP_SIZE, g_receive_st.data_len);
			g_receive_st.data = &(g_trans_common_rx_buffer[TRANS_RX_START_ADDR]);
			#endif

			g_receive_st.rec_flg = TRANS_RECEIVE_ING;
		}
		else
			return 3;
	}
	else if(g_receive_st.rec_flg == TRANS_RECEIVE_ING)
	{
		g_receive_st.frame_i = *data;                        // ֡���
        framelen = (length - 1);                     // һ֡���ݵĳ���
        offset = 16 + (g_receive_st.frame_i - 1) * 19; // ����buffer��ƫ�Ƶ�ַ

        if(g_receive_st.frame_i > TRANS_DEF_MAX_FRAME_SIZE) // ���֡��ų���һ���������֡�������buffer���½���
        {
            g_receive_st.data_len = 0; // ��ս��ճ���
            g_receive_st.package_len = 0;
            memset(g_receive_st.data, 0, sizeof(g_receive_st.data)); // ���buffer
            g_receive_st.rec_flg = 0;
			return 2;
        }

        if((g_receive_st.data_len + framelen) > g_receive_st.package_len) // ������ݳ��ȳ�����ǰ�����ȣ��ض϶��������
        {
            framelen = g_receive_st.package_len - g_receive_st.data_len;
        }
        g_receive_st.data_len += framelen;
        memcpy(&g_receive_st.data[offset], data + 1, framelen); // ��������
	}

	// �жϵ�ǰ���Ƿ�������
    if(g_receive_st.data_len < g_receive_st.package_len)
    {
        return 1;
    }
	else
	{
		g_receive_st.rec_flg = TRANS_RECEIVE_ED;
		return 0;
	}	
}

void transfer_connection(void)
{
	memset(&g_send_st, 0, sizeof(g_send_st));
	memset(&g_receive_st, 0, sizeof(g_receive_st));

	app_trans_connection();
}

void transfer_disconnection(void)
{
	memset(&g_send_st, 0, sizeof(g_send_st));
	memset(&g_receive_st, 0, sizeof(g_receive_st));

	app_trans_disconnection();
}
void trans_indicate_statue_set(ble_trans_evt_type_t statue)
{
	app_trans_indicate_statue_set(statue);

	if(statue == BLE_TRANS_EVT_INDICATION_CONFIRMED)
		g_send_st.indicate_available_flg = 0;
}

/*****************************************************************************
 * �� �� �� : transfer_receive
 * �������� : 
 * ������� :  uint8_t *data    ��ble_transferͨ��������յ�����
               uint16_t length  �������ݵĳ���
 * ������� : ��
 * �� �� ֵ : 	0:���ݽ������
 				1:���ݽ���δ���
 				2:֡��ų���һ���������֡��
 				3:���ݰ�ͷ��������
 * �޸���ʷ : ��
 * ˵    �� : ��
*****************************************************************************/
uint32_t transfer_receive(uint8_t *data,uint16_t length)
{
	uint32_t error = 0;
	error = transfer_receive_parse(data,length);
	if(error == 0)
	{
		app_trans_receive(g_receive_st.data,g_receive_st.data_len);
	}
	else
	{
		QPRINTF("error =%d\r\n",error);
	}
	return error;
}

/*****************************************************************************
 * �� �� �� : transfer_send_data
 * �������� : 
 * ������� :  uint8_t *data                     ���͵�����
               uint16_t length                   �������ݵĳ���
               wechat_channel_enum channel_type  A500��indicate����notifiͨ��ѡ��
               uint8_t channel                   Ŀǰ��notifiͨ���ڼ���ͨ��ѡ��
 * ������� : ��
 * �� �� ֵ : 	0:OK 	
 				1:ͨ��ѡ�����	
 				2:�ϴε����ݻ�û�з������
 * �޸���ʷ : ��
 * ˵    �� : ��
*****************************************************************************/
uint32_t transfer_send_data(uint8_t *data,uint16_t length,trans_channel_enum channel_type,uint8_t channel)
{
	uint32_t error = 0,i = 0;

	if(length > TRANS_SEND_DATA_SIZE)
		length = TRANS_SEND_DATA_SIZE;

	if(channel_type >= TRANS_CHANNEL_MAX)
		return 1;

	if(g_send_st.send_flg)//�ϴ�����Ϊ����
		return 2;

	#if DATA_TYPE == DATA_BUFFER_TYPE
	memset(g_send_st.data,0,TRANS_SEND_DATA_SIZE);
	
	for(i=0;i<length;i++)
		g_send_st.data[i] = data[i];
	#elif DATA_TYPE == DATA_POINTER_TYPE
	memset(&g_trans_common_tx_buffer[TRANS_TX_START_ADDR], 0, TRANS_TX_SIZE);
	for(i=0;i<length;i++)
		g_trans_common_tx_buffer[TRANS_TX_START_ADDR+i] = data[i];
	g_send_st.data = &g_trans_common_tx_buffer[TRANS_TX_START_ADDR];
	#endif
	
	g_send_st.send_index 	= 0;
	g_send_st.data_len 		= length;
	g_send_st.channel_type 	= channel_type;
	g_send_st.channel 		= channel;
	g_send_st.send_flg 		= 1;
	
	return error;
}

/*****************************************************************************
 * �� �� �� : transfer_periodic_send_data
 * �������� : 
 * ������� : void  void
 * ������� : void
 * �� �� ֵ : 	0:������ɻ���û�����ݷ���		
 				1:�����ݷ���
 * �޸���ʷ : ��
 * ˵    �� : A500 1�������ݷֶ�֡���ͣ�ÿ֡20byte����mainloopѭ�����淢��
*****************************************************************************/
uint8_t transfer_periodic_send_data(void)
{
	uint32_t error;
	uint16_t length = 0;
	
	if(g_send_st.send_flg == 1 && g_send_st.channel_type < TRANS_CHANNEL_MAX)
	{
		length = g_send_st.data_len - g_send_st.send_index;
		if(length > 20)
		{
			length = 20;
		}
		
		if(g_send_st.channel_type == TRANS_INDICATE_CHANNEL)
		{
			if(g_send_st.indicate_available_flg == 0)
			{
				g_send_st.indicate_available_flg = 1;
				error = transfer_indicate_send(&g_send_st.data[g_send_st.send_index],length);
			}
			else
				error = g_send_st.indicate_available_flg;//incadicate is busy!!!
		}
		else if(g_send_st.channel_type == TRANS_NOTIFI_CHANNEL)
		{
			error = transfer_notify_send(&g_send_st.data[g_send_st.send_index],length,g_send_st.channel);
		}

		if(error == NRF_SUCCESS)
		{
			g_send_st.send_index += length;
			if(g_send_st.send_index >= g_send_st.data_len)
			{
				g_send_st.send_flg = 0;
			}
			QPRINTF("transfer:send_index=%d,\r\n",g_send_st.send_index);
		}
	}
	return g_send_st.send_flg;
}

