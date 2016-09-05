#ifndef _USR_INIT_H_
#define _USR_INIT_H_
#include <stdint.h>

#define INIT_BASE_REQUEST_FIELD               0X01
#define INIT_RESP_FIELD_FILTER_FIELD          0X02
#define INIT_CHALLENGE_FIELD                  0X03

#define INIT_BASE_REQUEST_LENGTH              0X00
#define INIT_RESP_FIELD_FILTER_LENGTH         0X01

typedef struct
{
    uint8_t BaseRequest;
    uint8_t RespFieldFilter;
    uint8_t *Challenge;
} InitRequest_t;


uint32_t usr_wechat_init_evt(void *data);

#endif
