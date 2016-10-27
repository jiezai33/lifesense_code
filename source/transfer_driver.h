#ifndef _TRANSFER_DRIVER_H_
#define _TRANSFER_DRIVER_H_

#define TX_NOTIFY_PIN	(5)
#define RX_NOTIFY_PIN	(6)//(31)

#define TX_BUFFER_SIZE	(128)
#define RX_BUFFER_SIZE	(128)


void transfer_driver_init(void);

#endif
