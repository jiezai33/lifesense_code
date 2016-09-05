#ifndef _DATA_TRANSMIT_H_
#define _DATA_TRANSMIT_H_
#include <stdint.h>

#define DATA_TRANSMIT_TIME_OUT_EVERY_TIME		(5)
#define DATA_TRANSMIT_TIME_OUT_TOTAL_TIME		(20)

#define DATA_TRANSMIT_GROUP_SIZE		(32)

typedef struct
{
	uint8_t data_type;		//数据类型
	uint16_t sequences;		//发送序列号
	uint32_t addr;			//flash地址
	uint8_t group;			//组数
	uint8_t time_out;		//时间
}transmit_statue_st;




/*****************************************************************************
 * 函 数 名 : app_transmit_statue_init
 * 函数功能 : 
 * 输入参数 : 无
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 数据传输状态表初始化
*****************************************************************************/
void app_transmit_statue_init(void);




/*****************************************************************************
 * 函 数 名 : app_get_transmit_statue
 * 函数功能 : 
 * 输入参数 : uint16_t sequences  发送序列号
 * 输出参数 : uint8_t *index      数据状态下标
 * 返 回 值 : 	0:找到数据
 				1:没有找到数据
 * 修改历史 : 无
 * 说    明 : 这个函数是根据传入的sequences，数据传输状态表里面查询，如果找到了
 				会返回相应的下标值房子index
*****************************************************************************/
uint8_t app_get_transmit_statue(uint8_t *index,uint16_t sequences);




/*****************************************************************************
 * 函 数 名 : app_data_retransmission
 * 函数功能 : 
 * 输入参数 : transmit_statue_st * transmit  数据发送状态指针
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 这个函数当数据需要重新发送的时候，调用这个函数，邝总根据传入的数
 				据传输状态参数，进行去flash读取数据。如果数据已经被擦除了，或
 				者存放了新的数据，则认为发送成功了,清除状态表相应的一行，否则
 				读取数据重新发送
*****************************************************************************/
uint8_t app_data_retransmission(transmit_statue_st * transmit);




/*****************************************************************************
 * 函 数 名 : app_transmit_time_out_one_second
 * 函数功能 : 
 * 输入参数 : 无
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 这个函数在中断1秒里面调用，把数据状态表里面已发送，但未得到应答
 				的表项的timeOut时间进行减1操作，并根据结果进行重发或者断开BLE
*****************************************************************************/
void app_transmit_time_out_one_second(void);




/*****************************************************************************
 * 函 数 名 : app_data_transmit
 * 函数功能 : 
 * 输入参数 :  transmit_statue_st * transmit   数据传输状态指针
               uint8_t *data       数据指针
               uint8_t length      数据长度
 * 输出参数 : 无
 * 返 回 值 : 	0:发送数据操作完成
 				1:发送的数据状态表满，暂时不能进行发送
 * 修改历史 : 无
 * 说    明 : 这个函数邝总主动发送数据和重发数据的调用函数，根据传入的sequences
 				来区分，并调用app_add_heap_send_data这个函数把数据发送出去
*****************************************************************************/
uint8_t app_data_transmit(transmit_statue_st * transmit,uint8_t *data,uint8_t length);




/*****************************************************************************
 * 函 数 名 : app_data_transmit_ack
 * 函数功能 : 
 * 输入参数 : uint16_t sequences  发送序列号
 * 输出参数 : 无
 * 返 回 值 : 0:OK
 * 修改历史 : 无
 * 说    明 : 这个函数是发送数据以后，接收到了APP的应答，并根据应答中的sequences
 				把状态表中相应的一项清除,如果是flash里面的数据，则要告诉邝总这边
 				的相应的发送计数器减1
*****************************************************************************/
uint8_t app_data_transmit_ack(uint16_t sequences);




#endif

