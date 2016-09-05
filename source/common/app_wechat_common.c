#include "app_wechat_common.h"
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
uint8_t    WechatPacketHead(WeChatPackHeader struWeChatPackHead, uint8_t *pInData,uint8_t *pOutData)
{
    uint8_t total_len;
    // 微信数据包包头
    *pOutData ++ = struWeChatPackHead.ucMagicNumber;
    *pOutData ++ = struWeChatPackHead.ucVersion;
    *pOutData ++ = struWeChatPackHead.usLength >> 8;    /* 微信数据包总长度 */
    *pOutData ++ = struWeChatPackHead.usLength;
    *pOutData ++ = struWeChatPackHead.usCmdID >> 8;     /* 微信数据包命令 */
    *pOutData ++ = struWeChatPackHead.usCmdID;
    *pOutData ++ = struWeChatPackHead.usTxDataPackSequence >> 8;    /* 微信数据包序号 */
    *pOutData ++ = struWeChatPackHead.usTxDataPackSequence;

	memcpy(pOutData, pInData, struWeChatPackHead.usLength - WECHAT_PACKET_HEAD_LENGTH);
	total_len = struWeChatPackHead.usLength;

    return  total_len;      // 第一帧长度
}


