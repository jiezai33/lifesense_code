/***********************************************************************************
 * 文 件 名   : app_wechat.c
 * 创 建 者   : LiuYuanBin
 * 创建日期   : 2016年8月17日
 * 版权说明   : Copyright (c) 2016-2025   广东乐心医疗电子股份有限公司
 * 文件描述   : 微信数据收发接口，完全与ble底层无关的数据流控制
 * 修改历史   : 
***********************************************************************************/
#include "app_wechat.h"
#include <string.h>
#include "usr_design.h"
#include "debug.h"
#include "app_wechat_common.h"
#include "usr_login.h"
#include "usr_data.h"
#include "channel_select.h"
#include "wechat_usrdesign.h"

extern void sys_start_pair_mode(void);

void app_wechat_connection(void)
{
	QPRINTF("app_wechat_connection\r\n");
	memset(&g_communication_statue,0,sizeof(communication_statue_st));
}

void app_wechat_disconnection(void)
{
	QPRINTF("app_wechat_disconnection\r\n");
}

/*****************************************************************************
 * 函 数 名 : app_wechat_indicate_statue_set
 * 函数功能 : 
 * 输入参数 : ble_wechat_evt_type_t statue  微信indicate的状态
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 获取微信indicate的状态
*****************************************************************************/
void app_wechat_indicate_statue_set(ble_wechat_evt_type_t statue)
{
	switch(statue)
	{
		case BLE_WECHAT_EVT_INDICATION_ENABLED:
			QPRINTF("BLE_WECHAT_EVT_INDICATION_ENABLED\r\n");
			g_trans_evt_hander.bit.wechat_login_bit_1 = 1;
			usTxWeChatPackSeq = 1;
			usr_set_app_type(WECHAT_APP);
			break;
			
		case BLE_WECHAT_EVT_INDICATION_DISABLED:
			QPRINTF("BLE_WECHAT_EVT_INDICATION_DISABLED\r\n");
			break;
			
		case BLE_WECHAT_EVT_INDICATION_CONFIRMED:
			QPRINTF("BLE_WECHAT_EVT_INDICATION_CONFIRMED\r\n");
			break;
			
		default:break;
	}
}

/*****************************************************************************
 * 函 数 名 : app_wechat_send_data
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
uint32_t app_wechat_send_data(uint8_t *data,uint16_t length,wechat_channel_enum channel_type,uint8_t channel)
{
	return wechat_send_data(data,length,channel_type,channel);
}

/*****************************************************************************
 * 函 数 名 : app_wechat_receive
 * 函数功能 : 
 * 输入参数 :  uint16_t cmd_id  命令号
               uint8_t *data    变长包体数据指针
               uint8_t length   变长包体长度
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 微信数据的接收
*****************************************************************************/
void app_wechat_receive(uint16_t cmd_id,uint8_t *data,uint8_t length)
{
	uint8_t error,i;
    uint8_t  datalen = 0; // 解包数据长度
    uint8_t  offset = 0;  // 解包参数
    uint8_t  rcv_data[220] = {0}; // 组包后数据存放buffer	
	uint8_t dest_field = DATA_DATA_FIELD;	

	if(cmd_id == WECHAT_CMDID_RESQ_ENTRY)                      //登陆回复指令
	{
		dest_field = AUTH_AES_SESSION_KEY_FIELD;	
	}
	else if(cmd_id == WECHAT_CMDID_RESQ_INIT)                       //判断是否是init回复的命令ID
    {
    	dest_field = INIT_TIME_FIELD;
    }
	else if(cmd_id == WECHAT_CMDID_RESQ_USERINFO || cmd_id == 30001 || cmd_id == 30002 ||cmd_id == 30003) 
	{
		dest_field = DATA_DATA_FIELD;
	}

	error = response_unpack(dest_field,data, length, rcv_data, &datalen,&offset);
	
	if(error == false)       //解包错误则断开连接
    {
        QPRINTF("protobuf parse error\r\n");
		return;
    }
	
	switch(cmd_id)        
	{
		case WECHAT_CMDID_RESQ_ENTRY:	//登陆回复指令		
			if(TRUE == memcmp(rcv_data, "factory mode", datalen))        //判断是否进入烧码模式
	        {
	            QPRINTF("factory mode\r\n");
	        }
	        else if(TRUE == memcmp(rcv_data, "test mode", datalen))      //判断是否进入测试模式
	        {
	           	QPRINTF("test mode\r\n");
	        }
	        else if(TRUE == memcmp(rcv_data, "check mode", datalen))     //判断是否是验码模式
	        {
	            QPRINTF("check mode\r\n");
	        }
	        else if(TRUE == memcmp(rcv_data, "pair mode", datalen))      //判断是否是配对模式
	        {
	            QPRINTF("pair mode\r\n");
	        }
	        else                                                            //正常登陆微信
	        {
	        	QPRINTF("send initerq\r\n");
	            g_trans_evt_hander.bit.wechat_init_bit_2 = 1;
				g_communication_statue.transfer_statue = INIT_STATUE;
	        }
		break;
		
		case WECHAT_CMDID_RESQ_INIT:                      //判断是否是init回复的命令ID
			g_trans_evt_hander.bit.wechat_send_data_bit_3 = 1;
			g_communication_statue.transfer_statue = DATA_STATUE;

			error = response_unpack(INIT_TIME_STRING_FIELD,data, length, rcv_data, &datalen,&offset);
            if(error ==false)
            {
                QPRINTF("INIT_TIME_STRING_FIELD error = false\r\n");
            }
			else
			{
				QPRINTF("time:");
				for(i=0;i<datalen;i++)
					QPRINTF("%02x,",rcv_data[i]);
				QPRINTF("\r\n");
			}

			error = response_unpack(INIT_PLAT_FORM_TYPE_FIELD,data, length, rcv_data, &datalen,&offset);
            if(error ==false)
            {
                QPRINTF("INIT_PLAT_FORM_TYPE_FIELD error = false\r\n");
            }
			else
			{
				QPRINTF("INIT_PLAT_FORM_TYPE_FIELD:");
				for(i=0;i<datalen;i++)
					QPRINTF("%02x,",rcv_data[i]);
				QPRINTF("\r\n");
				if(rcv_data[0] == 1)//phone == ios
					sys_start_pair_mode();
			}
		break;
		
		case WECHAT_CMDID_RESQ_USERINFO:
			for(i=0;i<datalen-1;i++)
			{
				if(rcv_data[i] == 0xAA && rcv_data[i+1] == 0x01)
				{
					break;
				}
			}
			
			if(i<length-1)
				data_process(rcv_data+i+2,datalen - i -2);
		break;
		
		case 30001:                                 //push包解析		
			wechat_push_data_process(rcv_data,datalen);        
		break;
		
     	case 30003:              //微信进入后台和返回前台通知，仅在IOS上，目前在安卓上测试没有该通知
	       	if((*(data + 2) == 0x10) && (*(data + 3) == 0x01))        //  微信退入后台或关闭
	        {
	            //因为无法分辨是退入后台还是关闭，所以先统一当关闭处理。如果只是退入后台
	            //在返回前台的时候恢复标志位
				QPRINTF("wechat in background\r\n");
				g_communication_statue.app_statue = BACKGROUND_STATUE;
	        }
	        else if((*(data + 2) == 0x10) && (*(data + 3) == 0x02))   //微信返回前台
	        {
				QPRINTF("wechat in fornt\r\n");
				g_communication_statue.app_statue = FRONT_STATUE;
	        }
		break;

		default:break;
	}
}



