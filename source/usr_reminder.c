/*******************************************************
*	File	: ble_remainder.c
*	Author	: liuyuanbin
*	Time	: 2016-07-04
*	Version	: Copryight (c)
*	describe: call message remaind data analaye struct
*******************************************************/
#include "usr_reminder.h"
#include "time.h"
#include "debug.h"

remainder_head_st g_call_st = {0};//?????
remainder_st g_remainder_st = {0};//?????????

extern void RemindIncallingFlag_Set(void);
extern void RemindMessageFlag_Set(void);
extern void RemindWechatFlag_Set(void);

/*****************************************************************************
 * 函 数 名 	: 	find_mobilephone_no
 * 函数功能 	: 
 * 输入参数 	: 	uint8_t *in	:输入数组
 					uint16_t in_len	:输入数组长度
 					uint8_t *out	:输出数组
 					uint16_t *out_len:输出数组长度
 * 输出参数 	: 
 * 返 回 值 	: 	0:名字或者其他
 					1:返回手机号码
 * 修改历史 	: 	无
 * 说    明 		: 	查找手机输入数组里面是否为手机号码
*****************************************************************************/
uint8_t find_mobilephone_no(uint8_t *in,uint16_t in_len)
{
	uint16_t i;
	uint8_t find = 0;
	for(i=0;i<in_len;i++)
	{
		if(in[i] <= 0x7F)
			find = 1;
	}

	return find;
}

/*****************************************************************************
 * 函 数 名 	: 	wipe_off_specific_sysbol
 * 函数功能 	: 
 * 输入参数 	: 	uint8_t *in	:输入数组
 					uint16_t in_len	:输入数组长度
 					uint8_t *out	:输出数组
 					uint16_t *out_len:输出数组长度
 * 输出参数 	: 
 * 返 回 值 	: 	0:返回成功
 * 修改历史 	: 	无
 * 说    明 		: 	去除输入数组中特殊符号的函数
*****************************************************************************/
uint8_t wipe_off_specific_sysbol(uint8_t *in,uint16_t in_len, uint8_t *out,uint16_t *out_len)
{
	uint16_t i,j;
	for(i=0,j=0;i<in_len;i++)
	{
		if(!(in[i] == '-' || in[i] == ' '))
			out[j++] = in[i];
	}
	
	*out_len = j;

	return 0;
}

/*****************************************************************************
 * 函 数 名 	: get_current_newest_remainder
 * 函数功能 	: 
 * 输入参数 	: 	uint8_t * remaind_type  提醒类型
               				uint8_t *index          提醒结构体数组下标值
 * 输出参数 	: 
 * 返 回 值 	: 0:返回成功
 * 修改历史 	: 无
 * 说    明 		: 获取当前最新信息提醒的类型和下标
*****************************************************************************/
static uint8_t get_current_newest_remainder(uint8_t * remaind_type,uint8_t *index)
{
	uint8_t i = 0,l_index = 0,find=0;
	uint32_t current_time = 0;
	
	current_time = system_sec_get();
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
		if(current_time - g_remainder_st.body_st[l_index].head.time <= 30)
		{
			*remaind_type = g_remainder_st.body_st[l_index].head.remaind_type;
			*index = l_index;
			return 0;
		}
	}

	return 1;
}


void set_remainder_index(uint8_t index)
{
	g_remainder_st.index = index%REMAINDER_GROUP_COUNT;
}

uint8_t get_remainder_index(void)
{
	return g_remainder_st.index;
}

remainder_head_st* get_current_call_st(void)
{
	if(g_call_st.remaind_flg)
		return &g_call_st;
	else
		return NULL;
}

/*****************************************************************************
 * 函 数 名 : get_current_message_st
 * 函数功能 : 
 * 输入参数 : 无
 * 输出参数 : 无
 * 返 回 值 : remainder_body_st
 * 修改历史 : 无
 * 说    明 : 获取当前最新信息提醒的结构体
*****************************************************************************/
remainder_body_st* get_current_message_st(void)
{
	uint8_t remaind_type=0,index=0;
	uint32_t current_time = 0;
	if(get_current_newest_remainder(&remaind_type,&index) == 0)
	{
		if(index < REMAINDER_GROUP_COUNT)
		{
			current_time = system_sec_get();
			if(current_time - g_remainder_st.body_st[index].head.time  <= 30)
			{
				if(remaind_type == MESSAGE_REMAIND)
				{
					return &(g_remainder_st.body_st[index]);			
				}
				else if(remaind_type == WECHAT_REMAIND)
				{
					return &(g_remainder_st.body_st[index]);
				}
			}
			
		}		
	}
	return NULL;	
}

/*****************************************************************************
 * 函 数 名 : set_remainder_info
 * 函数功能 : 
 * 输入参数 : 	phone_typ:手机类型
 				remaind_type:提醒类型
 				count:提醒数量
 				time:提醒时间
 				title_data:title内容
 				title_length:title长度
 				message_data:message内容
 				message_length:message长度
 * 输出参数 : 无
 * 返 回 值 : remainder_body_st
 * 修改历史 : 无
 * 说    明 : 设置当前最新信息提到结构体中
*****************************************************************************/
void set_remainder_info(uint8_t phone_type,
						uint8_t remaind_type,
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


		if(find_mobilephone_no(g_call_st.title_data,g_call_st.title_length))
		{
			wipe_off_specific_sysbol(g_call_st.title_data,g_call_st.title_length,g_call_st.title_data,&(g_call_st.title_length));
		}
	}
	else if(remaind_type == MESSAGE_REMAIND || remaind_type == WECHAT_REMAIND)
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
				g_remainder_st.index = (++index)%REMAINDER_GROUP_COUNT;
		}
		
		if(find == 1)
		{
			index = g_remainder_st.index;
					
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

			if(remaind_type == MESSAGE_REMAIND)
			{
				if(find_mobilephone_no(g_remainder_st.body_st[index].head.title_data,g_remainder_st.body_st[index].head.title_length))
				{
					wipe_off_specific_sysbol(g_remainder_st.body_st[index].head.title_data,
						g_remainder_st.body_st[index].head.title_length,
						g_remainder_st.body_st[index].head.title_data,
						&(g_remainder_st.body_st[index].head.title_length));
				}
			}
			
			//去掉内容前面的空格
			if(g_remainder_st.body_st[index].message_data[0] == 0x20)// == ' '
			{
				for(i=0;i<message_length-1;i++)
					g_remainder_st.body_st[index].message_data[i] = g_remainder_st.body_st[index].message_data[i+1];
				g_remainder_st.body_st[index].message_length--;
			}
			
			g_remainder_st.index = (++index)%REMAINDER_GROUP_COUNT;
		}	
	}

	//进行提醒UI进行界面显示信息
	switch(remaind_type)
	{
		case CALL_REMAIND:
			//RemindIncallingFlag_Set();
			QPRINTF("In Call......\r\n");
			break;
		case MESSAGE_REMAIND:
			//RemindMessageFlag_Set();
			QPRINTF("SMS Message...\r\n");
			break;
		case WECHAT_REMAIND:
			//RemindWechatFlag_Set();
			QPRINTF("WeChat Message...\r\n");
			break;
	}

	if((g_remainder_st.message_count + g_remainder_st.wechat_count) >= 2)
	{
		if(remaind_type== MESSAGE_REMAIND)
			QPRINTF("SMS Message >2...\r\n");
		else if(remaind_type == WECHAT_REMAIND)
			QPRINTF("WeChat Message >2...\r\n");//RemindWechatFlag_Set();
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

void clear_reminder_count(void)
{
	g_remainder_st.message_count = 0;
	g_remainder_st.wechat_count = 0;
}

void remaind_do(void)
{
	uint16_t i=0;
	uint8_t index = 0;
	uint8_t remaind_type = 0;

	if(g_call_st.remaind_flg)
	{

		QPRINTF("CALL remainder:\r\n");
		QPRINTF( "msgid=%d\r\n",		g_call_st.msg_id);
		QPRINTF( "phone_type=%d\r\n",	g_call_st.phone_type);
		QPRINTF( "remaind_type=%d\r\n",	g_call_st.remaind_type);
		QPRINTF( "time=%d\r\n",			g_call_st.time);
		QPRINTF( "remaind_flg=%d\r\n",	g_call_st.remaind_flg);
		QPRINTF( "title_length=%d\r\n",	g_call_st.title_length);
		QPRINTF( "title_data:%s\r\n",	g_call_st.title_data);
		memset(&(g_call_st), 0, sizeof(remainder_head_st));
	}
	else if(get_current_newest_remainder(&remaind_type,&index) == 0)
	{
		if(remaind_type == MESSAGE_REMAIND || 
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
			QPRINTF( "message_length=%d\r\n",g_remainder_st.body_st[index].message_length);
			QPRINTF( "message_data:%s\r\n",g_remainder_st.body_st[index].message_data);

			for(i=0;i<g_remainder_st.body_st[index].message_length;i++)
			{
				QPRINTF( "%02x,",g_remainder_st.body_st[index].message_data[i]);
			}
			QPRINTF( "\r\n");
			memset(&g_remainder_st.body_st[index], 0, sizeof(remainder_body_st));
			#endif		
		}
	}
}

