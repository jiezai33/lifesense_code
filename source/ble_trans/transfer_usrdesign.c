/***********************************************************************************
 * 文 件 名   : transfer_usrdesign.c
 * 创 建 者   : LiuYuanBin
 * 创建日期   : 2016年8月16日
 * 版权说明   : Copyright (c) 2016-2025   广东乐心医疗电子股份有限公司
 * 文件描述   : 基于ble_transfer.c的数据通道进行A500的数据收发组包
 * 修改历史   : 
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
 * 函 数 名 : transfer_receive_parse
 * 函数功能 : 
 * 输入参数 :  uint8_t *data    接收ble_transfer通道的数据
               uint16_t length  接收数据的长度
 * 输出参数 : 无
 * 返 回 值 : 	0:数据接收完成
 				1:数据接收未完成
 				2:帧序号超过一包数据最大帧数
 				3:数据包头解析错误
 * 修改历史 : 无
 * 说    明 : 无
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
		g_receive_st.frame_i = *data;                        // 帧序号
        framelen = (length - 1);                     // 一帧数据的长度
        offset = 16 + (g_receive_st.frame_i - 1) * 19; // 数据buffer的偏移地址

        if(g_receive_st.frame_i > TRANS_DEF_MAX_FRAME_SIZE) // 如果帧序号超过一包数据最大帧数，清空buffer重新接收
        {
            g_receive_st.data_len = 0; // 清空接收长度
            g_receive_st.package_len = 0;
            memset(g_receive_st.data, 0, sizeof(g_receive_st.data)); // 清空buffer
            g_receive_st.rec_flg = 0;
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
 * 函 数 名 : transfer_receive
 * 函数功能 : 
 * 输入参数 :  uint8_t *data    从ble_transfer通道里面接收的数据
               uint16_t length  接收数据的长度
 * 输出参数 : 无
 * 返 回 值 : 	0:数据接收完成
 				1:数据接收未完成
 				2:帧序号超过一包数据最大帧数
 				3:数据包头解析错误
 * 修改历史 : 无
 * 说    明 : 无
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
 * 函 数 名 : transfer_send_data
 * 函数功能 : 
 * 输入参数 :  uint8_t *data                     发送的数据
               uint16_t length                   发送数据的长度
               wechat_channel_enum channel_type  A500的indicate或者notifi通道选择
               uint8_t channel                   目前是notifi通道第几个通道选择
 * 输出参数 : 无
 * 返 回 值 : 	0:OK 	
 				1:通道选择错误	
 				2:上次的数据还没有发送完成
 * 修改历史 : 无
 * 说    明 : 无
*****************************************************************************/
uint32_t transfer_send_data(uint8_t *data,uint16_t length,trans_channel_enum channel_type,uint8_t channel)
{
	uint32_t error = 0,i = 0;

	if(length > TRANS_SEND_DATA_SIZE)
		length = TRANS_SEND_DATA_SIZE;

	if(channel_type >= TRANS_CHANNEL_MAX)
		return 1;

	if(g_send_st.send_flg)//上次数据为发完
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
 * 函 数 名 : transfer_periodic_send_data
 * 函数功能 : 
 * 输入参数 : void  void
 * 输出参数 : void
 * 返 回 值 : 	0:发送完成或者没有数据发送		
 				1:有数据发送
 * 修改历史 : 无
 * 说    明 : A500 1包的数据分多帧发送，每帧20byte，在mainloop循环里面发送
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

