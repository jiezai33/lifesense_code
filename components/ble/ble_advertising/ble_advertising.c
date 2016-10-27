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

#include "app_error.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "nrf_soc.h"
#include "nrf_log.h"
#include "pstorage.h"
#include "fstorage.h"
#include "sdk_common.h"

static ble_gap_addr_t                  m_peer_address;     /**< Address of the most recently connected peer, used for direct advertising. */
static ble_advdata_t                   m_advdata;          /**< Used by the initialization function to set name, appearance, and UUIDs and advertising flags visible to peer devices. */
static ble_advdata_manuf_data_t        m_manuf_specific_data;                      /**< Manufacturer specific data structure*/
static uint8_t                         m_manuf_data_array[BLE_GAP_ADV_MAX_SIZE];   /**< Array to store the Manufacturer specific data*/
static ble_advdata_service_data_t      m_service_data;                             /**< Service data structure. */
static uint8_t                         m_service_data_array[BLE_GAP_ADV_MAX_SIZE]; /**< Array to store the service data. */
static ble_advdata_conn_int_t          m_slave_conn_int;                           /**< Connection interval range structure.*/
static int8_t                          m_tx_power_level;                           /**< TX power level*/

ble_gap_adv_params_t g_adv_params;

/**@brief Function for setting the stored peer address back to zero.
 */
static void ble_advertising_peer_address_clear()
{
    memset(&m_peer_address, 0, sizeof(m_peer_address));
}

uint32_t ble_advertising_init(ble_advdata_t const                 * p_advdata,
                              ble_advdata_t const                 * p_srdata,
                              ble_adv_modes_config_t const        * p_config)
{
    uint32_t err_code;

    VERIFY_PARAM_NOT_NULL(p_advdata);
    VERIFY_PARAM_NOT_NULL(p_config);

    ble_advertising_peer_address_clear();

    // Copy and set advertising data.
    memset(&m_advdata, 0, sizeof(m_advdata));

    // Copy advertising data.
    m_advdata.name_type            = p_advdata->name_type;
    m_advdata.include_appearance   = p_advdata->include_appearance;
    m_advdata.flags                = p_advdata->flags;
    m_advdata.short_name_len       = p_advdata->short_name_len;
   /* 
    if(p_advdata->uuids_complete != NULL)
    {
        m_advdata.uuids_complete = p_advdata->uuids_complete;
    }
    */
    m_advdata.uuids_complete       = p_advdata->uuids_complete;
    m_advdata.uuids_more_available = p_advdata->uuids_more_available;
    m_advdata.uuids_solicited      = p_advdata->uuids_solicited;
    
    if(p_advdata->p_manuf_specific_data != NULL)
    {
        m_advdata.p_manuf_specific_data   = &m_manuf_specific_data;
        m_manuf_specific_data.data.p_data = m_manuf_data_array;
        m_advdata.p_manuf_specific_data->company_identifier =
        p_advdata->p_manuf_specific_data->company_identifier;
        m_advdata.p_manuf_specific_data->data.size = p_advdata->p_manuf_specific_data->data.size;
        
        for(uint32_t i = 0; i < m_advdata.p_manuf_specific_data->data.size; i++)
        {
            m_manuf_data_array[i] = p_advdata->p_manuf_specific_data->data.p_data[i];
        }
    }
    
    if(p_advdata->p_service_data_array != NULL)
    {
        m_service_data.data.p_data                   = m_service_data_array;
        m_advdata.p_service_data_array               = &m_service_data;
        m_advdata.p_service_data_array->data.p_data  = m_service_data_array;
        m_advdata.p_service_data_array->data.size    = p_advdata->p_service_data_array->data.size;
        m_advdata.p_service_data_array->service_uuid = p_advdata->p_service_data_array->service_uuid;

        for(uint32_t i = 0; i < m_advdata.p_service_data_array->data.size; i++)
        {
            m_service_data_array[i] = p_advdata->p_service_data_array->data.p_data[i];
        }

        m_advdata.service_data_count = p_advdata->service_data_count;
    }


    if(p_advdata->p_slave_conn_int != NULL)
    {
        m_advdata.p_slave_conn_int                    = &m_slave_conn_int;
        m_advdata.p_slave_conn_int->max_conn_interval = p_advdata->p_slave_conn_int->max_conn_interval;
        m_advdata.p_slave_conn_int->min_conn_interval = p_advdata->p_slave_conn_int->min_conn_interval;
    }
    
    if(p_advdata->p_tx_power_level != NULL)
    {
        m_advdata.p_tx_power_level     = &m_tx_power_level;
        m_advdata.p_tx_power_level     = p_advdata->p_tx_power_level;
    }

	// Initialize advertising parameters with default values.
    memset(&g_adv_params, 0, sizeof(g_adv_params));

    g_adv_params.type        	= BLE_GAP_ADV_TYPE_ADV_IND;
    g_adv_params.p_peer_addr 	= NULL;
    g_adv_params.fp          	= BLE_GAP_ADV_FP_ANY;
    g_adv_params.p_whitelist 	= NULL;
	g_adv_params.timeout  		= 0;
	g_adv_params.interval 		= 100;
	
    err_code = ble_advdata_set(&m_advdata, p_srdata);
    return err_code;
}

void advertising_parm_configer(ble_gap_adv_params_t *adv_params)
{
	g_adv_params.type        	= adv_params->type;
    g_adv_params.p_peer_addr 	= adv_params->p_peer_addr;
    g_adv_params.fp          	= adv_params->fp;
    g_adv_params.p_whitelist 	= adv_params->p_whitelist;
	g_adv_params.timeout  		= adv_params->timeout;
	g_adv_params.interval		= adv_params->interval;
}

void advertising_start(void)
{
	uint32_t err_code;
	
	err_code = sd_ble_gap_adv_start(&g_adv_params);
    APP_ERROR_CHECK(err_code);
}

void advertising_stop(void)
{	
	uint32_t err_code;

    err_code = sd_ble_gap_adv_stop();
    APP_ERROR_CHECK(err_code);
}

