#ifndef _WECHAT_USRDESIGN_H_
#define _WECHAT_USRDESIGN_H_
#include <stdint.h>
#include "ble_wechat.h"
#include "data_products.h"


#define WECHAT_TX_START_ADDR			(0)
#define WECHAT_TX_SIZE					(WECHAT_SEND_DATA_SIZE)

#define WECHAT_RX_START_ADDR			(0)
#define WECHAT_RX_SIZE					(WECHAT_RECEIVE_DATA_SIZE)

typedef enum
{
	WECHAT_RECEIVE_ED = 0,
	WECHAT_RECEIVE_ING,
	WECHAT_RECEIVE_MAX
}wechat_receive_enum;

typedef enum
{
	WECHAT_INDICATE_CHANNEL= 0,
	WECHAT_NOTIFI_CHANNEL,
	WECHAT_CHANNEL_MAX,
}wechat_channel_enum;

typedef struct
{
	uint8_t rec_flg;
	uint16_t package_len;	//版本号
	uint16_t cmd_no;		//命令号
	uint16_t send_i;		//发送数据包序号
	uint8_t data_len;		//变长包体的长度
	
	#if DATA_TYPE == DATA_POINTER_TYPE
	uint8_t *data;			//变长包体
	#elif DATA_TYPE == DATA_BUFFER_TYPE
	uint8_t data[WECHAT_RECEIVE_DATA_SIZE];
	#endif
	
}wechat_receive_pack_st;


typedef struct
{
	uint8_t send_flg;
	uint8_t channel_type;
	uint8_t channel;
	uint8_t indicate_available_flg;
	uint16_t send_index;
	uint16_t data_len;
	#if DATA_TYPE == DATA_POINTER_TYPE
	uint8_t *data;
	#elif DATA_TYPE == DATA_POINTER_TYPE
	uint8_t data[WECHAT_SEND_DATA_SIZE];
	#endif
}wechat_send_pack_st;

void wechat_connection(void);
void wechat_disconnection(void);
void wechat_indicate_statue_set(ble_wechat_evt_type_t statue);




/*****************************************************************************
 * 函 数 名 : wechat_receive
 * 函数功能 : 
 * 输入参数 :  uint8_t *data    从ble_wechat通道里面接收的数据
               uint16_t length  接收数据的长度
 * 输出参数 : 无
 * 返 回 值 : 	0:数据接收完成
 				1:数据接收未完成
 				2:数据包头解析错误
 * 修改历史 : 无
 * 说    明 : 无
*****************************************************************************/
uint32_t wechat_receive(uint8_t *data,uint16_t length);




/*****************************************************************************
 * 函 数 名 : wechat_send_data
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
uint32_t wechat_send_data(uint8_t *data,uint16_t length,wechat_channel_enum channel_type,uint8_t channel);




/*****************************************************************************
 * 函 数 名 : wechat_periodic_send_data
 * 函数功能 : 
 * 输入参数 : void  void
 * 输出参数 : void
 * 返 回 值 : 	0:发送完成或者没有数据发送		
 				1:有数据发送
 * 修改历史 : 无
 * 说    明 : 微信1包的数据分多帧发送，每帧20byte，在mainloop循环里面发送
*****************************************************************************/
uint8_t wechat_periodic_send_data(void);




#endif


