#include <zephyr.h>
#include <misc/printk.h>
#include <logging/sys_log.h>
#include <uart.h>
#include <stdio.h>
#include <string.h>

#ifdef __BEAR_SOC_H_
#include <uart_bear.h>
#endif

#include "formats.h"
#include "sbc.h"

#define RECEIVE_BUF_SIZE  1024
#define DECODING_BUF_SIZE 8192


static sbc_t sbc;
static uint8_t receive_data[RECEIVE_BUF_SIZE];
static uint8_t decoding_data[DECODING_BUF_SIZE];

struct device *dev_1;

static char str[100];


//static void uart_poll_out_data(struct device *dev, uint8_t *data, size_t len)
//{
//	for (unsigned int i = 0; i < len; i++) {
//		uart_poll_out(dev, data[i]);
//	}
//}


static bool uart_poll_out_data(struct device *dev, const char *output)
{
    if (!dev) {
        return false;
    }

    for (int i = 0; i < strlen(output); i++) {
        uart_poll_out(dev, output[i]);
    }

    return true;
}


static void uart_fifo_callback(struct device *dev)
{
	uart_irq_update(dev);

	unsigned int read_bytes = uart_fifo_read(dev, receive_data, RECEIVE_BUF_SIZE);
	size_t len = 0;
	if (read_bytes > 100) {
		//uart_poll_out_data(dev_1, "read bytes: %d", read_bytes);
		sbc_decode(&sbc, receive_data, read_bytes,
			decoding_data, DECODING_BUF_SIZE, &len);
		//SYS_LOG_DBG("decoding len: %d", len);
		//uart_poll_out_data(dev, decoding_data, len);
	}
}


static void decode(struct device *dev)
{
	sbc_init(&sbc, 0L);
	sbc.endian = SBC_BE;
	uart_poll_out_data(dev_1, "sbc init\r\n");

	uart_irq_callback_set(dev, uart_fifo_callback);
	uart_irq_rx_enable(dev);
	uart_poll_out_data(dev_1, "rx enable. begin decoding\r\n");

	while (1);

	sbc_finish(&sbc);
	uart_poll_out_data(dev_1, "decoding end");
}


void main(void)
{
	struct device *dev_0 = device_get_binding(CONFIG_UART_CONSOLE_ON_DEV_NAME);
	dev_1 = device_get_binding(CONFIG_UART_BEAR_PORT_1_NAME);

	decode(dev_0);


#ifdef __BEAR_SOC_H_ //Send MSI
    while (!(uart_drv_cmd(dev_0, UART_TX_EMPTY_CMD, 0) && uart_drv_cmd(dev_0, UART_TX_IDLE_CMD, 0))) ;

    (*((volatile unsigned int *)(BEAR_PCIE_MSI_BASE))) = 1;
#endif
}