#ifndef _APP_WECHAT_H_
#define _APP_WECHAT_H_
#include "ble_wechat.h"
#include "wechat_usrdesign.h"

#define AUTH_BASE_RESPONSE_FIELD              0X01
#define AUTH_AES_SESSION_KEY_FIELD            0X02


#define INIT_BASE_RESPONSE_FIELD              0X01
#define INIT_USER_ID_HIGH_FIELD               0X02
#define INIT_USER_ID_LOW_FIELD                0X03
#define INIT_CHALLEANGE_ANSWER_FIELD          0X04
#define INIT_INIT_SCENCE_FIELD                0X05
#define INIT_AUTO_SYN_MAX_DURA_SEC_FIELD      0X06
#define INIT_USER_NICK_NAME_FIELD             0X0B
#define INIT_PLAT_FORM_TYPE_FIELD             0X0C
#define INIT_MODEL_FIELD                      0X0D
#define INIT_OS_FIELD                         0X0E
#define INIT_TIME_FIELD                       0X0F
#define INIT_TIME_ZONE_FIELD                  0X10
#define INIT_TIME_STRING_FIELD                0X11


void app_wechat_connection(void);
void app_wechat_disconnection(void);




/*****************************************************************************
 * �� �� �� : app_wechat_indicate_statue_set
 * �������� : 
 * ������� : ble_wechat_evt_type_t statue  ΢��indicate��״̬
 * ������� : ��
 * �� �� ֵ : ��
 * �޸���ʷ : ��
 * ˵    �� : ��ȡ΢��indicate��״̬
*****************************************************************************/
void app_wechat_indicate_statue_set(ble_wechat_evt_type_t statue);




/*****************************************************************************
 * �� �� �� : app_wechat_send_data
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
uint32_t app_wechat_send_data(uint8_t *data,uint16_t length,wechat_channel_enum channel_type,uint8_t channel);




/*****************************************************************************
 * �� �� �� : app_wechat_receive
 * �������� : 
 * ������� :  uint16_t cmd_id  �����
               uint8_t *data    �䳤��������ָ��
               uint8_t length   �䳤���峤��
 * ������� : ��
 * �� �� ֵ : ��
 * �޸���ʷ : ��
 * ˵    �� : ΢�����ݵĽ���
*****************************************************************************/
void app_wechat_receive(uint16_t cmd_id,uint8_t *data,uint8_t length);




#endif

