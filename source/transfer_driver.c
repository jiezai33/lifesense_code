#include "transfer_driver.h"
#include "nrf_drv_spis.h"
#include "debug.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>
#include "nrf_drv_gpiote.h"

#if defined(BOARD_PCA10036) || defined(BOARD_PCA10040)
#define SPIS_CS_PIN  29 /**< SPIS CS Pin. Should be shortened with @ref SPI_CS_PIN */
#elif defined(BOARD_PCA10028)
#define SPIS_CS_PIN  4 /**< SPIS CS Pin. Should be shortened with @ref SPI_CS_PIN */
#else
#error "Example is not supported on that board."
#endif

#define SPIS_INSTANCE 0 /**< SPIS instance index. */
static const nrf_drv_spis_t spis = NRF_DRV_SPIS_INSTANCE(SPIS_INSTANCE);/**< SPIS instance. */

static uint8_t m_tx_buf[TX_BUFFER_SIZE];    	/**< TX buffer. */
static uint8_t m_rx_buf[RX_BUFFER_SIZE];    	/**< RX buffer. */
static uint8_t m_tx_length = 0;        			/**< Transfer length. */
static uint8_t m_rx_length = RX_BUFFER_SIZE;    /**< Transfer length. */

uint8_t transfer_tx_data(uint8_t *data,uint8_t length)
{
	uint8_t i,ret=0;
	uint32_t err_code;
	
	if(length > TX_BUFFER_SIZE)
		length = TX_BUFFER_SIZE;

	for(i=0;i<length;i++)
		m_tx_buf[i] = data[i];

	m_tx_length = length;

	err_code = nrf_drv_spis_tx_buffers_set(&spis, m_tx_buf, m_tx_length);
	APP_ERROR_CHECK(err_code);

	return ret;
}

uint8_t transfer_rx_data(uint8_t *data,uint8_t length)
{
	uint8_t i,ret=0;
	if(length > RX_BUFFER_SIZE)
		length = RX_BUFFER_SIZE;

	QPRINTF("Received:");
	for(i=0;i<length;i++)
		QPRINTF("0x%02x,",data[i]);
	QPRINTF("\r\n\r\n");

	return ret;
}

static void spis_event_handler(nrf_drv_spis_event_t event)
{
	static uint8_t count = 0;
	uint8_t i=0,buffer[100],len =0;
	
    switch(event.evt_type)
    {
    	case NRF_DRV_SPIS_XFER_DONE:
			if(event.rx_amount > 0)
			{
				transfer_rx_data(m_rx_buf,event.rx_amount);

				QPRINTF("tx count=%d\r\n",event.tx_amount);
				count++;
				for(i=0;i<count;i++)
					buffer[i] = 'a'+count;
				len = count;
				
				transfer_tx_data(buffer,len);

				if(count %2 == 0)
					nrf_gpio_pin_set(TX_NOTIFY_PIN);
				else
					nrf_gpio_pin_clear(TX_NOTIFY_PIN);
				if(count > 20)
					count = 0;
			}
			break;

		case NRF_DRV_SPIS_BUFFERS_SET_DONE:
			break;

		default:break;
    }

}

static void usr_gpiote_event_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{

	QPRINTF("pin=%d,action=%d\r\n",pin,action);
	if(pin == RX_NOTIFY_PIN )
	{
		if(action == NRF_GPIOTE_POLARITY_HITOLO)
		{
			QPRINTF("have read data\r\n");	
		}
	}
}

uint32_t transfer_io_control_init(void)
{
	uint32_t err_code;
	
	nrf_gpio_cfg_output(TX_NOTIFY_PIN);
	nrf_gpio_cfg_input(RX_NOTIFY_PIN, NRF_GPIO_PIN_PULLUP);

	err_code = nrf_drv_gpiote_init();
    VERIFY_SUCCESS(err_code);

	nrf_drv_gpiote_in_config_t config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    config.pull = NRF_GPIO_PIN_PULLUP;
	config.sense = NRF_GPIOTE_POLARITY_HITOLO;
    
    err_code = nrf_drv_gpiote_in_init(RX_NOTIFY_PIN, &config, usr_gpiote_event_handler);
    VERIFY_SUCCESS(err_code);

	nrf_drv_gpiote_in_event_enable(RX_NOTIFY_PIN, true);

	return NRF_SUCCESS;
}

void transfer_driver_init(void)
{
	uint32_t err_code;
    NRF_POWER->TASKS_CONSTLAT = 1;

    QPRINTF("SPI Salve init\r\n");
	transfer_io_control_init();
	
    nrf_drv_spis_config_t spis_config = NRF_DRV_SPIS_DEFAULT_CONFIG(SPIS_INSTANCE);
    spis_config.csn_pin               = SPIS_CS_PIN;

	err_code = nrf_drv_spis_init(&spis, &spis_config, spis_event_handler);
    APP_ERROR_CHECK(err_code);

	#if 0
	err_code = nrf_drv_spis_buffers_set(&spis, m_tx_buf, m_tx_length, m_rx_buf, m_rx_length);
	APP_ERROR_CHECK(err_code);
	#else
	err_code = nrf_drv_spis_tx_buffers_set(&spis, m_tx_buf, m_tx_length);
	APP_ERROR_CHECK(err_code);

	err_code = nrf_drv_spis_rx_buffers_set(&spis, m_rx_buf, m_rx_length);
	APP_ERROR_CHECK(err_code);
	#endif
}



