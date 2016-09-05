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
 * �� �� �� : app_trans_indicate_statue_set
 * �������� : 
 * ������� : ble_trans_evt_type_t statue  ΢��indicate��״̬
 * ������� : ��
 * �� �� ֵ : ��
 * �޸���ʷ : ��
 * ˵    �� : ��ȡA500 indicate��״̬
*****************************************************************************/
void app_trans_indicate_statue_set(ble_trans_evt_type_t statue);




/*****************************************************************************
 * �� �� �� : app_transfer_send_data
 * �������� : 
 * ������� :  uint8_t *data                     ���͵�����
               uint16_t length                   �������ݵĳ���
               trans_channel_enum channel_type   A500 ��indicate����notifiͨ��ѡ��
               uint8_t channel                   Ŀǰ��notifiͨ���ڼ���ͨ��ѡ��
 * ������� : ��
 * �� �� ֵ : 	0:OK 	
 				1:ͨ��ѡ�����	
 				2:�ϴε����ݻ�û�з������
 * �޸���ʷ : ��
 * ˵    �� : ��
*****************************************************************************/
uint32_t app_transfer_send_data(uint8_t *data,uint16_t length,trans_channel_enum channel_type,uint8_t channel);




/*****************************************************************************
 * �� �� �� : app_trans_receive
 * �������� : 
 * ������� : uint8_t *data   A500 ������ָ��
               uint8_t length  A500 �����ݳ���
 * ������� : ��
 * �� �� ֵ : ��
 * �޸���ʷ : ��
 * ˵    �� : A500�����ݽ���
*****************************************************************************/
void app_trans_receive(uint8_t *data,uint8_t length);




#endif



