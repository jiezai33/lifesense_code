#include "app_ota.h"
#include "debug.h"
#include "ota_usrdesign.h"


void app_ota_connection(void)
{
	QPRINTF("app_ota_connection\r\n");
}

void app_ota_disconnection(void)
{
	QPRINTF("app_ota_disconnection\r\n");
}

/*****************************************************************************
 * 函 数 名 : app_ota_indicate_statue_set
 * 函数功能 : 
 * 输入参数 : ble_ota_evt_type_t statue  OTA indicate的状态
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 获取OTA indicate的状态
*****************************************************************************/
void app_ota_indicate_statue_set(ble_ota_evt_type_t statue)
{
	switch(statue)
	{
		case BLE_OTA_EVT_INDICATION_ENABLED:
			QPRINTF("BLE_OTA_EVT_INDICATION_ENABLED\r\n");
			break;

		case BLE_OTA_EVT_INDICATION_DISABLED:
			QPRINTF("BLE_OTA_EVT_INDICATION_DISABLED\r\n");
			break;

		case BLE_OTA_EVT_INDICATION_CONFIRMED:
			QPRINTF("BLE_OTA_EVT_INDICATION_CONFIRMED\r\n");
			break;
			
		default:break;
	}
}

/*****************************************************************************
 * 函 数 名 : app_ota_send_data
 * 函数功能 : 
 * 输入参数 :  uint8_t *data                     发送的数据
               uint16_t length                   发送数据的长度
               ota_channel_enum channel_type     OTA的indicate或者notifi通道选择
               uint8_t channel                   目前是notifi通道第几个通道选择
 * 输出参数 : 无
 * 返 回 值 : 	0:OK 	
 				1:通道选择错误	
 				2:上次的数据还没有发送完成
 * 修改历史 : 无
 * 说    明 : 无
*****************************************************************************/
uint32_t app_ota_send_data(uint8_t *data,uint16_t length,ota_channel_enum channel_type,uint8_t channel)
{
	return ota_send_data(data,length,channel_type,channel);
}

/*****************************************************************************
 * 函 数 名 : app_ota_cmd_receive
 * 函数功能 : 
 * 输入参数 :  uint16_t cmd_id  命令号
               uint8_t *data    变长包体数据指针
               uint8_t length   变长包体长度
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : OTA 命令的接收
*****************************************************************************/
void app_ota_cmd_receive(uint8_t cmd_i,uint8_t *data,uint16_t data_len)
{
	switch(cmd_i)
	{
		case 0x02:
			break;
			
		default:break;
	}
}

/*****************************************************************************
 * 函 数 名 : app_ota_data_receive
 * 函数功能 : 
 * 输入参数 :  uint8_t *data    变长包体数据指针
               uint8_t length   变长包体长度
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : OTA 数据的接收
*****************************************************************************/
void app_ota_data_receive(uint8_t *data,uint16_t data_len)
{

}



