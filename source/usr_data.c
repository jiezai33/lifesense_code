#include "usr_data.h"
#include <string.h>
#include "app_wechat_common.h"
#include "channel_select.h"
#include "debug.h"
#include "usr_device.h"
#include "crc_32.h"
#include "usr_design.h"

#define PRODUCT_TYPE               '4','1','5','B','0'
#define HW_VERSION                 'H','0','1'
#define MCU_TYPE                   '0'
#define MCU_ID                     '4'
#define FW_VERSION                 'T','0','0','1'
#define ALGORITHM_VERION           'A','0','1'
#define SOFTDIVCE_VERION           'D','7','1','0'
#define CODE_LOAD_ADDR             0x00001000
#define CODE_EXCUTE_ADDR           0x00016000
#define SOFT_RESERVED              0xFF,0xFF,0xFF

#define COMMAND_VERSION                       0xAA
#define VERSION_NUM                           0x01

void get_product_type(char *data)
{
    char product[]= {PRODUCT_TYPE,'\0'};
    memcpy(data,product,strlen(product)+1);
}

void get_software_version(char *data)
{
    char fw_ver[]= {FW_VERSION,'\0'};
    /*将fw_ver数组的内容复制到输出的数据指针*/
    memcpy(data,fw_ver,strlen(fw_ver));
}

void get_hardware_verion(char *data)
{
    char hw[]= {HW_VERSION};
    data[0]=hw[1];
    data[1]=hw[2];
    data[2]=MCU_TYPE;
    data[3]=MCU_ID;
    data[4]='\0';
}


uint32_t usr_wechat_send_data_evt(void *data)
{
	uint32_t error;
	uint8_t UploadInfoValue[220],UploadInfoLen,length;
	
	WeChatPackHeader struWeChatPackHead;
	SendDataRequest_t	 SendDataRequest;
	uint8_t 	out_len,*pInData,*pOutData;
	uint8_t data_len=0;
	uint8_t i,ucBUF[35];
	uint32_t crc32_check;

	if(ucBUF[ 0 ])
	{
		//此处没有任何操作，只是为了避免编译警告#550-D:variable "ucBUF" was set but never used
	}
	
	struWeChatPackHead.ucMagicNumber= WECHAT_PACK_HEAD_MAGICNUM;       //包头的magic number，这个值是固定的
	struWeChatPackHead.ucVersion	= WECHAT_PACK_HEAD_VERSIOM;//包头的versionr，这个值是固定的	
	struWeChatPackHead.usLength = 0;
	struWeChatPackHead.usCmdID = WECHAT_CMDID_REQ_UTC;
	struWeChatPackHead.usTxDataPackSequence = usTxWeChatPackSeq++;//0x0003;

	SendDataRequest.BaseRequest =0x00;
	i=0;
	ucBUF[i++] = COMMAND_VERSION;	//	命令字版本
	data_len++;
	ucBUF[i++] = VERSION_NUM;	//	版本号
	data_len++;

	ucBUF[i++] = BLE_SEND_INFO_CMD;   //  心率手环发送请求计步器下载信息命令0X50
	data_len++;

	for(uint8_t count=0; count<6; count++)
	{
		ucBUF[i++] = device_mac.addr[count];
		data_len++;
	}

	char *p;
	char tempbuf[6];
	get_product_type(tempbuf);
	p = tempbuf;
	for(uint8_t count=0; count<5; count++)
	{
		ucBUF[i++] = *(p + count);
		data_len++;
	}

	get_software_version(tempbuf);
	p = tempbuf;
	for(uint8_t count=0; count<4; count++)
	{
		ucBUF[i++] = *(p + count);
		data_len++;
	}

	get_hardware_verion(tempbuf);
	p = tempbuf;
	for(uint8_t count=0; count<4; count++)
	{
		ucBUF[i++] = *(p + count);
		data_len++;
	}

	ucBUF[i++] = 0x50;	//timezone_byte;
	data_len++;
#if 1
	//心率设置
	//memcpy(&ucBUF[i],&struHeartRateSet,sizeof(struHeartRateSet));
	ucBUF[i++] = 0x01;
	for(uint8_t cnt = 0;cnt<4;cnt++)
	{
		ucBUF[i++] = 0x00;
	}
	//i += 5;
	data_len += 5;
#endif
	crc32_check = crc32(ucBUF, i);
	ucBUF[i++] = (uint8_t)(crc32_check>>24);
	data_len++;
	ucBUF[i++] = (uint8_t)(crc32_check>>16);
	data_len++;
	ucBUF[i++] = (uint8_t)(crc32_check>>8);
	data_len++;
	ucBUF[i++] = (uint8_t)crc32_check;
	data_len++;

	SendDataRequest.Data = ucBUF;

	pInData = &SendDataRequest.BaseRequest;
	pOutData = ucDataAfterPack;
	out_len=PackDataType(DATA_BASE_REQUEST_FIELD, Length_delimit, pInData, DATA_BASE_REQUEST_LENGTH, pOutData);
	pOutData += out_len;
	length = out_len;

	pInData = SendDataRequest.Data;
	out_len=PackDataType(DATA_DATA_FIELD, Length_delimit, pInData, data_len, pOutData);

	length += out_len;

	struWeChatPackHead.usLength = length + WECHAT_PACKET_HEAD_LENGTH;
	memset(UploadInfoValue, 0, sizeof(UploadInfoValue));
	UploadInfoLen = WechatPacketHead(struWeChatPackHead, ucDataAfterPack, UploadInfoValue);
	
	
	QPRINTF("usr wechat send data....\r\n");

	error = app_send_data(UploadInfoValue,UploadInfoLen);
	QPRINTF("error = %d\r\n",error);
	for(uint8_t i=0;i<UploadInfoLen;i++)
		QPRINTF("%02x,",UploadInfoValue[i]);
	QPRINTF("\r\n");

	return error;
}


void wechat_push_data_process(uint8_t *rcv_data,uint8_t length)
{
	uint8_t i;
	
    if((rcv_data[i] == COMMAND_VERSION) && (rcv_data[i+1] == VERSION_NUM))          //命令版本识别
    {
        i=0x02;
        if(rcv_data[i] == APP_PUSH_USER_INFO_CMD)     //0x68，用户信息命令
        {
            QPRINTF("APP_PUSH_USER_INFO_CMD\r\n");
        }
        else if(rcv_data[i] == APP_PUSH_ALARM_SETTING_CMD)   //  0X69，闹铃设置命令
        {
            QPRINTF("APP_PUSH_ALARM_SETTING_CMD\r\n");
        }
        else if(rcv_data[i] == APP_PUSH_CALL_SETTING_CMD)   //  0X6A，来电设置命令
        {
			QPRINTF("APP_PUSH_CALL_SETTING_CMD\r\n");
        }
        else if(rcv_data[i] == APP_PUSH_LONG_SIT_SETTING_CMD)  //  0x6E
        {
            QPRINTF("APP_PUSH_LONG_SIT_SETTING_CMD\r\n");
        }
        else if(rcv_data[i] == BLE_PUSH_HR_SMART_SWITCH_INFO_CMD)
        {
            QPRINTF("BLE_PUSH_HR_SMART_SWITCH_INFO_CMD\r\n");
        }
    }
}

