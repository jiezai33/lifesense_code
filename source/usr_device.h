#ifndef _USR_DEVICE_H_
#define _USR_DEVICE_H_
#include <stdint.h>
#include "ble_gap.h"

extern uint8_t        device_id_hex[6];
extern uint8_t        device_type[15];
extern uint8_t        device_id[12];

extern ble_gap_addr_t device_mac;

void device_id_init(void);

#endif



