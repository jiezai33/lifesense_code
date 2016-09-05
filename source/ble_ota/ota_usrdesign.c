/***********************************************************************************
 * 文 件 名   : ota_usrdesign.c
 * 创 建 者   : LiuYuanBin
 * 创建日期   : 2016年8月16日
 * 版权说明   : Copyright (c) 2016-2025   广东乐心医疗电子股份有限公司
 * 文件描述   : 基于ble_ota.c的数据通道进行OTA的数据收发组包
 * 修改历史   : 
***********************************************************************************/
#include "ota_usrdesign.h"
#include <string.h>
#include "ble_ota.h"
#include "app_ota.h"
#include "debug.h"

#define	BLE_FRAME_LENGTH					0x14				/* OTA数据帧的长度 */

#if DATA_TYPE == DATA_POINTER_TYPE
uint8_t g_ota_common_tx_buffer[OTA_TX_SIZE];
uint8_t g_ota_common_rx_buffer[OTA_RX_SIZE];
#endif

static ota_receive_pack_st g_receive_st = {0};
static ota_send_pack_st g_send_st = {0};

static uint32_t ota_indicate_send(uint8_t *data,uint16_t length)
{
	return ble_ota_indicate_send(&m_ota, data,length);
}

static uint32_t ota_notify_send(uint8_t *data,uint16_t length,uint8_t chnl)
{
	return 0;
}

/*****************************************************************************
 * 函 数 名 : ota_cmd_receive_parse
 * 函数功能 : 
 * 输入参数 :  uint8_t *data    接收ble_ota通道的数据
               uint16_t length  接收数据的长度
 * 输出参数 : 无
 * 返 回 值 : 	0:数据接收完成
 				1:数据接收未完成
 				3:数据包头解析错误
 * 修改历史 : 无
 * 说    明 : 无
*****************************************************************************/
static uint32_t ota_cmd_receive_parse(uint8_t *data,uint16_t length)
{
	uint8_t i = 0;

	g_receive_st.cmd_i = *(data++);
	g_receive_st.frame_i = *(data++);
		
	if(g_receive_st.rec_flg == OTA_RECEIVE_ED)
	{
		
		if(g_receive_st.frame_i == 0x01)
		{
			g_receive_st.package_len = *(data++);
			g_receive_st.data_len = (length - 3);

			if(g_receive_st.package_len > OTA_RECEIVE_DATA_SIZE)
				g_receive_st.package_len = OTA_RECEIVE_DATA_SIZE;

			#if DATA_TYPE == DATA_BUFFER_TYPE
			memset(g_receive_st.data, 0, sizeof(g_receive_st.data)); 
        	memcpy(g_receive_st.data, data, g_receive_st.data_len); 
        	#elif DATA_TYPE == DATA_POINTER_TYPE
			if(g_receive_st.data_len > OTA_RX_SIZE)
				g_receive_st.data_len = OTA_RX_SIZE;
			memset(&g_ota_common_rx_buffer[OTA_RX_START_ADDR], 0, OTA_RX_SIZE); 
        	memcpy(&g_ota_common_rx_buffer[OTA_RX_START_ADDR], data, g_receive_st.data_len);
			g_receive_st.data = &(g_ota_common_rx_buffer[OTA_RX_START_ADDR]);
			#endif

			g_receive_st.rec_flg = OTA_RECEIVE_ING;
		}
		else
			return 3;
	}
	else if(g_receive_st.rec_flg == OTA_RECEIVE_ING)
	{       	
        for(i=0; i<(length-2); i++)
        {
            g_receive_st.data[g_receive_st.data_len++] = *(data++);
        }

		if(g_receive_st.data_len > g_receive_st.package_len)
			g_receive_st.data_len = g_receive_st.package_len;
	}

	// 判断当前包是否接收完成
    if(g_receive_st.data_len < g_receive_st.package_len)
    {
        return 1;
    }
	else
	{
		g_receive_st.rec_flg = OTA_RECEIVE_ED;
		return 0;
	}	
}

static uint32_t ota_data_receive_parse(uint8_t *data,uint16_t length)
{
	uint8_t i = 0;

	g_receive_st.cmd_i = *(data++);
	g_receive_st.frame_i = *(data++);
		
	if(g_receive_st.rec_flg == OTA_RECEIVE_ED)
	{
		
		if(g_receive_st.frame_i == 0x01)
		{
			g_receive_st.package_len = *(data++);
			g_receive_st.data_len = (length - 3);

			if(g_receive_st.package_len > OTA_RECEIVE_DATA_SIZE)
				g_receive_st.package_len = OTA_RECEIVE_DATA_SIZE;

			#if DATA_TYPE == DATA_BUFFER_TYPE
			memset(g_receive_st.data, 0, sizeof(g_receive_st.data)); 
        	memcpy(g_receive_st.data, data, g_receive_st.data_len); 
        	#elif DATA_TYPE == DATA_POINTER_TYPE
			if(g_receive_st.data_len > OTA_RX_SIZE)
				g_receive_st.data_len = OTA_RX_SIZE;
			memset(&g_ota_common_rx_buffer[OTA_RX_START_ADDR], 0, OTA_RX_SIZE); 
        	memcpy(&g_ota_common_rx_buffer[OTA_RX_START_ADDR], data, g_receive_st.data_len);
			g_receive_st.data = &(g_ota_common_rx_buffer[OTA_RX_START_ADDR]);
			#endif

			if(g_receive_st.data_len >= g_receive_st.package_len)
			{
				return 0;
			}
			else
			{
				g_receive_st.rec_flg = OTA_RECEIVE_ING;
			}
		}
		else
			return 3;
	}
	else if(g_receive_st.rec_flg == OTA_RECEIVE_ING)
	{			
		for(i=0; i<(length-2); i++)
		{
			g_receive_st.data[g_receive_st.data_len++] = *(data++);
		}

		if(g_receive_st.data_len > g_receive_st.package_len)
			g_receive_st.data_len = g_receive_st.package_len;
	}

	// 判断当前包是否接收完成
	if(g_receive_st.data_len < g_receive_st.package_len)
	{
		return 1;
	}
	else
	{
		g_receive_st.rec_flg = OTA_RECEIVE_ED;
		return 0;
	}	
}
void ota_connection(void)
{
	memset(&g_send_st, 0, sizeof(g_send_st));
	memset(&g_receive_st, 0, sizeof(g_receive_st));
	app_ota_connection();
}

void ota_disconnection(void)
{
	memset(&g_send_st, 0, sizeof(g_send_st));
	memset(&g_receive_st, 0, sizeof(g_receive_st));
	app_ota_disconnection();
}


void ota_indicate_statue_set(ble_ota_evt_type_t statue)
{
	app_ota_indicate_statue_set(statue);
	if(statue == BLE_OTA_EVT_INDICATION_CONFIRMED)
		g_send_st.indicate_available_flg = 0;
}

uint32_t ota_cmd_receive(uint8_t *data,uint16_t length)
{
	uint32_t error = 0,i;
	error = ota_cmd_receive_parse(data,length);
	if(error == 0)
	{
		QPRINTF("\r\nOTA receive:*************************************\r\n");
		QPRINTF("OTA receive CMD\r\n");
		QPRINTF("rec_flg=%d,\r\n",g_receive_st.rec_flg);
		QPRINTF("cmd_i=%d,\r\n",g_receive_st.cmd_i);
		QPRINTF("package_len=%d,\r\n",g_receive_st.package_len);
		QPRINTF("frame_i=%d,\r\n",g_receive_st.frame_i);
		QPRINTF("data_len=%d,\r\n",g_receive_st.data_len);
		QPRINTF("data:\r\n");
		for(i=0;i<g_receive_st.data_len;i++)
		{
			QPRINTF("%02x,",g_receive_st.data[i]);
		}
		QPRINTF("\r\n");
		QPRINTF("*************************************\r\n\r\n");
	}
	return error;
}


uint32_t ota_data_receive(uint8_t *data,uint16_t length)
{
	uint32_t error = 0,i;
	error = ota_data_receive_parse(data,length);
	if(error == 0)
	{
		QPRINTF("OTA receive data\r\n");
		QPRINTF("rec_flg=%d,\r\n",g_receive_st.rec_flg);
		QPRINTF("cmd_i=%d,\r\n",g_receive_st.cmd_i);
		QPRINTF("package_len=%d,\r\n",g_receive_st.package_len);
		QPRINTF("frame_i=%d,\r\n",g_receive_st.frame_i);
		QPRINTF("data_len=%d,\r\n",g_receive_st.data_len);
		QPRINTF("data:\r\n");
		for(i=0;i<g_receive_st.data_len;i++)
		{
			QPRINTF("%02x,",g_receive_st.data[i]);
		}
		QPRINTF("\r\n");
	}
	else
	{
		QPRINTF("error =%d\r\n",error);
	}
	return error;
}


uint32_t ota_send_data(uint8_t *data,uint16_t length,ota_channel_enum channel_type,uint8_t channel)
{
	uint32_t error = 0,i = 0;

	if(length > OTA_SEND_DATA_SIZE)
		length = OTA_SEND_DATA_SIZE;

	if(channel_type >= OTA_CHANNEL_MAX)
		return 1;

	if(g_send_st.send_flg)//上次数据为发完
		return 2;

	#if DATA_TYPE == DATA_BUFFER_TYPE
	memset(g_send_st.data,0,OTA_SEND_DATA_SIZE);
	
	for(i=0;i<length;i++)
		g_send_st.data[i] = data[i];
	#elif DATA_TYPE == DATA_POINTER_TYPE
	memset(&g_ota_common_tx_buffer[OTA_TX_START_ADDR], 0, OTA_TX_SIZE);
	for(i=0;i<length;i++)
		g_ota_common_tx_buffer[OTA_TX_START_ADDR+i] = data[i];
	g_send_st.data = &g_ota_common_tx_buffer[OTA_TX_START_ADDR];
	#endif
	
	g_send_st.send_index 	= 0;
	g_send_st.data_len 		= length;
	g_send_st.channel_type 	= channel_type;
	g_send_st.channel 		= channel;
	g_send_st.send_flg 		= 1;
	
	return error;
}

uint8_t ota_periodic_send_data(void)
{
	uint32_t error;
	uint16_t length = 0;
	
	if(g_send_st.send_flg == 1 && g_send_st.channel_type < OTA_CHANNEL_MAX)
	{
		length = g_send_st.data_len - g_send_st.send_index;
		if(length > 20)
		{
			length = 20;
		}
		
		if(g_send_st.channel_type == OTA_INDICATE_CHANNEL)
		{
			if(g_send_st.indicate_available_flg == 0)
			{
				g_send_st.indicate_available_flg = 1;
				error = ota_indicate_send(&g_send_st.data[g_send_st.send_index],length);
			}
			else
				error = g_send_st.indicate_available_flg;
		}
		else if(g_send_st.channel_type == OTA_NOTIFI_CHANNEL)
		{
			error = ota_notify_send(&g_send_st.data[g_send_st.send_index],length,g_send_st.channel);
		}

		if(error == NRF_SUCCESS)
		{
			g_send_st.send_index += length;
			if(g_send_st.send_index >= g_send_st.data_len)
			{
				g_send_st.send_flg = 0;
			}
			QPRINTF("ota:send_index=%d,\r\n",g_send_st.send_index);
		}
	}
	return g_send_st.send_flg;
}

