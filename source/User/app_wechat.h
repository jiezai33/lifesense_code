#ifndef _APP_WECHAT_H_
#define _APP_WECHAT_H_
#include "ble_wechat.h"
#include "wechat_usrdesign.h"

#define AUTH_BASE_RESPONSE_FIELD              0X01
#define AUTH_AES_SESSION_KEY_FIELD            0X02


#define INIT_BASE_RESPONSE_FIELD              0X01
#define INIT_USER_ID_HIGH_FIELD               0X02
#define INIT_USER_ID_LOW_FIELD                0X03
#define INIT_CHALLEANGE_ANSWER_FIELD          0X04
#define INIT_INIT_SCENCE_FIELD                0X05
#define INIT_AUTO_SYN_MAX_DURA_SEC_FIELD      0X06
#define INIT_USER_NICK_NAME_FIELD             0X0B
#define INIT_PLAT_FORM_TYPE_FIELD             0X0C
#define INIT_MODEL_FIELD                      0X0D
#define INIT_OS_FIELD                         0X0E
#define INIT_TIME_FIELD                       0X0F
#define INIT_TIME_ZONE_FIELD                  0X10
#define INIT_TIME_STRING_FIELD                0X11


void app_wechat_connection(void);
void app_wechat_disconnection(void);




/*****************************************************************************
 * 函 数 名 : app_wechat_indicate_statue_set
 * 函数功能 : 
 * 输入参数 : ble_wechat_evt_type_t statue  微信indicate的状态
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 获取微信indicate的状态
*****************************************************************************/
void app_wechat_indicate_statue_set(ble_wechat_evt_type_t statue);




/*****************************************************************************
 * 函 数 名 : app_wechat_send_data
 * 函数功能 : 
 * 输入参数 :  uint8_t *data                     发送的数据
               uint16_t length                   发送数据的长度
               wechat_channel_enum channel_type  微信的indicate或者notifi通道选择
               uint8_t channel                   目前是notifi通道第几个通道选择
 * 输出参数 : 无
 * 返 回 值 : 	0:OK 	
 				1:通道选择错误	
 				2:上次的数据还没有发送完成
 * 修改历史 : 无
 * 说    明 : 无
*****************************************************************************/
uint32_t app_wechat_send_data(uint8_t *data,uint16_t length,wechat_channel_enum channel_type,uint8_t channel);




/*****************************************************************************
 * 函 数 名 : app_wechat_receive
 * 函数功能 : 
 * 输入参数 :  uint16_t cmd_id  命令号
               uint8_t *data    变长包体数据指针
               uint8_t length   变长包体长度
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 微信数据的接收
*****************************************************************************/
void app_wechat_receive(uint16_t cmd_id,uint8_t *data,uint8_t length);




#endif

