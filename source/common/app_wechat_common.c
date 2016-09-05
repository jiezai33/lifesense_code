#include "app_wechat_common.h"
#include "crc_32.h"
#include <string.h>
uint16_t       usTxWeChatPackSeq;
uint8_t ucDataAfterPack[220];


/**************************************************************
Function:       封装发送的微信数据包第一帧数据内容
Description:    微信数据包第一帧数据由包头和前面部分包体内容组成
Input:          struWeChatPackHead 数据包头内容，pInData 数据包的包体内容
Output:         pOutData 输出发送的微信数据包第一帧内容
Return:         返回发送微信数据包的第一帧数据长度
***************************************************************/
uint8_t    app_add_wechat_head(WeChatPackHeader head, uint8_t *pInData,uint8_t *pOutData)
{
    uint8_t total_len;
    // 微信数据包包头
    *pOutData ++ = head.ucMagicNumber;
    *pOutData ++ = head.ucVersion;
    *pOutData ++ = head.usLength >> 8;    /* 微信数据包总长度 */
    *pOutData ++ = head.usLength;
    *pOutData ++ = head.usCmdID >> 8;     /* 微信数据包命令 */
    *pOutData ++ = head.usCmdID;
    *pOutData ++ = head.usTxDataPackSequence >> 8;    /* 微信数据包序号 */
    *pOutData ++ = head.usTxDataPackSequence;

	memcpy(pOutData, pInData, head.usLength - WECHAT_PACKET_HEAD_LENGTH);
	total_len = head.usLength;

    return  total_len;      // 第一帧长度
}

uint8_t app_pack_data(uint8_t* pInData, uint8_t Unlen, uint8_t* pOutData)
{
    memcpy(pOutData,pInData,Unlen);
    return  Unlen;
}

uint8_t app_add_pack_head(trans_header_st head,uint8_t *in_data,uint8_t *out_data,uint8_t bdata)
{
    uint8_t crclen;
    uint16_t i,j,index = 0;
    uint8_t temp_bufer[220];
    uint32_t crc = 0;

    temp_bufer[index++] = ((head.usTxDataPackSeq>> 8)|0x80);
    temp_bufer[index++] = head.usTxDataPackSeq;

    if(bdata)
    {
        crclen = 0;
        temp_bufer[index++] = head.usLength;

    }
    else
    {
        crclen = 4;
        temp_bufer[index++] = head.usLength + crclen; //加上CRC长度4
        crc = crc32(in_data,head.usLength);
    }

    for(i=0,j=0; i<head.usLength+crclen; i++) //加帧序号
    {
        if(i==0)
        {
            temp_bufer[index++] = head.usTxDataFrameSeq++;
        }
        else if(i==16)
        {
            temp_bufer[index++] = head.usTxDataFrameSeq++;
        }
        else if((i-16)%19==0)
        {
            temp_bufer[index++] = head.usTxDataFrameSeq++;
        }

        if(i<head.usLength)
        {
            temp_bufer[index++] = *in_data++;
        }
        else
        {
            temp_bufer[index++]=(uint8_t)(crc>>((3-j)*8));
            j++;
        }
    }

    memcpy(out_data,temp_bufer,index); // 更新ucDataAfterPack

    return  index;
}


