#ifndef _ANCS_ANDROID_USRDESIGN_H_
#define _ANCS_ANDROID_USRDESIGN_H_
#include <stdint.h>
#include "ble_ancs_android.h"
#include "data_products.h"

#define ANDROID_ANCS_DEF_MAX_FRAME_SIZE		(14)

#define ANCS_TX_START_ADDR				(0)
#define ANCS_TX_SIZE					(ANCS_SEND_DATA_SIZE)

#define ANCS_RX_START_ADDR				(0)
#define ANCS_RX_SIZE					(ANCS_RECEIVE_DATA_SIZE)


typedef enum
{
	ANDROID_ANCS_RECEIVE_ED = 0,
	ANDROID_ANCS_RECEIVE_ING,
	ANDROID_ANCS_RECEIVE_MAX
}android_ancs_receive_enum;

typedef enum
{
	ANDROID_ANCS_INDICATE_CHANNEL= 0,
	ANDROID_ANCS_NOTIFI_CHANNEL,
	ANDROID_ANCS_CHANNEL_MAX,
}android_ancs_channel_enum;

typedef struct
{
	uint8_t rec_flg;
	uint8_t frame_i;	//帧序号
	uint8_t package_len;//包长度
	uint8_t data_len;	//数据长度
	
	#if DATA_TYPE == DATA_POINTER_TYPE
	uint8_t *data;		//数据
	#elif DATA_TYPE == DATA_BUFFER_TYPE
	uint8_t data[ANCS_RECEIVE_DATA_SIZE];
	#endif
}android_ancs_receive_pack_st;


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
	uint8_t data[ANCS_SEND_DATA_SIZE];
	#endif
}android_ancs_send_pack_st;

void android_ancs_connection(void);
void android_ancs_disconnection(void);
void android_ancs_indicate_statue_set(ble_android_ancs_evt_type_t statue);
uint32_t android_ancs_receive(uint8_t *data,uint16_t length);
uint32_t android_ancs_send_data(uint8_t *data,uint16_t length,android_ancs_channel_enum channel_type,uint8_t channel);
uint8_t android_ancs_periodic_send_data(void);

#endif




