#include "app_android_ancs.h"
#include "ancs_android_usrdesign.h"
#include "usr_reminder.h"
#include "time.h"
#include "debug.h"

static uint8_t g_retry_count = 0;     //挂断重试次数
static uint8_t g_action_type = 0;     //备份接听还是挂断

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
 * 函 数 名  : app_android_send_calling_action
 * 函数功能  : 实现安卓来电接听或挂断功能
 * 输入参数  : ble_wechat_t * p_wechat  蓝牙服务指针
               uint8_t action           操作类型:1为接听，0为挂断
 * 输出参数  : 无
 * 返 回 值  : 无

*****************************************************************************/
static void app_android_send_calling_action(uint8_t action)
{
    uint8_t send_buf[20]= {0},i;// 20一帧
    send_buf[0] = 1;          // 帧序号
    send_buf[1] = 0x05;       // 数据长度
    send_buf[2] = 0xA0;       // 命令ID
    send_buf[3] = 0x01;       // 信息类型：1是来电信息
    send_buf[4] = action;     // 操作类型：1是接听，其他是挂断
    send_buf[5] = 0;          // 数据累加和高位
    send_buf[6] = 0;          // 数据累加和低位
    for(i=2; i<5; i++)// 更新累加和
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
			ucTxData[index++] = 0;                            //回复命令的长度
            ucTxData[index++] = 0xA1;                         //返回接收到的命令ID
            ucTxData[index++] = BLE_ANCS_CATEGORY_ID_SOCIAL;  //返回接收到的命令TYPE
            for (uint8_t i = 0; i < 4; i++)                   //帧序号
            {
                ucTxData[index++] = ((msg_id)>> ((3 - i) * 8)) & 0xff;
            } 

			ucTxData[index++] = 0x03;//消息的正文
			ucTxData[index++] = (MESSAGE_DATA_SIZE>>8)&0xFF;//length
			ucTxData[index++] = (MESSAGE_DATA_SIZE&0xFF);

			ucTxData[1] = index;

			ucCheckSumCal = 0;
            for (i = 0; i < (ucTxData[1] - 2); i++)   //计算返回数据的Checksum，并填充到数组
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
 * 函 数 名  : app_android_ancs_send_reponse
 * 函数功能  : 回复APP相应的提醒消息接收状态
               状态说明，0：检验码错误；1：成功 2：来电提醒关 3：下发的命令异常；4 :其他
 * 输入参数  : uint8_t RP_Type     提醒类型
               uint8_t ucFrameNum  最后帧序号
               uint8_t CmdID       命令ID
               uint8_t CmdType     命令类型
               uint32_t ucMsgID    消息ID
               uint8_t Status      返回状态
 * 输出参数  : 无
 * 返 回 值  : 无

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
            ucTxData[index++] = 0;                            //回复命令的长度
            ucTxData[index++] = CmdID;                        //返回接收到的命令ID
            ucTxData[index++] = CmdType;                      //返回接收到的命令TYPE
            ucTxData[index++] = Status;                       //返回接收结果
            ucTxData[1] = index;                              //回复命令的长度为5个字节
            ucCheckSumCal = 0;
            for (uint8_t i = 0; i < (ucTxData[1] - 2); i++)   //计算返回数据的Checksum，并填充到数组
            {
                ucCheckSumCal += ucTxData[i + 2];
            }
            ucTxData[index++] = (ucCheckSumCal >> 8);
            ucTxData[index++] = ucCheckSumCal;
            break;
        case MESSAGE_REMAIND:
        case WECHAT_REMAIND:
            ucTxData[index++] = ucFrameNum;
            ucTxData[index++] = 0;                            //回复命令的长度
            ucTxData[index++] = CmdID;                        //返回接收到的命令ID
            ucTxData[index++] = CmdType;                      //返回接收到的命令TYPE
            for (i = 0; i < 4; i++)                   //帧序号
            {
                ucTxData[index++] = (ucMsgID >> ((3 - i) * 8)) & 0xff;
            }
            ucTxData[index++] = Status;                       //接收状态
            ucTxData[1] = index;                              //回复命令的长度
            ucCheckSumCal = 0;
            for (i = 0; i < (ucTxData[1] - 2); i++)   //计算返回数据的Checksum，并填充到数组
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
 * 函 数 名  	: app_android_ancs_receive_data
 * 函数功能  : 对接收到的来电信息分析处理，并触发显示或振动操作
 * 输入参数  : pRxData--接收到的来电相关信息
 * 输出参数  : 无
 * 返 回 值  	: 1----来电信息正确接收处理
 *				  2----来电提示功能禁止
 *				  3----接收到的信息不是来电信息，或是来电信息不全，无法解析
 *				  4----接收到的来电信息为空，或是不在main模式(不在充电)下
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

	ucCmdID 	= *(data);														  //命令ID
	ucCmdType 	= *(data + 1); 													  //命令类型
	pContent 	= data + 2;														  //以下为命令内容

	for (i = 0; i < length - 2; i++) 										  //checksum计算
	{
		ucDchecksum += *(data + i);
	}
	ucRchecksum = *(data + length - 2);
	ucRchecksum <<= 8;
	ucRchecksum += *(data + length - 1);												 //解析数据checksum

	time = system_sec_get();
	
	if (ucCmdID == 0xC0 && ucCmdType == BLE_ANCS_CATEGORY_ID_INCOMING_CALL) 		  //挂断电话状态回复
	{
		if (*pContent == 0x00 && g_retry_count < 3)									  //判断接收到的数据是否正确，不正确需要重发
		{
			g_retry_count++;
			app_android_send_calling_action(g_action_type);								  //重发挂断指令
		}
	}
	else if (ucCmdID == BLE_ANCS_CATEGORY_ID_INCOMING_CALL) 						  //判断是否为来电通知
	{
		length -= 4; 																  //命令内容的真实长度澹(去除提示类型+信息类型+校验码共4个字节)
		if (ucCmdType == BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED)						  //判断是否是新增来电通知
		{
			set_remainder_info(ANDROID_TYPE,CALL_REMAIND,time,pContent, length,pContent, length);
		}
		else if (ucCmdType == BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED)				  //来电挂断事件处理
		{
			//Android_remaind_stop_process(CALL_REMAIND);
			QPRINTF("Call is miss\r\n");
		}

		app_android_ancs_send_reponse(CALL_REMAIND, ucFrameNum, ucCmdID, ucCmdType, 0, ret);//回复APP提醒解析结果
	}
	else if (ucCmdID == BLE_ANCS_CATEGORY_ID_SOCIAL)								  //判断是否为社交通知
	{
		uint8_t  ucRPType = 0;														  //提醒类型
		uint16_t ucMsgLen = 0;
		uint16_t ucAPPID = 0;														  //应用ID
		
		ucAPPID = *pContent++;
		ucAPPID <<= 8;
		ucAPPID += *pContent++; 													  //应用ID，2bytes
		++pContent;
		
		for (i = 0; i < 4; i++) 											  //信息ID，4bytes
		{
			ucMsgID <<= 8;
			ucMsgID |= *pContent++;
		}
		ucMsgLen = *pContent++;
		ucMsgLen <<= 8;
		ucMsgLen += *pContent++;													  //信息长度，2bytes

		if(time - preTime > 2)
		{
			rSt->message_count = 0;
			rSt->wechat_count = 0;
		}
		preTime = time;
		
		if (ucCmdType == BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED)						  //判断是否是新增信息通知
		{
			switch (ucAPPID)
			{
				case BLE_ANCS_APP_ID_MSG:
					rSt->message_count++;
					ucRPType = MESSAGE_REMAIND; 											   //短信提醒
					break;
				case BLE_ANCS_APP_ID_WECHAT:
					rSt->wechat_count++;
					ucRPType = WECHAT_REMAIND;												   //微信提醒
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
		else if (ucCmdType == BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED)				  //信息被删除事件处理
		{
			//Android_remaind_stop_process(ucRPType); 									 //关闭信息提醒显示
		}

		app_android_ancs_send_reponse(ucRPType, ucFrameNum, ucCmdID, ucCmdType, ucMsgID, ret);//回复APP提醒解析结果	
	}
	else if (ucCmdID == 0xC1)//获取通知的信息请求的回复
	{
		uint8_t total_pack = 0;
		uint8_t current_pack = 0;
		uint16_t ucMsgLen = 0;
		uint8_t mssage_type = 0,data_length=0;
		
		for (uint8_t i = 0; i < 4; i++) 											  //信息ID，4bytes
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
				data_length = length - 14;//14 减去前面的heap和后面的錭heckSun				
						
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



