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
	uint16_t package_i;		//�����
	uint8_t package_len;	//������
	uint8_t frame_i;		//֡���
	uint8_t data_len;		//�䳤���峤��
	
	#if DATA_TYPE == DATA_POINTER_TYPE
	uint8_t *data;			//�䳤����
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
 * �� �� �� : transfer_receive
 * �������� : 
 * ������� :  uint8_t *data    ��ble_transferͨ��������յ�����
               uint16_t length  �������ݵĳ���
 * ������� : ��
 * �� �� ֵ : 	0:���ݽ������
 				1:���ݽ���δ���
 				2:֡��ų���һ���������֡��
 				3:���ݰ�ͷ��������
 * �޸���ʷ : ��
 * ˵    �� : ��
*****************************************************************************/
uint32_t transfer_receive(uint8_t *data,uint16_t length);




/*****************************************************************************
 * �� �� �� : transfer_send_data
 * �������� : 
 * ������� :  uint8_t *data                     ���͵�����
               uint16_t length                   �������ݵĳ���
               wechat_channel_enum channel_type  A500��indicate����notifiͨ��ѡ��
               uint8_t channel                   Ŀǰ��notifiͨ���ڼ���ͨ��ѡ��
 * ������� : ��
 * �� �� ֵ : 	0:OK 	
 				1:ͨ��ѡ�����	
 				2:�ϴε����ݻ�û�з������
 * �޸���ʷ : ��
 * ˵    �� : ��
*****************************************************************************/
uint32_t transfer_send_data(uint8_t *data,uint16_t length,trans_channel_enum channel_type,uint8_t channel);




/*****************************************************************************
 * �� �� �� : transfer_periodic_send_data
 * �������� : 
 * ������� : void  void
 * ������� : void
 * �� �� ֵ : 	0:������ɻ���û�����ݷ���		
 				1:�����ݷ���
 * �޸���ʷ : ��
 * ˵    �� : A500 1�������ݷֶ�֡���ͣ�ÿ֡20byte����mainloopѭ�����淢��
*****************************************************************************/
uint8_t transfer_periodic_send_data(void);




#endif


