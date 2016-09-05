/***********************************************************************************
 * 文 件 名   : ancs_android_usrdesign.c
 * 创 建 者   : LiuYuanBin
 * 创建日期   : 2016年8月16日
 * 版权说明   : Copyright (c) 2016-2025   广东乐心医疗电子股份有限公司
 * 文件描述   : 基于ble_android_ancs.c的数据通道进行android来电信息提醒的的数据收发组包
 * 修改历史   : 
***********************************************************************************/

#include "ancs_android_usrdesign.h"
#include <string.h>
#include "ble_ancs_android.h"
#include "debug.h"
#include "app_android_ancs.h"

#if DATA_TYPE == DATA_POINTER_TYPE
uint8_t g_ancs_common_tx_buffer[ANCS_TX_SIZE];
uint8_t g_ancs_common_rx_buffer[ANCS_RX_SIZE];
#endif
static android_ancs_receive_pack_st g_receive_st = {0};
static android_ancs_send_pack_st g_send_st = {0};
static uint32_t android_ancs_indicate_send(uint8_t *data,uint16_t length)
{
	return ble_android_ancs_indicate_send(&m_android_ancs, data);
}

static uint32_t android_ancs_notify_send(uint8_t *data,uint16_t length,uint8_t chnl)
{
	return 0;
}

static uint8_t android_ancs_receive_parse(uint8_t *data,uint16_t length)
{
	uint8_t framelen = 0,offset = 0;
	
	if(g_receive_st.rec_flg == ANDROID_ANCS_RECEIVE_ED)
	{
		if(*data == 0x01)
		{
			g_receive_st.package_len 	= *(data+1);
			g_receive_st.data_len		= length - 2;	

			if(g_receive_st.package_len > ANCS_RECEIVE_DATA_SIZE)
				g_receive_st.package_len = ANCS_RECEIVE_DATA_SIZE;

			#if DATA_TYPE == DATA_BUFFER_TYPE
			memset(&g_receive_st.data, 0, sizeof(g_receive_st.data));
        	memcpy(g_receive_st.data, data + 2, g_receive_st.data_len); 
			#elif DATA_TYPE == DATA_POINTER_TYPE
			memcpy(&g_ancs_common_rx_buffer[ANCS_RX_START_ADDR], data + 2, g_receive_st.data_len); 
			g_receive_st.data = &g_ancs_common_rx_buffer[ANCS_RX_START_ADDR];
			#endif
			
			g_receive_st.rec_flg = ANDROID_ANCS_RECEIVE_ING;
		}
		else
			return 3;
	}
	else if(g_receive_st.rec_flg == ANDROID_ANCS_RECEIVE_ING)
	{
		g_receive_st.frame_i = *data;                        // 帧序号
        framelen = (length - 1);                     // 一帧数据的长度
        offset = 18 + (g_receive_st.frame_i - 2) * 19; // 数据buffer的偏移地址

        if(g_receive_st.frame_i > ANDROID_ANCS_DEF_MAX_FRAME_SIZE) // 如果帧序号超过一包数据最大帧数，清空buffer重新接收
        {
            memset(&g_receive_st, 0, sizeof(g_receive_st)); // 清空buffer
            g_receive_st.rec_flg = ANDROID_ANCS_RECEIVE_ED;
			return 2;
        }

        if((g_receive_st.data_len + framelen) > g_receive_st.package_len) // 如果数据长度超过当前包长度，截断多出的数据
        {
            framelen = g_receive_st.package_len - g_receive_st.data_len;
        }
        g_receive_st.data_len += framelen;
        memcpy(&g_receive_st.data[offset], data + 1, framelen); // 接收数据
	}

	// 判断当前包是否接收完成
    if(g_receive_st.data_len < g_receive_st.package_len)
    {
        return 1;
    }
	else
	{
		g_receive_st.rec_flg = ANDROID_ANCS_RECEIVE_ED;
		return 0;
	}	
}

void android_ancs_connection(void)
{
	memset(&g_send_st, 0, sizeof(g_send_st));
	memset(&g_receive_st, 0, sizeof(g_receive_st));

	app_android_ancs_connection();
}

void android_ancs_disconnection(void)
{
	memset(&g_send_st, 0, sizeof(g_send_st));
	memset(&g_receive_st, 0, sizeof(g_receive_st));

	app_android_ancs_disconnection();
}


void android_ancs_indicate_statue_set(ble_android_ancs_evt_type_t statue)
{
	 app_android_ancs_indicate_statue_set(statue);
	 if(statue == BLE_ANDROID_ANCS_EVT_INDICATION_CONFIRMED)
		g_send_st.indicate_available_flg = 0;
}

uint32_t android_ancs_receive(uint8_t *data,uint16_t length)
{
	uint32_t error = 0;
	
	error = android_ancs_receive_parse(data,length);

	if(error == 0)
	{
		app_android_ancs_receive_data(g_receive_st.data,g_receive_st.data_len,1);
	}

	return error;
}



uint32_t android_ancs_send_data(uint8_t *data,uint16_t length,android_ancs_channel_enum channel_type,uint8_t channel)
{
	uint32_t error = 0,i = 0;

	if(length > ANCS_SEND_DATA_SIZE)
		length = ANCS_SEND_DATA_SIZE;

	if(channel_type >= ANDROID_ANCS_CHANNEL_MAX)
		return 1;

	if(g_send_st.send_flg)//上次数据为发完
		return 2;

	memset(g_send_st.data,0,ANCS_SEND_DATA_SIZE);
	for(i=0;i<length;i++)
		g_send_st.data[i] = data[i];

	#if DATA_TYPE == DATA_BUFFER_TYPE
	memset(g_send_st.data,0,ANCS_SEND_DATA_SIZE);
	
	for(i=0;i<length;i++)
		g_send_st.data[i] = data[i];
	#elif DATA_TYPE == DATA_POINTER_TYPE
	memset(&g_ancs_common_tx_buffer[ANCS_TX_START_ADDR], 0, ANCS_TX_SIZE);
	for(i=0;i<length;i++)
		g_ancs_common_tx_buffer[ANCS_TX_START_ADDR+i] = data[i];
	g_send_st.data = &g_ancs_common_tx_buffer[ANCS_TX_START_ADDR];
	#endif
	
	g_send_st.send_index 	= 0;
	g_send_st.data_len 		= length;
	g_send_st.channel_type 	= channel_type;
	g_send_st.channel 		= channel;
	g_send_st.send_flg 		= 1;
	
	return error;
}


uint8_t android_ancs_periodic_send_data(void)
{
	uint32_t error;
	uint16_t length = 0;
	
	if(g_send_st.send_flg == 1 && g_send_st.channel_type < ANDROID_ANCS_CHANNEL_MAX)
	{
		length = g_send_st.data_len - g_send_st.send_index;
		if(length > 20)
		{
			length = 20;
		}
		
		if(g_send_st.channel_type == ANDROID_ANCS_INDICATE_CHANNEL)
		{
			if(g_send_st.indicate_available_flg == 0)
			{
				g_send_st.indicate_available_flg = 1;
				error = android_ancs_indicate_send(&g_send_st.data[g_send_st.send_index],length);
			}
			else
				error = g_send_st.indicate_available_flg;
		}
		else if(g_send_st.channel_type == ANDROID_ANCS_NOTIFI_CHANNEL)
		{
			error = android_ancs_notify_send(&g_send_st.data[g_send_st.send_index],length,g_send_st.channel);
		}

		if(error == NRF_SUCCESS)
		{
			g_send_st.send_index += length;
			if(g_send_st.send_index >= g_send_st.data_len)
			{
				g_send_st.send_flg = 0;
			}
			QPRINTF("android ancs:send_index=%d,\r\n",g_send_st.send_index);
		}
	}
	return g_send_st.send_flg;
}


