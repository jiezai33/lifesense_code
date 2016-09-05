#ifndef _OTA_APP_H_
#define _OTA_APP_H_
#include <stdint.h>
#include "ble_ota.h"
#include "ota_usrdesign.h"

void app_ota_connection(void);
void app_ota_disconnection(void);

/*****************************************************************************
 * �� �� �� : app_ota_indicate_statue_set
 * �������� : 
 * ������� : ble_ota_evt_type_t statue  OTA indicate��״̬
 * ������� : ��
 * �� �� ֵ : ��
 * �޸���ʷ : ��
 * ˵    �� : ��ȡOTA indicate��״̬
*****************************************************************************/
void app_ota_indicate_statue_set(ble_ota_evt_type_t statue);





/*****************************************************************************
 * �� �� �� : app_ota_send_data
 * �������� : 
 * ������� :  uint8_t *data                     ���͵�����
               uint16_t length                   �������ݵĳ���
               ota_channel_enum channel_type     OTA��indicate����notifiͨ��ѡ��
               uint8_t channel                   Ŀǰ��notifiͨ���ڼ���ͨ��ѡ��
 * ������� : ��
 * �� �� ֵ : 	0:OK 	
 				1:ͨ��ѡ�����	
 				2:�ϴε����ݻ�û�з������
 * �޸���ʷ : ��
 * ˵    �� : ��
*****************************************************************************/
uint32_t app_ota_send_data(uint8_t *data,uint16_t length,ota_channel_enum channel_type,uint8_t channel);




/*****************************************************************************
 * �� �� �� : app_ota_cmd_receive
 * �������� : 
 * ������� :  uint16_t cmd_id  �����
               uint8_t *data    �䳤��������ָ��
               uint8_t length   �䳤���峤��
 * ������� : ��
 * �� �� ֵ : ��
 * �޸���ʷ : ��
 * ˵    �� : OTA ����Ľ���
*****************************************************************************/
void app_ota_cmd_receive(uint8_t cmd_i,uint8_t *data,uint16_t data_len);




/*****************************************************************************
 * �� �� �� : app_ota_data_receive
 * �������� : 
 * ������� :  uint8_t *data    �䳤��������ָ��
               uint8_t length   �䳤���峤��
 * ������� : ��
 * �� �� ֵ : ��
 * �޸���ʷ : ��
 * ˵    �� : OTA ���ݵĽ���
*****************************************************************************/
void app_ota_data_receive(uint8_t *data,uint16_t data_len);

#endif



