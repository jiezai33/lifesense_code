#ifndef _APP_WECHAT_COMMON_H_
#define _APP_WECHAT_COMMON_H_
#include <stdint.h>
#include "protoBuf.h"

#define WECHAT_CMDID_REQ_ENTRY                0x2711
#define WECHAT_CMDID_RESQ_ENTRY               0x4E21
#define WECHAT_CMDID_REQ_UTC                  0x2712
#define WECHAT_CMDID_RESQ_UTC                 0x4E22
#define WECHAT_CMDID_REQ_UESERINFO            0x2712
#define WECHAT_CMDID_RESQ_USERINFO            0x4E22
#define WECHAT_CMDID_REQ_MEADATA              0x2712
#define WECHAT_CMDID_RESQ_MEADATA             0x4E22
#define WECHAT_CMDID_REQ_INIT                 0x2713
#define WECHAT_CMDID_RESQ_INIT                0x4E23
#define WECHAT_CMDID_REQ_FAC                  0x2712
#define WECHAT_CMDID_RESQ_FAC                 0x4E22

#define WECHAT_PACKET_HEAD_LENGTH             0X08
#define WECHAT_PACK_HEAD_MAGICNUM             0xFE
#define WECHAT_PACK_HEAD_VERSIOM              0x01


#define DATA_BASE_REQUEST_FIELD               0X01
#define DATA_DATA_FIELD                       0X02
#define DATA_TYPE_FIELD                       0X03



typedef struct
{
    uint8_t  ucMagicNumber;
    uint8_t  ucVersion;
    uint16_t usLength;
    uint16_t usCmdID;
    uint16_t usTxDataPackSequence;
} WeChatPackHeader;

typedef struct
{
    uint16_t usTxDataPackSeq;
    uint8_t  usTxDataFrameSeq;
    uint8_t  usLength;
    uint8_t  usCmdID;
    uint8_t  usTxDataType;
} trans_header_st;


extern uint16_t       usTxWeChatPackSeq;
extern uint8_t ucDataAfterPack[220];

uint8_t    app_add_wechat_head(WeChatPackHeader struWeChatPackHead, uint8_t *pInData,uint8_t *pOutData);

uint8_t app_pack_data(uint8_t* pInData, uint8_t Unlen, uint8_t* pOutData);
uint8_t app_add_pack_head(trans_header_st head,uint8_t *in_data,uint8_t *out_data,uint8_t bdata);

#endif



