#ifndef _OTA_USRDESIGN_H_
#define _OTA_USRDESIGN_H_
#include <stdint.h>
#include "ble_ota.h"
#include "data_products.h"


#define OTA_TX_START_ADDR			(0)
#define OTA_TX_SIZE					(OTA_SEND_DATA_SIZE)

#define OTA_RX_START_ADDR			(0)
#define OTA_RX_SIZE					(OTA_RECEIVE_DATA_SIZE)


typedef enum
{
	OTA_RECEIVE_ED = 0,
	OTA_RECEIVE_ING,
	OTA_RECEIVE_MAX
}ota_receive_enum;

typedef struct
{
	uint8_t rec_flg;
	uint8_t cmd_i;		//命令字
	uint8_t frame_i;	//帧序号
	uint8_t package_len;//包长度
	uint8_t data_len;	//数据内容长度
	
	#if DATA_TYPE == DATA_POINTER_TYPE
	uint8_t *data;		//数据内容
	#elif DATA_TYPE == DATA_BUFFER_TYPE
	uint8_t data[OTA_RECEIVE_DATA_SIZE];
	#endif
}ota_receive_pack_st;

typedef enum
{
	OTA_INDICATE_CHANNEL= 0,
	OTA_NOTIFI_CHANNEL,
	OTA_CHANNEL_MAX,
}ota_channel_enum;

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
	uint8_t data[OTA_SEND_DATA_SIZE];
	#endif
}ota_send_pack_st;

void ota_connection(void);
void ota_disconnection(void);
void ota_indicate_statue_set(ble_ota_evt_type_t statue);
uint32_t ota_cmd_receive(uint8_t *data,uint16_t length);
uint32_t ota_data_receive(uint8_t *data,uint16_t length);
uint32_t ota_send_data(uint8_t *data,uint16_t length,ota_channel_enum channel_type,uint8_t channel);
uint8_t ota_periodic_send_data(void);


#endif

