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

#ifndef BLE_TRANS_H__
#define BLE_TRANS_H__

#include "ble.h"
#include "ble_srv_common.h"
#include <stdint.h>
#include <stdbool.h>

#define BLE_TRANS_MAX_DATA_LEN            (GATT_MTU_SIZE_DEFAULT - 3)  /**< Maximum length of data (in bytes) that can be transmitted by the Nordic UART service module to the peer. */

#define BLE_TRANS_MAX_RX_CHAR_LEN         BLE_TRANS_MAX_DATA_LEN       /**< Maximum length of the RX Characteristic (in bytes). */
#define BLE_TRANS_MAX_TX_CHAR_LEN         20                           /**< Maximum length of the TX Characteristic (in bytes). */

/**@brief Wechat Service event type. */
typedef enum
{
    BLE_TRANS_EVT_INDICATION_ENABLED,                                         /**< transfer value indication enabled event. */
    BLE_TRANS_EVT_INDICATION_DISABLED,                                        /**< transfer value indication disabled event. */
    BLE_TRANS_EVT_INDICATION_CONFIRMED                                        /**< Confirmation of a transfer measurement indication has been received. */
} ble_trans_evt_type_t;

/**@brief Transfer Service event. */
typedef struct
{
    ble_trans_evt_type_t evt_type;                                            /**< Type of event. */
} ble_trans_evt_t;

// Forward declaration of the ble_trans_t type. 
typedef struct ble_trans_s ble_trans_t;

/**@brief transfer Service event handler type. */
typedef void (*ble_trans_evt_handler_t) (ble_trans_t * p_trans, ble_trans_evt_t * p_evt);


/**@brief   transfer Service init structure.
 *
 * @details This structure contains the initialization information for the service. The application
 *          needs to fill this structure and pass it to the service using the @ref ble_trans_init
 *          function.
 */
typedef struct
{
    ble_trans_evt_handler_t    evt_handler;            /**< Event handler to be called for confirm data received. */
} ble_trans_init_t;

/**@brief   transfer Service structure.
 *
 * @details This structure contains status information related to the service.
 */
typedef struct ble_trans_s
{
    uint8_t                  uuid_type;                   /**< UUID type for Nordic UART Service Base UUID. */
    uint16_t                 service_handle;              /**< Handle of Nordic UART Service (as provided by the S110 SoftDevice). */
    ble_gatts_char_handles_t indicate_handle;             /**< Handles related to the TX characteristic. (as provided by the S110 SoftDevice)*/
    ble_gatts_char_handles_t write_handle;                /**< Handles related to the RX characteristic. (as provided by the S110 SoftDevice)*/
    ble_gatts_char_handles_t notify_handles[6];           /**< Handles related to the RX characteristic. (as provided by the S110 SoftDevice)*/
    uint16_t                 notification_enabled;        /**< Variable to indicate if the peer has enabled notification of the RX characteristic.*/
	uint16_t                 conn_handle;                 /**< Handle of the current connection (as provided by the S110 SoftDevice). This will be BLE_CONN_HANDLE_INVALID if not in a connection. */
    ble_trans_evt_handler_t  evt_handler;                 /**< Event handler to be called for confirm data received. */
} ble_trans_t;
extern ble_trans_t    m_trans;
/**@brief       Function for initializing the transfer Service.
 *
 * @param[out]  p_trans     transfer Service structure. This structure will have to be supplied
 *                          by the application. It will be initialized by this function and will
 *                          later be used to identify this particular service instance.
 * @param[in]   p_trans_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 *              This function returns NRF_ERROR_NULL if either of the pointers p_trans or p_trans_init
 *              is NULL.
 */
uint32_t ble_trans_init(ble_trans_t * p_trans, const ble_trans_init_t * p_trans_init);

/**@brief       transfer Service BLE event handler.
 *
 * @details     The transfer service expects the application to call this function each time an
 *              event is received from the S110 SoftDevice. This function processes the event if it
 *              is relevant for it and calls the Nordic UART Service event handler of the
 *              application if necessary.
 *
 * @param[in]   p_trans    transfer Service structure.
 * @param[in]   p_ble_evt  Event received from the S110 SoftDevice.
 */
void ble_trans_on_ble_evt(ble_trans_t * p_trans, ble_evt_t * p_ble_evt);

/**@brief       Function for sending a string to the peer.
 *
 * @details     This function will send the input string as a RX characteristic notification to the
 *              peer.
  *
 * @param[in]   p_trans          Pointer to the Nordic UART Service structure.
 * @param[in]   string         String to be sent.
 * @param[in]   length         Length of string.
 *
 * @return      NRF_SUCCESS if the DFU Service has successfully requested the S110 SoftDevice to
 *              send the notification. Otherwise an error code.
 *              This function returns NRF_ERROR_INVALID_STATE if the device is not connected to a
 *              peer or if the notification of the RX characteristic was not enabled by the peer.
 *              It returns NRF_ERROR_NULL if the pointer p_trans is NULL.
 */
uint32_t ble_trans_notify_send(ble_trans_t * p_trans, uint8_t chnl, uint8_t * string);

uint32_t ble_trans_indicate_send(ble_trans_t * p_trans, uint8_t * string);

void on_trans_evt(ble_trans_t * p_trans, ble_trans_evt_t *p_evt);
#endif // BLE_TRANS_H__

/** @} */
