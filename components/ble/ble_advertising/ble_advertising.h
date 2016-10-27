/* Copyright (c) 2015 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 */


/**@file
 *
 * @defgroup ble_sdk_lib_advertising Advertising Module
 * @{
 * @ingroup  ble_sdk_lib
 * @brief    Module for handling connectable BLE advertising.
 *
 * @details  The Advertising Module handles connectable advertising for your application. It can 
 *           be configured with advertising modes to suit most typical use cases.
 *           Your main application can react to changes in advertising modes
 *           if an event handler is provided.
 *
 * @note     The Advertising Module supports only applications with a single peripheral link.
 *
 * The application must propagate BLE stack events to this module by calling
 * @ref ble_advertising_on_ble_evt() and system events by calling
 *
 */

#ifndef BLE_ADVERTISING_H__
#define BLE_ADVERTISING_H__

#include <stdint.h>
#include "ble_gattc.h"
#include "ble.h"
#include "nrf_error.h"
#include "ble_advdata.h"

/**@brief Advertising modes.
*/
typedef enum
{
    BLE_ADV_MODE_IDLE,          /**< Idle; no connectable advertising is ongoing.*/
    BLE_ADV_MODE_DIRECTED,      /**< Directed advertising attempts to connect to the most recently disconnected peer. */
    BLE_ADV_MODE_DIRECTED_SLOW, /**< Directed advertising (low duty cycle) attempts to connect to the most recently disconnected peer. */
    BLE_ADV_MODE_FAST,          /**< Fast advertising will connect to any peer device, or filter with a whitelist if one exists. */
    BLE_ADV_MODE_SLOW,          /**< Slow advertising is similar to fast advertising. By default, it uses a longer advertising interval and time-out than fast advertising. However, these options are defined by the user. */
} ble_adv_mode_t;

/**@brief Advertising events.
 *
 * @details These events are propagated to the main application if a handler was provided during
 *          initialization of the Advertising Module. Events for modes that are not used can be
 *          ignored. Similarly, BLE_ADV_EVT_WHITELIST_REQUEST and BLE_ADV_EVT_PEER_ADDR_REQUEST
 *          can be ignored if whitelist and direct advertising is not used.
 */
typedef enum
{
    BLE_ADV_EVT_IDLE,                /**< Idle; no connectable advertising is ongoing.*/
    BLE_ADV_EVT_DIRECTED,            /**< Direct advertising mode has started. */
    BLE_ADV_EVT_DIRECTED_SLOW,       /**< Directed advertising (low duty cycle) has started. */
    BLE_ADV_EVT_FAST,                /**< Fast advertising mode has started. */
    BLE_ADV_EVT_SLOW,                /**< Slow advertising mode has started.*/
    BLE_ADV_EVT_FAST_WHITELIST,      /**< Fast advertising mode using the whitelist has started. */
    BLE_ADV_EVT_SLOW_WHITELIST,      /**< Slow advertising mode using the whitelist has started.*/
    BLE_ADV_EVT_WHITELIST_REQUEST,   /**< Request a whitelist from the main application. For whitelist advertising to work, the whitelist must be set when this event occurs. */
    BLE_ADV_EVT_PEER_ADDR_REQUEST    /**< Request a peer address from the main application. For directed advertising to work, the peer address must be set when this event occurs. */
} ble_adv_evt_t;

/**@brief Options for the different advertisement modes.
 *
 * @details This structure is used to enable or disable advertising modes and to configure time-out
 *          periods and advertising intervals.
 */
typedef struct
{
    bool     ble_adv_whitelist_enabled;       /**< Enable or disable use of the whitelist. */
    bool     ble_adv_directed_enabled;        /**< Enable or disable direct advertising mode. */
    bool     ble_adv_directed_slow_enabled;   /**< Enable or disable direct advertising mode. */
    uint32_t ble_adv_directed_slow_interval;  /**< Advertising interval for directed advertising. */
    uint32_t ble_adv_directed_slow_timeout;   /**< Time-out (number of tries) for direct advertising. */
    bool     ble_adv_fast_enabled;            /**< Enable or disable fast advertising mode. */
    uint32_t ble_adv_fast_interval;           /**< Advertising interval for fast advertising. */
    uint32_t ble_adv_fast_timeout;            /**< Time-out (in seconds) for fast advertising. */
    bool     ble_adv_slow_enabled;            /**< Enable or disable slow advertising mode. */
    uint32_t ble_adv_slow_interval;           /**< Advertising interval for slow advertising. */
    uint32_t ble_adv_slow_timeout;            /**< Time-out (in seconds) for slow advertising. */
}ble_adv_modes_config_t;


/**@brief BLE advertising event handler type. */
typedef void (*ble_advertising_evt_handler_t) (ble_adv_evt_t const adv_evt);

/**@brief BLE advertising error handler type. */
typedef void (*ble_advertising_error_handler_t) (uint32_t nrf_error);

/**@brief Initialization parameters for the Advertising Module.
 * @details This structure is used to pass advertising options, advertising data, and an event handler to the Advertising Module during initialization. */
typedef struct
{
    ble_adv_modes_config_t        options;     /**< Parameters for advertising modes.*/
    ble_advdata_t                 advdata;     /**< Advertising data. */
    ble_advertising_evt_handler_t evt_handler; /**< Event handler. */
}ble_adv_init_t;


/* Defines to make the mode options easier to set during advertising init.*/
#define BLE_ADV_DIRECTED_ENABLED       true
#define BLE_ADV_DIRECTED_DISABLED      false

#define BLE_ADV_DIRECTED_SLOW_ENABLED  true
#define BLE_ADV_DIRECTED_SLOW_DISABLED false

#define BLE_ADV_FAST_ENABLED           true
#define BLE_ADV_FAST_DISABLED          false
    
#define BLE_ADV_SLOW_ENABLED           true
#define BLE_ADV_SLOW_DISABLED          false

#define BLE_ADV_WHITELIST_ENABLED      true
#define BLE_ADV_WHITELIST_DISABLED     false


/**@brief Function for initializing the Advertising Module.
 *
 * @details Encodes the required advertising data and passes it to the stack.
 *          Also builds a structure to be passed to the stack when starting advertising.
 *          The supplied advertising data is copied to a local structure and is manipulated
 *          depending on what advertising modes are started in @ref ble_advertising_start.
 *
 * @param[in] p_advdata     Advertising data: name, appearance, discovery flags, and more.
 * @param[in] p_srdata      Scan response data: Supplement to advertising data.
 * @param[in] p_config      Select which advertising modes and intervals will be utilized.
 * @param[in] evt_handler   Event handler that will be called upon advertising events.
 * @param[in] error_handler Error handler that will propogate internal errors to the main applications.
 *
 * @retval NRF_SUCCESS If initialization was successful. Otherwise, an error code is returned.
 */
uint32_t ble_advertising_init(ble_advdata_t const                 * p_advdata,
                              ble_advdata_t const                 * p_srdata,
                              ble_adv_modes_config_t const        * p_config);

void advertising_parm_configer(ble_gap_adv_params_t *adv_params);

void advertising_start(void);

void advertising_stop(void);


/** @} */

#endif // BLE_ADVERTISING_H__

/** @} */
