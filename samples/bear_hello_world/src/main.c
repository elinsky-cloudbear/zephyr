/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <misc/printk.h>
#include <uart.h>

#ifdef __BEAR_SOC_H_
#include <uart_bear.h>
#endif

void main(void)
{
    printk("Hello CloudBEAR!!! %s\n", CONFIG_ARCH);

    #ifdef __BEAR_SOC_H_ //Send MSI
        struct device *dev = device_get_binding(CONFIG_UART_CONSOLE_ON_DEV_NAME);

        while (!(uart_drv_cmd(dev, UART_TX_EMPTY_CMD, 0) && uart_drv_cmd(dev, UART_TX_IDLE_CMD, 0)))
        ;

        (*((volatile unsigned int *)(BEAR_PCIE_MSI_BASE))) = 1;
    #endif
}
