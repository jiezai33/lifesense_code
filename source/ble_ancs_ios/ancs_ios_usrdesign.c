#include "ancs_ios_usrdesign.h"
#include "ancs_android_usrdesign.h"
#include "ble_ancs_ios.h"
#include "usr_reminder.h"
#include "app_error.h"
#include "time.h"
#include "debug.h"

#define WECHAT_MSG              	"com.tencent.xin"
#define MOBILE_SMS              	"com.apple.MobileSMS"
#define MOBILE_PHONE            	"com.apple.mobilephone"
#define QQ_MSG						"com.tencent.mqq"

#define WECHAT_DIFF  ":"
char wechat_chn[] = {0xE5,0xBE,0xAE,0xE4,0xBF,0xA1}; //??
char wechat_eng[] = {0x57,0x65,0x43,0x68,0x61,0x74}; //WeChat
const uint8_t specified_wx_tital[] = {0xE6, 0x82, 0xA8, 0xe6, 0x9c, 0x89, 0xe6, 0x96, 0xb0, 0xe4, 0xbf, 0xa1, 0xe6, 0x81, 0xaf};
#if DATA_TYPE == DATA_POINTER_TYPE
extern uint8_t g_ancs_common_rx_buffer[ANCS_RX_SIZE];
#endif

extern ble_ancs_c_t              		m_ios_ancs;                                
static remind_info_analysis_st	usr_remind;

static void evt_notif_attribute(ble_ancs_c_evt_notif_attr_t *p_attr)
{
    uint16_t attr_id;
	attr_id = p_attr->attr_id;
	
	if(attr_id == BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER
		|| attr_id == BLE_ANCS_NOTIF_ATTR_ID_TITLE
		|| attr_id == BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE
		|| attr_id == BLE_ANCS_NOTIF_ATTR_ID_MESSAGE)
	{
		if(attr_id == BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER)
		{
			memset(&usr_remind, 0, sizeof(usr_remind));
			usr_remind.time = system_sec_get();
			#if DATA_TYPE == DATA_POINTER_TYPE
			memset(&g_ancs_common_rx_buffer[ANCS_RX_START_ADDR], 0, ANCS_RX_START_ADDR);
			#endif
		}
		
		if(p_attr->attr_len >= (ANCS_DATA_SIZE-usr_remind.use_index))
		{
			p_attr->attr_len = (ANCS_DATA_SIZE-usr_remind.use_index) - 1;
		}

		usr_remind.id[attr_id].addr = usr_remind.use_index;
		usr_remind.id[attr_id].size = p_attr->attr_len;
		
		#if DATA_TYPE == DATA_BUFFER_TYPE
		memcpy(&(usr_remind.data[usr_remind.id[attr_id].addr]), p_attr->p_attr_data, p_attr->attr_len);
		usr_remind.data[usr_remind.id[attr_id].addr + usr_remind.id[attr_id].size] = '\0';
		#elif DATA_TYPE == DATA_POINTER_TYPE
		memcpy(&(g_ancs_common_rx_buffer[ANCS_RX_START_ADDR+usr_remind.id[attr_id].addr]), p_attr->p_attr_data, p_attr->attr_len);
		g_ancs_common_rx_buffer[ANCS_RX_START_ADDR+usr_remind.id[attr_id].addr + usr_remind.id[attr_id].size] = '\0';

		usr_remind.data = (char*)(&g_ancs_common_rx_buffer[IOS_ANCS_RECEIVE_DATA_START_ADDR]);
		#endif
		
		usr_remind.use_index += (usr_remind.id[attr_id].size+1);
	}
}


void parse_notif(const uint8_t * p_data_src,const uint16_t hvx_data_len)
{
    ble_ancs_c_evt_t ancs_evt;
    uint32_t         err_code;

    if (hvx_data_len != BLE_ANCS_NOTIFICATION_DATA_LENGTH)
        return;

    /*lint --e{415} --e{416} -save suppress Warning 415: possible access out of bond */
    ancs_evt.notif.evt_id                    =
            (ble_ancs_c_evt_id_values_t) p_data_src[BLE_ANCS_NOTIF_EVT_ID_INDEX];

    ancs_evt.notif.evt_flags.silent          =
            (p_data_src[BLE_ANCS_NOTIF_FLAGS_INDEX] >> BLE_ANCS_EVENT_FLAG_SILENT) & 0x01;

    ancs_evt.notif.evt_flags.important       =
            (p_data_src[BLE_ANCS_NOTIF_FLAGS_INDEX] >> BLE_ANCS_EVENT_FLAG_IMPORTANT) & 0x01;

    ancs_evt.notif.evt_flags.pre_existing    =
            (p_data_src[BLE_ANCS_NOTIF_FLAGS_INDEX] >> BLE_ANCS_EVENT_FLAG_PREEXISTING) & 0x01;

    ancs_evt.notif.evt_flags.positive_action =
            (p_data_src[BLE_ANCS_NOTIF_FLAGS_INDEX] >> BLE_ANCS_EVENT_FLAG_POSITIVE_ACTION) & 0x01;

    ancs_evt.notif.evt_flags.negative_action =
            (p_data_src[BLE_ANCS_NOTIF_FLAGS_INDEX] >> BLE_ANCS_EVENT_FLAG_NEGATIVE_ACTION) & 0x01;

    ancs_evt.notif.category_id               =
        (ble_ancs_c_category_id_values_t) p_data_src[BLE_ANCS_NOTIF_CATEGORY_ID_INDEX];

    ancs_evt.notif.category_count            = p_data_src[BLE_ANCS_NOTIF_CATEGORY_CNT_INDEX];
    ancs_evt.notif.notif_uid = uint32_decode(&p_data_src[BLE_ANCS_NOTIF_NOTIF_UID]);
			
   	// 2016-03-06 陈长升 判断通知类型是否来电通知
	if(ancs_evt.notif.category_id == BLE_ANCS_CATEGORY_ID_INCOMING_CALL)
	{
		// 2016-03-06 陈长升 判断通知是否增加的通知
		if(ancs_evt.notif.evt_id == BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED)
		{
			//在mainmode下才通知来电
			for(uint8_t i = 0; i < BLE_ANCS_NB_OF_ATTRS; i ++)
			{
					m_ios_ancs.ancs_attr_list[i].get = false;
			}
			m_ios_ancs.ancs_attr_list[BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER].get = true;
			m_ios_ancs.ancs_attr_list[BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER].attr_id = BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER;
			m_ios_ancs.ancs_attr_list[BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER].attr_len = ANCS_DATA_SIZE;  //此处不能用其他值，会影响到attr_data_parse（）解析
			m_ios_ancs.ancs_attr_list[BLE_ANCS_NOTIF_ATTR_ID_TITLE].get = true;
			m_ios_ancs.ancs_attr_list[BLE_ANCS_NOTIF_ATTR_ID_TITLE].attr_id = BLE_ANCS_NOTIF_ATTR_ID_TITLE;
			m_ios_ancs.ancs_attr_list[BLE_ANCS_NOTIF_ATTR_ID_TITLE].attr_len = ANCS_DATA_SIZE;
			err_code = ble_ancs_c_request_attrs(&m_ios_ancs, &ancs_evt.notif);

			APP_ERROR_CHECK(err_code);
			//此处要确认下uid的大小端是否正确
			//memcpy(ucAncsCurrentUid, (uint8_t *)&ancs_evt.notif.notif_uid, 4);// 2016-03-06 陈长升 这里4使用sizeof会好一些
		}
		// 2016-03-06 陈长升 判断通知是否删除的通知
		else if(ancs_evt.notif.evt_id == BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED)
		{
			//关闭马达振动，但不关显示，2015032503
		}
	}
	// 2016-03-06 陈长升 判断通知是否社交信息，如微信、短信 
	else if(ancs_evt.notif.category_id != BLE_ANCS_CATEGORY_ID_MISSED_CALL)
	{
		// 2016-03-06 陈长升 判断是否新增加的通知
		if(ancs_evt.notif.evt_id == BLE_ANCS_EVENT_ID_NOTIFICATION_ADDED)
		{
			// 过时的消息不再获取
			if(ancs_evt.notif.evt_flags.pre_existing)
				return;

			for(uint8_t i = 0; i < BLE_ANCS_NB_OF_ATTRS; i ++)
			{
				m_ios_ancs.ancs_attr_list[i].get = false;
			}
			m_ios_ancs.ancs_attr_list[BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER].get = true;
			m_ios_ancs.ancs_attr_list[BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER].attr_id = BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER;
			m_ios_ancs.ancs_attr_list[BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER].attr_len = ANCS_DATA_SIZE;//此处不能用其他值，会影响到attr_data_parse（）解析
			m_ios_ancs.ancs_attr_list[BLE_ANCS_NOTIF_ATTR_ID_TITLE].get = true;
			m_ios_ancs.ancs_attr_list[BLE_ANCS_NOTIF_ATTR_ID_TITLE].attr_id = BLE_ANCS_NOTIF_ATTR_ID_TITLE;
			m_ios_ancs.ancs_attr_list[BLE_ANCS_NOTIF_ATTR_ID_TITLE].attr_len = ANCS_DATA_SIZE;
			m_ios_ancs.ancs_attr_list[BLE_ANCS_NOTIF_ATTR_ID_MESSAGE].get = true;
			m_ios_ancs.ancs_attr_list[BLE_ANCS_NOTIF_ATTR_ID_MESSAGE].attr_id = BLE_ANCS_NOTIF_ATTR_ID_MESSAGE;
			m_ios_ancs.ancs_attr_list[BLE_ANCS_NOTIF_ATTR_ID_MESSAGE].attr_len = ANCS_DATA_SIZE;
			err_code = ble_ancs_c_request_attrs(&m_ios_ancs, &ancs_evt.notif);

			APP_ERROR_CHECK(err_code);
		}
		// 2016-03-06 陈长升 判断通知是否删除的通知
		else if(ancs_evt.notif.evt_id == BLE_ANCS_EVENT_ID_NOTIFICATION_REMOVED)
		{

		}
	}
}


void parse_get_notif_attrs_response(const uint8_t * p_data_src,const uint16_t  hvx_data_len)
{

	static uint8_t *ptr;
    static uint16_t current_len;
	static ble_ancs_c_evt_t event;
    static uint16_t buff_idx;
	static uint8_t parse_state = COMMAND_ID;
	
    int i;
	uint32_t time;
	uint8_t remaind_type = REMAIND_MAX;
	static uint32_t preTime;

	uint8_t title_data[TITLE_DATA_SIZE],title_length;
	uint8_t *message_data,message_length;
	uint16_t heap_len,messs_len;
	char *str = NULL;
	remainder_st *rSt = get_remainder_info();

	//一条完整的通知包括1byte command ID + 4bytes UID + 1 byte attribute ID + 2 byte attribute Len + attribute (attribute Len bytes)
    for(i = 0; i < hvx_data_len; i++)
    {
        switch(parse_state)
        {
            case COMMAND_ID:
                event.attr.command_id = p_data_src[i];
                // get notification时，byte[0]为BLE_ANCS_COMMAND_ID_GET_NOTIFICATION_ATTRIBUTES
                if(event.attr.command_id == BLE_ANCS_COMMAND_ID_GET_NOTIF_ATTRIBUTES)
                {
                    parse_state = NOTIFICATION_UID1;
                }
                break;

            case NOTIFICATION_UID1:
                event.attr.notif_uid = p_data_src[i];
                parse_state = NOTIFICATION_UID2;
                break;

            case NOTIFICATION_UID2:
                event.attr.notif_uid |= (p_data_src[i]<<8);
                parse_state = NOTIFICATION_UID3;
                break;

            case NOTIFICATION_UID3:
                event.attr.notif_uid |= (p_data_src[i]<<16);
                parse_state = NOTIFICATION_UID4;
                break;

            case NOTIFICATION_UID4:
                event.attr.notif_uid |= (p_data_src[i]<<24);
                parse_state = ATTRIBUTE_ID;
                break;

            case ATTRIBUTE_ID:
                event.attr.attr_id = (ble_ancs_c_notif_attr_id_values_t)p_data_src[i];
                parse_state = ATTRIBUTE_LEN1;
                break;

            case ATTRIBUTE_LEN1:
                event.attr.attr_len = p_data_src[i];
                parse_state = ATTRIBUTE_LEN2;
                break;

            case ATTRIBUTE_LEN2:
                event.attr.attr_len |= (p_data_src[i] << 8);
                // 如果长度大于此值为错误的数据
                if(event.attr.attr_len > ANCS_DATA_SIZE)
                {
                    parse_state = COMMAND_ID;
                    break;
                }
                
                parse_state = ATTRIBUTE_READY;
                ptr = event.attr.p_attr_data;
                buff_idx = 0;
                current_len = 0;
                break;

            case ATTRIBUTE_READY:	
				if(event.attr.attr_len == 0)
				{
					event.attr.attr_len = 0;
					evt_notif_attribute(&(event.attr));
					
					event.attr.attr_id = (ble_ancs_c_notif_attr_id_values_t)p_data_src[i];
					parse_state = ATTRIBUTE_LEN1;
				}
                if(buff_idx < (ANCS_DATA_SIZE - 1))  // 留一个放结束符
                {
                    ptr[buff_idx++] = p_data_src[i];
                }				
                current_len++;
				
				//检测信息是否全部接收完成			
                if(current_len == event.attr.attr_len)
                {
                    if(buff_idx >= ANCS_DATA_SIZE)
                        buff_idx = (ANCS_DATA_SIZE-1);
                    ptr[buff_idx] = '\0';

                    event.attr.attr_len = buff_idx;
                    event.evt_type = BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE;
                    evt_notif_attribute(&(event.attr));

					time = system_sec_get(); 
                    if(event.attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER)
                    {
						parse_state = ATTRIBUTE_ID;
                    }
                    else if(event.attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_TITLE)
                    {
						if(memcmp(&(usr_remind.data[usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER].addr]), MOBILE_PHONE, (sizeof(MOBILE_PHONE)-1)))	// 电话只获取2个字段
					    {
					    	parse_state = ATTRIBUTE_ID;
							break;
					    }

                        parse_state = COMMAND_ID;
                		set_remainder_info(IOS_TYPE,CALL_REMAIND,time,
							(uint8_t*)&(usr_remind.data[usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_TITLE].addr]),
							usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_TITLE].size,
							(uint8_t*)&(usr_remind.data[usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_MESSAGE].addr]),
							usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_MESSAGE].size);
                    }
                    else if(event.attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_MESSAGE) // 信息获取3个字段
                    {
                        parse_state = COMMAND_ID;

						if(memcmp(&(usr_remind.data[usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER].addr]),MOBILE_SMS,sizeof(MOBILE_SMS))==0)
							remaind_type = MESSAGE_REMAIND;
					    else if(memcmp(&(usr_remind.data[usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER].addr]),WECHAT_MSG, (sizeof(WECHAT_MSG)))==0)
					    	remaind_type = WECHAT_REMAIND;
						else if(memcmp(&(usr_remind.data[usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER].addr]),QQ_MSG, (sizeof(QQ_MSG)))==0)
							remaind_type = QQ_REMAIND;
						else
							remaind_type = REMAIND_MAX;

						if(remaind_type == MESSAGE_REMAIND || remaind_type == WECHAT_REMAIND)
						{
							if(time - preTime > 2)
							{							
								rSt->message_count = 0;
								rSt->wechat_count = 0;
							}
							preTime = time;

							if(remaind_type == MESSAGE_REMAIND)
								rSt->message_count++;
							else if(remaind_type == WECHAT_REMAIND)
								rSt->wechat_count++;
						
							title_length = usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_TITLE].size;
							if(title_length > TITLE_DATA_SIZE)
								title_length = TITLE_DATA_SIZE;
							memcpy(title_data, &(usr_remind.data[usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_TITLE].addr]), title_length);
							
							message_data = (uint8_t *)&(usr_remind.data[usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_MESSAGE].addr]);
							message_length = usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_MESSAGE].size;
							if(message_length > MESSAGE_DATA_SIZE)
								message_length = MESSAGE_DATA_SIZE;

							if(remaind_type == WECHAT_REMAIND)
							{
								str = strstr((char*)message_data, WECHAT_DIFF);
								if(memcmp(title_data, wechat_chn, sizeof(wechat_chn)) == 0
									|| memcmp(title_data, wechat_eng, sizeof(wechat_eng)) == 0)
						        {
									if(str == NULL)
									{
										heap_len = sizeof(specified_wx_tital);
										memcpy(title_data, specified_wx_tital, heap_len);
										title_length = heap_len;
									}
									else
									{
										heap_len = str - (char*)message_data;
										if(heap_len >= TITLE_DATA_SIZE)
											heap_len = (TITLE_DATA_SIZE - 1);
										memcpy(title_data,message_data , heap_len);
										title_data[heap_len] = '\0';
										title_length = heap_len;
									}
						        }

								if(str != NULL)
								{
									heap_len = (str - (char*)message_data)+1;
									messs_len = message_length - heap_len;
									for(i=0;i<messs_len;i++)
									{
										message_data[i] = message_data[i+heap_len];
									}
									message_data[i] = '\0';
									message_length = messs_len;
								}
							}
							
							set_remainder_info(IOS_TYPE,remaind_type,time,
								title_data,
								title_length,
								message_data,
								message_length);
						}		
                    }
                }
                break;
        }
    }
}

