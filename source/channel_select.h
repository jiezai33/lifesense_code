#ifndef _CHANNEL_SELECT_H_
#define _CHANNEL_SELECT_H_
#include <stdint.h>


typedef enum
{
	ANDROID_TYPE,
	IOS_TYPE,
	PHONE_MAX
}phone_enumm;

typedef enum
{
	LIFESENSE_APP = 0x01,
	WECHAT_APP = 0x02,
	APP_MAX
}app_enum;

typedef enum
{
	LOGIN_STATUE = 0,
	INIT_STATUE,
	DATA_STATUE,
	STATUE_MAX
}statue_enum;

typedef enum
{
	FRONT_STATUE = 0x01,
	BACKGROUND_STATUE,
	APP_ATATUE_MAX
}app_statue_enum;

typedef struct
{
	uint8_t phone_type;			//连接的手机类型
	uint8_t app_type;			//连接的APP类型
	uint8_t transfer_statue;	//手环处于什么状态
	uint8_t app_statue;			//处于前台还是后台
}communication_statue_st;

extern communication_statue_st g_communication_statue;

void usr_set_app_type(app_enum type);
uint32_t app_send_data(uint8_t *data,uint16_t length);
uint32_t app_add_heap_send_data(uint8_t data_type,uint8_t data_id,uint8_t *data,uint16_t length);
#endif

