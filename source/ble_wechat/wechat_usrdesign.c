/***********************************************************************************
 * 文 件 名   : wechat_usrdesign.c
 * 创 建 者   : LiuYuanBin
 * 创建日期   : 2016年8月16日
 * 版权说明   : Copyright (c) 2016-2025   广东乐心医疗电子股份有限公司
 * 文件描述   : 基于ble_wechat.c蓝牙数据通道，进行微信的数据发送和接收功能组合
 * 修改历史   : 
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
 * 函 数 名 : wechat_receive_parse
 * 函数功能 : 
 * 输入参数 : uint8_t *data    接收ble_wechat通道的数据
               uint16_t length  接收数据的长度
 * 输出参数 : 无
 * 返 回 值 : 	0:数据接收完成
 				1:数据接收未完成
 				3:数据包头解析错误
 * 修改历史 : 无
 * 说    明 : 无
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
        offset = g_receive_st.data_len; // 数据buffer的偏移地址
		data_len = length;
		
        if((g_receive_st.data_len + length + WECHAT_PACKAGE_HEAP_SIZE) > g_receive_st.package_len) // 如果数据长度超过当前包长度，截断多出的数据
        {
            data_len = g_receive_st.package_len - g_receive_st.data_len - WECHAT_PACKAGE_HEAP_SIZE;
        }
		
        g_receive_st.data_len += data_len;
        memcpy(&g_receive_st.data[offset], data, data_len); // 接收数据     
	}

	// 判断当前包是否接收完成
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
 * 函 数 名 : wechat_receive
 * 函数功能 : 
 * 输入参数 :  uint8_t *data    从ble_wechat通道里面接收的数据
               uint16_t length  接收数据的长度
 * 输出参数 : 无
 * 返 回 值 : 	0:数据接收完成
 				1:数据接收未完成
 				3:数据包头解析错误
 * 修改历史 : 无
 * 说    明 : 无
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
 * 函 数 名 : wechat_send_data
 * 函数功能 : 
 * 输入参数 :  uint8_t *data                     发送的数据
               uint16_t length                   发送数据的长度
               wechat_channel_enum channel_type  微信的indicate或者notifi通道选择
               uint8_t channel                   目前是notifi通道第几个通道选择
 * 输出参数 : 无
 * 返 回 值 : 	0:OK 	
 				1:通道选择错误	
 				2:上次的数据还没有发送完成
 * 修改历史 : 无
 * 说    明 : 无
*****************************************************************************/
uint32_t wechat_send_data(uint8_t *data,uint16_t length,wechat_channel_enum channel_type,uint8_t channel)
{
	uint32_t error = 0,i = 0;

	if(length > WECHAT_SEND_DATA_SIZE)
		length = WECHAT_SEND_DATA_SIZE;

	if(channel_type >= WECHAT_CHANNEL_MAX)
		return 1;
	
	if(g_send_st.send_flg)//上次数据没发完
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
 * 函 数 名 : wechat_periodic_send_data
 * 函数功能 : 
 * 输入参数 : void  void
 * 输出参数 : void
 * 返 回 值 : 	0:发送完成或者没有数据发送		
 				1:有数据发送
 * 修改历史 : 无
 * 说    明 : 微信1包的数据分多帧发送，每帧20byte，在mainloop循环里面发送
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


