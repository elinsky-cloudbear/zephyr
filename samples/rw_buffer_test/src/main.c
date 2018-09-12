#include <uart.h>
#include <stdint.h>
#include <misc/printk.h>
#include <logging/sys_log.h>

#ifdef __BEAR_SOC_H_
#include <uart_bear.h>
#endif


#define SIZE 256

static void uart_fifo_force_read(struct device *dev, uint8_t *buf, uint32_t size)
{
	SYS_LOG_DBG("reading size: %d", size);

    uint32_t status = 0;
    for (int i = 0; i < size; ) {
    	status = uart_poll_in(dev, &buf[i]);
    	if (!status) {
    		i++;
    	}
    }
}


static void poll_out_buf(struct device *dev, uint8_t *buf, uint32_t size)
{
	SYS_LOG_DBG("writing size: %d", size);

    for (uint32_t i = 0; i < size; i++) {
        uart_poll_out(dev, buf[i]);
    }
}


void main(void) 
{
    SYS_LOG_DBG("test begin");

	struct device *dev = device_get_binding(CONFIG_UART_BEAR_PORT_0_NAME);
	uint8_t buf[SIZE];

	while (1) {
		uart_fifo_force_read(dev, buf, SIZE);
		poll_out_buf(dev, buf, SIZE);
	}

#ifdef __BEAR_SOC_H_ //Send MSI
    while (!(uart_drv_cmd(dev, UART_TX_EMPTY_CMD, 0) && uart_drv_cmd(dev, UART_TX_IDLE_CMD, 0))) ;

    (*((volatile unsigned int *)(BEAR_PCIE_MSI_BASE))) = 1;
#endif
}