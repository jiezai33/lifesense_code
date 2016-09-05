/*******************************************************
*	File	: ble_remainder.c
*	Author	: liuyuanbin
*	Time	: 2016-07-04
*	Version	: Copryight (c)
*	describe: call message remaind data analaye struct
*******************************************************/
#include "usr_reminder.h"
#include "debug.h"


remainder_head_st g_call_st = {0};//来电结构体
remainder_st g_remainder_st = {0};//短信或者微信结构体

extern void RemindCallFlag_Set(void);
extern void RemindMessageFlag_Set(void);
extern void RemindWechatFlag_Set(void);

static uint8_t get_current_newest_remainder(uint8_t * remaind_type,uint8_t *index)
{
	uint8_t i = 0,l_index = 0,find=0;
	
	if(g_call_st.remaind_flg)
	{
		*remaind_type = CALL_REMAIND;
		return 0;
	}
	else
	{
		l_index = g_remainder_st.index;
		l_index = l_index > 0 ? (l_index -1):(REMAINDER_GROUP_COUNT-1);

		for(i=0;i<=REMAINDER_GROUP_COUNT;i++)
		{
			if(g_remainder_st.body_st[l_index].head.remaind_flg)
			{
				find = 1;
				break;
			}
			else
				l_index = (++l_index)%REMAINDER_GROUP_COUNT;
		}

		if(find == 1)
		{
			*remaind_type = g_remainder_st.body_st[l_index].head.remaind_type;
			*index = l_index;
			return 0;
		}
	}
	return 1;
}

remainder_head_st* get_current_call_st(void)
{
	return &g_call_st;
}

remainder_body_st* get_current_message_st(void)
{
	uint8_t remaind_type=0,index=0;
	if(get_current_newest_remainder(&remaind_type,&index) == 0)
	{
		if(remaind_type == MESSAGE_REMAIND || 
			remaind_type == WECHAT_REMAIND ||
			remaind_type == QQ_REMAIND)
		{
			if(index < REMAIND_MAX)
				return &(g_remainder_st.body_st[index]);
		}
	}
	return NULL;	
}

void set_remainder_info(uint8_t phone_type,
						uint8_t remaind_type,
						uint8_t count,
						uint32_t time,
						
						uint8_t *title_data,
						uint16_t title_length,
						uint8_t *message_data,
						uint16_t message_length)
{
	uint16_t i=0,index= 0,find = 0;
	if(remaind_type >= REMAIND_MAX || phone_type >= PHONE_MAX)
		return;

	if(title_length >= TITLE_DATA_SIZE)
		title_length = TITLE_DATA_SIZE-1;
	
	if(message_length >= MESSAGE_DATA_SIZE)
		message_length = MESSAGE_DATA_SIZE-1;


	if(remaind_type == CALL_REMAIND && g_call_st.lock == LOCK_OFF)
	{
		g_call_st.phone_type 	= phone_type;
		g_call_st.remaind_type	= remaind_type;
		g_call_st.time 			= time;
		g_call_st.remaind_flg 	= 1;
		
		g_call_st.title_length = title_length;
		for(i=0;i<title_length;i++)
			g_call_st.title_data[i] = title_data[i];
		g_call_st.title_data[i] = '\0';
	}
	else if(remaind_type == MESSAGE_REMAIND || 
			remaind_type == WECHAT_REMAIND ||
			remaind_type == QQ_REMAIND)
	{	
		index = g_remainder_st.index;

		for(i=0;i<REMAINDER_GROUP_COUNT;i++)
		{
			if(g_remainder_st.body_st[index].head.lock == LOCK_OFF)
			{
				find = 1;
				break;
			}
			else
				g_remainder_st.index = (index++)%REMAINDER_GROUP_COUNT;
		}
		if(find == 1)
		{
			index = g_remainder_st.index;
			if(remaind_type == MESSAGE_REMAIND)
				g_remainder_st.message_count = count;
			else if(remaind_type == WECHAT_REMAIND)
				g_remainder_st.wechat_count = count;
			
			g_remainder_st.body_st[index].head.phone_type 	= phone_type;
			g_remainder_st.body_st[index].head.remaind_type	= remaind_type;
			g_remainder_st.body_st[index].head.time 		= time;
			g_remainder_st.body_st[index].head.remaind_flg 	= 1;
			
			g_remainder_st.body_st[index].head.title_length = title_length;
			for(i=0;i<title_length;i++)
				g_remainder_st.body_st[index].head.title_data[i] = title_data[i];
			g_remainder_st.body_st[index].head.title_data[i] = '\0';
			
			g_remainder_st.body_st[index].message_length = message_length;
			for(i=0;i<message_length;i++)
				g_remainder_st.body_st[index].message_data[i] = message_data[i];
			g_remainder_st.body_st[index].message_data[i] = '\0';

			g_remainder_st.index = (index++)%REMAINDER_GROUP_COUNT;
		}	
	}
}


remainder_st* get_remainder_info(void)
{
	return &g_remainder_st;
}

uint8_t get_remainder_message_count(void)
{
	return g_remainder_st.message_count;
}

uint8_t get_remainder_wechat_count(void)
{
	return g_remainder_st.wechat_count;
}

void clear_all_remainder_info(void)
{
	memset(&g_remainder_st,0,sizeof(remainder_st));
}

/*
void remaind_do(void)
这个函数需要在while循环里面调用
*/
void remaind_do(void)
{
	uint16_t i=0;
	uint8_t index = 0;
	uint8_t remaind_type = 0;
	uint32_t current_time = 0;

	//UtcCntRelativeGet(&current_time);//获取当前最新时间
	
	if(get_current_newest_remainder(&remaind_type,&index) == 0)
	{
		if(remaind_type == CALL_REMAIND)
		{
			#if 1
			QPRINTF("CALL remainder:\r\n");
			QPRINTF( "msgid=%d\r\n",		g_call_st.msg_id);
			QPRINTF( "phone_type=%d\r\n",	g_call_st.phone_type);
			QPRINTF( "remaind_type=%d\r\n",	g_call_st.remaind_type);
			QPRINTF( "time=%d\r\n",			g_call_st.time);
			QPRINTF( "remaind_flg=%d\r\n",	g_call_st.remaind_flg);
			QPRINTF( "title_length=%d\r\n",	g_call_st.title_length);
			QPRINTF( "title_data:%s\r\n",	g_call_st.title_data);
			memset(&(g_call_st), 0, sizeof(remainder_head_st));
			#endif
			//RemindCallFlag_Set();
		}
		else if(remaind_type == MESSAGE_REMAIND || 
			remaind_type == WECHAT_REMAIND ||
			remaind_type == QQ_REMAIND)
		{
			#if 1
			QPRINTF( "g_index=%d\r\n",		index);
			QPRINTF( "message_count=%d\r\n",g_remainder_st.message_count);
			QPRINTF( "wechat_count=%d\r\n",	g_remainder_st.wechat_count);
			QPRINTF( "msgid=%d\r\n",		g_remainder_st.body_st[index].head.msg_id);
			QPRINTF( "phone_type=%d\r\n",	g_remainder_st.body_st[index].head.phone_type);
			QPRINTF( "remaind_type=%d\r\n",	g_remainder_st.body_st[index].head.remaind_type);
			QPRINTF( "time=%d\r\n",			g_remainder_st.body_st[index].head.time);
			QPRINTF( "remaind_flg=%d\r\n",	g_remainder_st.body_st[index].head.remaind_flg);
			QPRINTF( "title_length=%d\r\n",	g_remainder_st.body_st[index].head.title_length);
			QPRINTF( "title_data:%s\r\n",	g_remainder_st.body_st[index].head.title_data);
			QPRINTF( "message_length=%d\r\n\r\n",g_remainder_st.body_st[index].message_length);
			QPRINTF( "message_data:%s\r\n\r\n",g_remainder_st.body_st[index].message_data);

			for(i=0;i<g_remainder_st.body_st[index].message_length;i++)
			{
				QPRINTF( "%02x,",g_remainder_st.body_st[index].message_data[i]);
			}
			QPRINTF( "\r\n");
			memset(&g_remainder_st.body_st[index], 0, sizeof(remainder_body_st));
			#endif
			
			if(current_time - g_remainder_st.body_st[index].head.time <= 30)
			{
				//if(remaind_type == MESSAGE_REMAIND)
				//	RemindMessageFlag_Set();
				//else if(remaind_type == WECHAT_REMAIND)
				//	RemindWechatFlag_Set();
			}
		}
	}
}

