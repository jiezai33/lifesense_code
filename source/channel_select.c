#include "channel_select.h"
#include "transfer_usrdesign.h"
#include "wechat_usrdesign.h"
#include "debug.h"

communication_statue_st g_communication_statue = {0};

void usr_set_app_type(app_enum type)
{
	g_communication_statue.app_type |= type;
}

uint32_t app_send_data(uint8_t *data,uint16_t length)
{
	uint32_t error;

	if(g_communication_statue.app_type == WECHAT_APP)
	{	
		error = wechat_send_data(data, length,WECHAT_INDICATE_CHANNEL,0);
	}
	else if(g_communication_statue.app_type == LIFESENSE_APP)
	{	
		error = transfer_send_data(data, length,TRANS_INDICATE_CHANNEL,0);
	}
	return error;
}

uint32_t app_add_heap_send_data(uint8_t data_type,uint8_t data_id,uint8_t *data,uint16_t length)
{
	return 0;
}

