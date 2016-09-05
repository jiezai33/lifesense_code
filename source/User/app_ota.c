#include "app_ota.h"
#include "debug.h"
#include "ota_usrdesign.h"


void app_ota_connection(void)
{
	QPRINTF("app_ota_connection\r\n");
}

void app_ota_disconnection(void)
{
	QPRINTF("app_ota_disconnection\r\n");
}

/*****************************************************************************
 * �� �� �� : app_ota_indicate_statue_set
 * �������� : 
 * ������� : ble_ota_evt_type_t statue  OTA indicate��״̬
 * ������� : ��
 * �� �� ֵ : ��
 * �޸���ʷ : ��
 * ˵    �� : ��ȡOTA indicate��״̬
*****************************************************************************/
void app_ota_indicate_statue_set(ble_ota_evt_type_t statue)
{
	switch(statue)
	{
		case BLE_OTA_EVT_INDICATION_ENABLED:
			QPRINTF("BLE_OTA_EVT_INDICATION_ENABLED\r\n");
			break;

		case BLE_OTA_EVT_INDICATION_DISABLED:
			QPRINTF("BLE_OTA_EVT_INDICATION_DISABLED\r\n");
			break;

		case BLE_OTA_EVT_INDICATION_CONFIRMED:
			QPRINTF("BLE_OTA_EVT_INDICATION_CONFIRMED\r\n");
			break;
			
		default:break;
	}
}

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
uint32_t app_ota_send_data(uint8_t *data,uint16_t length,ota_channel_enum channel_type,uint8_t channel)
{
	return ota_send_data(data,length,channel_type,channel);
}

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
void app_ota_cmd_receive(uint8_t cmd_i,uint8_t *data,uint16_t data_len)
{
	switch(cmd_i)
	{
		case 0x02:
			break;
			
		default:break;
	}
}

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
void app_ota_data_receive(uint8_t *data,uint16_t data_len)
{

}



