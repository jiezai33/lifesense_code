#include "usr_login.h"
#include <string.h>
#include "usr_device.h"
#include "channel_select.h"
#include "debug.h"
#include "md5.h"
#include "crc_32.h"
#include "app_wechat_common.h"

#define COMMAND_VERSION                       0xAA
#define VERSION_NUM                           0x01

static uint8_t app_pack_data(uint8_t* pInData, uint8_t Unlen, uint8_t* pOutData)
{
    memcpy(pOutData,pInData,Unlen);
    return  Unlen;
}

static uint8_t app_pack_head(AppPackHeader struAppPackHead, uint8_t *pInData,uint8_t *pOutData,uint8_t bdata)
{
    uint8_t crclen;
    uint16_t i,j,index = 0;
    uint8_t temp_bufer[220];
    uint32_t crc = 0;
    uint8_t *p_pInData;

    p_pInData  = pInData;

    temp_bufer[index++] = ((struAppPackHead.usTxDataPackSeq>> 8)|0x80);
    temp_bufer[index++] = struAppPackHead.usTxDataPackSeq;

    if(bdata)
    {
        crclen = 0;
        temp_bufer[index++] = struAppPackHead.usLength;

    }
    else
    {
        crclen = 4;
        temp_bufer[index++] = struAppPackHead.usLength + crclen; //加上CRC长度4
        crc = crc32(p_pInData,struAppPackHead.usLength);
    }

    for(i=0,j=0; i<struAppPackHead.usLength+crclen; i++) //加帧序号
    {
        if(i==0)
        {
            temp_bufer[index++] = struAppPackHead.usTxDataFrameSeq++;
        }
        else if(i==16)
        {
            temp_bufer[index++] = struAppPackHead.usTxDataFrameSeq++;
        }
        else if((i-16)%19==0)
        {
            temp_bufer[index++] = struAppPackHead.usTxDataFrameSeq++;
        }

        if(i<struAppPackHead.usLength)
        {
            temp_bufer[index++] = *p_pInData++;
        }
        else
        {
            temp_bufer[index++]=(uint8_t)(crc>>((3-j)*8));
            j++;
        }
    }

    memcpy(pInData,temp_bufer,index); // 更新ucDataAfterPack

    return  index;
}

uint32_t usr_lifesense_login_evt(void *data)
{
	AppPackHeader	 struAppPackHead;
	static uint8_t AppUploadInfoValue[20];
	uint8_t UploadInfoLen , i =0,out_len,length,*pInData,*pOutData;
	uint8_t ucDeviceIDandTypeMD5[AUTH_MD5_LENGTH];
	uint32_t error;
	uint8_t ucDeviceIDandType[27];	   /* 用于存放device type和device ID */

    memcpy(ucDeviceIDandType, device_type, sizeof(device_type));
    memcpy(ucDeviceIDandType + sizeof(device_type), device_id, sizeof(device_id));
    md5_Code(ucDeviceIDandType, sizeof(ucDeviceIDandType), ucDeviceIDandTypeMD5);

    pInData = ucDeviceIDandTypeMD5;
    pOutData = ucDataAfterPack;

    uint8_t command_version[2];
    command_version[0] = COMMAND_VERSION;
    command_version[1] = VERSION_NUM;
    memcpy(pOutData,command_version,sizeof(command_version));

    out_len=app_pack_data(pInData, AUTH_MD5_LENGTH, pOutData+2);
    length = out_len+sizeof(command_version);

	struAppPackHead.usTxDataType = 0;
    struAppPackHead.usTxDataPackSeq= 0x0001;//包序号
    struAppPackHead.usLength = length;
    struAppPackHead.usTxDataFrameSeq= 0x01; //帧序号
    memset(AppUploadInfoValue, 0, sizeof(AppUploadInfoValue));
    UploadInfoLen = app_pack_head(struAppPackHead, ucDataAfterPack, AppUploadInfoValue, 0);
	
	QPRINTF("usr lifesense login....\r\n");
	error = app_send_data(ucDataAfterPack,UploadInfoLen);
	for(i=0;i<UploadInfoLen;i++)
		QPRINTF("%02x,",ucDataAfterPack[i]);
	QPRINTF("\r\n");

	return error;
}

uint32_t usr_wechat_login_evt(void *data)
{
	uint32_t error = 0 ;
	uint8_t *aes = 0,out_len,length,*pInData,*pOutData;
	uint8_t UploadInfoValue[220],UploadInfoLen;
	
	WeChatPackHeader struWeChatPackHead;
	AuthRequest_t AuthRequest;
	
	struWeChatPackHead.ucMagicNumber = WECHAT_PACK_HEAD_MAGICNUM;       //包头的magic number，这个值是固定的
	struWeChatPackHead.ucVersion= WECHAT_PACK_HEAD_VERSIOM;//包头的versionr，这个值是固定的
	struWeChatPackHead.usLength = 0;
	struWeChatPackHead.usCmdID = WECHAT_CMDID_REQ_ENTRY;//登陆请求命令id
	struWeChatPackHead.usTxDataPackSequence = usTxWeChatPackSeq++;//0x0001;

    AuthRequest.BaseRequest=0;
    AuthRequest.ProtoVersion = PROTO_VERSION;
    AuthRequest.AuthProto = AUTH_PROTO;
    AuthRequest.AuthMethod = AUTH_METHOD;
    AuthRequest.AesSign = aes;
    AuthRequest.MacAddress = device_mac.addr;//把mac地址放在登陆请求中用于识别设备

	pOutData = ucDataAfterPack;         //protobuf打包后的数据存放在ucDataAfterPack
	
	//protobuf 打包
    pInData = &AuthRequest.BaseRequest;
    out_len=PackDataType(AUTH_BASE_REQUEST_FIELD, Length_delimit, pInData, AUTH_BASE_REQUEST_LENGTH, pOutData);
    pOutData += out_len;
    length = out_len;

    pInData = (uint8_t *)(&AuthRequest.ProtoVersion);
    out_len=PackDataType(AUTH_PROTO_VERSION_FIELD, Varint, pInData, AUTH_PROTO_VERSION_LENGTH, pOutData);
    pOutData += out_len;
    length += out_len;

    pInData = (uint8_t *)(&AuthRequest.AuthProto);
    out_len=PackDataType(AUTH_AUTH_PROTO_FIELD, Varint, pInData, AUTH_AUTH_PROTO_LENGTH, pOutData);
    pOutData += out_len;
    length += out_len;

    pInData = (uint8_t *)(&AuthRequest.AuthMethod);
    out_len=PackDataType(AUTH_AUTH_METHOD_FIELD, Varint, pInData, AUTH_AUTH_METHOD_LENGTH, pOutData);
    pOutData += out_len;
    length += out_len;

    pInData = AuthRequest.MacAddress;
    out_len=PackDataType(AUTH_MAC_ADDRESS_FIELD, Length_delimit, pInData, AUTH_MAC_ADDRESS_LENGTH, pOutData);
    pOutData += out_len;
    length += out_len;   
	
	struWeChatPackHead.usLength = length + WECHAT_PACKET_HEAD_LENGTH;
	memset(UploadInfoValue, 0, sizeof(UploadInfoValue));
	UploadInfoLen = WechatPacketHead(struWeChatPackHead, ucDataAfterPack,UploadInfoValue);	
	
	QPRINTF("usr wechat login....\r\n");

	error = app_send_data(UploadInfoValue,UploadInfoLen);
	for(uint8_t i=0;i<UploadInfoLen;i++)
		QPRINTF("%02x,",UploadInfoValue[i]);
	QPRINTF("\r\n");
	
	return error;
}


