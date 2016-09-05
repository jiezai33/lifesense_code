#ifndef _APP_TRANS_H_
#define _APP_TRANS_H_
#include <stdint.h>
#include "ble_transfer.h"
#include "transfer_usrdesign.h"


#define CMD_VERSION_0		(0xAA)
#define CMD_VERSION_1		(0x01)


void app_trans_connection(void);
void app_trans_disconnection(void);




/*****************************************************************************
 * 函 数 名 : app_trans_indicate_statue_set
 * 函数功能 : 
 * 输入参数 : ble_trans_evt_type_t statue  微信indicate的状态
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 获取A500 indicate的状态
*****************************************************************************/
void app_trans_indicate_statue_set(ble_trans_evt_type_t statue);




/*****************************************************************************
 * 函 数 名 : app_transfer_send_data
 * 函数功能 : 
 * 输入参数 :  uint8_t *data                     发送的数据
               uint16_t length                   发送数据的长度
               trans_channel_enum channel_type   A500 的indicate或者notifi通道选择
               uint8_t channel                   目前是notifi通道第几个通道选择
 * 输出参数 : 无
 * 返 回 值 : 	0:OK 	
 				1:通道选择错误	
 				2:上次的数据还没有发送完成
 * 修改历史 : 无
 * 说    明 : 无
*****************************************************************************/
uint32_t app_transfer_send_data(uint8_t *data,uint16_t length,trans_channel_enum channel_type,uint8_t channel);




/*****************************************************************************
 * 函 数 名 : app_trans_receive
 * 函数功能 : 
 * 输入参数 : uint8_t *data   A500 的数据指针
               uint8_t length  A500 的数据长度
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : A500的数据接收
*****************************************************************************/
void app_trans_receive(uint8_t *data,uint8_t length);




#endif



