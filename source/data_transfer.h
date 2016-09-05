#ifndef _TRANSFER_H_
#define _TRANSFER_H_
#include <stdint.h>

#define USR_DATA_TABLE_SIZE			(32)	//数据传输状态表的最大数量

typedef struct
{
	uint32_t addr;	//flash的地址
	uint8_t group;	//flash地址里面的组数量
}flash_addr_struct;


typedef struct
{
	uint8_t data_type;	//数据类型
	uint8_t data_id_max;//数据ID的最大数
}current_data_statue_struct;

typedef struct
{
	uint8_t data_id;		//数据ID
	uint8_t earse_flag;		//擦除标志
	uint8_t time_out_count;	//超时的次数
	uint32_t time;			//发送当前包的时间
	flash_addr_struct *addr;//数据ID对应的flash地址和组数
}data_struct;

typedef struct
{
	current_data_statue_struct statue;
	data_struct data[USR_DATA_TABLE_SIZE];
}data_transfer_table_struct;


typedef enum
{
	STEP_DATA = 1,	//记步数据
	SLEEP_DATA,		//睡眠数据
	HOUR_DATA,		//小时数据
	FLASH_DATA_MAX
}flash_data_enum;




/*****************************************************************************
 * 函 数 名 : get_data_falsh_addr
 * 函数功能 : 
 * 输入参数 : uint8_t data_type  数据类型
               int8_t data_id     数据ID
 * 输出参数 : 无
 * 返 回 值 : 参考 flash_addr_struct 结构体
 * 修改历史 : 无
 * 说    明 : 根据传入的数据类型和ID获取对应的flash地址和组数量
*****************************************************************************/
flash_addr_struct *get_data_falsh_addr(uint8_t data_type,int8_t data_id);




/*****************************************************************************
 * 函 数 名 : data_transfer_statue_init
 * 函数功能 : 
 * 输入参数 : flash_data_enum data_type  数据类型
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 这个函数是在每次发数据前，把传输数据状态表的给初始化一次，因为目前传输的机制是每次只能传输一种类型的数据
 				所以可以根据传进来的数据类型进行初始化传输状态机，把相应的flash的地址赋值到状态机
*****************************************************************************/
void data_transfer_statue_init(flash_data_enum data_type);




/*****************************************************************************
 * 函 数 名 : data_retransmission
 * 函数功能 : 
 * 输入参数 : 	uint8_t data_type        数据类型 
 				uint8_t data_id          数据ID
                flash_addr_struct *addr  对应flash 的地址和组数量 
 * 输出参数 : 无
 * 返 回 值 : 无
 * 修改历史 : 无
 * 说    明 : 数据重复机制，当数据没有发送成功，调用这个函数进行重新发送相应包的数据
*****************************************************************************/
uint8_t data_retransmission(uint8_t data_type,int8_t data_id,flash_addr_struct *addr);




/*****************************************************************************
 * 函 数 名 : data_transmit
 * 函数功能 : 
 * 输入参数 :  uint8_t data_type  数据类型
               uint8_t data_id    数据ID
               uint8_t *data      数据指针
               uint8_t length     数据长度
 * 输出参数 : 无
 * 返 回 值 :   0:数据传输完成或者已经把当前包的数据传输到发送buffer去了
 				5:数据类型与当前传输数据状态机的 数据类型不一致
 				6:数据ID超过当前数据类型的ID的最大值
 				7:在超时的时间范围内，传输过当前包
 * 修改历史 : 无
 * 说    明 : 这个函数是flash查询是否有数据发送给APP，如果有的话调用下面的函数，发送相应的数据
 *****************************************************************************/
uint32_t data_transmit(uint8_t data_type,uint8_t data_id,uint8_t *data,uint8_t length);




/*****************************************************************************
 * 函 数 名 : data_transmit_ack
 * 函数功能 : 
 * 输入参数 : uint8_t data_type  数据类型
               uint8_t data_id    数据ID
 * 输出参数 : 无
 * 返 回 值 :   0:当前数据包发送完
 				5:当前发送的数据类型不对
 				6:当前发送的数据ID越界
 * 修改历史 : 无
 * 说    明 : 同步或者异步发送数据后，得到服务器的应答包，在相应的包擦除位置1
*****************************************************************************/
uint8_t data_transmit_ack(uint8_t data_type,uint8_t data_id);




/*****************************************************************************
 * 函 数 名 : data_transfer_statue_loop
 * 函数功能 : 
 * 输入参数 : void  无
 * 输出参数 : 无
 * 返 回 值 :   0:说明当前数据类型的所有包都已经发送完成，可以擦除了
 				1:重复或者是重复超过最大次数，进行蓝牙断开操作
 				5:状态或者类型错误
 * 修改历史 : 无
 * 说    明 : 这个函数mainloop里面循环调用
*****************************************************************************/
uint8_t data_transfer_statue_mainloop(void);

#endif

