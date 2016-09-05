#include "app_wechat_common.h"
#include "crc_32.h"
#include <string.h>
uint16_t       usTxWeChatPackSeq;
uint8_t ucDataAfterPack[220];


/**************************************************************
Function:       ��װ���͵�΢�����ݰ���һ֡��������
Description:    ΢�����ݰ���һ֡�����ɰ�ͷ��ǰ�沿�ְ����������
Input:          struWeChatPackHead ���ݰ�ͷ���ݣ�pInData ���ݰ��İ�������
Output:         pOutData ������͵�΢�����ݰ���һ֡����
Return:         ���ط���΢�����ݰ��ĵ�һ֡���ݳ���
***************************************************************/
uint8_t    app_add_wechat_head(WeChatPackHeader head, uint8_t *pInData,uint8_t *pOutData)
{
    uint8_t total_len;
    // ΢�����ݰ���ͷ
    *pOutData ++ = head.ucMagicNumber;
    *pOutData ++ = head.ucVersion;
    *pOutData ++ = head.usLength >> 8;    /* ΢�����ݰ��ܳ��� */
    *pOutData ++ = head.usLength;
    *pOutData ++ = head.usCmdID >> 8;     /* ΢�����ݰ����� */
    *pOutData ++ = head.usCmdID;
    *pOutData ++ = head.usTxDataPackSequence >> 8;    /* ΢�����ݰ���� */
    *pOutData ++ = head.usTxDataPackSequence;

	memcpy(pOutData, pInData, head.usLength - WECHAT_PACKET_HEAD_LENGTH);
	total_len = head.usLength;

    return  total_len;      // ��һ֡����
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
        temp_bufer[index++] = head.usLength + crclen; //����CRC����4
        crc = crc32(in_data,head.usLength);
    }

    for(i=0,j=0; i<head.usLength+crclen; i++) //��֡���
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

    memcpy(out_data,temp_bufer,index); // ����ucDataAfterPack

    return  index;
}


