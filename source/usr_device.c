#include "usr_device.h"
#include "ble_gap.h"
#include <string.h>


#define DEVICE_ID                  "e40415100020"
#define DEVICE_TYPE				   "gh_4bc17495d97a"

ble_gap_addr_t device_mac;

uint8_t        device_id_hex[6];
uint8_t        device_type[15];
uint8_t        device_id[12];

void my_memcpy(void *dest, const void *src, uint16_t len)
{
    uint8_t *copy_dest = (uint8_t *)dest, *copy_src = (uint8_t *)src;
    while(len--)
    {
        *(copy_dest)++ = *(copy_src)++;
    }
}

void device_id_init(void)
{
	uint8_t i;
	device_mac.addr_type = 0;
	device_mac.addr[5] = 0xE2;
	device_mac.addr[4] = 0xDC;
	device_mac.addr[3] = 0x52;
	device_mac.addr[2] = 0x00;
	device_mac.addr[1] = 0x31;
	device_mac.addr[0] = 0xAB;

	sd_ble_gap_address_set(BLE_GAP_ADDR_CYCLE_MODE_NONE, &device_mac);
	memset((uint8_t *)&device_mac, 0 ,sizeof(device_mac));
	sd_ble_gap_address_get( &device_mac);

	for (i = 0; i < 3; i++)
    {
        uint8_t temp = device_mac.addr[5 - i] ^ device_mac.addr[i];
        device_mac.addr[i] = temp ^ device_mac.addr[i];
        device_mac.addr[5 - i] = temp ^ device_mac.addr[5 - i];
    }
	my_memcpy(device_id_hex, device_mac.addr, 6);

	my_memcpy(device_id, DEVICE_ID, 12);
    my_memcpy(device_type, DEVICE_TYPE, 15);
}



