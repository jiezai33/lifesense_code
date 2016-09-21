#ifndef _USR_REMINDER_H_
#define _USR_REMINDER_H_
#include <stdint.h>
#include "ble_ancs_android.h"
#include "ble_ancs_ios.h"

#define REMAINDER_GROUP_COUNT	(3)

#define TITLE_DATA_SIZE			(34)
#define MESSAGE_DATA_SIZE		(100)

typedef enum
{
	IOS_TYPE = 0,
	ANDROID_TYPE,
	PHONE_MAX
}phone_type;

typedef enum
{
	CALL_REMAIND = 0,
	MESSAGE_REMAIND,
	WECHAT_REMAIND,
	QQ_REMAIND,
	REMAIND_MAX
}remaind_type_enum;

typedef enum
{
	UTF_8 = 0,
	UNICODE,
	CODE_MAX
}code_type_enum;

typedef enum
{
	LOCK_OFF = 0,
	LOCK_ON,
	LOCK_MAX
}lock_type_enum;

typedef struct
{
	uint32_t msg_id;
	uint8_t phone_type;		//phone type  Reference phone_type
	uint8_t remaind_type;	//remaind type Reference remaind_type_enum 
	uint8_t remaind_flg;	//read remainf flg 1:no		0:yes
	uint32_t time;			//remaind time
	uint8_t code_format;	//format
	uint8_t lock;			//lock
	uint16_t title_length;
	uint8_t title_data[TITLE_DATA_SIZE];
}remainder_head_st;


typedef struct
{
	remainder_head_st head;
	uint16_t message_length;		
	uint8_t message_data[MESSAGE_DATA_SIZE];	//depend on phone_type and remaind_type
}remainder_body_st;

typedef struct
{
	uint8_t index;
	uint8_t message_count;
	uint8_t wechat_count;
	remainder_body_st body_st[REMAINDER_GROUP_COUNT]; 
}remainder_st;

/********************************************************
*	remaind_type == CALL_REMAIND
*		title_data save call name or telephone number
*		message_data is NULL
*
*	remaind_type == MESSAGE_REMAIND
*		title_data save call name or telephone number
*		message_data save message data
*
*	remaind_type == WECHAT_REMAIND
*		title_data is NULL
*		message_data save Wechat data
*
*	remaind_type == QQ_REMAIND
*		title_data is NULL
*		message_data save QQ data
*
*	struDispTitleCode:
*	struDispMessageCode:
*	reference IncomingCall_Source_Parse

IOS:
call:		title_data:"ºÅÂë"or"ÐÕÃû"
message:	title_data:"ºÅÂë"or"ÐÕÃû"		message_data:"ÄÚÈÝ"
wecaht:		title_data:"Î¢ÐÅ"				message_data:"êÇ³Æ":"ÄÚÈÝ"

Android:
call:		title_data:"ºÅÂë"or"ÐÕÃû"
message:	title_data:"ºÅÂë"or"ÐÕÃû"		message_data:"ÄÚÈÝ"
wecaht:		title_data:"êÇ³Æ"				message_data:"ÄÚÈÝ"

**********************************************************/

/*****************************************************************************
 * ? ? ? 	: 	find_mobilephone_no
 * ???? 	: 
 * ???? 	: 	uint8_t *in	:????
 					uint16_t in_len	:??????
 					uint8_t *out	:????
 					uint16_t *out_len:??????
 * ???? 	: 
 * ? ? ? 	: 	0:??????
 					1:??????
 * ???? 	: 	?
 * ?    ? 		: 	?????????????????
*****************************************************************************/
uint8_t find_mobilephone_no(uint8_t *in,uint16_t in_len);




/*****************************************************************************
 * ? ? ? 	: 	wipe_off_specific_sysbol
 * ???? 	: 
 * ???? 	: 	uint8_t *in	:????
 					uint16_t in_len	:??????
 					uint8_t *out	:????
 					uint16_t *out_len:??????
 * ???? 	: 
 * ? ? ? 	: 	0:????
 * ???? 	: 	?
 * ?    ? 		: 	??????????????
*****************************************************************************/
uint8_t wipe_off_specific_sysbol(uint8_t *in,uint16_t in_len, uint8_t *out,uint16_t *out_len);




void set_remainder_index(uint8_t index);

uint8_t get_remainder_index(void);

remainder_head_st* get_current_call_st(void);
remainder_body_st* get_current_message_st(void);

void set_remainder_info(uint8_t phone_type,
						uint8_t remaind_type,
						uint32_t time,
						
						uint8_t *title_data,
						uint16_t title_length,
						uint8_t *message_data,
						uint16_t message_length);
remainder_st* get_remainder_info(void);
uint8_t get_remainder_message_count(void);
uint8_t get_remainder_wechat_count(void);
void clear_all_remainder_info(void);
void clear_reminder_count(void);
void remaind_do(void);

#endif


