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
 * @defgroup ble_sdk_srv_bps Blood Pressure Service
 * @{
 * @ingroup ble_sdk_srv
 * @brief Blood Pressure Service module.
 *
 * @details This module implements the Blood Pressure Service.
 *
 *          If an event handler is supplied by the application, the Blood Pressure 
 *          Service will generate Blood Pressure Service events to the application.
 *
 * @note The application must propagate BLE stack events to the Blood Pressure Service
 *       module by calling ble_bps_on_ble_evt() from the from the @ref ble_stack_handler function.
 *
 * @note Attention! 
 *  To maintain compliance with Nordic Semiconductor ASA Bluetooth profile 
 *  qualification listings, this section of source code must not be modified.
 */

#ifndef BLE_WECHAT_H__
#define BLE_WECHAT_H__



#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"
#include "ble_date_time.h"
//////////////////////////////////////////////////////////////////////////////////////////////
#define         WAIT_OPEN_WECHAT_TIME                                   30    //uint:s
#define OPCODE_LENGTH  1                                                    /**< Length of opcode inside Blood Pressure Measurement packet. */
#define HANDLE_LENGTH  2                                                    /**< Length of handle inside Blood Pressure Measurement packet. */
#define MAX_BPM_LEN    (BLE_L2CAP_MTU_DEF - OPCODE_LENGTH - HANDLE_LENGTH)  /**< Maximum size of a transmitted Blood Pressure Measurement. */
/**@brief Wechat Service event type. */
typedef enum
{
    BLE_WECHAT_EVT_INDICATION_ENABLED,                                         /**< Wechat value indication enabled event. */
    BLE_WECHAT_EVT_INDICATION_DISABLED,                                        /**< Wechat value indication disabled event. */
    BLE_WECHAT_EVT_INDICATION_CONFIRMED                                        /**< Confirmation of a Wechat measurement indication has been received. */
} ble_wechat_evt_type_t;

/**@brief Wechat Service event. */
typedef struct
{
    ble_wechat_evt_type_t evt_type;                                            /**< Type of event. */
} ble_wechat_evt_t;

// Forward declaration of the ble_bps_t type. 
typedef struct ble_wechat_s ble_wechat_t;

/**@brief wechat Service event handler type. */
typedef void (*ble_wechat_evt_handler_t) (ble_wechat_t * p_wechat, ble_wechat_evt_t * p_evt);
/**@brief Wechat Service init structure. This contains all options and data
 *        needed for initialization of the service. */
typedef struct
{
    ble_wechat_evt_handler_t        evt_handler;                               /**< Event handler to be called for handling events in the Blood Pressure Service. */
    ble_srv_cccd_security_mode_t wechat_meas_attr_md;                          /**< Initial security level for blood pressure measurement attribute */
    ble_srv_security_mode_t      wechat_userinfo_attr_md;                       /**< Initial security level for blood pressure feature attribute */
    //		#ifdef	AUTH_MAC
    ble_srv_security_mode_t      		wechat_read_attr_md;                    	 /**< Initial security level for wechat read attribute */
    uint8_t                     		*data;																		 /**< Initial value for wechat read attribute */
//		#endif
} ble_wechat_init_t;

/**@brief Wechat Service structure. This contains various status information for
 *        the service. */
typedef struct ble_wechat_s
{
    ble_wechat_evt_handler_t        evt_handler;                               /**< Event handler to be called for handling events in the Wechat Service. */
    uint16_t                     service_handle;                            /**< Handle of Wechat Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t     write_handles;                              /**< Handles related to the Wechat Write characteristic. */
    ble_gatts_char_handles_t     indicate_handles;                           /**< Handles related to the Wechat Indicate characteristic. */
    ble_gatts_char_handles_t     read_handles;                           		/**< Handles related to the Wechat Read characteristic. */
    uint16_t                     conn_handle;                               /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                      *data;                                   	/**< Value of read. */
} ble_wechat_t;

/**@brief Wechat Service measurement structure. This contains */
typedef struct ble_wechat_meas_s
{
    uint8_t indicate_data[20];
} ble_wechat_meas_t;

extern ble_wechat_t   m_wechat;
uint32_t ble_wechat_init(ble_wechat_t * p_wechat, const ble_wechat_init_t * p_wechat_init);
void ble_wechat_on_ble_evt(ble_wechat_t * p_wechat, ble_evt_t * p_ble_evt);

uint32_t ble_wechat_indicate_send(ble_wechat_t * p_wechat, uint8_t *data);
void on_wechat_evt(ble_wechat_t * p_wechat, ble_wechat_evt_t *p_evt);
#endif // BLE_BPS_H__
