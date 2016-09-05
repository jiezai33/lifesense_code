#ifndef _TRANSFER_USRDESIGN_H
#define _TRANSFER_USRDESIGN_H
#include <stdint.h>
#include "data_products.h"
#include "ble_transfer.h"

#define TRANS_DEF_MAX_FRAME_SIZE          (6)


#define TRANS_TX_START_ADDR			(0)
#define TRANS_TX_SIZE				(TRANS_SEND_DATA_SIZE)

#define TRANS_RX_START_ADDR			(0)
#define TRANS_RX_SIZE				(TRANS_RECEIVE_DATA_SIZE)

typedef enum
{
	TRANS_RECEIVE_ED = 0,
	TRANS_RECEIVE_ING,
	TRANS_RECEIVE_MAX
}trans_receive_enum;

typedef enum
{
	TRANS_INDICATE_CHANNEL= 0,
	TRANS_NOTIFI_CHANNEL,
	TRANS_CHANNEL_MAX,
}trans_channel_enum;

typedef struct
{
	uint8_t rec_flg;
	uint16_t package_i;		//包序号
	uint8_t package_len;	//包长度
	uint8_t frame_i;		//帧序号
	uint8_t data_len;		//变长包体长度
	
	#if DATA_TYPE == DATA_POINTER_TYPE
	uint8_t *data;			//变长包体
	#elif DATA_TYPE == DATA_BUFFER_TYPE
	uint8_t data[TRANS_RECEIVE_DATA_SIZE];
	#endif
	
}trans_receive_pack_st;


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
	#elif DATA_TYPE == DATA_BUFFER_TYPE
	uint8_t data[TRANS_SEND_DATA_SIZE];
	#endif
}trans_send_pack_st;

void transfer_connection(void);
void transfer_disconnection(void);
void trans_indicate_statue_set(ble_trans_evt_type_t statue);




/*****************************************************************************
 * 函 数 名 : transfer_receive
 * 函数功能 : 
 * 输入参数 :  uint8_t *data    从ble_transfer通道里面接收的数据
               uint16_t length  接收数据的长度
 * 输出参数 : 无
 * 返 回 值 : 	0:数据接收完成
 				1:数据接收未完成
 				2:帧序号超过一包数据最大帧数
 				3:数据包头解析错误
 * 修改历史 : 无
 * 说    明 : 无
*****************************************************************************/
uint32_t transfer_receive(uint8_t *data,uint16_t length);




/*****************************************************************************
 * 函 数 名 : transfer_send_data
 * 函数功能 : 
 * 输入参数 :  uint8_t *data                     发送的数据
               uint16_t length                   发送数据的长度
               wechat_channel_enum channel_type  A500的indicate或者notifi通道选择
               uint8_t channel                   目前是notifi通道第几个通道选择
 * 输出参数 : 无
 * 返 回 值 : 	0:OK 	
 				1:通道选择错误	
 				2:上次的数据还没有发送完成
 * 修改历史 : 无
 * 说    明 : 无
*****************************************************************************/
uint32_t transfer_send_data(uint8_t *data,uint16_t length,trans_channel_enum channel_type,uint8_t channel);




/*****************************************************************************
 * 函 数 名 : transfer_periodic_send_data
 * 函数功能 : 
 * 输入参数 : void  void
 * 输出参数 : void
 * 返 回 值 : 	0:发送完成或者没有数据发送		
 				1:有数据发送
 * 修改历史 : 无
 * 说    明 : A500 1包的数据分多帧发送，每帧20byte，在mainloop循环里面发送
*****************************************************************************/
uint8_t transfer_periodic_send_data(void);




#endif


