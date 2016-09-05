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
	
	
	struWeChatPackHead.ucMagicNumber= WECHAT_PACK_HEAD_MAGICNUM;       //包头的magic number，这个值是固定的
	struWeChatPackHead.ucVersion	= WECHAT_PACK_HEAD_VERSIOM;//包头的versionr，这个值是固定的	
    struWeChatPackHead.usLength 	= 0;
    struWeChatPackHead.usCmdID 		= WECHAT_CMDID_REQ_INIT;
    struWeChatPackHead.usTxDataPackSequence = usTxWeChatPackSeq;

    InitRequest.BaseRequest 		= 0x00;
    InitRequest.RespFieldFilter 	= 0x72;//只下载时间信息和手机平台

	pOutData = ucDataAfterPack;         //打包后数据存放在ucDataAfterPack
	
    pInData = &InitRequest.BaseRequest;
    out_len=PackDataType(INIT_BASE_REQUEST_FIELD, Length_delimit, pInData, INIT_BASE_REQUEST_LENGTH, pOutData);
    pOutData += out_len;
    length = out_len;

    pInData = &InitRequest.RespFieldFilter;
    out_len=PackDataType(INIT_RESP_FIELD_FILTER_FIELD, Length_delimit, pInData, INIT_RESP_FIELD_FILTER_LENGTH, pOutData);
    length += out_len;                //打包后的包体长度

    struWeChatPackHead.usLength = length + WECHAT_PACKET_HEAD_LENGTH;//整包命令的长度，包体+包头
    	
    memset(UploadInfoValue, 0, sizeof(UploadInfoValue));
    UploadInfoLen = app_add_wechat_head(struWeChatPackHead, ucDataAfterPack, UploadInfoValue);//组包第一帧数据，并返回第一帧的长度

	QPRINTF("usr wechat init....\r\n");

	error = app_send_data(UploadInfoValue,UploadInfoLen);
	for(uint8_t i=0;i<20;i++)
		QPRINTF("%02x,",UploadInfoValue[i]);
	QPRINTF("\r\n");

	return error;
}


