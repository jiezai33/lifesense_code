#include "app_wechat_common.h"
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
uint8_t    WechatPacketHead(WeChatPackHeader struWeChatPackHead, uint8_t *pInData,uint8_t *pOutData)
{
    uint8_t total_len;
    // ΢�����ݰ���ͷ
    *pOutData ++ = struWeChatPackHead.ucMagicNumber;
    *pOutData ++ = struWeChatPackHead.ucVersion;
    *pOutData ++ = struWeChatPackHead.usLength >> 8;    /* ΢�����ݰ��ܳ��� */
    *pOutData ++ = struWeChatPackHead.usLength;
    *pOutData ++ = struWeChatPackHead.usCmdID >> 8;     /* ΢�����ݰ����� */
    *pOutData ++ = struWeChatPackHead.usCmdID;
    *pOutData ++ = struWeChatPackHead.usTxDataPackSequence >> 8;    /* ΢�����ݰ���� */
    *pOutData ++ = struWeChatPackHead.usTxDataPackSequence;

	memcpy(pOutData, pInData, struWeChatPackHead.usLength - WECHAT_PACKET_HEAD_LENGTH);
	total_len = struWeChatPackHead.usLength;

    return  total_len;      // ��һ֡����
}


