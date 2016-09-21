/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 */
 
/* Disclaimer: This client implementation of the Apple Notification Center Service can and will be changed at any time by Nordic Semiconductor ASA.
 * Server implementations such as the ones found in iOS can be changed at any time by Apple and may cause this client implementation to stop working.
 */

#include "ble_ancs_ios.h"
#include "ble_err.h"
#include "ble_srv_common.h"
#include "nrf_assert.h"
#include "device_manager.h"
#include "ble_db_discovery.h"
#include "app_error.h"
#include "app_trace.h"
#include "sdk_common.h"
#include "usr_reminder.h"
#include "nrf_delay.h"
#include "debug.h"
#include "ancs_android_usrdesign.h"
#include "time.h"

#define WECHAT_MSG              	"com.tencent.xin"
#define MOBILE_SMS              	"com.apple.MobileSMS"
#define MOBILE_PHONE            	"com.apple.mobilephone"
#define QQ_MSG						"com.tencent.mqq"

#define WECHAT_DIFF  ":"
char wechat_chn[] = {0xE5,0xBE,0xAE,0xE4,0xBF,0xA1}; //??
char wechat_eng[] = {0x57,0x65,0x43,0x68,0x61,0x74}; //WeChat
const uint8_t specified_wx_tital[] = {0xE6, 0x82, 0xA8, 0xe6, 0x9c, 0x89, 0xe6, 0x96, 0xb0, 0xe4, 0xbf, 0xa1, 0xe6, 0x81, 0xaf};

static remind_info_analysis_st	usr_remind;
#if DATA_TYPE == DATA_POINTER_TYPE
extern uint8_t g_ancs_common_rx_buffer[ANCS_RX_SIZE];
#endif
extern ble_ancs_c_t              m_ios_ancs;

#define BLE_ANCS_NOTIF_EVT_ID_INDEX       0                       /**< Index of the Event ID field when parsing notifications. */
#define BLE_ANCS_NOTIF_FLAGS_INDEX        1                       /**< Index of the Flags field when parsing notifications. */
#define BLE_ANCS_NOTIF_CATEGORY_ID_INDEX  2                       /**< Index of the Category ID field when parsing notifications. */
#define BLE_ANCS_NOTIF_CATEGORY_CNT_INDEX 3                       /**< Index of the Category Count field when parsing notifications. */
#define BLE_ANCS_NOTIF_NOTIF_UID          4                       /**< Index of the Notification UID field when patsin notifications. */

#define ANCS_LOG                         NRF_LOG_PRINTF_DEBUG     /**< Debug logger macro that will be used in this file to do logging of important information over UART. */

#define START_HANDLE_DISCOVER            0x0001                   /**< Value of start handle during discovery. */

#define TX_BUFFER_MASK                   0x07                     /**< TX buffer mask. Must be a mask of contiguous zeroes followed by a contiguous sequence of ones: 000...111. */
#define TX_BUFFER_SIZE                   (TX_BUFFER_MASK + 1)     /**< Size of send buffer, which is 1 higher than the mask. */
#define WRITE_MESSAGE_LENGTH             20                       /**< Length of the write message for CCCD/control point. */
#define BLE_CCCD_NOTIFY_BIT_MASK         0x0001                   /**< Enable notification bit. */

#define BLE_ANCS_MAX_DISCOVERED_CENTRALS DEVICE_MANAGER_MAX_BONDS /**< Maximum number of discovered services that can be stored in the flash. This number should be identical to maximum number of bonded peer devices. */

#define TIME_STRING_LEN                  15                       /**< Unicode Technical Standard (UTS) #35 date format pattern "yyyyMMdd'T'HHmmSS" + "'\0'". */

#define DISCOVERED_SERVICE_DB_SIZE \
    CEIL_DIV(sizeof(ble_ancs_c_service_t) * BLE_ANCS_MAX_DISCOVERED_CENTRALS, sizeof(uint32_t)) /**< Size of bonded peer's database in word size (4 byte). */


/**@brief ANCS request types.
 */
typedef enum
{
    READ_REQ = 1,  /**< Type identifying that this tx_message is a read request. */
    WRITE_REQ      /**< Type identifying that this tx_message is a write request. */
} ancs_tx_request_t;


/**@brief Structure for writing a message to the central, i.e. Control Point or CCCD.
 */
typedef struct
{
    uint8_t                  gattc_value[WRITE_MESSAGE_LENGTH]; /**< The message to write. */
    ble_gattc_write_params_t gattc_params;                      /**< GATTC parameters for this message. */
} write_params_t;


/**@brief Structure for holding data to be transmitted to the connected master.
 */
typedef struct
{
    uint16_t          conn_handle;  /**< Connection handle to be used when transmitting this message. */
    ancs_tx_request_t type;         /**< Type of this message, i.e. read or write message. */
    union
    {
        uint16_t       read_handle; /**< Read request message. */
        write_params_t write_req;   /**< Write request message. */
    } req;
} tx_message_t;


static tx_message_t m_tx_buffer[TX_BUFFER_SIZE];                           /**< Transmit buffer for messages to be transmitted to the Notification Provider. */
static uint32_t     m_tx_insert_index = 0;                                 /**< Current index in the transmit buffer where the next message should be inserted. */
static uint32_t     m_tx_index        = 0;                                 /**< Current index in the transmit buffer from where the next message to be transmitted resides. */

ble_ancs_c_evt_notif_t m_notification_latest;                       /**< Local copy to keep track of the newest arriving notifications. */
static uint8_t             incoming_call_uid[4];

/**@brief 128-bit service UUID for the Apple Notification Center Service.
 */
const ble_uuid128_t ble_ancs_base_uuid128 =
{
    {
        // 7905F431-B5CE-4E99-A40F-4B1E122D00D0
        0xd0, 0x00, 0x2d, 0x12, 0x1e, 0x4b, 0x0f, 0xa4,
        0x99, 0x4e, 0xce, 0xb5, 0x31, 0xf4, 0x05, 0x79
    }
};


/**@brief 128-bit control point UUID.
 */
const ble_uuid128_t ble_ancs_cp_base_uuid128 =
{
    {
        // 69d1d8f3-45e1-49a8-9821-9BBDFDAAD9D9
        0xd9, 0xd9, 0xaa, 0xfd, 0xbd, 0x9b, 0x21, 0x98,
        0xa8, 0x49, 0xe1, 0x45, 0xf3, 0xd8, 0xd1, 0x69
    }
};

/**@brief 128-bit notification source UUID.
*/
const ble_uuid128_t ble_ancs_ns_base_uuid128 =
{
    {
        // 9FBF120D-6301-42D9-8C58-25E699A21DBD
        0xbd, 0x1d, 0xa2, 0x99, 0xe6, 0x25, 0x58, 0x8c,
        0xd9, 0x42, 0x01, 0x63, 0x0d, 0x12, 0xbf, 0x9f

    }
};

/**@brief 128-bit data source UUID.
*/
const ble_uuid128_t ble_ancs_ds_base_uuid128 =
{
    {
        // 22EAC6E9-24D6-4BB5-BE44-B36ACE7C7BFB
        0xfb, 0x7b, 0x7c, 0xce, 0x6a, 0xb3, 0x44, 0xbe,
        0xb5, 0x4b, 0xd6, 0x24, 0xe9, 0xc6, 0xea, 0x22
    }
};


/**@brief  Function for handling Disconnected event received from the SoftDevice.
 *
 * @details This function check if the disconnect event is happening on the link
 *          associated with the current instance of the module, if so it will set its
 *          conn_handle to invalid.
 *
 * @param[in] p_ancs    Pointer to the ANCS client structure.
 * @param[in] p_ble_evt Pointer to the BLE event received.
 */
static void on_disconnected(ble_ancs_c_t * p_ancs, const ble_evt_t * p_ble_evt)
{
    if (p_ancs->conn_handle == p_ble_evt->evt.gap_evt.conn_handle)
    {
        p_ancs->conn_handle = BLE_CONN_HANDLE_INVALID;
    }
}

static void on_connected(ble_ancs_c_t * p_ancs, const ble_evt_t * p_ble_evt)
{
    p_ancs->conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
}

void ble_ancs_c_on_db_disc_evt(ble_ancs_c_t * p_ancs, ble_db_discovery_evt_t * p_evt)
{
    QPRINTF("[ANCS]: Database Discovery handler called with event 0x%02x\r\n", p_evt->evt_type);

    ble_ancs_c_evt_t     evt;
    ble_gatt_db_char_t * p_chars;

    p_chars = p_evt->params.discovered_db.charateristics;

    // Check if the ANCS Service was discovered.
    if (p_evt->evt_type == BLE_DB_DISCOVERY_COMPLETE &&
        p_evt->params.discovered_db.srv_uuid.uuid == ANCS_UUID_SERVICE &&
        p_evt->params.discovered_db.srv_uuid.type == p_ancs->service.service.uuid.type)
    {
        // Find the handles of the ANCS characteristic.
        uint32_t i;

        for (i = 0; i < p_evt->params.discovered_db.char_count; i++)
        {
            switch (p_chars[i].characteristic.uuid.uuid)
            {
                case ANCS_UUID_CHAR_CONTROL_POINT:
                    QPRINTF("[ANCS]: Control Point Characteristic found.\r\n");
                    memcpy(&evt.service.control_point_char,
                           &p_chars[i].characteristic,
                           sizeof(ble_gattc_char_t));
                    break;

                case ANCS_UUID_CHAR_DATA_SOURCE:
                    QPRINTF("[ANCS]: Data Source Characteristic found.\r\n");
                    memcpy(&evt.service.data_source_char,
                           &p_chars[i].characteristic,
                           sizeof(ble_gattc_char_t));
                    evt.service.data_source_cccd.handle = p_chars[i].cccd_handle;
                    break;

                case ANCS_UUID_CHAR_NOTIFICATION_SOURCE:
                    QPRINTF("[ANCS]: Notification point Characteristic found.\r\n");
                    memcpy(&evt.service.notif_source_char,
                           &p_chars[i].characteristic,
                           sizeof(ble_gattc_char_t));
                    evt.service.notif_source_cccd.handle = p_chars[i].cccd_handle;
                    break;

                default:
                    break;
            }
        }
        evt.evt_type    = BLE_ANCS_C_EVT_DISCOVERY_COMPLETE;
        evt.conn_handle = p_evt->conn_handle;
        p_ancs->evt_handler(&evt);
    }
    else
    {
        evt.evt_type = BLE_ANCS_C_EVT_DISCOVERY_FAILED;
        p_ancs->evt_handler(&evt);
    }
}


/**@brief Function for passing any pending request from the buffer to the stack.
 */
static void tx_buffer_process(void)
{
    if (m_tx_index != m_tx_insert_index)
    {
        uint32_t err_code;

        if (m_tx_buffer[m_tx_index].type == READ_REQ)
        {
            err_code = sd_ble_gattc_read(m_tx_buffer[m_tx_index].conn_handle,
                                         m_tx_buffer[m_tx_index].req.read_handle,
                                         0);
        }
        else
        {
            err_code = sd_ble_gattc_write(m_tx_buffer[m_tx_index].conn_handle,
                                          &m_tx_buffer[m_tx_index].req.write_req.gattc_params);
        }
        if (err_code == NRF_SUCCESS)
        {
            ++m_tx_index;
            m_tx_index &= TX_BUFFER_MASK;
        }
		// 2016-03-06 陈长升 这里需要增加错误处理
    }
}

/**@brief Function for checking if data in an iOS notification is out of bounds.
 *
 * @param[in] notif  An iOS notification.
 *
 * @retval NRF_SUCCESS             If the notification is within bounds.
 * @retval NRF_ERROR_INVALID_PARAM If the notification is out of bounds.
 */
static uint32_t ble_ancs_verify_notification_format(const ble_ancs_c_evt_notif_t * notif)
{
    if(   (notif->evt_id >= BLE_ANCS_NB_OF_EVT_ID)
       || (notif->category_id >= BLE_ANCS_NB_OF_CATEGORY_ID))
    {
        return NRF_ERROR_INVALID_PARAM;
    }
    return NRF_SUCCESS;
}

/**@brief Function for receiving and validating notifications received from the Notification Provider.
 * 
 * @param[in] p_ancs     Pointer to an ANCS instance to which the event belongs.
 * @param[in] p_data_src Pointer to data that was received from the Notification Provider.
 * @param[in] hvx_len    Length of the data that was received by the Notification Provider.
 */
static void parse_notif(const ble_ancs_c_t * p_ancs,
                        const uint8_t      * p_data_src,
                        const uint16_t       hvx_data_len)
{
    ble_ancs_c_evt_t ancs_evt;
    uint32_t         err_code;
    if (hvx_data_len != BLE_ANCS_NOTIFICATION_DATA_LENGTH)
    {
        ancs_evt.evt_type = BLE_ANCS_C_EVT_INVALID_NOTIF;
        p_ancs->evt_handler(&ancs_evt);
    }

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
		else
		{

			return;			//跳过后面的打印信息
		}
				
    err_code = ble_ancs_verify_notification_format(&ancs_evt.notif);
    if (err_code == NRF_SUCCESS)
    {
        ancs_evt.evt_type = BLE_ANCS_C_EVT_NOTIF;
    }
    else
    {
        ancs_evt.evt_type = BLE_ANCS_C_EVT_INVALID_NOTIF;
    }

    p_ancs->evt_handler(&ancs_evt);
}

/**@brief Function for parsing received notification attribute response data.
 *
 * @details The data that comes from the Notification Provider can be much longer than what
 *          would fit in a single GATTC notification. Therefore, this function relies on a
 *          state-oriented switch case.
 *          UID and command ID will be received only once at the beginning of the first
 *          GATTC notification of a new attribute request for a given iOS notification.
 *          After this, we can loop several ATTR_ID > LENGTH > DATA > ATTR_ID > LENGTH > DATA until
 *          we have received all attributes we wanted as a Notification Consumer.
 *          The Notification Provider can also simply stop sending attributes.
 *
 *    |1 Byte  |  4 Bytes    |1 Byte |2 Bytes | X Bytes            |1 Bytes| 2 Bytes| X Bytes   
 *    +--------+-------------+-------+--------+- - - - - - - - - - +-------+--------+- - - - - - -
 *    | CMD_ID |  NOTIF_UID  |ATTR_ID| LENGTH |        DATA        |ATTR_ID| LENGTH |    DATA
 *    +--------+-------------+-------+--------+- - - - - - - - - - +-------+--------+- - - - - - -
 *
 * @param[in] p_ancs     Pointer to an ANCS instance to which the event belongs.
 * @param[in] p_data_src Pointer to data that was received from the Notification Provider.
 * @param[in] hvx_len    Length of the data that was received from the Notification Provider.
 */
static void parse_get_notif_attrs_response(ble_ancs_c_t  * p_ancs,
                                           const uint8_t * p_data_src,
                                           const uint16_t  hvx_data_len)
{

	static uint8_t *ptr;
    static uint16_t current_len;
    static ble_ancs_c_evt_t event;
    static uint16_t buff_idx;
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
        switch(p_ancs->parse_state)
        {
            case COMMAND_ID:
                event.attr.command_id = p_data_src[i];
                // get notification时，byte[0]为BLE_ANCS_COMMAND_ID_GET_NOTIFICATION_ATTRIBUTES
                if(event.attr.command_id == BLE_ANCS_COMMAND_ID_GET_NOTIF_ATTRIBUTES)
                {
                    p_ancs->parse_state = NOTIFICATION_UID1;
                }
                break;

            case NOTIFICATION_UID1:
                event.attr.notif_uid = p_data_src[i];
                p_ancs->parse_state = NOTIFICATION_UID2;
                break;

            case NOTIFICATION_UID2:
                event.attr.notif_uid |= (p_data_src[i]<<8);
                p_ancs->parse_state = NOTIFICATION_UID3;
                break;

            case NOTIFICATION_UID3:
                event.attr.notif_uid |= (p_data_src[i]<<16);
                p_ancs->parse_state = NOTIFICATION_UID4;
                break;

            case NOTIFICATION_UID4:
                event.attr.notif_uid |= (p_data_src[i]<<24);
                p_ancs->parse_state = ATTRIBUTE_ID;
                break;

            case ATTRIBUTE_ID:
                event.attr.attr_id = (ble_ancs_c_notif_attr_id_values_t)p_data_src[i];
                p_ancs->parse_state = ATTRIBUTE_LEN1;
                break;

            case ATTRIBUTE_LEN1:
                event.attr.attr_len = p_data_src[i];
                p_ancs->parse_state = ATTRIBUTE_LEN2;
                break;

            case ATTRIBUTE_LEN2:
                event.attr.attr_len |= (p_data_src[i] << 8);
                // 如果长度大于此值为错误的数据
                if(event.attr.attr_len > ANCS_DATA_SIZE)
                {
                    p_ancs->parse_state = COMMAND_ID;
                    break;
                }
                
                p_ancs->parse_state = ATTRIBUTE_READY;
                ptr = event.attr.p_attr_data;
                buff_idx = 0;
                current_len = 0;
                break;

            case ATTRIBUTE_READY:				
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
                    p_ancs->evt_handler(&event);
					
					time = system_sec_get(); 
                    if(event.attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER)
                    {
						p_ancs->parse_state = ATTRIBUTE_ID;
                    }
                    else if(event.attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_TITLE)
                    {
						if(memcmp(&(usr_remind.data[usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_APP_IDENTIFIER].addr]), MOBILE_PHONE, (sizeof(MOBILE_PHONE)-1)))	// 电话只获取2个字段
					    {
					    	p_ancs->parse_state = ATTRIBUTE_ID;
							break;
					    }

                        p_ancs->parse_state = COMMAND_ID;
                		set_remainder_info(IOS_TYPE,CALL_REMAIND,time,
							(uint8_t*)&(usr_remind.data[usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_TITLE].addr]),
							usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_TITLE].size,
							(uint8_t*)&(usr_remind.data[usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_MESSAGE].addr]),
							usr_remind.id[BLE_ANCS_NOTIF_ATTR_ID_MESSAGE].size);
                    }
                    else if(event.attr.attr_id == BLE_ANCS_NOTIF_ATTR_ID_MESSAGE) // 信息获取3个字段
                    {
                        p_ancs->parse_state = COMMAND_ID;

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


/**@brief Function for receiving and validating notifications received from the Notification Provider.
 * 
 * @param[in] p_ancs    Pointer to an ANCS instance to which the event belongs.
 * @param[in] p_ble_evt Bluetooth stack event.
 */
static void on_evt_gattc_notif(ble_ancs_c_t * p_ancs, const ble_evt_t * p_ble_evt)
{
    const ble_gattc_evt_hvx_t * p_notif = &p_ble_evt->evt.gattc_evt.params.hvx;

    if(p_ble_evt->evt.gattc_evt.conn_handle != p_ancs->conn_handle)
    {
        return;
    }

    if (p_notif->handle == p_ancs->service.notif_source_char.handle_value)
    {
        parse_notif(p_ancs, p_notif->data, p_notif->len);
    }
    else if (p_notif->handle == p_ancs->service.data_source_char.handle_value)
    {
        parse_get_notif_attrs_response(p_ancs, p_notif->data, p_notif->len);
    }
}

/**@brief Function for handling write response events.
 *
 * @param[in] p_ancs_c   Pointer to the Battery Service Client Structure.
 * @param[in] p_ble_evt Pointer to the SoftDevice event.
 */
static void on_write_rsp(ble_ancs_c_t * p_ancs, const ble_evt_t * p_ble_evt)
{
    // Check if the event if on the link for this instance
    if (p_ancs->conn_handle != p_ble_evt->evt.gattc_evt.conn_handle)
    {
        return;
    }
    // Check if there is any message to be sent across to the peer and send it.
    tx_buffer_process();
}

void ble_ancs_c_on_ble_evt(ble_ancs_c_t * p_ancs, const ble_evt_t * p_ble_evt)
{
    uint16_t evt = p_ble_evt->header.evt_id;

    switch (evt)
    {
        case BLE_GATTC_EVT_WRITE_RSP:
            on_write_rsp(p_ancs, p_ble_evt);
            break;

        case BLE_GATTC_EVT_HVX:
            on_evt_gattc_notif(p_ancs, p_ble_evt);
            break;
			
        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnected(p_ancs, p_ble_evt);
            break;
			
		case BLE_GAP_EVT_CONNECTED:
			on_connected(p_ancs, p_ble_evt);
			break;
        default:
            break;
    }
}


uint32_t ble_ancs_c_init(ble_ancs_c_t * p_ancs, const ble_ancs_c_init_t * p_ancs_init)
{
    uint32_t err_code;
    
    //Verify that the parameters needed for to initialize this instance of ANCS are not NULL.
    VERIFY_PARAM_NOT_NULL(p_ancs);
    VERIFY_PARAM_NOT_NULL(p_ancs_init);
    VERIFY_PARAM_NOT_NULL(p_ancs_init->evt_handler);
    
    p_ancs->parse_state = COMMAND_ID;
    p_ancs->p_data_dest = NULL;
    p_ancs->current_attr_index = 0;

    p_ancs->evt_handler    = p_ancs_init->evt_handler;
    p_ancs->error_handler  = p_ancs_init->error_handler;
    p_ancs->conn_handle    = BLE_CONN_HANDLE_INVALID;

    p_ancs->service.data_source_cccd.uuid.uuid  = BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG;
    p_ancs->service.notif_source_cccd.uuid.uuid = BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG;

    // Make sure instance of service is clear. GATT handles inside the service and characteristics are set to @ref BLE_GATT_HANDLE_INVALID.
    memset(&p_ancs->service, 0, sizeof(ble_ancs_c_service_t));
    memset(m_tx_buffer, 0, TX_BUFFER_SIZE);

    // Assign UUID types.
    err_code = sd_ble_uuid_vs_add(&ble_ancs_base_uuid128, &p_ancs->service.service.uuid.type);
    VERIFY_SUCCESS(err_code);

    err_code = sd_ble_uuid_vs_add(&ble_ancs_cp_base_uuid128, &p_ancs->service.control_point_char.uuid.type);
    VERIFY_SUCCESS(err_code);

    err_code = sd_ble_uuid_vs_add(&ble_ancs_ns_base_uuid128, &p_ancs->service.notif_source_char.uuid.type);
    VERIFY_SUCCESS(err_code);

    err_code = sd_ble_uuid_vs_add(&ble_ancs_ds_base_uuid128, &p_ancs->service.data_source_char.uuid.type);
    VERIFY_SUCCESS(err_code);

    // Assign UUID to the service.
    p_ancs->service.service.uuid.uuid = ANCS_UUID_SERVICE;
    p_ancs->service.service.uuid.type = p_ancs->service.service.uuid.type;

    return ble_db_discovery_evt_register(&p_ancs->service.service.uuid);
}


/**@brief Function for creating a TX message for writing a CCCD.
 *
 * @param[in] conn_handle  Connection handle on which to perform the configuration.
 * @param[in] handle_cccd  Handle of the CCCD.
 * @param[in] enable       Enable or disable GATTC notifications.
 *
 * @retval NRF_SUCCESS              If the message was created successfully.
 * @retval NRF_ERROR_INVALID_PARAM  If one of the input parameters was invalid.
 */
static uint32_t cccd_configure(const uint16_t conn_handle, const uint16_t handle_cccd, bool enable)
{
    tx_message_t * p_msg;
    uint16_t       cccd_val = enable ? BLE_CCCD_NOTIFY_BIT_MASK : 0;

    p_msg              = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index &= TX_BUFFER_MASK;

    p_msg->req.write_req.gattc_params.handle   = handle_cccd;
    p_msg->req.write_req.gattc_params.len      = 2;
    p_msg->req.write_req.gattc_params.p_value  = p_msg->req.write_req.gattc_value;
    p_msg->req.write_req.gattc_params.offset   = 0;
    p_msg->req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;
    p_msg->req.write_req.gattc_value[0]        = LSB_16(cccd_val);
    p_msg->req.write_req.gattc_value[1]        = MSB_16(cccd_val);
    p_msg->conn_handle                         = conn_handle;
    p_msg->type                                = WRITE_REQ;

    tx_buffer_process();
    return NRF_SUCCESS;
}


uint32_t ble_ancs_c_notif_source_notif_enable(const ble_ancs_c_t * p_ancs)
{
    QPRINTF("[ANCS]: Enable Notification Source notifications. writing to handle: %i \n\r",
         p_ancs->service.notif_source_cccd.handle);
    return cccd_configure(p_ancs->conn_handle, p_ancs->service.notif_source_cccd.handle, true);
}


uint32_t ble_ancs_c_notif_source_notif_disable(const ble_ancs_c_t * p_ancs)
{
    return cccd_configure(p_ancs->conn_handle, p_ancs->service.notif_source_cccd.handle, false);
}


uint32_t ble_ancs_c_data_source_notif_enable(const ble_ancs_c_t * p_ancs)
{
    QPRINTF("[ANCS]: Enable Data Source notifications. Writing to handle: %i \n\r",
        p_ancs->service.data_source_cccd.handle);
    return cccd_configure(p_ancs->conn_handle, p_ancs->service.data_source_cccd.handle, true);
}


uint32_t ble_ancs_c_data_source_notif_disable(const ble_ancs_c_t * p_ancs)
{
    return cccd_configure(p_ancs->conn_handle, p_ancs->service.data_source_cccd.handle, false);
}


uint32_t ble_ancs_get_notif_attrs(ble_ancs_c_t       * p_ancs,
                                  const uint32_t       p_uid)
{
    tx_message_t * p_msg;

    uint32_t index                   = 0;
    p_ancs->number_of_requested_attr = 0;
    p_msg              = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index &= TX_BUFFER_MASK;

    p_msg->req.write_req.gattc_params.handle   = p_ancs->service.control_point_char.handle_value;
    p_msg->req.write_req.gattc_params.p_value  = p_msg->req.write_req.gattc_value;
    p_msg->req.write_req.gattc_params.offset   = 0;
    p_msg->req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;

    //Encode Command ID.
    p_msg->req.write_req.gattc_value[index++] = BLE_ANCS_COMMAND_ID_GET_NOTIF_ATTRIBUTES;
    
    //Encode Notification UID.
    index += uint32_encode(p_uid, &p_msg->req.write_req.gattc_value[index]);

    //Encode Attribute ID.
    for (uint32_t attr = 0; attr < BLE_ANCS_NB_OF_ATTRS; attr++)
    {
        if (p_ancs->ancs_attr_list[attr].get == true)
        {
            p_msg->req.write_req.gattc_value[index++] = attr;
            if ((attr == BLE_ANCS_NOTIF_ATTR_ID_TITLE) ||
                (attr == BLE_ANCS_NOTIF_ATTR_ID_SUBTITLE) ||
                (attr == BLE_ANCS_NOTIF_ATTR_ID_MESSAGE))
            {
                //Encode Length field, only applicable for Title, Subtitle and Message
                index += uint16_encode(p_ancs->ancs_attr_list[attr].attr_len,
                              &p_msg->req.write_req.gattc_value[index]);
            }
            p_ancs->number_of_requested_attr++;
        }
    }
    p_msg->req.write_req.gattc_params.len = index;
    p_msg->conn_handle                    = p_ancs->conn_handle;
    p_msg->type                           = WRITE_REQ;
    p_ancs->expected_number_of_attrs      = p_ancs->number_of_requested_attr;

    tx_buffer_process();

    return NRF_SUCCESS;
}


uint32_t ble_ancs_c_request_attrs(ble_ancs_c_t * p_ancs,
                                  const ble_ancs_c_evt_notif_t * p_notif)
{
    uint32_t err_code;
    err_code = ble_ancs_verify_notification_format(p_notif);
    VERIFY_SUCCESS(err_code);

    err_code            = ble_ancs_get_notif_attrs(p_ancs, p_notif->notif_uid);
    p_ancs->parse_state = COMMAND_ID;
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}

uint32_t ble_ancs_c_handles_assign(ble_ancs_c_t * p_ancs,
                                   const uint16_t conn_handle,
                                   const ble_ancs_c_service_t * p_peer_handles)
{
    VERIFY_PARAM_NOT_NULL(p_ancs);

    p_ancs->conn_handle = conn_handle;
    
    if(p_peer_handles != NULL)
    {
        p_ancs->service.control_point_char.handle_value = p_peer_handles->control_point_char.handle_value;
        p_ancs->service.data_source_cccd.handle         = p_peer_handles->data_source_cccd.handle;
        p_ancs->service.data_source_char.handle_value   = p_peer_handles->data_source_char.handle_value;
        p_ancs->service.notif_source_cccd.handle        = p_peer_handles->notif_source_cccd.handle;
        p_ancs->service.notif_source_char.handle_value  = p_peer_handles->notif_source_char.handle_value;
    }
    
    return NRF_SUCCESS;
}

uint32_t ble_ancs_perform_notification_action(const ble_ancs_c_t *p_ancs,uint8_t *p_uid,bool positive_action)
{
    tx_message_t *p_msg;
    uint32_t i = 0;

    p_msg              = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index &= TX_BUFFER_MASK;					// 2016-03-06 陈长升 最大值不超过0x07

		p_msg->type                                = WRITE_REQ;
    p_msg->req.write_req.gattc_params.handle   = p_ancs->service.control_point_char.handle_value;
    p_msg->req.write_req.gattc_params.p_value  = p_msg->req.write_req.gattc_value;
    p_msg->req.write_req.gattc_params.offset   = 0;
    p_msg->req.write_req.gattc_params.write_op = BLE_GATT_OP_WRITE_REQ;

    p_msg->req.write_req.gattc_value[i++]      = BLE_ANCS_COMMAND_ID_GET_PERFORM_NOTIF_ACTION;
    memcpy(&p_msg->req.write_req.gattc_value[1],p_uid,4);
    i+=4;

    if(positive_action)
        p_msg->req.write_req.gattc_value[i++]  = 0;
    else
        p_msg->req.write_req.gattc_value[i++]  = 1;

    p_msg->req.write_req.gattc_params.len      = i;
    p_msg->conn_handle                         = p_ancs->conn_handle;

    tx_buffer_process();
    return NRF_SUCCESS;
}


/****************add by zhangfei 2015022401 end*******************/
/***************************************************************************************************************
 * 函 数 名  	: ble_ancs_notification_ccc_read
 * 函数功能  : 发送命令去读取IOS来电服务notification属性开关状态
 * 输入参数  : p_ancs--Apple Notification structure
 * 输出参数  : 无
 * 返 回 值  	: 无
***************************************************************************************************************/
uint32_t ble_ancs_notification_ccc_read(ble_ancs_c_t * p_ancs)
{
    tx_message_t * msg;

    msg                  = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index   &= TX_BUFFER_MASK;                  // 2016-03-06 陈长升 最大值不超过0x07

    msg->req.read_handle = p_ancs->service.notif_source_cccd.handle;
    msg->conn_handle     = p_ancs->conn_handle;
    msg->type            = READ_REQ;

    tx_buffer_process();
    return NRF_SUCCESS;
}
/***************************************************************************************************************
 * 函 数 名  	: ble_ancs_datasource_ccc_read
 * 函数功能  : 发送命令去读取IOS来电服务datasource属性开关状态
 * 输入参数  : p_ancs--Apple Notification structure
 * 输出参数  : 无
 * 返 回 值  	: 无
***************************************************************************************************************/
uint32_t ble_ancs_datasource_ccc_read(ble_ancs_c_t * p_ancs)
{
    tx_message_t * msg;

    msg                  = &m_tx_buffer[m_tx_insert_index++];
    m_tx_insert_index   &= TX_BUFFER_MASK;					// 2016-03-06 陈长升 最大值不超过0x07

    msg->req.read_handle = p_ancs->service.data_source_cccd.handle;
    msg->conn_handle     = p_ancs->conn_handle;
    msg->type            = READ_REQ;

    tx_buffer_process();
    return NRF_SUCCESS;
}

/***************************************************************************************************************
 * 函 数 名  	: ble_ancs_ccc_read
 * 函数功能  : 发送命令去读取IOS来电服务属性开关状态
 * 输入参数  : p_ancs--Apple Notification structure
 * 输出参数  : 无
 * 返 回 值  	: 无
***************************************************************************************************************/
void ble_ancs_ccc_read(ble_ancs_c_t * p_ancs)
{

}


void Hang_up_Photo(bool action_state)
{
    uint32_t err_code = ble_ancs_perform_notification_action(&m_ios_ancs, incoming_call_uid, action_state);
    if(err_code !=NRF_ERROR_INVALID_STATE)
    { 
        APP_ERROR_CHECK(err_code);
    }
}

/**@brief Function for setting up GATTC notifications from the Notification Provider.
 *
 * @details This function is called when a successful connection has been established.
 */
static void apple_notification_setup(void)
{
    uint32_t err_code;

    nrf_delay_ms(100); // Delay because we cannot add a CCCD to close to starting encryption. iOS specific.

    err_code = ble_ancs_c_notif_source_notif_enable(&m_ios_ancs);
    APP_ERROR_CHECK(err_code);

    err_code = ble_ancs_c_data_source_notif_enable(&m_ios_ancs);
    APP_ERROR_CHECK(err_code);

    QPRINTF("Notifications Enabled.\r\n");
}


static void evt_ios_notification(ble_ancs_c_evt_notif_t *p_notice)
{
	if(p_notice->category_id == BLE_ANCS_CATEGORY_ID_INCOMING_CALL)
	{
		for(uint8_t i=0; i<4; i++)
		{
			//此处还需检查下大小端
		    incoming_call_uid[i] = p_notice->notif_uid >> (i * 8);
		}
	}
}


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


/**@brief Function for handling the Apple Notification Service client.
 *
 * @details This function is called for all events in the Apple Notification client that
 *          are passed to the application.
 *
 * @param[in] p_evt  Event received from the Apple Notification Service client.
 */
static void on_ancs_c_evt(ble_ancs_c_evt_t * p_evt)
{
    uint32_t err_code = NRF_SUCCESS;

    switch (p_evt->evt_type)
    {
        case BLE_ANCS_C_EVT_DISCOVERY_COMPLETE:
            QPRINTF("Apple Notification Service discovered on the server.\r\n");
            err_code = ble_ancs_c_handles_assign(&m_ios_ancs,p_evt->conn_handle, &p_evt->service);
            APP_ERROR_CHECK(err_code);
            apple_notification_setup();
            break;

        case BLE_ANCS_C_EVT_NOTIF:
            m_notification_latest = p_evt->notif;
			evt_ios_notification(&p_evt->notif);
            break;

        case BLE_ANCS_C_EVT_NOTIF_ATTRIBUTE:
			evt_notif_attribute(&p_evt->attr);
            break;

        case BLE_ANCS_C_EVT_DISCOVERY_FAILED:
            QPRINTF("Apple Notification Service not discovered on the server.\r\n");
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for handling the Apple Notification Service client errors.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void apple_notification_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

void ios_ancs_service_init(void)
{
    ble_ancs_c_init_t ancs_init_obj;
    uint32_t          err_code;

    memset(&ancs_init_obj, 0, sizeof(ancs_init_obj));

    ancs_init_obj.evt_handler   = on_ancs_c_evt;
    ancs_init_obj.error_handler = apple_notification_error_handler;

    err_code = ble_ancs_c_init(&m_ios_ancs, &ancs_init_obj);
    APP_ERROR_CHECK(err_code);
}

