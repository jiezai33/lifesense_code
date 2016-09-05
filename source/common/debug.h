#ifndef _DEBUG_H_
#define _DEBUG_H_
#include <stdio.h>

#define DEBUG_ENABLE		(1)

#if DEBUG_ENABLE

#define DEBUG_UART_TYPE		(0)
#define DEBUG_RTT_TYPE		(1)

#define DEBUG_TYPE			DEBUG_RTT_TYPE			


#if DEBUG_TYPE == DEBUG_UART_TYPE
/* 串口引脚定义 */
#define RX_PIN_NUMBER  8
#define TX_PIN_NUMBER  6

void debug_init(void);
#define QPRINTF		printf

#elif DEBUG_TYPE == DEBUG_RTT_TYPE

#include "nrf_log.h"
#include "SEGGER_RTT_Conf.h"
#include "SEGGER_RTT.h"

#define QPRINTF(...)   NRF_LOG_PRINTF(__VA_ARGS__)
#endif

#else

#define QPRINTF(...)

#endif




#endif

