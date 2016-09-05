/***********************************************************************************
 * �� �� ��   : wechat_usrdesign.c
 * �� �� ��   : LiuYuanBin
 * ��������   : 2016��8��16��
 * ��Ȩ˵��   : Copyright (c) 2016-2025   �㶫����ҽ�Ƶ��ӹɷ����޹�˾
 * �ļ�����   : ����ble_wechat.c��������ͨ��������΢�ŵ����ݷ��ͺͽ��չ������
 * �޸���ʷ   : 
***********************************************************************************/

#include "wechat_usrdesign.h"
#include <string.h>
#include "ble_wechat.h"
#include "app_wechat.h"
#include "debug.h"
#define WECHAT_PACKAGE_HEAP_SIZE		(8)

#if DATA_TYPE == DATA_POINTER_TYPE
uint8_t g_wechat_common_tx_buffer[WECHAT_TX_SIZE];
uint8_t g_wechat_common_rx_buffer[WECHAT_RX_SIZE];
#endif
static wechat_receive_pack_st g_receive_st = {0};
static wechat_send_pack_st g_send_st = {0};

static uint32_t wechat_indicate_send(uint8_t *data,uint16_t length)
{
	return ble_wechat_indicate_send(&m_wechat, data);
}

static uint32_t wechat_notify_send(uint8_t *data,uint16_t length,uint8_t chnl)
{
	return 0;
}

/*****************************************************************************
 * �� �� �� : wechat_receive_parse
 * �������� : 
 * ������� : uint8_t *data    ����ble_wechatͨ��������
               uint16_t length  �������ݵĳ���
 * ������� : ��
 * �� �� ֵ : 	0:���ݽ������
 				1:���ݽ���δ���
 				3:���ݰ�ͷ��������
 * �޸���ʷ : ��
 * ˵    �� : ��
*****************************************************************************/
static uint8_t wechat_receive_parse(uint8_t *data,uint16_t length)
{
	uint8_t data_len = 0,offset = 0;
	
	if(g_receive_st.rec_flg == WECHAT_RECEIVE_ED)
	{
		if(*data == 0xFE && *(data+1) == 0x01)
		{
			g_receive_st.package_len 	= *(data+2);
			g_receive_st.package_len 	<<= 8;
			g_receive_st.package_len 	+= *(data+3);
			g_receive_st.cmd_no 		= *(data+4);
			g_receive_st.cmd_no 		<<= 8;
			g_receive_st.cmd_no 		+= *(data+5);

			g_receive_st.send_i 		= *(data+6);
			g_receive_st.send_i 		<<= 8;
			g_receive_st.send_i 		+= *(data+7);

			g_receive_st.data_len		= length - WECHAT_PACKAGE_HEAP_SIZE;
			
			if(g_receive_st.package_len > WECHAT_RECEIVE_DATA_SIZE)
				g_receive_st.package_len = WECHAT_RECEIVE_DATA_SIZE;

			#if DATA_TYPE == DATA_BUFFER_TYPE
			memset(g_receive_st.data, 0, sizeof(g_receive_st.data)); 
        	memcpy(g_receive_st.data, data + WECHAT_PACKAGE_HEAP_SIZE, g_receive_st.data_len); 
        	#elif DATA_TYPE == DATA_POINTER_TYPE
			if(g_receive_st.data_len > WECHAT_RX_SIZE)
				g_receive_st.data_len = WECHAT_RX_SIZE;
			memset(&g_wechat_common_rx_buffer[WECHAT_RX_START_ADDR], 0, WECHAT_RX_SIZE); 
        	memcpy(&g_wechat_common_rx_buffer[WECHAT_RX_START_ADDR], data + WECHAT_PACKAGE_HEAP_SIZE, g_receive_st.data_len);
			g_receive_st.data = &(g_wechat_common_rx_buffer[WECHAT_RX_START_ADDR]);
			#endif
			
			g_receive_st.rec_flg = WECHAT_RECEIVE_ING;
		}
		else
			return 3;
	}
	else if(g_receive_st.rec_flg == WECHAT_RECEIVE_ING)
	{
        offset = g_receive_st.data_len; // ����buffer��ƫ�Ƶ�ַ
		data_len = length;
		
        if((g_receive_st.data_len + length + WECHAT_PACKAGE_HEAP_SIZE) > g_receive_st.package_len) // ������ݳ��ȳ�����ǰ�����ȣ��ض϶��������
        {
            data_len = g_receive_st.package_len - g_receive_st.data_len - WECHAT_PACKAGE_HEAP_SIZE;
        }
		
        g_receive_st.data_len += data_len;
        memcpy(&g_receive_st.data[offset], data, data_len); // ��������     
	}

	// �жϵ�ǰ���Ƿ�������
    if((g_receive_st.data_len+WECHAT_PACKAGE_HEAP_SIZE) < g_receive_st.package_len)
    {
        return 1;
    }
	else
	{
		g_receive_st.rec_flg = WECHAT_RECEIVE_ED;
		return 0;
	}	
}

void wechat_connection(void)
{
	memset(&g_send_st, 0, sizeof(g_send_st));
	memset(&g_receive_st, 0, sizeof(g_receive_st));
	app_wechat_connection();
}

void wechat_disconnection(void)
{
	memset(&g_send_st, 0, sizeof(g_send_st));
	memset(&g_receive_st, 0, sizeof(g_receive_st));
	app_wechat_disconnection();
}

void wechat_indicate_statue_set(ble_wechat_evt_type_t statue)
{
	app_wechat_indicate_statue_set(statue);
	if(statue == BLE_WECHAT_EVT_INDICATION_CONFIRMED)
		g_send_st.indicate_available_flg = 0;
}

/*****************************************************************************
 * �� �� �� : wechat_receive
 * �������� : 
 * ������� :  uint8_t *data    ��ble_wechatͨ��������յ�����
               uint16_t length  �������ݵĳ���
 * ������� : ��
 * �� �� ֵ : 	0:���ݽ������
 				1:���ݽ���δ���
 				3:���ݰ�ͷ��������
 * �޸���ʷ : ��
 * ˵    �� : ��
*****************************************************************************/
uint32_t wechat_receive(uint8_t *data,uint16_t length)
{
	uint32_t error = 0,i=0;
	error = wechat_receive_parse(data,length);
	if(error == 0)
	{
		QPRINTF("\r\nwechat recevie:*************************************\r\n");
		QPRINTF("rec_flg=%d,\r\n",	g_receive_st.rec_flg);
		QPRINTF("package_len=%d,\r\n",g_receive_st.package_len);
		QPRINTF("cmd_no=%d,\r\n",	g_receive_st.cmd_no);
		QPRINTF("send_i=%d,\r\n",	g_receive_st.send_i);
		QPRINTF("data_len=%d,\r\n",	g_receive_st.data_len);
		QPRINTF("data:\r\n");
		for(i=0;i<g_receive_st.data_len;i++)
		{
			QPRINTF("%02x,",g_receive_st.data[i]);
		}
		QPRINTF("\r\n");
		QPRINTF("*************************************\r\n\r\n");
		app_wechat_receive(g_receive_st.cmd_no,g_receive_st.data,g_receive_st.data_len);
	}

	return error;
}

/*****************************************************************************
 * �� �� �� : wechat_send_data
 * �������� : 
 * ������� :  uint8_t *data                     ���͵�����
               uint16_t length                   �������ݵĳ���
               wechat_channel_enum channel_type  ΢�ŵ�indicate����notifiͨ��ѡ��
               uint8_t channel                   Ŀǰ��notifiͨ���ڼ���ͨ��ѡ��
 * ������� : ��
 * �� �� ֵ : 	0:OK 	
 				1:ͨ��ѡ�����	
 				2:�ϴε����ݻ�û�з������
 * �޸���ʷ : ��
 * ˵    �� : ��
*****************************************************************************/
uint32_t wechat_send_data(uint8_t *data,uint16_t length,wechat_channel_enum channel_type,uint8_t channel)
{
	uint32_t error = 0,i = 0;

	if(length > WECHAT_SEND_DATA_SIZE)
		length = WECHAT_SEND_DATA_SIZE;

	if(channel_type >= WECHAT_CHANNEL_MAX)
		return 1;
	
	if(g_send_st.send_flg)//�ϴ�����û����
		return 2;
	
	#if DATA_TYPE == DATA_BUFFER_TYPE
	memset(g_send_st.data,0,WECHAT_SEND_DATA_SIZE);
	
	for(i=0;i<length;i++)
		g_send_st.data[i] = data[i];
	#elif DATA_TYPE == DATA_POINTER_TYPE
	memset(&g_wechat_common_tx_buffer[WECHAT_TX_START_ADDR], 0, WECHAT_TX_SIZE);
	for(i=0;i<length;i++)
		g_wechat_common_tx_buffer[WECHAT_TX_START_ADDR+i] = data[i];
	g_send_st.data = &g_wechat_common_tx_buffer[WECHAT_TX_START_ADDR];
	#endif
	
	g_send_st.data_len 		= length;
	g_send_st.send_index 	= 0;
	g_send_st.channel_type 	= channel_type;
	g_send_st.channel 		= channel;
	g_send_st.send_flg 		= 1;

	return error;
}

/*****************************************************************************
 * �� �� �� : wechat_periodic_send_data
 * �������� : 
 * ������� : void  void
 * ������� : void
 * �� �� ֵ : 	0:������ɻ���û�����ݷ���		
 				1:�����ݷ���
 * �޸���ʷ : ��
 * ˵    �� : ΢��1�������ݷֶ�֡���ͣ�ÿ֡20byte����mainloopѭ�����淢��
*****************************************************************************/
uint8_t wechat_periodic_send_data(void)
{
	uint32_t error;
	uint16_t length = 0;

	if(g_send_st.send_flg == 1 && g_send_st.channel_type < WECHAT_CHANNEL_MAX)
	{
		length = g_send_st.data_len - g_send_st.send_index;
		if(length > 20)
		{
			length = 20;
		}
		
		if(g_send_st.channel_type == WECHAT_INDICATE_CHANNEL)
		{
			if(g_send_st.indicate_available_flg == 0)
			{
				g_send_st.indicate_available_flg = 1;
				error = wechat_indicate_send(&g_send_st.data[g_send_st.send_index],length);
			}
			else
				error = g_send_st.indicate_available_flg;
			
		}
		else if(g_send_st.channel_type == WECHAT_NOTIFI_CHANNEL)
		{
			error = wechat_notify_send(&g_send_st.data[g_send_st.send_index],length,g_send_st.channel);
		}

		if(error == NRF_SUCCESS)
		{
			g_send_st.send_index += length;
			if(g_send_st.send_index >= g_send_st.data_len)
			{
				g_send_st.send_flg = 0;
			}
			QPRINTF("wechat:send_index=%d,\r\n",g_send_st.send_index);
		}	
	}
	return g_send_st.send_flg;
}


