#include "usr_design.h"
#include "debug.h"
#include "ancs_android_usrdesign.h"
#include "md5.h"
#include "crc_32.h"
#include "transfer_usrdesign.h"
#include "ota_usrdesign.h"
#include "wechat_usrdesign.h"
#include "usr_login.h"
#include "app_android_ancs.h"
#include "usr_init.h"
#include "usr_data.h"

#define USRDESIGN_SEND_DATA_INDEX_MAX		(4)
typedef uint8_t (*ble_send_data)(void);

trans_evt_st g_trans_evt_hander;
typedef uint32_t (*trans_evt)(void *data);
const trans_evt trans_evt_hander[] = {
	usr_lifesense_login_evt,
	usr_wechat_login_evt,
	usr_wechat_init_evt,
	usr_wechat_send_data_evt,
};

const ble_send_data g_usrdesign_send_data[] = {
	android_ancs_periodic_send_data,
	transfer_periodic_send_data,
	ota_periodic_send_data,
	wechat_periodic_send_data
};

uint32_t usrdesign_send_data(void)
{
	uint32_t statue = 0;
	for(uint8_t i=0;i<USRDESIGN_SEND_DATA_INDEX_MAX;i++)
	{
		if(g_usrdesign_send_data[i]())
		{
			statue |= (1<<i);
		}
	}
	return statue;	
}

void trans_evt_call_back(void)
{
	uint8_t i,error;
	
	if(g_trans_evt_hander.evt)
	{
		for(i=0;i<32;i++)
		{
			if((1<<i) & g_trans_evt_hander.evt)
			{
				error = trans_evt_hander[i](NULL);
				if(error == 0)
				{
					g_trans_evt_hander.evt &= ~(1<<i);
				}
			}
		}
	}
}



void data_process(uint8_t *data,uint8_t length)
{
	uint8_t cmd,*pData;
	cmd = data[0];
	pData = data + 1;
	switch(cmd)
	{
		case APP_RETURN_DOWNLOAD_CMD:
			QPRINTF("APP_RETURN_DOWNLOAD_CMD\r\n");
			break;
			
		case APP_RETURN_GET_DAY_CMD:
			QPRINTF("APP_RETURN_GET_DAY_CMD\r\n");
			break;
			
		case APP_RETURN_SLEEP_DATA_CMD:
			QPRINTF("APP_RETURN_SLEEP_DATA_CMD\r\n");
			break;
			
		case APP_RETURN_HEART_DATA_CMD:
			QPRINTF("APP_RETURN_HEART_DATA_CMD\r\n");
			break;
			
		case APP_RETURN_PAIR_CMD:
			QPRINTF("APP_RETURN_PAIR_CMD\r\n");
			break;
			
		case APP_RETURN_GET_HOUR_DATA_CMD:
			QPRINTF("APP_RETURN_GET_HOUR_DATA_CMD\r\n");
			break;
			
		case APP_RETURN_USER_INFO_CONFIRE_CMD:
			QPRINTF("APP_RETURN_USER_INFO_CONFIRE_CMD\r\n");
			break;
			
		case APP_RETURN_DIVICE_CMD:
			QPRINTF("APP_RETURN_DIVICE_CMD\r\n");
			break;
			
		case APP_RETURN_QUIT_DOWN_MAC_MODE_CMD:
			QPRINTF("APP_RETURN_DIVICE_CMD\r\n");
			break;
			
		case APP_RETURN_GET_SWIING_CMD:
			QPRINTF("APP_RETURN_GET_SWIING_CMD\r\n");
			break;
			
		case APP_RETURN_GET_BLOOD_CMD:
			QPRINTF("APP_RETURN_GET_BLOOD_CMD\r\n");
			break;

		case APP_PUSH_GET_DEVICE_INFO_CMD:
			switch(*pData)
			{
				case GET_FLASH_IONFO_CMD:
					QPRINTF("GET_FLASH_IONFO_CMD\r\n");
					break;
					
				case GET_USER_INFO_CMD:
					QPRINTF("GET_USER_INFO_CMD\r\n");
					break;

				case GET_ALARM_SETTING_CMD:
					QPRINTF("GET_ALARM_SETTING_CMD\r\n");
					break;

				case GET_CALL_SETTING_CMD:
					QPRINTF("GET_CALL_SETTING_CMD\r\n");
					break;

				case GET_HEART_SETTING_CMD:
					QPRINTF("GET_HEART_SETTING_CMD\r\n");
					break;

				case GET_LONG_SIT_SETTING_CMD:
					QPRINTF("GET_LONG_SIT_SETTING_CMD\r\n");
					break;

				case GET_FIND_ME_CMD:
					QPRINTF("GET_FIND_ME_CMD\r\n");
					break;

				default:break;
			}
			break;

		case APP_PUSH_GET_RESULT_CMD:
			QPRINTF("APP_PUSH_GET_RESULT_CMD\r\n");
			break;
			
		case APP_PUSH_USER_INFO_CMD:
			QPRINTF("APP_PUSH_USER_INFO_CMD\r\n");
			break;
			
		case APP_PUSH_ALARM_SETTING_CMD:
			QPRINTF("APP_PUSH_ALARM_SETTING_CMD\r\n");
			break;
			
		case APP_PUSH_CALL_SETTING_CMD:
			QPRINTF("APP_PUSH_CALL_SETTING_CMD\r\n");
			break;
			
		case APP_PUSH_HEART_SETTING_CMD:
			QPRINTF("APP_PUSH_HEART_SETTING_CMD\r\n");
			break;
			
		case APP_PUSH_LONG_SIT_SETTING_CMD:
			QPRINTF("APP_PUSH_LONG_SIT_SETTING_CMD\r\n");
			break;
			
		case APP_PUSH_FIND_ME_SETTING_CMD:
			QPRINTF("APP_PUSH_FIND_ME_SETTING_CMD\r\n");
			break;
			
		case APP_PUSH_ENCOURAGE_STTING_CMD:
			QPRINTF("APP_PUSH_ENCOURAGE_STTING_CMD\r\n");
			break;
				
		case APP_PUSH_SPORT_HEART_CMD:
			QPRINTF("APP_PUSH_SPORT_HEART_CMD\r\n");
			break;
			
		case APP_RETURN_GET_RUN_STATUE_CMD:
			QPRINTF("APP_RETURN_GET_RUN_STATUE_CMD\r\n");
			break;
			
		case APP_RETURN_GET_RUN_HEART_CMD:
			QPRINTF("APP_RETURN_GET_RUN_HEART_CMD\r\n");
			break;
			
		case APP_PUSH_HEART_RANG_CMD:
			QPRINTF("APP_PUSH_HEART_RANG_CMD\r\n");
			break;
			
		case APP_RETURN_GET_HEART_RANG_CMD:
			QPRINTF("APP_RETURN_GET_HEART_RANG_CMD\r\n");
			break;
			
		case APP_PUSH_HEART_TURN_CMD:
			QPRINTF("APP_PUSH_HEART_TURN_CMD\r\n");
			break;
			
		default:
			
			break;
	}
}






