#include "app_android_ancs.h"
#include "ancs_android_usrdesign.h"
#include "usr_reminder.h"
#include "time.h"
#include "debug.h"

static uint8_t g_retry_count = 0;     //�Ҷ����Դ���
static uint8_t g_action_type = 0;     //���ݽ������ǹҶ�

static remainder_head_st g_reminder_head;


void app_android_ancs_connection(void)
{
	QPRINTF("app_android_ancs_connection\r\n");
}

void app_android_ancs_disconnection(void)
{
	QPRINTF("app_android_ancs_disconnection\r\n");
}


void app_android_ancs_indicate_statue_set(ble_android_ancs_evt_type_t statue)
{
	switch(statue)
	{
		case BLE_ANDROID_ANCS_EVT_INDICATION_ENABLED:
			QPRINTF("BLE_ANDROID_ANCS_EVT_INDICATION_ENABLED\r\n");
			break;

		case BLE_ANDROID_ANCS_EVT_INDICATION_DISABLED:
			QPRINTF("BLE_ANDROID_ANCS_EVT_INDICATION_DISABLED\r\n");
			break;

		case BLE_ANDROID_ANCS_EVT_INDICATION_CONFIRMED:
			QPRINTF("BLE_ANDROID_ANCS_EVT_INDICATION_CONFIRMED\r\n");
			break;
		default:break;
	}
}


/*****************************************************************************
 * �� �� ��  : app_android_send_calling_action
 * ��������  : ʵ�ְ�׿���������ҶϹ���
 * �������  : ble_wechat_t * p_wechat  ��������ָ��
               uint8_t action           ��������:1Ϊ������0Ϊ�Ҷ�
 * �������  : ��
 * �� �� ֵ  : ��

*****************************************************************************/
static void app_android_send_calling_action(uint8_t action)
{
    uint8_t send_buf[20]= {0},i;// 20һ֡
    send_buf[0] = 1;          // ֡���
    send_buf[1] = 0x05;       // ���ݳ���
    send_buf[2] = 0xA0;       // ����ID
    send_buf[3] = 0x01;       // ��Ϣ���ͣ�1��������Ϣ
    send_buf[4] = action;     // �������ͣ�1�ǽ����������ǹҶ�
    send_buf[5] = 0;          // �����ۼӺ͸�λ
    send_buf[6] = 0;          // �����ۼӺ͵�λ
    for(i=2; i<5; i++)// �����ۼӺ�
        send_buf[6] += send_buf[i];
    g_action_type = action;
    android_ancs_send_data(send_buf,7,ANDROID_ANCS_INDICATE_CHANNEL,0);
}

static uint32_t app_android_ancs_send_request(uint8_t msg_type,uint8_t msg_id)
{
	uint8_t index = 0,i=0;
    uint8_t ucTxData[20] = {0};
    uint16_t ucCheckSumCal = 0;
	uint32_t err_code;
	
    switch (msg_type)
    {
    	case MESSAGE_REMAIND:
		case WECHAT_REMAIND:
			ucTxData[index++] = 1;
			ucTxData[index++] = 0;                            //�ظ�����ĳ���
            ucTxData[index++] = 0xA1;                         //���ؽ��յ�������ID
            ucTxData[index++] = BLE_ANCS_CATEGORY_ID_SOCIAL;  //���ؽ��յ�������TYPE
            for (uint8_t i = 0; i < 4; i++)                   //֡���
            {
                ucTxData[index++] = ((msg_id)>> ((3 - i) * 8)) & 0xff;
            } 

			ucTxData[index++] = 0x03;//��Ϣ������
			ucTxData[index++] = (MESSAGE_DATA_SIZE>>8)&0xFF;//length
			ucTxData[index++] = (MESSAGE_DATA_SIZE&0xFF);

			ucTxData[1] = index;

			ucCheckSumCal = 0;
            for (i = 0; i < (ucTxData[1] - 2); i++)   //���㷵�����ݵ�Checksum������䵽����
            {
                ucCheckSumCal += ucTxData[i + 2];
            }
            ucTxData[index++] = (ucCheckSumCal >> 8);
            ucTxData[index++] = ucCheckSumCal;
			break;
			
		default:break;
    }

	err_code = android_ancs_send_data(ucTxData,index,ANDROID_ANCS_INDICATE_CHANNEL,0);

	return err_code;
}

/*****************************************************************************
 * �� �� ��  : app_android_ancs_send_reponse
 * ��������  : �ظ�APP��Ӧ��������Ϣ����״̬
               ״̬˵����0�����������1���ɹ� 2���������ѹ� 3���·��������쳣��4 :����
 * �������  : uint8_t RP_Type     ��������
               uint8_t ucFrameNum  ���֡���
               uint8_t CmdID       ����ID
               uint8_t CmdType     ��������
               uint32_t ucMsgID    ��ϢID
               uint8_t Status      ����״̬
 * �������  : ��
 * �� �� ֵ  : ��

*****************************************************************************/
static void app_android_ancs_send_reponse(uint8_t RP_Type,
                        uint8_t ucFrameNum,
                        uint8_t CmdID,
                        uint8_t CmdType,
                        uint32_t ucMsgID,
                        uint8_t Status)
{
    uint8_t index = 0,i=0;
    uint8_t ucTxData[20] = {0};
    uint16_t ucCheckSumCal = 0;

    switch (RP_Type)
    {
        case CALL_REMAIND:
            ucTxData[index++] = ucFrameNum;
            ucTxData[index++] = 0;                            //�ظ�����ĳ���
            ucTxData[index++] = CmdID;                        //���ؽ��յ�������ID
            ucTxData[index++] = CmdType;                      //���ؽ��յ�������TYPE
            ucTxData[index++] = Status;                       //���ؽ��ս��
            ucTxData[1] = index;                              //�ظ�����ĳ���Ϊ5���ֽ�
            ucCheckSumCal = 0;
            for (uint8_t i = 0; i < (ucTxData[1] - 2); i++)   //���㷵�����ݵ�Checksum������䵽����
            {
                ucCheckSumCal += ucTxData[i + 2];
            }
            ucTxData[index++] = (ucCheckSumCal >> 8);
            ucTxData[index++] = ucCheckSumCal;
            break;
        case MESSAGE_REMAIND:
        case WECHAT_REMAIND:
            ucTxData[index++] = ucFrameNum;
            ucTxData[index++] = 0;                            //�ظ�����ĳ���
            ucTxData[index++] = CmdID;                        //���ؽ��յ�������ID
            ucTxData[index++] = CmdType;                      //���ؽ��յ�������TYPE
            for (i = 0; i < 4; i++)                   //֡���
            {
                ucTxData[index++] = (ucMsgID >> ((3 - i) * 8)) & 0xff;
            }
            ucTxData[index++] = Status;                       //����״̬
            ucTxData[1] = index;                              //�ظ�����ĳ���
            ucCheckSumCal = 0;
            for (i = 0; i < (ucTxData[1] - 2); i++)   //���㷵�����ݵ�Checksum������䵽����
            {
                ucCheckSumCal += ucTxData[i + 2];
            }
            ucTxData[index++] = (ucCheckSumCal >> 8);
            ucTxData[index++] = ucCheckSumCal;
            break;
    }

    android_ancs_send_data(ucTxData,index,ANDROID_ANCS_INDICATE_CHANNEL,0);
}

/***************************************************************************************************************
 * �� �� ��  	: app_android_ancs_receive_data
 * ��������  : �Խ��յ���������Ϣ����������������ʾ���񶯲���
 * �������  : pRxData--���յ������������Ϣ
 * �������  : ��
 * �� �� ֵ  	: 1----������Ϣ��ȷ���մ���
 *				  2----������ʾ���ܽ�ֹ
 *				  3----���յ�����Ϣ����������Ϣ������������Ϣ��ȫ���޷�����
 *				  4----���յ���������ϢΪ�գ����ǲ���mainģʽ(���ڳ��)��
***************************************************************************************************************/
void app_android_ancs_receive_data(uint8_t *data, uint16_t length,uint8_t ucFrameNum)
{
	uint8_t  ret = 1;
	uint8_t  ucCmdID = 0;
	uint8_t  ucCmdType = 0;
	uint8_t  *pContent = NULL;
	uint16_t ucRchecksum = 0;
	uint16_t ucDchecksum = 0;
	uint32_t time;
	uint16_t i;
	uint32_t ucMsgID = 0;
	static uint32_t preTime = 0;
	remainder_st *rSt = get_remainder_info();

	ucCmdID 	= *(data);														  //����ID
	ucCmdType 	= *(data + 1); 													  //��������
	pContent 	= data + 2;														  //����Ϊ��������

	for (i = 0; i < length - 2; i++) 										  //checksum����
	{
		ucDchecksum += *(data + i);
	}
	ucRchecksum = *(data + length - 2);
	ucRchecksum <<= 8;
	ucRchecksum += *(data + length - 1);												 //��������checksum

	time = system_sec_get();
	
	if (ucCmdID == 0xC0 && ucCmdType == BLE_ANCS_CATEGORY_ID_INCOMING_CALL) 		  //�Ҷϵ绰״̬�ظ�
	{
		if (*pContent == 0x00 && g_retry_count < 3)									  //�жϽ��յ��������Ƿ���ȷ������ȷ��Ҫ�ط�
		{
			g_retry_count++;
			app_android_send_calling_action(g_action_type);								  //�ط��Ҷ�ָ��
		}
	}
	else if (ucCmdID == BLE_ANCS_CATEGORY_ID_INCOMING_CALL) 						  //�ж��Ƿ�Ϊ����֪ͨ
	{
		length -= 4; 																  //�������ݵ���ʵ�����(ȥ����ʾ����+��Ϣ����+У���빲4���ֽ�)
		if (ucCmdType == BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED)						  //�ж��Ƿ�����������֪ͨ
		{
			set_remainder_info(ANDROID_TYPE,CALL_REMAIND,time,pContent, length,pContent, length);
		}
		else if (ucCmdType == BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED)				  //����Ҷ��¼�����
		{
			//Android_remaind_stop_process(CALL_REMAIND);
			QPRINTF("Call is miss\r\n");
		}

		app_android_ancs_send_reponse(CALL_REMAIND, ucFrameNum, ucCmdID, ucCmdType, 0, ret);//�ظ�APP���ѽ������
	}
	else if (ucCmdID == BLE_ANCS_CATEGORY_ID_SOCIAL)								  //�ж��Ƿ�Ϊ�罻֪ͨ
	{
		uint8_t  ucRPType = 0;														  //��������
		uint16_t ucMsgLen = 0;
		uint16_t ucAPPID = 0;														  //Ӧ��ID
		
		ucAPPID = *pContent++;
		ucAPPID <<= 8;
		ucAPPID += *pContent++; 													  //Ӧ��ID��2bytes
		++pContent;
		
		for (i = 0; i < 4; i++) 											  //��ϢID��4bytes
		{
			ucMsgID <<= 8;
			ucMsgID |= *pContent++;
		}
		ucMsgLen = *pContent++;
		ucMsgLen <<= 8;
		ucMsgLen += *pContent++;													  //��Ϣ���ȣ�2bytes

		if(time - preTime > 2)
		{
			rSt->message_count = 0;
			rSt->wechat_count = 0;
		}
		preTime = time;
		
		if (ucCmdType == BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED)						  //�ж��Ƿ���������Ϣ֪ͨ
		{
			switch (ucAPPID)
			{
				case BLE_ANCS_APP_ID_MSG:
					rSt->message_count++;
					ucRPType = MESSAGE_REMAIND; 											   //��������
					break;
				case BLE_ANCS_APP_ID_WECHAT:
					rSt->wechat_count++;
					ucRPType = WECHAT_REMAIND;												   //΢������
					break;
				default:
					ucRPType = 0xFF;
					break;
			}
			
			if(ucRPType == MESSAGE_REMAIND || ucRPType == WECHAT_REMAIND)
			{	
				g_reminder_head.phone_type 	= ANDROID_TYPE;
				g_reminder_head.msg_id 		= ucMsgID;
				g_reminder_head.remaind_type= ucRPType;
				g_reminder_head.time		= time;

				if(ucMsgLen >= TITLE_DATA_SIZE)
					ucMsgLen = TITLE_DATA_SIZE-1;
	
				g_reminder_head.title_length = ucMsgLen;
				for(i=0;i<g_reminder_head.title_length;i++)
					g_reminder_head.title_data[i] = pContent[i];
				g_reminder_head.title_data[i] = '\0';
				
				app_android_ancs_send_request(ucRPType,ucMsgID);
			}
		}
		else if (ucCmdType == BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED)				  //��Ϣ��ɾ���¼�����
		{
			//Android_remaind_stop_process(ucRPType); 									 //�ر���Ϣ������ʾ
		}

		app_android_ancs_send_reponse(ucRPType, ucFrameNum, ucCmdID, ucCmdType, ucMsgID, ret);//�ظ�APP���ѽ������	
	}
	else if (ucCmdID == 0xC1)//��ȡ֪ͨ����Ϣ����Ļظ�
	{
		uint8_t total_pack = 0;
		uint8_t current_pack = 0;
		uint16_t ucMsgLen = 0;
		uint8_t mssage_type = 0,data_length=0;
		
		for (uint8_t i = 0; i < 4; i++) 											  //��ϢID��4bytes
		{
			ucMsgID <<= 8;
			ucMsgID |= *pContent++;
		}

		total_pack		= *pContent++;
		current_pack	= *pContent++;
		mssage_type 	= *pContent++;	
		ucMsgLen		= *pContent++;
		ucMsgLen		<<= 8;
		ucMsgLen		+= *pContent++;   
		
		if(mssage_type == 0x03)//get message data
		{	
			if(g_reminder_head.msg_id == ucMsgID)
			{				
				data_length = length - 14;//14 ��ȥǰ���heap�ͺ�����checkSun				
						
				if(current_pack == total_pack)
				{
					set_remainder_info(g_reminder_head.phone_type,
						g_reminder_head.remaind_type,
						g_reminder_head.time,						
						g_reminder_head.title_data,
						g_reminder_head.title_length,
						pContent,
						data_length);
				}		
			}	
		}
	}
}



