#include "debug.h"
#include "nrf_gpio.h"

#if DEBUG_ENABLE

#if DEBUG_TYPE == DEBUG_UART_TYPE

void debug_init(void)
{
  /** @snippet [Configure UART RX and TX pin] */
  nrf_gpio_cfg_output(TX_PIN_NUMBER);
  nrf_gpio_cfg_input(RX_PIN_NUMBER, NRF_GPIO_PIN_NOPULL);  

  NRF_UART0->PSELTXD = TX_PIN_NUMBER;
  NRF_UART0->PSELRXD = RX_PIN_NUMBER;

  NRF_UART0->BAUDRATE         = (UART_BAUDRATE_BAUDRATE_Baud115200 << UART_BAUDRATE_BAUDRATE_Pos);
  NRF_UART0->ENABLE           = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);
  NRF_UART0->TASKS_STARTTX    = 1;
  NRF_UART0->TASKS_STARTRX    = 1;
  NRF_UART0->EVENTS_RXDRDY    = 0;

}


/*******************************************************************************
 * Function      : 重定向printf函数
 * Description   : 
 * Input         : None
 * Output        : None
 * Return        : 
 * Others        : 
 * Modification  : Created file // 2016-01-08

*******************************************************************************/
int fputc(int ch, FILE *f)  
{
    NRF_UART0->TXD = (unsigned char)ch;
    while (NRF_UART0->EVENTS_TXDRDY!=1)
    {
    // Wait for TXD data to be sent
    }
    NRF_UART0->EVENTS_TXDRDY=0;
    
    return ch;  
}

#endif


#endif
