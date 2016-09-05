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

#include <string.h>
#include "ble_transfer.h"
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "debug.h"
#include "transfer_usrdesign.h"

extern void trans_indicate_statue_set(ble_trans_evt_type_t statue);

ble_trans_t    m_trans;
static uint8_t uuid_index = 0;

static void on_trans_connect(ble_trans_t * p_trans, ble_evt_t * p_ble_evt)
{
    p_trans->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
	transfer_connection();
}

static void on_trans_disconnect(ble_trans_t * p_trans, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_trans->conn_handle = BLE_CONN_HANDLE_INVALID;
	transfer_disconnection();
}

/**@brief Function for handling the write events to the Blood Pressure Measurement characteristic.
 *
 * @param[in]   p_trans       transfer Service structure.
 * @param[in]   p_evt_write   Write event received from the BLE stack.
 */
static void trans_notify_on_cccd_write(ble_trans_t * p_trans, ble_gatts_evt_write_t * p_evt_write)
{
    if (p_evt_write->len == 2)
    {
        // CCCD written, update notify state
        uint8_t len = sizeof(p_trans->notify_handles)/sizeof(p_trans->notify_handles[0]);
        for(uint8_t i=0;i<len;i++)
        {
            if (p_evt_write->handle == p_trans->notify_handles[i].cccd_handle)
            {
                uint8_t val = 1;
                val = val << i;
                if (ble_srv_is_notification_enabled(p_evt_write->data))
                {
                    p_trans->notification_enabled |= val;
                }
                else
                {
                    p_trans->notification_enabled &= (~val);
                }
            }
        }
    }
}


/**@brief Function for handling the write events to the Blood Pressure Measurement characteristic.
 *
 * @param[in]   p_trans       transfer Service structure.
 * @param[in]   p_evt_write   Write event received from the BLE stack.
 */
static void trans_on_cccd_write(ble_trans_t * p_trans, ble_gatts_evt_write_t * p_evt_write)
{
    if (p_evt_write->len == 2)
    {
        // CCCD written, update indication state
        if (p_trans->evt_handler != NULL)
        {
            ble_trans_evt_t evt;
            
            if (ble_srv_is_indication_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_TRANS_EVT_INDICATION_ENABLED;
            }
            else
            {
                evt.evt_type = BLE_TRANS_EVT_INDICATION_DISABLED;
            }
            
            p_trans->evt_handler(p_trans, &evt);
        }
    }
}


/**@brief Function for handling a write event to the Record Access Control Point.
 *
 * @param[in]   p_trans      Service instance.
 * @param[in]   p_evt_write   WRITE event to be handled.
 */
static void trans_on_value_write(ble_trans_t * p_trans, ble_gatts_evt_write_t * p_evt_write)
{
	transfer_receive(p_evt_write->data,p_evt_write->len);
}


/**@brief     Function for handling the @ref BLE_GATTS_EVT_WRITE event from the S110 SoftDevice.
 *
 * @param[in] p_dfu     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void trans_on_write(ble_trans_t * p_trans, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    
    if (p_evt_write->handle == p_trans->write_handle.value_handle)
    {
        trans_on_value_write(p_trans,p_evt_write);
    }
    else if(p_evt_write->handle == p_trans->indicate_handle.cccd_handle)
    {
        trans_on_cccd_write(p_trans, p_evt_write);
    }
    else
    {
        trans_notify_on_cccd_write(p_trans, p_evt_write);
    }
}


 /**@brief Function for handling the HVC event.
 *
 * @details Handles HVC events from the BLE stack.
 *
 * @param[in]   p_trans     transfer Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void trans_on_hvc(ble_trans_t * p_trans, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_hvc_t * p_hvc = &p_ble_evt->evt.gatts_evt.params.hvc;

    if (p_hvc->handle == p_trans->indicate_handle.value_handle)
    {
        ble_trans_evt_t evt;
        
        evt.evt_type = BLE_TRANS_EVT_INDICATION_CONFIRMED;
        p_trans->evt_handler(p_trans, &evt);
    }
}


/**@brief       Function for adding TX characteristic.
 *
 * @param[in]   p_trans        Nordic UART Service structure.
 * @param[in]   p_trans_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t indicate_char_add(ble_trans_t * p_trans, const ble_trans_init_t * p_trans_init)
{
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    
    memset(&char_md, 0, sizeof(char_md));
    
    char_md.char_props.indicate = 1;
    char_md.p_char_user_desc    = NULL;
    char_md.p_char_pf           = NULL;
    char_md.p_user_desc_md      = NULL;
    char_md.p_cccd_md           = &cccd_md;
    char_md.p_sccd_md           = NULL;
    
	ble_uuid.type               = BLE_UUID_TYPE_BLE;
    ble_uuid.uuid               = BLE_UUID_TRANS_INDICATE_CHAR;
    
    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
    attr_md.vloc                = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth             = 0;
    attr_md.wr_auth             = 0;
    attr_md.vlen                = 1;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid      = &ble_uuid;
    attr_char_value.p_attr_md   = &attr_md;
    attr_char_value.init_len    = sizeof(uint8_t);
    attr_char_value.init_offs   = 0;
    attr_char_value.max_len     = BLE_TRANS_MAX_RX_CHAR_LEN;
    return sd_ble_gatts_characteristic_add(p_trans->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_trans->indicate_handle);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}


/**@brief       Function for adding RX characteristic.
 *
 * @param[in]   p_trans        Nordic UART Service structure.
 * @param[in]   p_trans_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t write_char_add(ble_trans_t * p_trans, const ble_trans_init_t * p_trans_init)
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
    
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_TRANS_WRITE_CHAR);
    
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
    
    return sd_ble_gatts_characteristic_add(p_trans->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_trans->write_handle);
}


/**@brief       Function for adding RX characteristic.
 *
 * @param[in]   p_trans        Nordic UART Service structure.
 * @param[in]   p_trans_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t notify_char_add(ble_trans_t * p_trans, const ble_trans_init_t * p_trans_init)
{
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    
    memset(&char_md, 0, sizeof(char_md));
    
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;
    
	ble_uuid.type             = BLE_UUID_TYPE_BLE;
    ble_uuid.uuid             = BLE_UUID_TRANS_NOTIFY_CHAR + uuid_index;
    
    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
    attr_md.vloc              = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth           = 0;
    attr_md.wr_auth           = 0;
    attr_md.vlen              = 1;
    
    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_TRANS_MAX_RX_CHAR_LEN;
    return sd_ble_gatts_characteristic_add(p_trans->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_trans->notify_handles[uuid_index]);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */

}


void ble_trans_on_ble_evt(ble_trans_t * p_trans, ble_evt_t * p_ble_evt)
{
    if ((p_trans == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_trans_connect(p_trans, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_trans_disconnect(p_trans, p_ble_evt);			
			break;

        case BLE_GATTS_EVT_WRITE:
            trans_on_write(p_trans, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVC:
            trans_on_hvc(p_trans, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}


uint32_t ble_trans_init(ble_trans_t * p_trans, const ble_trans_init_t * p_trans_init)
{
    uint8_t         i;
    uint32_t        err_code;
    ble_uuid_t      ble_uuid;
    
    if ((p_trans == NULL) || (p_trans_init == NULL))
    {
        return NRF_ERROR_NULL;
    }
    
    // Initialize service structure.
    p_trans->conn_handle              = BLE_CONN_HANDLE_INVALID;
    p_trans->evt_handler              = p_trans_init->evt_handler;
    
    p_trans->notification_enabled     = 0;

    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
	ble_uuid.type = BLE_UUID_TYPE_BLE;
    ble_uuid.uuid = BLE_UUID_TRANS_SERVICE;

    // Add service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_trans->service_handle);
    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    // Add Indicate Characteristic.
    err_code = indicate_char_add(p_trans, p_trans_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    // Add Write Characteristic.
    err_code = write_char_add(p_trans, p_trans_init);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    
    uuid_index = 0;
    uint8_t len = sizeof(p_trans->notify_handles)/sizeof(p_trans->notify_handles[0]);
    for(i=0;i<len;i++)
    {
        err_code = notify_char_add(p_trans, p_trans_init);
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
        uuid_index++;
    }
    
    return NRF_SUCCESS;
}


uint32_t ble_trans_notify_send(ble_trans_t * p_trans, uint8_t chnl, uint8_t * string)
{
    uint16_t len = 0;
    ble_gatts_hvx_params_t hvx_params;
    if (p_trans == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint8_t is_notify = (p_trans->notification_enabled >> chnl) & 0x01;
    if ((p_trans->conn_handle == BLE_CONN_HANDLE_INVALID) || (is_notify == 0))
    {
        return NRF_ERROR_INVALID_STATE;
    }

    len = 20;
    memset(&hvx_params, 0, sizeof(hvx_params));
    
	hvx_params.handle = p_trans->notify_handles[chnl].value_handle;
    hvx_params.p_data = string;
    hvx_params.p_len  = &len;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
    
    return sd_ble_gatts_hvx(p_trans->conn_handle, &hvx_params);
}

uint32_t ble_trans_indicate_send(ble_trans_t * p_trans, uint8_t * string)
{
    uint16_t len = 0;
    ble_gatts_hvx_params_t hvx_params;
    if (p_trans == NULL)
    {
        return NRF_ERROR_NULL;
    }
    
    if (p_trans->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }
    
    len = 20;
    memset(&hvx_params, 0, sizeof(hvx_params));
    
    hvx_params.handle = p_trans->indicate_handle.value_handle;
    hvx_params.p_data = string;
    hvx_params.p_len  = &len;
    hvx_params.type   = BLE_GATT_HVX_INDICATION;
    
    return sd_ble_gatts_hvx(p_trans->conn_handle, &hvx_params);
}

/**@brief    Function for handling the indicate from the transfer Service.
 *
 * @details  This function will process the data received from the Nordic UART BLE Service and send
 *           it to the UART module.
 */
/**@snippet [Handling the data received over BLE] */

void on_trans_evt(ble_trans_t * p_trans, ble_trans_evt_t *p_evt)
{
    switch(p_evt->evt_type)
    {
        case BLE_TRANS_EVT_INDICATION_ENABLED:
        case BLE_TRANS_EVT_INDICATION_CONFIRMED:
        case BLE_TRANS_EVT_INDICATION_DISABLED:
			trans_indicate_statue_set(p_evt->evt_type);
		break;
		
        default:
            // No implementation needed.
            break;
    }
}

