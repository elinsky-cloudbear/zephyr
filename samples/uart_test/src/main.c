/*****************************************************************************
*
* @brief Main of UART test
*
* @author alexey.kovalov@cloudbear.ru
*
* @TODO: Finalize Rx implementation and errors handling
*
* Copyright (c) 2017 CloudBEAR LLC, all rights reserved.
* THIS PROGRAM IS AN UNPUBLISHED WORK FULLY PROTECTED BY COPYRIGHT LAWS AND
* IS CONSIDERED A TRADE SECRET BELONGING TO THE CLOUDBEAR LLC.
*
*****************************************************************************/

#include <zephyr.h>
#include <misc/printk.h>
#include <uart.h>

#include <string.h>

#ifdef __BEAR_SOC_H_
#include <uart_bear.h>
#endif

#define BUF_SIZE 250

static volatile bool data_transmitted;
static volatile bool data_received;

static const char *poll_data = "This is a POLL test.\r\n";
static const char *fifo_data = "This is a FIFO test.\r\n";

static volatile unsigned int char_sent;
static volatile unsigned int char_recev;

static volatile bool buf_size_out;

static char recev_data[BUF_SIZE];

static void uart_fifo_callback(struct device *dev)
{
    uart_irq_update(dev);

    if (uart_irq_tx_ready(dev)) {
        unsigned int data_size = strlen(fifo_data);
        char_sent += uart_fifo_fill(dev, (uint8_t *)(fifo_data + char_sent), data_size - char_sent);

        if (char_sent >= data_size) {
            data_transmitted = true;
            uart_irq_tx_disable(dev);
        }
    }


    if (uart_irq_rx_ready(dev)) {
        unsigned int read_num = uart_fifo_read(dev, (uint8_t *)(recev_data + char_recev),
                                               (BUF_SIZE - 2) - char_recev);  // \n\0 -> (BUF_SIZE - 2)
        if (read_num != 0) {
            char_recev += read_num;
            buf_size_out = (char_recev >= (BUF_SIZE - 3));

            if (recev_data[char_recev - 1] == '\r' || buf_size_out) {
                data_received = true;
                uart_irq_rx_disable(dev);

                if (buf_size_out) {
                	char_recev = BUF_SIZE - 2;
                	recev_data[char_recev - 1] = '\r';
                }
                
                recev_data[char_recev] = '\n';
                recev_data[char_recev + 1] = '\0';
            }
        }
    }
}

static bool test_fifo_read(void)
{
    struct device *uart_dev = device_get_binding(CONFIG_UART_CONSOLE_ON_DEV_NAME);

    char_recev = 0;
    memset(recev_data, '\0', sizeof(recev_data));
    data_received = false;

    uart_irq_callback_set(uart_dev, uart_fifo_callback);

    uart_irq_rx_enable(uart_dev);

    printk("Please send characters to serial console\n");

    while (data_received == false)
    ;

    printk("%s", recev_data);

    return true;
}

static bool test_fifo_fill(void)
{
    struct device *uart_dev = device_get_binding(CONFIG_UART_CONSOLE_ON_DEV_NAME);

    char_sent = 0;
    data_transmitted = false;

    uart_irq_callback_set(uart_dev, uart_fifo_callback);

    uart_irq_tx_enable(uart_dev);


    while (data_transmitted == false)
    ;

    return true;
}


static bool test_poll_in(void)
{
    struct device *uart_dev = device_get_binding(CONFIG_UART_CONSOLE_ON_DEV_NAME);

    if (!uart_dev) {
        printk("Cannot get UART device\n");
        return false;
    }

    printk("Please send characters to serial console\n");

    unsigned char recvChar;
    while (1) {
        while (uart_poll_in(uart_dev, &recvChar) != 0)
        ;

        printk("%c", recvChar);

        if (recvChar == '\r') {
            printk("\n");
            break;
        }
    }

    return true;
}

static bool test_poll_out(void)
{
    struct device *uart_dev = device_get_binding(CONFIG_UART_CONSOLE_ON_DEV_NAME);

    if (!uart_dev) {
        printk("Cannot get UART device\n");
        return false;
    }

    for (int i = 0; i < strlen(poll_data); i++) {
        uart_poll_out(uart_dev, poll_data[i]);
    }

    return true;
}



void main(void)
{
    printk("Test(s) started\r\n");

    test_poll_in();
    test_poll_out();
    test_fifo_fill();
    test_fifo_read();

    printk("UART test(s): Done %s\r\n", CONFIG_ARCH);

#ifdef __BEAR_SOC_H_ //Send MSI
    struct device *dev = device_get_binding(CONFIG_UART_CONSOLE_ON_DEV_NAME);

    while (!(uart_drv_cmd(dev, UART_TX_EMPTY_CMD, 0) && uart_drv_cmd(dev, UART_TX_IDLE_CMD, 0)))
    ;

    (*((volatile unsigned int *)(BEAR_PCIE_MSI_BASE))) = 1;
#endif
}
