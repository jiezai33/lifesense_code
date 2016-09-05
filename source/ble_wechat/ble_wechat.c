/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/* Attention! 
*  To maintain compliance with Nordic Semiconductor ASA抯 Bluetooth profile 
*  qualification listings, this section of source code must not be modified.
*/

#include "ble_wechat.h"
#include <string.h>
#include "nordic_common.h"
#include "ble_l2cap.h"
#include "ble_srv_common.h"
#include "app_util.h"
#include "debug.h"
#include "wechat_usrdesign.h"


ble_wechat_t   m_wechat;

extern ble_gap_addr_t device_mac;


/////////////////////////////////////////////////////////////////////////////////////////////////////
/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_wechat    wechat Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void wechat_on_connect(ble_wechat_t * p_wechat, ble_evt_t * p_ble_evt)
{
    p_wechat->conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
	wechat_connection();
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_wechat    p_wechat Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void wechat_on_disconnect(ble_wechat_t * p_wechat, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_wechat->conn_handle = BLE_CONN_HANDLE_INVALID;
	wechat_disconnection();
}


/**@brief Function for handling the write events to the Blood Pressure Measurement characteristic.
 *
 * @param[in]   p_wechat      wechat Service structure.
 * @param[in]   p_evt_write   Write event received from the BLE stack.
 */
static void wechat_on_cccd_write(ble_wechat_t * p_wechat, ble_gatts_evt_write_t * p_evt_write)
{
    if (p_evt_write->len == 2)
    {
        // CCCD written, update indication state
        if (p_wechat->evt_handler != NULL)
        {
            ble_wechat_evt_t evt;
            
            if (ble_srv_is_indication_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_WECHAT_EVT_INDICATION_ENABLED;
            }
            else
            {
                evt.evt_type = BLE_WECHAT_EVT_INDICATION_DISABLED;
            }
            
            p_wechat->evt_handler(p_wechat, &evt);
        }
    }
}
/**@brief Function for handling a write event to the Record Access Control Point.
 *
 * @param[in]   p_wechat         Service instance.
 * @param[in]   p_evt_write   WRITE event to be handled.
 */
static void wechat_on_value_write(ble_wechat_t * p_wechat, ble_gatts_evt_write_t * p_evt_write)
{
	wechat_receive(p_evt_write->data,p_evt_write->len);
}

/**@brief Function for handling the Write event.
 *
 * @param[in]   p_wechat    WECHAT Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void wechat_on_write(ble_wechat_t * p_wechat, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    if (p_evt_write->handle == p_wechat->indicate_handles.cccd_handle)
    {
        wechat_on_cccd_write(p_wechat, p_evt_write);
    }
    else if (p_evt_write->handle == p_wechat->write_handles.value_handle)
    {
        //手机写回来的数据处理
        wechat_on_value_write(p_wechat, p_evt_write);
    }
}


/**@brief Function for handling the HVC event.
 *
 * @details Handles HVC events from the BLE stack.
 *
 * @param[in]   p_wechat    wechat Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void wechat_on_hvc(ble_wechat_t * p_wechat, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_hvc_t * p_hvc = &p_ble_evt->evt.gatts_evt.params.hvc;

    if (p_hvc->handle == p_wechat->indicate_handles.value_handle)
    {
        ble_wechat_evt_t evt;
        
        evt.evt_type = BLE_WECHAT_EVT_INDICATION_CONFIRMED;
        p_wechat->evt_handler(p_wechat, &evt);
    }
}

/**@brief Function for adding Blood Pressure Measurement characteristics.
 *
 * @param[in]   p_wechat        wechat Service structure.
 * @param[in]   p_wechat_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t wechat_indicate_char_add(ble_wechat_t * p_wechat, const ble_wechat_init_t * p_wechat_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    ble_wechat_meas_t      initial_wechat;
    uint8_t             encoded_wechat[MAX_BPM_LEN];
    
    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    cccd_md.write_perm = p_wechat_init->wechat_meas_attr_md.cccd_write_perm;

    memset(&char_md, 0, sizeof(char_md));
    
    char_md.char_props.indicate = 1;			//indicate属性
    char_md.p_char_user_desc    = NULL;
    char_md.p_char_pf           = NULL;
    char_md.p_user_desc_md      = NULL;
    char_md.p_cccd_md           = &cccd_md;
    char_md.p_sccd_md           = NULL;
    
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_WECHRT_INDICATE_CHAR);
    
    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.read_perm  = p_wechat_init->wechat_meas_attr_md.read_perm;
    attr_md.write_perm = p_wechat_init->wechat_meas_attr_md.write_perm;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    memset(&initial_wechat, 0, sizeof(initial_wechat));
    
    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = 20;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = MAX_BPM_LEN;
    attr_char_value.p_value      = encoded_wechat;
    
    return sd_ble_gatts_characteristic_add(p_wechat->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_wechat->indicate_handles);
}


/**@brief Function for adding wechat write Feature characteristics.
 *
 * @param[in]   p_wechat     wechat Service structure.
 * @param[in]   p_wechat_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */

static uint32_t wechat_write_char_add(ble_wechat_t * p_wechat, const ble_wechat_init_t * p_wechat_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    ble_gatts_attr_md_t cccd_md;
    
	
	memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
	
    memset(&char_md, 0, sizeof(char_md));
    
    char_md.char_props.indicate = 0;
    char_md.char_props.write    = 1;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf        = NULL;
    char_md.p_user_desc_md   = NULL;
    char_md.p_cccd_md        = &cccd_md;
    char_md.p_sccd_md        = NULL;
    
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_WECHRT_WRITE_CHAR);
    
    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
	BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    
    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = 0;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = BLE_L2CAP_MTU_DEF;
    attr_char_value.p_value      = 0;
    
    return sd_ble_gatts_characteristic_add(p_wechat->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_wechat->write_handles);

}

//#ifdef	AUTH_MAC
/**@brief Function for adding Weight Scale Feature characteristics.
 *
 * @param[in]   p_bps        Weight Scale Service structure.
 * @param[in]   p_bps_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t wechat_feature_char_add(ble_wechat_t * p_wechat, const ble_wechat_init_t * p_wechat_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
//    uint8_t             init_value_encoded[2];
    uint8_t             init_value_len;
    memset(&char_md, 0, sizeof(char_md));
    
    char_md.char_props.read  = 1;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf        = NULL;
    char_md.p_user_desc_md   = NULL;
    char_md.p_cccd_md        = NULL;
    char_md.p_sccd_md        = NULL;
    
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_WECHRT_READ_CHAR);
    
    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.read_perm  = p_wechat_init->wechat_read_attr_md.read_perm;
    attr_md.write_perm = p_wechat_init->wechat_read_attr_md.write_perm;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    
    init_value_len = sizeof(device_mac.addr);		//uint16_encode(p_wechat_init->data, init_value_encoded);

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = init_value_len;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = init_value_len;

	attr_char_value.p_value      = device_mac.addr;		//mac地址放入read属性
    return sd_ble_gatts_characteristic_add(p_wechat->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_wechat->read_handles);
}

/*****************************************************************************
 * 函 数 名  : ble_wechat_init
 * 函数功能  : 微信服务初始化，包括添加服务、属性
 * 输入参数  : p_wechat    wechat Service structure.
 * 				 p_wechat_init --初始值
 * 输出参数  : 无
 * 返 回 值  : 无

*****************************************************************************/
uint32_t ble_wechat_init(ble_wechat_t * p_wechat, const ble_wechat_init_t * p_wechat_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_wechat->evt_handler = p_wechat_init->evt_handler;
    p_wechat->conn_handle = BLE_CONN_HANDLE_INVALID;


    // Add service
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_WECHAT_SERVICE);

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_wechat->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;		//添加服务失败则退出
    }
    
    // Add indicate characteristic
    err_code = wechat_indicate_char_add(p_wechat, p_wechat_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    // Add write characteristic
    err_code = wechat_write_char_add(p_wechat, p_wechat_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    //添加微信read属性
    err_code = wechat_feature_char_add(p_wechat, p_wechat_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
	//	#endif
    
    return NRF_SUCCESS;
}

/*****************************************************************************
 * 函 数 名  : ble_wechat_indicate_send
 * 函数功能  : 微信服务的indicate通道发送数据
 * 输入参数  : p_wechat    wechat Service structure.
 * 				 data --需发送的数据
 * 输出参数  : 无
 * 返 回 值  : 无

*****************************************************************************/
uint32_t ble_wechat_indicate_send(ble_wechat_t * p_wechat, uint8_t *data)
{
    uint32_t err_code;
 
    // Send value if connected
    if (p_wechat->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint16_t               len;
        uint16_t               hvx_len;
        ble_gatts_hvx_params_t hvx_params;

		//每笔数据长度固定20bytes，实际需要发送的长度不够20bytes的时候剩余位置补零
        len     = 20;				
        hvx_len = len;

        memset(&hvx_params, 0, sizeof(hvx_params));
        
        hvx_params.handle = p_wechat->indicate_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_INDICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &hvx_len;
        hvx_params.p_data = data;
        
        err_code = sd_ble_gatts_hvx(p_wechat->conn_handle, &hvx_params);
        if ((err_code == NRF_SUCCESS) && (hvx_len != len))
        {
            err_code = NRF_ERROR_DATA_SIZE;
        }
    }
    else
    {
        err_code = NRF_ERROR_INVALID_STATE;
    }

    return err_code;
}

/*****************************************************************************
 * 函 数 名  : ble_wechat_on_ble_evt
 * 函数功能  : 协议栈事件处理
 * 输入参数  : p_wechat    wechat Service structure.
 * 				 p_ble_evt   Event received from the BLE stack.
 * 输出参数  : 无
 * 返 回 值  : 无

*****************************************************************************/
void ble_wechat_on_ble_evt(ble_wechat_t * p_wechat, ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            wechat_on_connect(p_wechat, p_ble_evt);	//保存connect handle
            break;

		case BLE_GAP_EVT_DISCONNECTED:
			wechat_on_disconnect(p_wechat, p_ble_evt);
			break;
			
        case BLE_GATTS_EVT_WRITE:
			//app写数据事件
            wechat_on_write(p_wechat, p_ble_evt);
            break;
            
        case BLE_GATTS_EVT_HVC:
			//下发的confirm信息
            wechat_on_hvc(p_wechat, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief Function for handling the wechat Service events.
 *
 * @details This function will be called for all wechat Service events which are passed to
 *          the application.
 *
 * @param[in]   p_wechat   Blood Pressure Service structure.
 * @param[in]   p_evt   Event received from the Blood Pressure Service.
 */
void on_wechat_evt(ble_wechat_t * p_wechat, ble_wechat_evt_t *p_evt)
{
    switch(p_evt->evt_type)
    {
        case BLE_WECHAT_EVT_INDICATION_ENABLED:
        case BLE_WECHAT_EVT_INDICATION_CONFIRMED:
        case BLE_WECHAT_EVT_INDICATION_DISABLED:
			wechat_indicate_statue_set(p_evt->evt_type);
            break;

        default:
            // No implementation needed.
            break;
    }
}
