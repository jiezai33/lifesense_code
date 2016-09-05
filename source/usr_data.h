#ifndef _USR_DATA_H_
#define _USR_DATA_H_
#include <stdint.h>

typedef struct
{
    uint8_t  BaseRequest;
    uint8_t  *Data;
    uint32_t Type;
} SendDataRequest_t;

#define DATA_BASE_REQUEST_LENGTH              0X00
#define DATA_TYPE_LENGTH                      0X04

//======================协议通信命令号========================
#define     BLE_SEND_INFO_CMD                       0x50
#define     BLE_SEND_DAY_DATA_CMD                   0x51
#define     BLE_SEND_ONE_HOUR_DATA_CMD              0x57
#define     BLE_SEND_SLEEP_DATA_CMD                 0x52
#define     BLE_SEND_HEART_RATE_DATA_CMD            0x53

#define     BLE_PUSH_ENCOURAGE_INFO_CMD             0x70
#define     BLE_SEND_RUNNING_DATA_CMD               0x72
#define     BLE_SEND_RUNNING_HR_DATA_CMD            0x73
#define     BLE_PUSH_HEARTRATE_SECTION_INFO_CMD     0x74
#define     BLE_PUSH_HR_SMART_SWITCH_INFO_CMD       0x76
#define     BLE_PUSH_HAND_UP_DISP_INFO_CMD          0x77
#define     BLE_PUSH_L_R_HAND_DISP_INFO_CMD         0x7a
#define     BLE_PUSH_HORIZ_VORTI_DISP_INFO_CMD      0x7d
#define     BLE_PUSH_USER_DEFINED_INFO_CMD          0x7e
#define     BLE_SEND_RUNNING_CAL_DATA_CMD           0x7f


uint32_t usr_wechat_send_data_evt(void *data);

void wechat_push_data_process(uint8_t *data,uint8_t length);

#endif


