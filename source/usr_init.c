#include "usr_init.h"
#include <string.h>
#include "app_wechat_common.h"
#include "channel_select.h"
#include "debug.h"

uint32_t usr_wechat_init_evt(void *data)
{
	WeChatPackHeader struWeChatPackHead;
	InitRequest_t   InitRequest;
    uint8_t     length,out_len,*pInData,*pOutData;
	uint8_t UploadInfoValue[220],UploadInfoLen;
	uint32_t error;
	
	
	struWeChatPackHead.ucMagicNumber= WECHAT_PACK_HEAD_MAGICNUM;       //��ͷ��magic number�����ֵ�ǹ̶���
	struWeChatPackHead.ucVersion	= WECHAT_PACK_HEAD_VERSIOM;//��ͷ��versionr�����ֵ�ǹ̶���	
    struWeChatPackHead.usLength 	= 0;
    struWeChatPackHead.usCmdID 		= WECHAT_CMDID_REQ_INIT;
    struWeChatPackHead.usTxDataPackSequence = usTxWeChatPackSeq;

    InitRequest.BaseRequest 		= 0x00;
    InitRequest.RespFieldFilter 	= 0x72;//ֻ����ʱ����Ϣ���ֻ�ƽ̨

	pOutData = ucDataAfterPack;         //��������ݴ����ucDataAfterPack
	
    pInData = &InitRequest.BaseRequest;
    out_len=PackDataType(INIT_BASE_REQUEST_FIELD, Length_delimit, pInData, INIT_BASE_REQUEST_LENGTH, pOutData);
    pOutData += out_len;
    length = out_len;

    pInData = &InitRequest.RespFieldFilter;
    out_len=PackDataType(INIT_RESP_FIELD_FILTER_FIELD, Length_delimit, pInData, INIT_RESP_FIELD_FILTER_LENGTH, pOutData);
    length += out_len;                //�����İ��峤��

    struWeChatPackHead.usLength = length + WECHAT_PACKET_HEAD_LENGTH;//��������ĳ��ȣ�����+��ͷ
    	
    memset(UploadInfoValue, 0, sizeof(UploadInfoValue));
    UploadInfoLen = app_add_wechat_head(struWeChatPackHead, ucDataAfterPack, UploadInfoValue);//�����һ֡���ݣ������ص�һ֡�ĳ���

	QPRINTF("usr wechat init....\r\n");

	error = app_send_data(UploadInfoValue,UploadInfoLen);
	for(uint8_t i=0;i<20;i++)
		QPRINTF("%02x,",UploadInfoValue[i]);
	QPRINTF("\r\n");

	return error;
}


