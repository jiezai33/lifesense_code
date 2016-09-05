#ifndef _USR_LOGIN_H_
#define _USR_LOGIN_H_

#include <stdint.h>


typedef struct
{
    uint16_t usTxDataPackSeq;
    uint8_t  usTxDataFrameSeq;
    uint8_t  usLength;
    uint8_t  usCmdID;
    uint8_t  usTxDataType;
} AppPackHeader;





typedef struct
{
    uint8_t  BaseRequest;
    uint8_t  *MD5;
    uint32_t ProtoVersion;
    uint32_t AuthProto;
    uint32_t AuthMethod;
    uint8_t  *AesSign;
    uint8_t  *MacAddress;
    uint8_t  *TimeZone;
    uint8_t  *Language;
    uint8_t  *DeviceName;
} AuthRequest_t;





#define PROTO_VERSION                         0X010003
#define AUTH_PROTO                            0X01
#define AUTH_METHOD                           0X02


#define AUTH_BASE_REQUEST_FIELD               0X01
#define AUTH_MD5_FIELD                        0X02
#define AUTH_PROTO_VERSION_FIELD              0X03
#define AUTH_AUTH_PROTO_FIELD                 0X04
#define AUTH_AUTH_METHOD_FIELD                0X05
#define AUTH_AES_SIGN_FIELD                   0X06
#define AUTH_MAC_ADDRESS_FIELD                0X07
#define AUTH_TIME_ZONE_FIELD                  0X0A
#define AUTH_LANGUAGE_FIELD                   0X0B
#define AUTH_DEVICE_NAME_FIELD                0X0C

#define AUTH_BASE_REQUEST_LENGTH              0X00
#define AUTH_MD5_LENGTH                       0X10
#define AUTH_PROTO_VERSION_LENGTH             0X04
#define AUTH_AUTH_PROTO_LENGTH                0X04
#define AUTH_AUTH_METHOD_LENGTH               0X04
#define AUTH_MAC_ADDRESS_LENGTH               0X06
#define AUTH_AES_SIGN_LENGTH                  0X00


uint32_t usr_lifesense_login_evt(void *data);
uint32_t usr_wechat_login_evt(void *data);
#endif


