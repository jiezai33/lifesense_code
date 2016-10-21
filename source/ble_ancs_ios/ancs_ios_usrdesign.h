#ifndef _ANCS_IOS_USRDESIGN_H_
#define _ANCS_IOS_USRDESIGN_H_
#include <stdint.h>
#include <stdbool.h>
#include "data_products.h"


/** @brief Length of the iOS notification data.
 *
 * @details 8 bytes:
 * Event ID |Event flags |Category ID |Category count|Notification UID
 * ---------|------------|------------|--------------|----------------
 * 1 byte   | 1 byte     | 1 byte     | 1 byte       | 4 bytes        
 */
#define BLE_ANCS_NOTIFICATION_DATA_LENGTH   8

#define BLE_ANCS_NOTIF_EVT_ID_INDEX       0                       /**< Index of the Event ID field when parsing notifications. */
#define BLE_ANCS_NOTIF_FLAGS_INDEX        1                       /**< Index of the Flags field when parsing notifications. */
#define BLE_ANCS_NOTIF_CATEGORY_ID_INDEX  2                       /**< Index of the Category ID field when parsing notifications. */
#define BLE_ANCS_NOTIF_CATEGORY_CNT_INDEX 3                       /**< Index of the Category Count field when parsing notifications. */
#define BLE_ANCS_NOTIF_NOTIF_UID          4                       /**< Index of the Notification UID field when patsin notifications. */

#define BLE_ANCS_EVENT_FLAG_SILENT          0       /**< 0b.......1 Silent: First (LSB) bit is set. All flags can be active at the same time.*/
#define BLE_ANCS_EVENT_FLAG_IMPORTANT       1       /**< 0b......1. Important: Second (LSB) bit is set. All flags can be active at the same time.*/
#define BLE_ANCS_EVENT_FLAG_PREEXISTING     2       /**< 0b.....1.. Pre-existing: Third (LSB) bit is set. All flags can be active at the same time.*/
#define BLE_ANCS_EVENT_FLAG_POSITIVE_ACTION 3       /**< 0b....1... Positive action: Fourth (LSB) bit is set. All flags can be active at the same time.*/
#define BLE_ANCS_EVENT_FLAG_NEGATIVE_ACTION 4       /**< 0b...1.... Negative action: Fifth (LSB) bit is set. All flags can be active at the same time. */


#define IOS_ANCS_RECEIVE_DATA_START_ADDR	(0)
#define IOS_ANCS_RECEIVE_DATA_SIZE			ANCS_DATA_SIZE
typedef struct
{
	uint8_t addr;
	uint8_t size;
}data_st;

typedef struct 
{
	uint32_t time;
	data_st id[8];//BLE_ANCS_NOTIF_ATTR_ID_MAX
	uint8_t use_index;
	
	#if DATA_TYPE == DATA_BUFFER_TYPE
	char data[ANCS_DATA_SIZE];
	#elif DATA_TYPE == DATA_POINTER_TYPE
	char *data;
	#endif
}remind_info_analysis_st;


void parse_notif(const uint8_t * p_data_src,const uint16_t hvx_data_len);
void parse_get_notif_attrs_response(const uint8_t * p_data_src,const uint16_t  hvx_data_len);

#endif



