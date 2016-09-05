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

/** @file
 *
 * @defgroup ble_sdk_apple_notification_main main.c
 * @{
 * @ingroup ble_sdk_app_apple_notification
 * @brief Apple Notification Client Sample Application main file. Disclaimer: 
 * This client implementation of the Apple Notification Center Service can and 
 * will be changed at any time by Nordic Semiconductor ASA.
 *
 
 * Server implementations such as the ones found in iOS can be changed at any 
 * time by Apple and may cause this client implementation to stop working.
 *
 * This file contains the source code for a sample application using the Apple 
 * Notification Center Service Client.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include "ble_ancs_ios.h"
#include "ble_db_discovery.h"
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble_hci.h"
#include "ble_gap.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "device_manager.h"
#include "app_timer.h"
#include "pstorage.h"
#include "nrf_soc.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
#include "softdevice_handler.h"
#include "nrf_delay.h"
#include "app_scheduler.h"
#include "app_timer_appsh.h"
#include "debug.h"
#include "usr_reminder.h"
#include "ble_wechat.h"
#include "ble_ancs_android.h"
#include "ble_transfer.h"
#include "ble_ota.h"
#include "ble_dis.h"
#include "usr_design.h"
#include "usr_device.h"

#if BUTTONS_NUMBER < 2
#error "Not enough resources on board"
#endif

#define CENTRAL_LINK_COUNT              0                                           /**< The number of central links used by the application. When changing this number remember to adjust the RAM settings. */
#define PERIPHERAL_LINK_COUNT           1                                           /**< The number of peripheral links used by the application. When changing this number remember to adjust the RAM settings. */
#define VENDOR_SPECIFIC_UUID_COUNT      4                                           /**< The number of vendor specific UUIDs used by this example. */
#define DISPLAY_MESSAGE_BUTTON_ID       1                                           /**< Button used to request notification attributes. */

#define DEVICE_NAME                     "MT2.0"                                      /**< Name of the device. Will be included in the advertising data. */
#define APP_ADV_FAST_INTERVAL           40                                          /**< The advertising interval (in units of 0.625 ms). The default value corresponds to 25 ms. */
#define APP_ADV_SLOW_INTERVAL           3200                                        /**< Slow advertising interval (in units of 0.625 ms). The default value corresponds to 2 seconds. */
#define APP_ADV_FAST_TIMEOUT            180                                         /**< The advertising time-out in units of seconds. */
#define APP_ADV_SLOW_TIMEOUT            180                                         /**< The advertising time-out in units of seconds. */
#define ADV_INTERVAL_FAST_PERIOD        30                                          /**< The duration of the fast advertising period (in seconds). */

#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         6//5                                           /**< Size of timer operation queues. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(500, UNIT_1_25_MS)            /**< Minimum acceptable connection interval (0.5 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(1000, UNIT_1_25_MS)           /**< Maximum acceptable connection interval (1 second). */
#define SLAVE_LATENCY                   0                                           /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)             /**< Connection supervisory time-out (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000, APP_TIMER_PRESCALER)  /**< Time from initiating an event (connect or start of notification) to the first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000, APP_TIMER_PRESCALER) /**< Time between each call to sd_ble_gap_conn_param_update after the first (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                           /**< Number of attempts before giving up the connection parameter negotiation. */

#define MESSAGE_BUFFER_SIZE             18                                          /**< Size of buffer holding optional messages in notifications. */

#define SECURITY_REQUEST_DELAY          APP_TIMER_TICKS(1500, APP_TIMER_PRESCALER)  /**< Delay after connection until security request is sent, if necessary (ticks). */
#define FIND_ANCS_SERVER_REQUEST_DELAY  APP_TIMER_TICKS(1500, APP_TIMER_PRESCALER)  /**< Delay after connection until security request is sent, if necessary (ticks). */

#define SEC_PARAM_BOND                  1                                           /**< Perform bonding. */
#define SEC_PARAM_MITM                  0                                           /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                  0                                           /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS              0                                           /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                        /**< No I/O capabilities. */
#define SEC_PARAM_OOB                   0                                           /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE          7                                           /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE          16                                          /**< Maximum encryption key size. */

#define DEAD_BEEF                       0xDEADBEEF                                  /**< Value used as error code on stack dump. Can be used to identify stack location on stack unwind. */

#define SCHED_MAX_EVENT_DATA_SIZE       MAX(APP_TIMER_SCHED_EVT_SIZE, \
                                            BLE_STACK_HANDLER_SCHED_EVT_SIZE) /**< Maximum size of scheduler events. */
#define SCHED_QUEUE_SIZE                10                                    /**< Maximum number of events in the scheduler queue. */

#define MANUFACTURER_NAME          "lifesense"                      /**< Manufacturer. Will be passed to Device Information Service. */

ble_ancs_c_t              		m_ios_ancs;                                 /**< Structure used to identify the Apple Notification Service Client. */
static ble_db_discovery_t        m_ble_db_discovery;                       /**< Structure used to identify the DB Discovery module. */

static dm_application_instance_t m_app_handle;                             /**< Application identifier allocated by the Device Manager. */
static dm_handle_t               m_peer_handle;                            /**< Identifies the peer that is currently connected. */
static ble_gap_sec_params_t      m_sec_param;                              /**< Security parameter for use in security requests. */
APP_TIMER_DEF(m_sec_req_timer_id);                                         /**< Security request timer. The timer lets us start pairing request if one does not arrive from the Central. */
APP_TIMER_DEF(m_ancs_server_find_timer_id);                                         /**< Security request timer. The timer lets us start pairing request if one does not arrive from the Central. */

extern ble_ancs_c_evt_notif_t m_notification_latest;                       /**< Local copy to keep track of the newest arriving notifications. */


uint16_t g_connect_handle;
/**@brief Callback function for handling asserts in the SoftDevice.
 *
 * @details This function is called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. 
 *          You must analyze how your product should react to asserts.
 * @warning On assert from the SoftDevice, the system can recover only on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for handling the security request timer time-out.
 *
 * @details This function is called each time the security request timer expires.
 *
 * @param[in] p_context  Pointer used for passing context information from the
 *                       app_start_timer() call to the time-out handler.
 */
static void sec_req_timeout_handler(void * p_context)
{
    
    uint32_t             err_code;
    dm_security_status_t status;

    if (m_peer_handle.connection_id != DM_INVALID_ID)
    {
        err_code = dm_security_status_req(&m_peer_handle, &status);
        APP_ERROR_CHECK(err_code);

        // If the link is still not secured by the peer, initiate security procedure.
        if (status == NOT_ENCRYPTED)
        {
            err_code = dm_security_setup_req(&m_peer_handle);
            APP_ERROR_CHECK(err_code);
        }
    }
}

static void ancs_find_timeout_handler(void * p_context)
{
    
    uint32_t             err_code;
	QPRINTF("ancs_find_timeout_handler\r\n");
	err_code = ble_db_discovery_start(&m_ble_db_discovery,g_connect_handle);
    APP_ERROR_CHECK(err_code);
}



/**@brief Function for initializing the timer module.
 */
static void timers_init(void)
{
    uint32_t err_code;

    // Initialize timer module, making it use the scheduler.
    APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, true);

    // Create security request timer.
    err_code = app_timer_create(&m_sec_req_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                sec_req_timeout_handler);
	
	// Create security request timer.
    err_code = app_timer_create(&m_ancs_server_find_timer_id,
                                APP_TIMER_MODE_SINGLE_SHOT,
                                ancs_find_timeout_handler);
	
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the GAP.
 *
 * @details Use this function to set up all necessary GAP (Generic Access Profile)
 *          parameters of the device. It also sets the permissions and appearance.
 */
static void gap_params_init(void)
{
    uint32_t                err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    uint32_t               err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = true;
    cp_init.evt_handler                    = NULL;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Database Discovery events.
 *
 * @details This function is a callback function to handle events from the database discovery module.
 *          Depending on the UUIDs that are discovered, this function should forward the events
 *          to their respective service instances.
 *
 * @param[in] p_event  Pointer to the database discovery event.
 */
static void db_disc_handler(ble_db_discovery_evt_t * p_evt)
{
    ble_ancs_c_on_db_disc_evt(&m_ios_ancs, p_evt);
}

/**@brief Function for handling the Device Manager events.
 *
 * @param[in] p_evt  Data associated to the Device Manager event.
 */
static uint32_t device_manager_evt_handler(dm_handle_t const * p_handle,
                                           dm_event_t const  * p_evt,
                                           ret_code_t          event_result)
{
    uint32_t err_code;

    switch (p_evt->event_id)
    {
        case DM_EVT_CONNECTION:
			QPRINTF("DM_EVT_CONNECTION\r\n");
            m_peer_handle = (*p_handle);

			g_connect_handle = p_evt->event_param.p_gap_param->conn_handle;
			err_code      = app_timer_start(m_ancs_server_find_timer_id, FIND_ANCS_SERVER_REQUEST_DELAY, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case DM_EVT_LINK_SECURED:
			QPRINTF("DM_EVT_LINK_SECURED\r\n");
            err_code = ble_db_discovery_start(&m_ble_db_discovery,
                                              p_evt->event_param.p_gap_param->conn_handle);
            APP_ERROR_CHECK(err_code);
            break; 

        default:
            break;

    }
    return NRF_SUCCESS;
}


/**@brief Function for initializing the Device Manager.
 *
 * @param[in] erase_bonds  Indicates whether bonding information should be cleared from
 *                         persistent storage during initialization of the Device Manager.
 */
static void device_manager_init(bool erase_bonds)
{
    uint32_t               err_code;
    dm_init_param_t        init_param = {.clear_persistent_data = erase_bonds};
    dm_application_param_t register_param;

    // Initialize persistent storage module.
    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    err_code = dm_init(&init_param);
    APP_ERROR_CHECK(err_code);

    memset(&register_param.sec_param, 0, sizeof(ble_gap_sec_params_t));

    register_param.sec_param.bond         = SEC_PARAM_BOND;
    register_param.sec_param.mitm         = SEC_PARAM_MITM;
    register_param.sec_param.lesc         = SEC_PARAM_LESC;
    register_param.sec_param.keypress     = SEC_PARAM_KEYPRESS;
    register_param.sec_param.io_caps      = SEC_PARAM_IO_CAPABILITIES;
    register_param.sec_param.oob          = SEC_PARAM_OOB;
    register_param.sec_param.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
    register_param.sec_param.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
    register_param.evt_handler            = device_manager_evt_handler;
    register_param.service_type           = DM_PROTOCOL_CNTXT_GATT_SRVR_ID;
    
    memcpy(&m_sec_param, &register_param.sec_param, sizeof(ble_gap_sec_params_t));

    err_code = dm_register(&m_app_handle, &register_param);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function is called for advertising events that are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    uint32_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_DIRECTED:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_DIRECTED);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_FAST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_FAST_WHITELIST:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_WHITELIST);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_SLOW:
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING_SLOW);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;

        case BLE_ADV_EVT_WHITELIST_REQUEST:
        {
            ble_gap_whitelist_t whitelist;
            ble_gap_addr_t    * p_whitelist_addr[BLE_GAP_WHITELIST_ADDR_MAX_COUNT];
            ble_gap_irk_t     * p_whitelist_irk[BLE_GAP_WHITELIST_IRK_MAX_COUNT];

            whitelist.addr_count = BLE_GAP_WHITELIST_ADDR_MAX_COUNT;
            whitelist.irk_count  = BLE_GAP_WHITELIST_IRK_MAX_COUNT;
            whitelist.pp_addrs   = p_whitelist_addr;
            whitelist.pp_irks    = p_whitelist_irk;

            err_code = dm_whitelist_create(&m_app_handle, &whitelist);
            APP_ERROR_CHECK(err_code);

            err_code = ble_advertising_whitelist_reply(&whitelist);
            APP_ERROR_CHECK(err_code);
            break;
        }

        default:
            break;
    }
}


/**@brief Function for handling the application's BLE stack events.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void on_ble_evt(ble_evt_t * p_ble_evt)
{
    uint32_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            QPRINTF("Connected.\r\n");
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            QPRINTF("Disconnected.\r\n");
            if (p_ble_evt->evt.gap_evt.conn_handle == m_ios_ancs.conn_handle)
            {
                m_ios_ancs.conn_handle = BLE_CONN_HANDLE_INVALID;
            }
            break;

        case BLE_GATTS_EVT_TIMEOUT:

            NRF_LOG("Timeout.\n\r");
            // Disconnect on GATT Server and Client time-out events.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:

            NRF_LOG("Timeout.\n\r");
            // Disconnect on GATT Server and Client time-out events.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // No implementation needed.
            break;
    }
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling events from the BSP module.
 *
 * @param[in] event  Event generated by button press.
 */
static void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_ios_ancs.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_WHITELIST_OFF:
            err_code = ble_advertising_restart_without_whitelist();
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        case BSP_EVENT_KEY_1:
            err_code = ble_ancs_c_request_attrs(&m_ios_ancs, &m_notification_latest);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
}


/**@brief Function for dispatching a BLE stack event to all modules with a BLE stack event handler.
 *
 * @details This function is called from the BLE Stack event interrupt handler after a BLE stack
 *          event has been received.
 *
 * @param[in] p_ble_evt  Bluetooth stack event.
 */
static void ble_evt_dispatch(ble_evt_t * p_ble_evt)
{
    dm_ble_evt_handler(p_ble_evt);
    ble_db_discovery_on_ble_evt(&m_ble_db_discovery, p_ble_evt);
    ble_conn_params_on_ble_evt(p_ble_evt);
    bsp_btn_ble_on_ble_evt(p_ble_evt);
    on_ble_evt(p_ble_evt);
    ble_advertising_on_ble_evt(p_ble_evt);
	
	ble_ancs_c_on_ble_evt(&m_ios_ancs, p_ble_evt);
	ble_wechat_on_ble_evt(&m_wechat, p_ble_evt);
    ble_ancs_on_ble_evt(&m_android_ancs, p_ble_evt);
    ble_trans_on_ble_evt(&m_trans, p_ble_evt);
    ble_ota_on_ble_evt(&m_ota,p_ble_evt);
}


/**@brief Function for dispatching a system event to interested modules.
 *
 * @details This function is called from the system event interrupt handler after a system
 *          event has been received.
 *
 * @param[in] sys_evt  System stack event.
 */
static void sys_evt_dispatch(uint32_t sys_evt)
{
    pstorage_sys_event_handler(sys_evt);
    ble_advertising_on_sys_evt(sys_evt);
}


/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
static void ble_stack_init(void)
{
    uint32_t err_code;
    
    nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;
    
    // Initialize the SoftDevice handler module.
    SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);
    
    ble_enable_params_t ble_enable_params;
    err_code = softdevice_enable_get_default_config(CENTRAL_LINK_COUNT,
                                                    PERIPHERAL_LINK_COUNT,
                                                    &ble_enable_params);
    APP_ERROR_CHECK(err_code);

    ble_enable_params.common_enable_params.vs_uuid_count = VENDOR_SPECIFIC_UUID_COUNT;

    //Check the ram settings against the used number of links
    CHECK_RAM_START_ADDR(CENTRAL_LINK_COUNT,PERIPHERAL_LINK_COUNT);
    
    // Enable BLE stack.
    err_code = softdevice_enable(&ble_enable_params);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for BLE events.
    err_code = softdevice_ble_evt_handler_set(ble_evt_dispatch);
    APP_ERROR_CHECK(err_code);

    // Register with the SoftDevice handler module for System events.
    err_code = softdevice_sys_evt_handler_set(sys_evt_dispatch);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the Apple Notification Center Service.
*/



/**@brief Function for initializing the advertising functionality.
 */
static void advertising_init(void)
{
    uint32_t      err_code;
    ble_advdata_t advdata;
    ble_advdata_t srdata;
	ble_advdata_manuf_data_t manuf_specific_data;
	uint8_t manuf_id[9] = {0};
    ble_uuid_t adv_uuids[] =
    {
        {BLE_UUID_WECHAT_SERVICE,     BLE_UUID_TYPE_BLE},
		{BLE_UUID_TRANS_SERVICE,     BLE_UUID_TYPE_BLE},
    };

	manuf_id[0]=0XFE;
    manuf_id[1]=0X01;
    manuf_id[2]=0X01;
    memcpy(&manuf_id[3], device_id_hex, sizeof(device_id_hex));
		
    manuf_specific_data.data.p_data = manuf_id;
    manuf_specific_data.data.size = sizeof(manuf_id);
		        
     // Build and set advertising data
    memset(&advdata, 0, sizeof(advdata));
    advdata.name_type                = BLE_ADVDATA_FULL_NAME;
    advdata.include_appearance       = false;
    advdata.flags                    = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;//BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;
    advdata.uuids_complete.uuid_cnt  = sizeof(adv_uuids) / sizeof(adv_uuids[0]);//0;
    advdata.uuids_complete.p_uuids   = adv_uuids;//NULL;
    advdata.uuids_solicited.uuid_cnt = 0;
    advdata.uuids_solicited.p_uuids  = NULL;
	manuf_specific_data.company_identifier = 0x0000;
    advdata.p_manuf_specific_data = &manuf_specific_data;
	
    memset(&srdata, 0, sizeof(srdata));
    srdata.name_type               = BLE_ADVDATA_NO_NAME;
    srdata.include_appearance      = false;
    srdata.uuids_complete.uuid_cnt = 0;//sizeof(adv_uuids) / sizeof(adv_uuids[0]);
    srdata.uuids_complete.p_uuids  = NULL;//adv_uuids;
    srdata.uuids_solicited.uuid_cnt = 0;//1;
    srdata.uuids_solicited.p_uuids  = NULL;//&ancs_uuid;

    ble_adv_modes_config_t options = {0};
    options.ble_adv_whitelist_enabled = BLE_ADV_WHITELIST_DISABLED;//BLE_ADV_WHITELIST_ENABLED;
    options.ble_adv_fast_enabled      = BLE_ADV_FAST_ENABLED;
    options.ble_adv_fast_interval     = APP_ADV_FAST_INTERVAL;
    options.ble_adv_fast_timeout      = APP_ADV_FAST_TIMEOUT;
    options.ble_adv_slow_enabled      = BLE_ADV_SLOW_ENABLED;
    options.ble_adv_slow_interval     = APP_ADV_SLOW_INTERVAL;
    options.ble_adv_slow_timeout      = APP_ADV_SLOW_TIMEOUT;

    err_code = ble_advertising_init(&advdata, &srdata, &options, on_adv_evt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing buttons and LEDs.
 *
 * @param[out] p_erase_bonds  True if the clear bonds button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                                 bsp_event_handler);

    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**@brief Function for initializing the Event Scheduler.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}


/**@brief Function for initializing the database discovery module.
 */
static void db_discovery_init(void)
{
    uint32_t err_code = ble_db_discovery_init(db_disc_handler);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    uint32_t err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}

void sys_start_pair_mode(void)
{
	uint32_t err_code;

	QPRINTF("system start pair mode\r\n\r\n");
	err_code      = app_timer_start(m_sec_req_timer_id, SECURITY_REQUEST_DELAY, NULL);
	APP_ERROR_CHECK(err_code);
}

void services_add(void)
{
    uint32_t         	err_code;
    ble_wechat_init_t 	wechat_init;
	ble_android_ancs_init_t android_ancs_init;
	ble_trans_init_t 	trans_init;
    ble_ota_init_t 		ota_init;


	// Initialize Wechat Service.
    memset(&wechat_init, 0, sizeof(wechat_init));
    wechat_init.evt_handler = on_wechat_evt;

    //wechat_init.data=device_mac.addr;

    // Here the sec level for the wechat Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&wechat_init.wechat_meas_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&wechat_init.wechat_meas_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&wechat_init.wechat_meas_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&wechat_init.wechat_read_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&wechat_init.wechat_read_attr_md.write_perm);

	//微信服务初始化
    err_code = ble_wechat_init(&m_wechat, &wechat_init);
    APP_ERROR_CHECK(err_code);

	// Initialize Wechat Service.
    memset(&android_ancs_init, 0, sizeof(android_ancs_init));
	// Here the sec level for the wechat Service can be changed/increased.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&android_ancs_init.wechat_meas_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&android_ancs_init.wechat_meas_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&android_ancs_init.wechat_meas_attr_md.write_perm);

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&android_ancs_init.wechat_read_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&android_ancs_init.wechat_read_attr_md.write_perm);
	
	//安卓来电服务初始化
    android_ancs_init.evt_handler = on_Ancs_evt;
    err_code = ble_Ancs_init(&m_android_ancs, &android_ancs_init);
    APP_ERROR_CHECK(err_code);

	//A500服务初始化
    trans_init.evt_handler  = on_trans_evt;
    err_code = ble_trans_init(&m_trans, &trans_init);

	//OTA服务初始化
    ota_init.evt_handler  = on_ota_evt;
    err_code = ble_ota_init(&m_ota, &ota_init);
    APP_ERROR_CHECK(err_code);

	
}

void device_informayion_server_add(void)
{
	uint32_t         	err_code;
	ble_dis_init_t   	dis_init;
	ble_dis_sys_id_t 	sys_id;

	// Initialize Device Information Service.
    char fw_ver[5]={'t','e','s','t','F'};
    char hw_ver[5]={'t','e','s','t','H'};
	char all_ver[20]={'t','e','s','t','_','V'};
    char product_type[6]={'t','y','p','e'};
    memset(&dis_init, 0, sizeof(dis_init));
	
    ble_srv_ascii_to_utf8(&dis_init.manufact_name_str, MANUFACTURER_NAME);
    //get_product_type(product_type);
    ble_srv_ascii_to_utf8(&dis_init.model_num_str,product_type);
    //ble_srv_ascii_to_utf8(&dis_init.serial_num_str,(char *)mac_ascii);
    //get_software_version(fw_ver);
    ble_srv_ascii_to_utf8(&dis_init.fw_rev_str,fw_ver);
    //get_hardware_verion(hw_ver);
    ble_srv_ascii_to_utf8(&dis_init.hw_rev_str,hw_ver);
    //get_all_software_version(all_ver);
    ble_srv_ascii_to_utf8(&dis_init.sw_rev_str,all_ver);

	sys_id.manufacturer_id            = 1;
    sys_id.organizationally_unique_id = 2;
    //---------------------------------------------------
    dis_init.p_sys_id                 = &sys_id;
		
		
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dis_init.dis_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&dis_init.dis_attr_md.write_perm);

    err_code = ble_dis_init(&dis_init);
    APP_ERROR_CHECK(err_code);
}

#define MOTOR_PIN                13

#define PWM_OUTPUT_PIN_NUMBER    (MOTOR_PIN)    /**< Pin number for PWM output. */

void motor_init(void)
{
    // 设置引脚为输出下拉
    nrf_gpio_cfg_output(PWM_OUTPUT_PIN_NUMBER);
    nrf_gpio_pin_clear(PWM_OUTPUT_PIN_NUMBER);
}


/**@brief Function for application main entry.
 */
int main(void)
{
    bool erase_bonds;

    // Initialize.
#if DEBUG_TYPE == DEBUG_UART_TYPE
    debug_init();
	QPRINTF("UART init...\r\n");
#elif DEBUG_TYPE == DEBUG_UART_TYPE
    ret_code_t errCode = NRF_LOG_INIT();
    APP_ERROR_CHECK(errCode);
	QPRINTF("RTT init...\r\n");
#endif
    timers_init();
    buttons_leds_init(&erase_bonds);
    ble_stack_init();
	
	device_id_init();
	motor_init();
    device_manager_init(erase_bonds);
    db_discovery_init();
    scheduler_init();
    gap_params_init();
    ios_ancs_service_init();
	services_add();
	device_informayion_server_add();
    advertising_init();
    conn_params_init();

    // Start execution.
    advertising_start();

    QPRINTF("device name: <%s>  start....\r\n",DEVICE_NAME);
    // Enter main loop.
    for (;;)
    {
        app_sched_execute();
		remaind_do();
		
		trans_evt_call_back();
		usrdesign_send_data();	
		
        power_manage();

		
		
    }

}


/**
 * @}
 */

