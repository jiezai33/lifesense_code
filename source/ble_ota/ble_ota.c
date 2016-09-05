#include <string.h>
#include "nordic_common.h"
#include "ble_ota.h"
#include "debug.h"
#include "ota_usrdesign.h"

ble_ota_t      m_ota;
static void on_ota_connect(ble_ota_t * p_ota, ble_evt_t * p_ble_evt)
{
    p_ota->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
	ota_connection();
}

static void on_ota_disconnect(ble_ota_t * p_ota, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_ota->conn_handle = BLE_CONN_HANDLE_INVALID;
	ota_disconnection();
}

/**@brief Function for handling the write events to the Blood Pressure Measurement characteristic.
 *
 * @param[in]   p_ota       ota Service structure.
 * @param[in]   p_evt_write   Write event received from the BLE stack.
 */
static void ota_on_cccd_write(ble_ota_t * p_ota, ble_gatts_evt_write_t * p_evt_write)
{
    if(p_evt_write->len == 2)
    {
        // CCCD written, update indication state
        if(p_ota->evt_handler != NULL)
        {
            ble_ota_evt_t evt;

            if(ble_srv_is_indication_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_OTA_EVT_INDICATION_ENABLED;
            }
            else
            {
                evt.evt_type = BLE_OTA_EVT_INDICATION_DISABLED;
            }

            p_ota->evt_handler(p_ota, &evt);
        }
    }
}


/**@brief Function for handling a write event to the Record Access Control Point.
 *
 * @param[in]   p_ota      Service instance.
 * @param[in]   p_evt_write   WRITE event to be handled.
 */
static void wirsp_on_value_write(ble_ota_t * p_ota, ble_gatts_evt_write_t * p_evt_write)
{
    if(p_evt_write->len<21)
    {
		ota_cmd_receive(p_evt_write->data,p_evt_write->len);
    }
}

/**@brief Function for handling a write event to the Record Access Control Point.
 *
 * @param[in]   p_ota      Service instance.
 * @param[in]   p_evt_write   WRITE event to be handled.
 */
static void worsp_on_value_write(ble_ota_t * p_ota, ble_gatts_evt_write_t * p_evt_write)
{
    if(p_evt_write->len<21)
    {
        ota_data_receive(p_evt_write->data,p_evt_write->len);
    }
}


/**@brief     Function for handling the @ref BLE_GATTS_EVT_WRITE event from the S110 SoftDevice.
 *
 * @param[in] p_dfu     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_ota_write(ble_ota_t * p_ota, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    if(p_evt_write->handle == p_ota->write_wirsp_handle.value_handle)
    {
        wirsp_on_value_write(p_ota,p_evt_write);
    }
    else if(p_evt_write->handle == p_ota->write_worsp_handle.value_handle)
    {
        worsp_on_value_write(p_ota,p_evt_write);
    }
    else if(p_evt_write->handle == p_ota->indicate_handle.cccd_handle)
    {
        ota_on_cccd_write(p_ota, p_evt_write);
    }
    else
    {
        // Do Nothing
    }
}


/**@brief Function for handling the HVC event.
*
* @details Handles HVC events from the BLE stack.
*
* @param[in]   p_ota     ota Service structure.
* @param[in]   p_ble_evt   Event received from the BLE stack.
*/
static void ota_on_hvc(ble_ota_t * p_ota, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_hvc_t * p_hvc = &p_ble_evt->evt.gatts_evt.params.hvc;

    if(p_hvc->handle == p_ota->indicate_handle.value_handle)
    {
        ble_ota_evt_t evt;

        evt.evt_type = BLE_OTA_EVT_INDICATION_CONFIRMED;
        p_ota->evt_handler(p_ota, &evt);
    }
}


/**@brief       Function for adding TX characteristic.
 *
 * @param[in]   p_ota        Nordic UART Service structure.
 * @param[in]   p_ota_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t indicate_char_add(ble_ota_t * p_ota, const ble_ota_init_t * p_ota_init)
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
    ble_uuid.uuid               = BLE_UUID_OTA_INDICATE_CHAR;

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
    attr_char_value.max_len     = BLE_OTA_MAX_RX_CHAR_LEN;
    return sd_ble_gatts_characteristic_add(p_ota->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ota->indicate_handle);
    /**@snippet [Adding proprietary characteristic to S110 SoftDevice] */
}


/**@brief       Function for adding write with response characteristic.
 *
 * @param[in]   p_ota        Nordic UART Service structure.
 * @param[in]   p_ota_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t write_wirsp_char_add(ble_ota_t * p_ota, const ble_ota_init_t * p_ota_init)
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

    char_md.char_props.write    = 1;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf        = NULL;
    char_md.p_user_desc_md   = NULL;
    char_md.p_cccd_md        = &cccd_md;
    char_md.p_sccd_md        = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_OTA_WRITE_WIRSP_CHAR);

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

    return sd_ble_gatts_characteristic_add(p_ota->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ota->write_wirsp_handle);
}


/**@brief       Function for adding write without response characteristic.
 *
 * @param[in]   p_ota        Nordic UART Service structure.
 * @param[in]   p_ota_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t write_worsp_char_add(ble_ota_t * p_ota, const ble_ota_init_t * p_ota_init)
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

    char_md.char_props.write_wo_resp = 1;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf        = NULL;
    char_md.p_user_desc_md   = NULL;
    char_md.p_cccd_md        = &cccd_md;
    char_md.p_sccd_md        = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_OTA_WRITE_WORSP_CHAR);

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

    return sd_ble_gatts_characteristic_add(p_ota->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_ota->write_worsp_handle);
}

/**@brief       ota Service BLE event handler.
 *
 * @details     The ota service expects the application to call this function each time an
 *              event is received from the S110 SoftDevice. This function processes the event if it
 *              is relevant for it and calls the Nordic UART Service event handler of the
 *              application if necessary.
 *
 * @param[in]   p_ota    ota Service structure.
 * @param[in]   p_ble_evt  Event received from the S110 SoftDevice.
 */
void ble_ota_on_ble_evt(ble_ota_t * p_ota, ble_evt_t * p_ble_evt)
{
    if((p_ota == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    switch(p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_ota_connect(p_ota, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_ota_disconnect(p_ota, p_ble_evt);
            break;
			
        case BLE_GATTS_EVT_WRITE:
            on_ota_write(p_ota, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVC:
            ota_on_hvc(p_ota, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

/**@brief       Function for initializing the ota Service.
 *
 * @param[out]  p_ota       ota Service structure. This structure will have to be supplied
 *                          by the application. It will be initialized by this function and will
 *                          later be used to identify this particular service instance.
 * @param[in]   p_ota_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 *              This function returns NRF_ERROR_NULL if either of the pointers p_ota or p_ota_init
 *              is NULL.
 */
uint32_t ble_ota_init(ble_ota_t * p_ota, const ble_ota_init_t * p_ota_init)
{
    uint32_t        err_code;
    ble_uuid_t      ble_uuid;

    if((p_ota == NULL) || (p_ota_init == NULL))
    {
        return NRF_ERROR_NULL;
    }

    // Initialize service structure.
    p_ota->conn_handle              = BLE_CONN_HANDLE_INVALID;
    p_ota->evt_handler              = p_ota_init->evt_handler;

    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    ble_uuid.type = BLE_UUID_TYPE_BLE;
    ble_uuid.uuid = BLE_UUID_OTA_SERVICE;

    // Add service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_ota->service_handle);
    /**@snippet [Adding proprietary Service to S110 SoftDevice] */
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Indicate Characteristic.
    err_code = indicate_char_add(p_ota, p_ota_init);
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Write with response Characteristic.
    err_code = write_wirsp_char_add(p_ota, p_ota_init);
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add Write without response Characteristic.
    err_code = write_worsp_char_add(p_ota, p_ota_init);
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

/**@brief       Function for sending a string to the peer.
 *
 * @details     This function will send the input string as a RX characteristic notification to the
 *              peer.
  *
 * @param[in]   p_ota          Pointer to the Nordic UART Service structure.
 * @param[in]   string         String to be sent.
 * @param[in]   length         Length of string.
 *
 * @return      NRF_SUCCESS if the DFU Service has successfully requested the S110 SoftDevice to
 *              send the notification. Otherwise an error code.
 *              This function returns NRF_ERROR_INVALID_STATE if the device is not connected to a
 *              peer or if the notification of the RX characteristic was not enabled by the peer.
 *              It returns NRF_ERROR_NULL if the pointer p_ota is NULL.
 */
uint32_t ble_ota_indicate_send(ble_ota_t * p_ota, uint8_t * string, uint16_t len)
{
    ble_gatts_hvx_params_t hvx_params;
    if(p_ota == NULL)
    {
        return NRF_ERROR_NULL;
    }

    if(p_ota->conn_handle == BLE_CONN_HANDLE_INVALID)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    if(len == 0)
    {
        return NRF_ERROR_INVALID_LENGTH;
    }

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = p_ota->indicate_handle.value_handle;
    hvx_params.p_data = string;
    hvx_params.p_len  = &len;
    hvx_params.type   = BLE_GATT_HVX_INDICATION;

    return sd_ble_gatts_hvx(p_ota->conn_handle, &hvx_params);
}
/**@brief    Function for handling the indicate from the transfer Service.
 *
 * @details  This function will process the data received from the Nordic UART BLE Service and send
 *           it to the UART module.
 */
/**@snippet [Handling the data received over BLE] */

void on_ota_evt(ble_ota_t * p_trans, ble_ota_evt_t *p_evt)
{
    switch(p_evt->evt_type)
    {
        case BLE_OTA_EVT_INDICATION_ENABLED:
        case BLE_OTA_EVT_INDICATION_CONFIRMED:
        case BLE_OTA_EVT_INDICATION_DISABLED:
			ota_indicate_statue_set(p_evt->evt_type);
        break;

        default:
            // No implementation needed.
            break;
    }
}

