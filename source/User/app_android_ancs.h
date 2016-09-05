#ifndef _APP_ANDROID_ANCS_H_
#define _APP_ANDROID_ANCS_H_
#include <stdint.h>
#include "ble_ancs_android.h"

typedef enum
{
    BLE_ANCS_APP_ID_MSG = 0,
    BLE_ANCS_APP_ID_WECHAT
}ble_ancs_c_remind_type_t;

void app_android_ancs_connection(void);
void app_android_ancs_disconnection(void);
void app_android_ancs_indicate_statue_set(ble_android_ancs_evt_type_t statue);
void app_android_ancs_receive_data(uint8_t *data, uint16_t length,uint8_t ucFrameNum);
#endif

