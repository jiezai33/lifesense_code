/***********************************************************************************
 * 文 件 名   : data_products.h
 * 创 建 者   : LiuYuanBin
 * 创建日期   : 2016年8月16日
 * 版权说明   : Copyright (c) 2016-2025   广东乐心医疗电子股份有限公司
 * 文件描述   : 每个BLE传输通道数据buffer的大小设置 和  数据buffer使用数组或者指针控制开关
 * 修改历史   : 
***********************************************************************************/

#ifndef _DATA_PRODUCTS_H_
#define _DATA_PRODUCTS_H_

#define DATA_BUFFER_TYPE	(0)
#define DATA_POINTER_TYPE	(1)

#define DATA_TYPE			DATA_POINTER_TYPE

#define ANCS_SEND_DATA_SIZE					(250)
#define ANCS_RECEIVE_DATA_SIZE				(250)

#define OTA_SEND_DATA_SIZE					(250)
#define OTA_RECEIVE_DATA_SIZE				(250)

#define WECHAT_SEND_DATA_SIZE				(250)
#define WECHAT_RECEIVE_DATA_SIZE			(250)

#define TRANS_SEND_DATA_SIZE				(250)
#define TRANS_RECEIVE_DATA_SIZE				(250)

#endif


