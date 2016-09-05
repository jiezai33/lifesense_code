#ifndef _PROTOBUF_H_
#define _PROTOBUF_H_
#include <stdint.h>

#define true 	1
#define false 	0

#define TRUE	true
#define FALSE	false

#define SIZE_OF_BYTE_LEN                      1 /* һ���ֽڳ��� */
#define SIZE_OF_WORD_LEN                      2 /* һ���ֳ��� */
#define SIZE_OF_TWO_WORD_LEN                  4 /* �����ֳ��� */
#define SIZE_OF_FOUR_WORD_LEN                 8 /* �ĸ��ֳ��� */
#define SIZE_OF_32_BIT_LEN                    4 /* ����32λ���� */
#define SIZE_OF_64_BIT_LEN                    8 /* ����64λ���� */

/* ö��protocol buffer������������� */
typedef enum
{
    Varint=0,           /*  int32, int64, uint32, uint64, sint32, sint64, bool, enum    */
    Bit64,              /*  fixed64, sfixed64, double                                   */
    Length_delimit,     /*  string, bytes, embedded messages, packed repeated fields    */
    Start_group,        /*  Groups (deprecated)                                         */
    End_group,          /*  Groups (deprecated)                                         */
    Bit32               /*  fixed32, sfixed32, float                                    */
} data_type;


uint8_t response_unpack(uint8_t dest_field,uint8_t *pData, uint8_t Len,
                           uint8_t *pUnData, uint8_t *pdatalen,uint8_t *offset);

uint8_t PackDataType(uint8_t field, uint8_t type,uint8_t* pInData, uint8_t Unlen, uint8_t* pOutData);

#endif

