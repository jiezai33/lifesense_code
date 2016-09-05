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
	uint16_t package_len;	//�汾��
	uint16_t cmd_no;		//�����
	uint16_t send_i;		//�������ݰ����
	uint8_t data_len;		//�䳤����ĳ���
	
	#if DATA_TYPE == DATA_POINTER_TYPE
	uint8_t *data;			//�䳤����
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
 * �� �� �� : wechat_receive
 * �������� : 
 * ������� :  uint8_t *data    ��ble_wechatͨ��������յ�����
               uint16_t length  �������ݵĳ���
 * ������� : ��
 * �� �� ֵ : 	0:���ݽ������
 				1:���ݽ���δ���
 				2:���ݰ�ͷ��������
 * �޸���ʷ : ��
 * ˵    �� : ��
*****************************************************************************/
uint32_t wechat_receive(uint8_t *data,uint16_t length);




/*****************************************************************************
 * �� �� �� : wechat_send_data
 * �������� : 
 * ������� :  uint8_t *data                     ���͵�����
               uint16_t length                   �������ݵĳ���
               wechat_channel_enum channel_type  ΢�ŵ�indicate����notifiͨ��ѡ��
               uint8_t channel                   Ŀǰ��notifiͨ���ڼ���ͨ��ѡ��
 * ������� : ��
 * �� �� ֵ : 	0:OK 	
 				1:ͨ��ѡ�����	
 				2:�ϴε����ݻ�û�з������
 * �޸���ʷ : ��
 * ˵    �� : ��
*****************************************************************************/
uint32_t wechat_send_data(uint8_t *data,uint16_t length,wechat_channel_enum channel_type,uint8_t channel);




/*****************************************************************************
 * �� �� �� : wechat_periodic_send_data
 * �������� : 
 * ������� : void  void
 * ������� : void
 * �� �� ֵ : 	0:������ɻ���û�����ݷ���		
 				1:�����ݷ���
 * �޸���ʷ : ��
 * ˵    �� : ΢��1�������ݷֶ�֡���ͣ�ÿ֡20byte����mainloopѭ�����淢��
*****************************************************************************/
uint8_t wechat_periodic_send_data(void);




#endif


