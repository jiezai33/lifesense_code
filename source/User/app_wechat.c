/***********************************************************************************
 * �� �� ��   : app_wechat.c
 * �� �� ��   : LiuYuanBin
 * ��������   : 2016��8��17��
 * ��Ȩ˵��   : Copyright (c) 2016-2025   �㶫����ҽ�Ƶ��ӹɷ����޹�˾
 * �ļ�����   : ΢�������շ��ӿڣ���ȫ��ble�ײ��޹ص�����������
 * �޸���ʷ   : 
***********************************************************************************/
#include "app_wechat.h"
#include <string.h>
#include "usr_design.h"
#include "debug.h"
#include "app_wechat_common.h"
#include "usr_login.h"
#include "usr_data.h"
#include "channel_select.h"
#include "wechat_usrdesign.h"

extern void sys_start_pair_mode(void);

void app_wechat_connection(void)
{
	QPRINTF("app_wechat_connection\r\n");
	memset(&g_communication_statue,0,sizeof(communication_statue_st));
}

void app_wechat_disconnection(void)
{
	QPRINTF("app_wechat_disconnection\r\n");
}

/*****************************************************************************
 * �� �� �� : app_wechat_indicate_statue_set
 * �������� : 
 * ������� : ble_wechat_evt_type_t statue  ΢��indicate��״̬
 * ������� : ��
 * �� �� ֵ : ��
 * �޸���ʷ : ��
 * ˵    �� : ��ȡ΢��indicate��״̬
*****************************************************************************/
void app_wechat_indicate_statue_set(ble_wechat_evt_type_t statue)
{
	switch(statue)
	{
		case BLE_WECHAT_EVT_INDICATION_ENABLED:
			QPRINTF("BLE_WECHAT_EVT_INDICATION_ENABLED\r\n");
			g_trans_evt_hander.bit.wechat_login_bit_1 = 1;
			usTxWeChatPackSeq = 1;
			usr_set_app_type(WECHAT_APP);
			break;
			
		case BLE_WECHAT_EVT_INDICATION_DISABLED:
			QPRINTF("BLE_WECHAT_EVT_INDICATION_DISABLED\r\n");
			break;
			
		case BLE_WECHAT_EVT_INDICATION_CONFIRMED:
			QPRINTF("BLE_WECHAT_EVT_INDICATION_CONFIRMED\r\n");
			break;
			
		default:break;
	}
}

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
uint32_t app_wechat_send_data(uint8_t *data,uint16_t length,wechat_channel_enum channel_type,uint8_t channel)
{
	return wechat_send_data(data,length,channel_type,channel);
}

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
void app_wechat_receive(uint16_t cmd_id,uint8_t *data,uint8_t length)
{
	uint8_t error,i;
    uint8_t  datalen = 0; // ������ݳ���
    uint8_t  offset = 0;  // �������
    uint8_t  rcv_data[220] = {0}; // ��������ݴ��buffer	
	uint8_t dest_field = DATA_DATA_FIELD;	

	if(cmd_id == WECHAT_CMDID_RESQ_ENTRY)                      //��½�ظ�ָ��
	{
		dest_field = AUTH_AES_SESSION_KEY_FIELD;	
	}
	else if(cmd_id == WECHAT_CMDID_RESQ_INIT)                       //�ж��Ƿ���init�ظ�������ID
    {
    	dest_field = INIT_TIME_FIELD;
    }
	else if(cmd_id == WECHAT_CMDID_RESQ_USERINFO || cmd_id == 30001 || cmd_id == 30002 ||cmd_id == 30003) 
	{
		dest_field = DATA_DATA_FIELD;
	}

	error = response_unpack(dest_field,data, length, rcv_data, &datalen,&offset);
	
	if(error == false)       //���������Ͽ�����
    {
        QPRINTF("protobuf parse error\r\n");
		return;
    }
	
	switch(cmd_id)        
	{
		case WECHAT_CMDID_RESQ_ENTRY:	//��½�ظ�ָ��		
			if(TRUE == memcmp(rcv_data, "factory mode", datalen))        //�ж��Ƿ��������ģʽ
	        {
	            QPRINTF("factory mode\r\n");
	        }
	        else if(TRUE == memcmp(rcv_data, "test mode", datalen))      //�ж��Ƿ�������ģʽ
	        {
	           	QPRINTF("test mode\r\n");
	        }
	        else if(TRUE == memcmp(rcv_data, "check mode", datalen))     //�ж��Ƿ�������ģʽ
	        {
	            QPRINTF("check mode\r\n");
	        }
	        else if(TRUE == memcmp(rcv_data, "pair mode", datalen))      //�ж��Ƿ������ģʽ
	        {
	            QPRINTF("pair mode\r\n");
	        }
	        else                                                            //������½΢��
	        {
	        	QPRINTF("send initerq\r\n");
	            g_trans_evt_hander.bit.wechat_init_bit_2 = 1;
				g_communication_statue.transfer_statue = INIT_STATUE;
	        }
		break;
		
		case WECHAT_CMDID_RESQ_INIT:                      //�ж��Ƿ���init�ظ�������ID
			g_trans_evt_hander.bit.wechat_send_data_bit_3 = 1;
			g_communication_statue.transfer_statue = DATA_STATUE;

			error = response_unpack(INIT_TIME_STRING_FIELD,data, length, rcv_data, &datalen,&offset);
            if(error ==false)
            {
                QPRINTF("INIT_TIME_STRING_FIELD error = false\r\n");
            }
			else
			{
				QPRINTF("time:");
				for(i=0;i<datalen;i++)
					QPRINTF("%02x,",rcv_data[i]);
				QPRINTF("\r\n");
			}

			error = response_unpack(INIT_PLAT_FORM_TYPE_FIELD,data, length, rcv_data, &datalen,&offset);
            if(error ==false)
            {
                QPRINTF("INIT_PLAT_FORM_TYPE_FIELD error = false\r\n");
            }
			else
			{
				QPRINTF("INIT_PLAT_FORM_TYPE_FIELD:");
				for(i=0;i<datalen;i++)
					QPRINTF("%02x,",rcv_data[i]);
				QPRINTF("\r\n");
				if(rcv_data[0] == 1)//phone == ios
					sys_start_pair_mode();
			}
		break;
		
		case WECHAT_CMDID_RESQ_USERINFO:
			for(i=0;i<datalen-1;i++)
			{
				if(rcv_data[i] == 0xAA && rcv_data[i+1] == 0x01)
				{
					break;
				}
			}
			
			if(i<length-1)
				data_process(rcv_data+i+2,datalen - i -2);
		break;
		
		case 30001:                                 //push������		
			wechat_push_data_process(rcv_data,datalen);        
		break;
		
     	case 30003:              //΢�Ž����̨�ͷ���ǰ̨֪ͨ������IOS�ϣ�Ŀǰ�ڰ�׿�ϲ���û�и�֪ͨ
	       	if((*(data + 2) == 0x10) && (*(data + 3) == 0x01))        //  ΢�������̨��ر�
	        {
	            //��Ϊ�޷��ֱ��������̨���ǹرգ�������ͳһ���رմ������ֻ�������̨
	            //�ڷ���ǰ̨��ʱ��ָ���־λ
				QPRINTF("wechat in background\r\n");
				g_communication_statue.app_statue = BACKGROUND_STATUE;
	        }
	        else if((*(data + 2) == 0x10) && (*(data + 3) == 0x02))   //΢�ŷ���ǰ̨
	        {
				QPRINTF("wechat in fornt\r\n");
				g_communication_statue.app_statue = FRONT_STATUE;
	        }
		break;

		default:break;
	}
}



