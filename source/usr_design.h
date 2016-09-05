#ifndef _USR_DESIGN_H_
#define _USR_DESIGN_H_
#include <stdint.h>


typedef union
{
	uint32_t evt;
	struct{
		uint32_t lifesense_login_bit_0:1;	//usr_lifesense_login_evt
		uint32_t wechat_login_bit_1:1;		//usr_wechat_login_evt
		uint32_t wechat_init_bit_2:1;
		uint32_t wechat_send_data_bit_3:1;
		uint32_t bit_4:1;
	}bit;
}trans_evt_st;

typedef enum
{
	APP_RETURN_DOWNLOAD_CMD = 0X50,
	APP_RETURN_GET_DAY_CMD,
	APP_RETURN_SLEEP_DATA_CMD,
	APP_RETURN_HEART_DATA_CMD,
	APP_RETURN_PAIR_CMD = 0X55,
	APP_RETURN_GET_HOUR_DATA_CMD = 0X57,
	APP_RETURN_USER_INFO_CONFIRE_CMD = 0X61,
	APP_RETURN_DIVICE_CMD,
	APP_RETURN_QUIT_DOWN_MAC_MODE_CMD,
	APP_RETURN_GET_SWIING_CMD,
	APP_RETURN_GET_BLOOD_CMD,	
	APP_PUSH_GET_DEVICE_INFO_CMD,
	APP_PUSH_GET_RESULT_CMD,
	APP_PUSH_USER_INFO_CMD,
	APP_PUSH_ALARM_SETTING_CMD,
	APP_PUSH_CALL_SETTING_CMD,
	APP_PUSH_HEART_SETTING_CMD = 0X6D,
	APP_PUSH_LONG_SIT_SETTING_CMD,
	APP_PUSH_FIND_ME_SETTING_CMD,
	APP_PUSH_ENCOURAGE_STTING_CMD,
	APP_PUSH_SPORT_HEART_CMD,
	APP_RETURN_GET_RUN_STATUE_CMD,
	APP_RETURN_GET_RUN_HEART_CMD,
	APP_PUSH_HEART_RANG_CMD,
	APP_RETURN_GET_HEART_RANG_CMD,
	APP_PUSH_HEART_TURN_CMD,
}cmd_type_enum;

typedef enum
{
	GET_FLASH_IONFO_CMD = 0X00,
	GET_USER_INFO_CMD,
	GET_ALARM_SETTING_CMD,
	GET_CALL_SETTING_CMD,
	GET_HEART_SETTING_CMD,
	GET_LONG_SIT_SETTING_CMD,
	GET_FIND_ME_CMD,
}get_setting_cmd_enum;


extern trans_evt_st g_trans_evt_hander;

void trans_evt_call_back(void);


uint32_t usrdesign_send_data(void);


void data_process(uint8_t *data,uint8_t length);

#endif



