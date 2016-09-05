//*****************************************************************************
//
//! @file lbe_ota_apollo.h
//!
//! @brief An example to handle apollo ota
//
//
//*****************************************************************************

#ifndef BLE_OTA_APOLLO_H
#define BLE_OTA_APOLLO_H

#include "ble.h"
#include "ble_srv_common.h"
#include <stdint.h>


#define BLE_OTA_MAX_DATA_LEN            (GATT_MTU_SIZE_DEFAULT - 3)  /**< Maximum length of data (in bytes) that can be transmitted by the Nordic UART service module to the peer. */

#define BLE_OTA_MAX_RX_CHAR_LEN         BLE_OTA_MAX_DATA_LEN      /**< Maximum length of the RX Characteristic (in bytes). */
#define BLE_OTA_MAX_TX_CHAR_LEN         20                           /**< Maximum length of the TX Characteristic (in bytes). */

/**@brief Wechat Service event type. */
typedef enum
{
    BLE_OTA_EVT_INDICATION_ENABLED,                                  /**< transfer value indication enabled event. */
    BLE_OTA_EVT_INDICATION_DISABLED,                                 /**< transfer value indication disabled event. */
    BLE_OTA_EVT_INDICATION_CONFIRMED                                 /**< Confirmation of a transfer measurement indication has been received. */
} ble_ota_evt_type_t;

/**@brief ota Service event. */
typedef struct
{
    ble_ota_evt_type_t evt_type;                                     /**< Type of event. */
} ble_ota_evt_t;

// Forward declaration of the ble_ota_t type.
typedef struct ble_ota_s ble_ota_t;

/**@brief transfer Service event handler type. */
typedef void (*ble_ota_evt_handler_t)(ble_ota_t * p_ota, ble_ota_evt_t * p_evt);


/**@brief   transfer Service init structure.
 *
 * @details This structure contains the initialization information for the service. The application
 *          needs to fill this structure and pass it to the service using the @ref ble_ota_init
 *          function.
 */
typedef struct
{
    ble_ota_evt_handler_t    evt_handler;            /**< Event handler to be called for confirm data received. */
} ble_ota_init_t;

/**@brief   transfer Service structure.
 *
 * @details This structure contains status information related to the service.
 */
typedef struct ble_ota_s
{
    uint8_t                  uuid_type;                   /**< UUID type for Nordic UART Service Base UUID. */
    uint16_t                 service_handle;              /**< Handle of Nordic UART Service (as provided by the S110 SoftDevice). */
    ble_gatts_char_handles_t indicate_handle;             /**< Handles related to the TX characteristic. (as provided by the S110 SoftDevice)*/
    ble_gatts_char_handles_t write_wirsp_handle;          /**< Handles related to the RX characteristic. (as provided by the S110 SoftDevice)*/
    ble_gatts_char_handles_t write_worsp_handle;          /**< Handles related to the RX characteristic. (as provided by the S110 SoftDevice)*/
    uint16_t                 conn_handle;                 /**< Handle of the current connection (as provided by the S110 SoftDevice). This will be BLE_CONN_HANDLE_INVALID if not in a connection. */
    ble_ota_evt_handler_t evt_handler;                 /**< Event handler to be called for confirm data received. */
} ble_ota_t;

extern ble_ota_t      m_ota;

/**@brief       Function for initializing the ota Service.
 *
 * @param[out]  p_ota     transfer Service structure. This structure will have to be supplied
 *                          by the application. It will be initialized by this function and will
 *                          later be used to identify this particular service instance.
 * @param[in]   p_ota_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 *              This function returns NRF_ERROR_NULL if either of the pointers p_ota or p_ota_init
 *              is NULL.
 */
uint32_t ble_ota_init(ble_ota_t * p_ota, const ble_ota_init_t * p_ota_init);

/**@brief       ota Service BLE event handler.
 *
 * @details     The transfer service expects the application to call this function each time an
 *              event is received from the S110 SoftDevice. This function processes the event if it
 *              is relevant for it and calls the Nordic UART Service event handler of the
 *              application if necessary.
 *
 * @param[in]   p_ota    transfer Service structure.
 * @param[in]   p_ble_evt  Event received from the S110 SoftDevice.
 */
void ble_ota_on_ble_evt(ble_ota_t * p_ota, ble_evt_t * p_ble_evt);

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

uint32_t ble_ota_indicate_send(ble_ota_t * p_ota, uint8_t * string, uint16_t len);
void on_ota_evt(ble_ota_t * p_trans, ble_ota_evt_t *p_evt);
#endif
