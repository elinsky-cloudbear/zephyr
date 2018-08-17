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

#define BUF_SIZE 255


static const char *fifo_data;
static const char *fifo_data_default = "This is a FIFO test.\r\n";

static volatile unsigned int char_sent;
static volatile unsigned int char_recev;

static volatile bool data_transmitted;
static volatile bool data_received;
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

            if (recev_data[char_recev - 1] == '\r' ||
                recev_data[char_recev - 1] == '\n' || buf_size_out) {
                    data_received = true;
                    uart_irq_rx_disable(dev);

                    if (buf_size_out) {
                	    char_recev = BUF_SIZE - 2;
                    }
                
                    recev_data[char_recev - 1] = '\r';
                    recev_data[char_recev] = '\n';
                    recev_data[char_recev + 1] = '\0';
            }
        }
    }
}


static bool test_fifo_fill(struct device *uart_dev, const char *output)
{
    if (!uart_dev) {
        printk("Cannot get UART device\r\n");
        return false;
    }

    char_sent = 0;
    data_transmitted = false;
    fifo_data = output;

    uart_irq_callback_set(uart_dev, uart_fifo_callback);
    uart_irq_tx_enable(uart_dev);

    while (data_transmitted == false) ;

    fifo_data = fifo_data_default;

    return true;
}


// call test_fifo_fill
static bool test_fifo_read(struct device *uart_dev, const char *output)
{
    if (!uart_dev) {
        printk("Cannot get UART device\r\n");
        return false;
    }

    test_fifo_fill(uart_dev, output);

    char_recev = 0;
    memset(recev_data, '\0', sizeof(recev_data));
    data_received = false;

    uart_irq_callback_set(uart_dev, uart_fifo_callback);
    uart_irq_rx_enable(uart_dev);

    while (data_received == false) ;

    return true;
}


static bool test_poll_out(struct device *uart_dev, const char *output)
{
    if (!uart_dev) {
        printk("Cannot get UART device\r\n");
        return false;
    }

    for (int i = 0; i < strlen(output); i++) {
        uart_poll_out(uart_dev, output[i]);
    }

    return true;
}


// call test_poll_out
static bool test_poll_in(struct device *uart_dev, const char *output)
{
    if (!uart_dev) {
        printk("Cannot get UART device\r\n");
        return false;
    }

    test_poll_out(uart_dev, output);

    unsigned char recvChar;
    char_recev = 0;

    while (1) {
        while (uart_poll_in(uart_dev, &recvChar) != 0) ;

        recev_data[char_recev++] = recvChar;
        buf_size_out = (char_recev > (BUF_SIZE - 3));

        if (recvChar == '\r' || recvChar == '\n' || buf_size_out) {
            if (buf_size_out) {
                char_recev = BUF_SIZE - 2;
            }

            recev_data[char_recev - 1] = '\r';
            recev_data[char_recev] = '\n';
            recev_data[char_recev + 1] = '\0';

            break;
        }
    }

    return true;
}


void main(void)
{
    //(*((volatile unsigned int *)(0x10015200))) = 1;
    struct device *uart_dev_0 = device_get_binding(CONFIG_UART_CONSOLE_ON_DEV_NAME);
    struct device *uart_dev_1 = device_get_binding(CONFIG_UART_BEAR_PORT_1_NAME);

    test_poll_in(uart_dev_0, "POLL. Send characters: console -> BT\r\n");
    test_poll_out(uart_dev_1, recev_data);

    test_poll_in(uart_dev_1, "POLL. Send characters: BT -> console\r\n");
    test_poll_out(uart_dev_0, recev_data);

    test_fifo_read(uart_dev_0, "FIFO. Send characters: console -> BT\r\n");
    test_fifo_fill(uart_dev_1, recev_data);

    test_fifo_read(uart_dev_1, "FIFO. Send characters: BT -> console\r\n");
    test_fifo_fill(uart_dev_0, recev_data);

    printk("UART test(s): Done %s\r\n", CONFIG_ARCH);

#ifdef __BEAR_SOC_H_ //Send MSI
    while (!(uart_drv_cmd(uart_dev_0, UART_TX_EMPTY_CMD, 0) && uart_drv_cmd(uart_dev_0, UART_TX_IDLE_CMD, 0))) ;

    (*((volatile unsigned int *)(BEAR_PCIE_MSI_BASE))) = 1;
#endif
}
