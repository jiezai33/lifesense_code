/***********************************************************************************
 * 文 件 名   	: ble_ancs_android.c
 * 创 建 者   	: zhangfei
 * 创建日期   : 2016年3月24日(实际为增加注释日期)
 * 版权说明   : Copyright (c) 2016-2025   广东乐心医疗电子股份有限公司
 * 文件描述   : 该文件用于安卓系统来电服务功能
 * 修改历史   :
 *					1 201603024 by zhangfei, 增加程序注释
  *             
***********************************************************************************/
#include <string.h>
#include "nordic_common.h"
#include "ble_l2cap.h"
#include "ble_srv_common.h"
#include "app_util.h"
#include "ble_ancs_android.h"
#include "ancs_android_usrdesign.h"

ble_android_ancs_t          m_android_ancs;

static void android_ancs_on_connect(ble_android_ancs_t * p_android_ancs, ble_evt_t * p_ble_evt)
{
    p_android_ancs->conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
	android_ancs_connection();
}

static void android_ancs_on_disconnect(ble_android_ancs_t * p_android_ancs, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_android_ancs->conn_handle = BLE_CONN_HANDLE_INVALID;
	android_ancs_disconnection();
}

/***************************************************************************************************************
 * 函 数 名  	: Ancs_indicate_char_add
 * 函数功能  : 安卓来电服务下增加indicate属性
 * 输入参数  : p_wechat--android ancs Service structure
 *				  p_wechat_init---android ancs Service init structure
 * 输出参数  : 无
 * 返 回 值  	: NRF_SUCCESS---增加成功
 *				  其他----失败，具体请参考Nrf_error.h
***************************************************************************************************************/
static uint32_t Ancs_indicate_char_add(ble_android_ancs_t * p_android_ancs, const ble_android_ancs_init_t * p_wechat_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
    uint8_t             encoded_wechat[20];

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    cccd_md.vloc = BLE_GATTS_VLOC_STACK;
    cccd_md.write_perm = p_wechat_init->wechat_meas_attr_md.cccd_write_perm;

	//Characteristic metadata
    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.indicate = 1;											//Indicate属性
    char_md.p_char_user_desc    = NULL;
    char_md.p_char_pf           = NULL;
    char_md.p_user_desc_md      = NULL;
    char_md.p_cccd_md           = &cccd_md;
    char_md.p_sccd_md           = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_ANCS_INDICATE_CHAR);				//设置indicate 属性的UUID type和vlaue

 	//Attribute metadata
    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.read_perm  = p_wechat_init->wechat_meas_attr_md.read_perm;
    attr_md.write_perm = p_wechat_init->wechat_meas_attr_md.write_perm;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = 20;//bps_measurement_encode(p_wechat, &initial_wechat, encoded_wechat);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = 20;
    attr_char_value.p_value      = encoded_wechat;

    return sd_ble_gatts_characteristic_add(p_android_ancs->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_android_ancs->indicate_handles);
}
/***************************************************************************************************************
 * 函 数 名  	: Ancs_write_char_add
 * 函数功能  : 安卓来电服务下增加write属性
 * 输入参数  : p_wechat--android ancs Service structure
 *				  p_wechat_init---android ancs Service init structure
 * 输出参数  : 无
 * 返 回 值  	: NRF_SUCCESS---增加成功
 *				  其他----失败，具体请参考Nrf_error.h
***************************************************************************************************************/
static uint32_t Ancs_write_char_add(ble_android_ancs_t * p_android_ancs, const ble_android_ancs_init_t * p_wechat_init)
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

	//Characteristic metadata
    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.indicate = 0;								//不是indicate属性
    char_md.char_props.write    = 1;								//wirte属性
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf        = NULL;
    char_md.p_user_desc_md   = NULL;
    char_md.p_cccd_md        = &cccd_md;
    char_md.p_sccd_md        = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_ANCS_WRITE_CHAR);	//设置write 属性的UUID type和vlaue

	 //Attribute metadata
    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    // attr_md.read_perm  = p_wechat_init->wechat_userinfo_attr_md.read_perm;
    // attr_md.write_perm = p_wechat_init->wechat_userinfo_attr_md.write_perm;
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

    return sd_ble_gatts_characteristic_add(p_android_ancs->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_android_ancs->write_handles);

}
/***************************************************************************************************************
 * 函 数 名  	: Ancs_feature_char_add
 * 函数功能  : 安卓来电服务下增加feature属性(read)
 * 输入参数  : p_wechat--android ancs Service structure
 *				  p_wechat_init---android ancs Service init structure
 * 输出参数  : 无
 * 返 回 值  	: NRF_SUCCESS---增加成功
 *				  其他----失败，具体请参考Nrf_error.h
***************************************************************************************************************/
static uint8_t test_buf[] ={'a','n','c','s'};
static uint32_t Ancs_feature_char_add(ble_android_ancs_t * ble_android_ancs_t, const ble_android_ancs_init_t * p_wechat_init)
{
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
//    uint8_t             init_value_encoded[2];
    uint8_t             init_value_len;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read  = 1;									//read属性
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf        = NULL;
    char_md.p_user_desc_md   = NULL;
    char_md.p_cccd_md        = NULL;
    char_md.p_sccd_md        = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_ANCS_READ_CHAR);				//设置read 属性的UUID type和vlaue

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.read_perm  = p_wechat_init->wechat_read_attr_md.read_perm;
    attr_md.write_perm = p_wechat_init->wechat_read_attr_md.write_perm;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));
	
    init_value_len = sizeof(test_buf);       								//read属性的值初始化为本地的mac地址

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = init_value_len;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = init_value_len;
    attr_char_value.p_value      = test_buf;
	
    return sd_ble_gatts_characteristic_add(ble_android_ancs_t->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &ble_android_ancs_t->read_handles);
}
/***************************************************************************************************************
 * 函 数 名  	: ble_Ancs_init
 * 函数功能  : 安卓来电服务初始化
 * 输入参数  : p_wechat--android ancs Service structure
 *				  p_wechat_init---android ancs Service init structure
 * 输出参数  : 无
 * 返 回 值  	: NRF_SUCCESS---增加成功
 *				  err_code----失败，具体请参考Nrf_error.h
***************************************************************************************************************/
uint32_t ble_Ancs_init(ble_android_ancs_t * p_android_ancs, const ble_android_ancs_init_t * p_wechat_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_android_ancs->evt_handler = p_wechat_init->evt_handler;
    p_android_ancs->conn_handle = BLE_CONN_HANDLE_INVALID;										//connect handle初始化为0xFFFF


    // Add service
    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_ANCS_SERVICE);								//设置安卓来电服务的UUID type和vlaue

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_android_ancs->service_handle); //Add andriod ancs service declaration to the local server ATT table
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = Ancs_indicate_char_add(p_android_ancs, p_wechat_init);							// Add indicate characteristic
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = Ancs_write_char_add(p_android_ancs, p_wechat_init);								    // Add write characteristic
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // #ifdef    AUTH_MAC
    err_code = Ancs_feature_char_add(p_android_ancs, p_wechat_init);								    // Add read characteristic
    if(err_code != NRF_SUCCESS)
    {
        return err_code;
    }
    //  #endif

    return NRF_SUCCESS;
}

/***************************************************************************************************************
 * 函 数 名  	: android_ancs_on_value_write
 * 函数功能  : 接收来电服务通过write属性下发的数据，同时对数据做判断分析，
 *					然后回复接收结果
 * 输入参数  : p_wechat--android ancs Service structure
 *				  p_wechat_init---android ancs Service init structure
 * 输出参数  : 无
 * 返 回 值  	: 无
***************************************************************************************************************/
static void android_ancs_on_value_write(ble_android_ancs_t * p_android_ancs, ble_gatts_evt_write_t * p_evt_write)
{
	android_ancs_receive(p_evt_write->data,p_evt_write->len);
}

/**@brief Function for handling the write events to the Blood Pressure Measurement characteristic.
 *
 * @param[in]   p_wechat      wechat Service structure.
 * @param[in]   p_evt_write   Write event received from the BLE stack.
 */
static void android_ancs_on_cccd_write(ble_android_ancs_t * p_android_ancs, ble_gatts_evt_write_t * p_evt_write)
{
    if (p_evt_write->len == 2)
    {
        // CCCD written, update indication state
        if (p_android_ancs->evt_handler != NULL)
        {
            ble_android_ancs_evt_t evt;
            
            if (ble_srv_is_indication_enabled(p_evt_write->data))
            {
                evt.evt_type = BLE_ANDROID_ANCS_EVT_INDICATION_ENABLED;
            }
            else
            {
                evt.evt_type = BLE_ANDROID_ANCS_EVT_INDICATION_DISABLED;
            }
            
            p_android_ancs->evt_handler(p_android_ancs, &evt);
        }
    }
}


/***************************************************************************************************************
 * 函 数 名  	: android_ancs_on_write
 * 函数功能  : 接收来电服务下发的数据，包括indicate属性的设置信息和通过write属性下发的信息
 * 输入参数  : p_wechat--android ancs Service structure
 *				  p_wechat_init---android ancs Service init structure
 * 输出参数  : 无
 * 返 回 值  	: 无
***************************************************************************************************************/
static void android_ancs_on_write(ble_android_ancs_t * p_android_ancs, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
	
    if(p_evt_write->handle == p_android_ancs->indicate_handles.cccd_handle)						//indicate通道设置命令处理
    {
        android_ancs_on_cccd_write(p_android_ancs, p_evt_write);
    }
    else if(p_evt_write->handle == p_android_ancs->write_handles.value_handle)					//write通道数据处理
    {
        //手机写回来的数据处理
        android_ancs_on_value_write(p_android_ancs, p_evt_write);
    }
}

static void android_ancs_on_hvc(ble_android_ancs_t * p_android_ancs, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_hvc_t * p_hvc = &p_ble_evt->evt.gatts_evt.params.hvc;

    if (p_hvc->handle == p_android_ancs->indicate_handles.value_handle)
    {
        ble_android_ancs_evt_t evt;
        
        evt.evt_type = BLE_ANDROID_ANCS_EVT_INDICATION_CONFIRMED;
        p_android_ancs->evt_handler(p_android_ancs, &evt);
    }
}

/***************************************************************************************************************
 * 函 数 名  	: ble_ancs_on_ble_evt
 * 函数功能  : 处理BLE协议栈事件
 * 输入参数  : p_wechat--android ancs Service structure
 *				  p_wechat_init---android ancs Service init structure
 * 输出参数  : 无
 * 返 回 值  	: 无
***************************************************************************************************************/
void ble_ancs_on_ble_evt(ble_android_ancs_t * p_android_ancs, ble_evt_t * p_ble_evt)
{
    switch(p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            android_ancs_on_connect(p_android_ancs, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            android_ancs_on_disconnect(p_android_ancs, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            android_ancs_on_write(p_android_ancs, p_ble_evt);									
            break;

        case BLE_GATTS_EVT_HVC:											//indicate通道confirm信息
            android_ancs_on_hvc(p_android_ancs, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}
/***************************************************************************************************************
 * 函 数 名  	: on_Ancs_evt
 * 函数功能  : 安卓ancs服务的indicate通道事件处理
 * 输入参数  : p_wechat--android ancs Service structure
 *				  p_wechat_init---android ancs Service init structure
 * 输出参数  : 无
 * 返 回 值  	: 无
***************************************************************************************************************/
void on_Ancs_evt(ble_android_ancs_t * p_android_ancs, ble_android_ancs_evt_t *p_evt)
{
    switch(p_evt->evt_type)
    {
        case BLE_ANDROID_ANCS_EVT_INDICATION_ENABLED:
        case BLE_ANDROID_ANCS_EVT_INDICATION_CONFIRMED:
        case BLE_ANDROID_ANCS_EVT_INDICATION_DISABLED:
			android_ancs_indicate_statue_set(p_evt->evt_type);
            break;

        default:
            // No implementation needed.
            break;
    }
}

uint32_t ble_android_ancs_indicate_send(ble_android_ancs_t * p_android_ancs, uint8_t *data)
{
    uint32_t err_code;
 
    // Send value if connected
    if (p_android_ancs->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint16_t               len;
        uint16_t               hvx_len;
        ble_gatts_hvx_params_t hvx_params;

		//每笔数据长度固定20bytes，实际需要发送的长度不够20bytes的时候剩余位置补零
        len     = 20;				
        hvx_len = len;

        memset(&hvx_params, 0, sizeof(hvx_params));
        
        hvx_params.handle = p_android_ancs->indicate_handles.value_handle;
        hvx_params.type   = BLE_GATT_HVX_INDICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &hvx_len;
        hvx_params.p_data = data;
        
        err_code = sd_ble_gatts_hvx(p_android_ancs->conn_handle, &hvx_params);
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

