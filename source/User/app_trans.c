#include "app_trans.h"
#include <string.h>
#include "usr_design.h"
#include "debug.h"
#include "channel_select.h"
#include "transfer_usrdesign.h"
#include "time.h"

extern void sys_start_pair_mode(void);

void app_trans_connection(void)
{
	QPRINTF("app_trans_connection\r\n");
	memset(&g_communication_statue,0,sizeof(communication_statue_st));
}

void app_trans_disconnection(void)
{
	QPRINTF("app_trans_disconnection\r\n");
}

/*****************************************************************************
 * 函 数 名 : app_trans_indicate_statue_set
 * 函数功能 : 
 * 输入参数 : ble_trans_evt_type_t statue  微信indicate的状态
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 获取A500 indicate的状态
*****************************************************************************/
void app_trans_indicate_statue_set(ble_trans_evt_type_t statue)
{
	switch(statue)
	{
		case BLE_TRANS_EVT_INDICATION_ENABLED:
			QPRINTF("BLE_TRANS_EVT_INDICATION_ENABLED\r\n");
			g_trans_evt_hander.bit.lifesense_login_bit_0 = 1;
			usr_set_app_type(LIFESENSE_APP);
			break;
			
		case BLE_TRANS_EVT_INDICATION_DISABLED:
			QPRINTF("BLE_TRANS_EVT_INDICATION_DISABLED\r\n");
			break;
			
		case BLE_TRANS_EVT_INDICATION_CONFIRMED:
			QPRINTF("BLE_TRANS_EVT_INDICATION_CONFIRMED\r\n");
			break;
			
		default:break;
	}
}

/*****************************************************************************
 * 函 数 名 : app_transfer_send_data
 * 函数功能 : 
 * 输入参数 :  uint8_t *data                     发送的数据
               uint16_t length                   发送数据的长度
               trans_channel_enum channel_type   A500 的indicate或者notifi通道选择
               uint8_t channel                   目前是notifi通道第几个通道选择
 * 输出参数 : 无
 * 返 回 值 : 	0:OK 	
 				1:通道选择错误	
 				2:上次的数据还没有发送完成
 * 修改历史 : 无
 * 说    明 : 无
*****************************************************************************/
uint32_t app_transfer_send_data(uint8_t *data,uint16_t length,trans_channel_enum channel_type,uint8_t channel)
{
	return transfer_send_data(data,length,channel_type,channel);
}

/*****************************************************************************
 * 函 数 名 : app_trans_receive
 * 函数功能 : 
 * 输入参数 : uint8_t *data   A500 的数据指针
               uint8_t length  A500 的数据长度
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : A500的数据接收
*****************************************************************************/
void app_trans_receive(uint8_t *data,uint8_t length)
{
	uint8_t cmd = 0,*pData,data_len;
	uint32_t time = 0;
	if(*data == CMD_VERSION_0 && *(data+1) == CMD_VERSION_1)
	{
		cmd = *(data+2);
		data_len = length - 7;
		pData = data+2;
			
		if(g_communication_statue.transfer_statue == LOGIN_STATUE)//首次连接成功后，是先要进入登录状态，并接收登录返回信息进行判断
		{
			if(data_len == 6)
			{
				if(cmd == 0x01)
				{
					QPRINTF("log in success\r\n");
					QPRINTF("UTC:%02x,%02x,%02x,%02x\r\n",*(pData+1),*(pData+2),*(pData+3),*(pData+4));
					QPRINTF("time:%02x\r\n",*(pData+5));
					QPRINTF("phone type:%02x\r\n",*(pData+6));
					g_communication_statue.transfer_statue = DATA_STATUE;

					system_timezone_set(*(pData+5));
					time |= (uint32_t)(*(pData+1)<<24);
					time |= (uint32_t)(*(pData+2)<<16);
					time |= (uint32_t)(*(pData+3)<<8);
					time |= (uint32_t)(*(pData+4));
					system_sec_set(time);
					
					if(*(pData+6) == 1)//phone == ios
						sys_start_pair_mode();

					g_trans_evt_hander.bit.wechat_send_data_bit_3 = 1; 
				}
				else
				{
					QPRINTF("log in false\r\n");
				}
			}
		}
		else if(g_communication_statue.transfer_statue == DATA_STATUE)//登录成功后进入用户数据传输状态
		{	
			data_process(pData,data_len);
		}
		
	}	
}

	
