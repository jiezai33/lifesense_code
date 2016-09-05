/***********************************************************************************
 * 文 件 名   	: ble_ancs_android.h
 * 创 建 者   	: zhangfei
 * 创建日期   : 2016年3月24日
 * 版权说明   : Copyright (c) 2016-2025   广东乐心医疗电子股份有限公司
 * 文件描述   : ble_ancs_android.c 头文件
 * 修改历史   : 
 *					1 201603024 by zhangfei, 增加程序注释
***********************************************************************************/
#ifndef BLE_ANCS_ANDRIOD_H__
#define BLE_ANCS_ANDRIOD_H__

#include "ble.h"
#include "ble_srv_common.h"

typedef enum
{
    BLE_ANDROID_ANCS_EVT_INDICATION_ENABLED,                                         /**< Wechat value indication enabled event. */
    BLE_ANDROID_ANCS_EVT_INDICATION_DISABLED,                                        /**< Wechat value indication disabled event. */
    BLE_ANDROID_ANCS_EVT_INDICATION_CONFIRMED                                        /**< Confirmation of a Wechat measurement indication has been received. */
} ble_android_ancs_evt_type_t;

// Forward declaration of the ble_bps_t type. 
typedef struct ble_android_ancs_s ble_android_ancs_t;

/**@brief Wechat Service event. */
typedef struct
{
    ble_android_ancs_evt_type_t evt_type;                                            /**< Type of event. */
} ble_android_ancs_evt_t;

typedef void (*ble_android_ancs_evt_handler_t) (ble_android_ancs_t * p_android_ancs, ble_android_ancs_evt_t * p_evt);

typedef struct ble_android_ancs_s
{
    ble_android_ancs_evt_handler_t     evt_handler;                               /**< Event handler to be called for handling events in the Wechat Service. */
    uint16_t                     service_handle;                            /**< Handle of Wechat Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t     write_handles;                              /**< Handles related to the Wechat Write characteristic. */
    ble_gatts_char_handles_t     indicate_handles;                           /**< Handles related to the Wechat Indicate characteristic. */
    ble_gatts_char_handles_t     read_handles;                           		/**< Handles related to the Wechat Read characteristic. */
    uint16_t                     conn_handle;                               /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                      *data;                                   	/**< Value of read. */
} ble_android_ancs_t;

typedef struct
{
    ble_android_ancs_evt_handler_t        evt_handler;                               /**< Event handler to be called for handling events in the Blood Pressure Service. */
    ble_srv_cccd_security_mode_t wechat_meas_attr_md;                          /**< Initial security level for blood pressure measurement attribute */
    ble_srv_security_mode_t      wechat_userinfo_attr_md;                       /**< Initial security level for blood pressure feature attribute */
    //		#ifdef	AUTH_MAC
    ble_srv_security_mode_t      		wechat_read_attr_md;                    	 /**< Initial security level for wechat read attribute */
    uint8_t                     		*data;																		 /**< Initial value for wechat read attribute */
//		#endif
} ble_android_ancs_init_t;

extern ble_android_ancs_t          m_android_ancs;

extern uint32_t ble_Ancs_init(ble_android_ancs_t * p_android_ancs, const ble_android_ancs_init_t * p_wechat_init);
extern void ble_ancs_on_ble_evt(ble_android_ancs_t * p_android_ancs, ble_evt_t * p_ble_evt);
extern void on_Ancs_evt(ble_android_ancs_t * p_android_ancs, ble_android_ancs_evt_t *p_evt);

uint32_t ble_android_ancs_indicate_send(ble_android_ancs_t * p_android_ancs, uint8_t *data);

#endif

