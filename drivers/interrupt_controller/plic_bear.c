/*
 * Copyright (c) 2017 Alexander Kozlov <alexander.kozlov@cloudbear.ru>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <arch/cpu.h>
#include <init.h>
#include <soc.h>
#include <sw_isr_table.h>

#define PLIC_BEAR_IRQS        (CONFIG_NUM_IRQS - RISCV_MAX_GENERIC_IRQ)
#define PLIC_BEAR_EN_SIZE     ((PLIC_BEAR_IRQS + 32) >> 5)

struct plic_ctx_regs_t {
    uint32_t pri_thr; /* Interrupt priority threshold */
    uint32_t icc;   /* Interrupt Claiming/Completion Register */
};

static int save_irq;

/**
 *
 * @brief Enable a riscv PLIC-specific interrupt line
 *
 * This routine enables a RISCV PLIC-specific interrupt line.
 * riscv_plic_irq_enable is called by SOC_FAMILY_RISCV_PRIVILEGE
 * _arch_irq_enable function to enable external interrupts for
 * IRQS > RISCV_MAX_GENERIC_IRQ, whenever CONFIG_RISCV_HAS_PLIC
 * variable is set.
 * @param irq IRQ number to enable
 *
 * @return N/A
 */
void riscv_plic_irq_enable(uint32_t irq)
{
    uint32_t key;
    uint32_t bear_irq = irq - RISCV_MAX_GENERIC_IRQ;
    volatile uint32_t *en =
        (volatile uint32_t *)BEAR_PLIC_IEM_BASE;

    key = irq_lock();
    en += (bear_irq >> 5);
    *en |= (1 << (bear_irq & 31));
    irq_unlock(key);
}

/**
 *
 * @brief Disable a riscv PLIC-specific interrupt line
 *
 * This routine disables a RISCV PLIC-specific interrupt line.
 * riscv_plic_irq_disable is called by SOC_FAMILY_RISCV_PRIVILEGE
 * _arch_irq_disable function to disable external interrupts, for
 * IRQS > RISCV_MAX_GENERIC_IRQ, whenever CONFIG_RISCV_HAS_PLIC
 * variable is set.
 * @param irq IRQ number to disable
 *
 * @return N/A
 */
void riscv_plic_irq_disable(uint32_t irq)
{
    uint32_t key;
    uint32_t bear_irq = irq - RISCV_MAX_GENERIC_IRQ;
    volatile uint32_t *en =
        (volatile uint32_t *)BEAR_PLIC_IEM_BASE;

    key = irq_lock();
    en += (bear_irq >> 5);
    *en &= ~(1 << (bear_irq & 31));
    irq_unlock(key);
}

/**
 *
 * @brief Check if a riscv PLIC-specific interrupt line is enabled
 *
 * This routine checks if a RISCV PLIC-specific interrupt line is enabled.
 * @param irq IRQ number to check
 *
 * @return 1 or 0
 */
int riscv_plic_irq_is_enabled(uint32_t irq)
{
    volatile uint32_t *en =
        (volatile uint32_t *)BEAR_PLIC_IEM_BASE;
    uint32_t bear_irq = irq - RISCV_MAX_GENERIC_IRQ;

    en += (bear_irq >> 5);
    return !!(*en & (1 << (bear_irq & 31)));
}

/**
 *
 * @brief Set priority of a riscv PLIC-specific interrupt line
 *
 * This routine set the priority of a RISCV PLIC-specific interrupt line.
 * riscv_plic_irq_set_prio is called by riscv32 _ARCH_IRQ_CONNECT to set
 * the priority of an interrupt whenever CONFIG_RISCV_HAS_PLIC variable is set.
 * @param irq IRQ number for which to set priority
 *
 * @return N/A
 */
void riscv_plic_set_priority(uint32_t irq, uint32_t priority)
{
    volatile uint32_t *pri =
        (volatile uint32_t *)BEAR_PLIC_PRI_BASE;

    volatile uint32_t *max_pri =
        (volatile uint32_t *)BEAR_SOC_MAX_PRI_ADDR;

    /* Can set priority only for PLIC-specific interrupt line */
    if (irq <= RISCV_MAX_GENERIC_IRQ)
        return;

    if (priority > *max_pri)
        priority = *max_pri;

    pri += (irq - RISCV_MAX_GENERIC_IRQ);
    *pri = priority;
}

/**
 *
 * @brief Get riscv PLIC-specific interrupt line causing an interrupt
 *
 * This routine returns the RISCV PLIC-specific interrupt line causing an
 * interrupt.
 * @param irq IRQ number for which to set priority
 *
 * @return N/A
 */
int riscv_plic_get_irq(void)
{
    return save_irq;
}

static void plic_bear_irq_handler(void *arg)
{
    volatile struct plic_ctx_regs_t *regs =
        (volatile struct plic_ctx_regs_t *)BEAR_PLIC_CTX_BASE;

    uint32_t irq;
    struct _isr_table_entry *ite;

    /* Get the IRQ number generating the interrupt */
    irq = regs->icc;

    /*
     * Save IRQ in save_irq. To be used, if need be, by
     * subsequent handlers registered in the _sw_isr_table table,
     * as IRQ number held by the claim_complete register is
     * cleared upon read.
     */
    save_irq = irq;

    /* Only process IRQ which is in range, two cases possible:
     * 1. icc could returned (-1) if no interrupt for this hart
     * 2. Some error occured (not valid interrupt number)
     */
    if ((irq != 0) && (irq <= PLIC_BEAR_IRQS)) {
        irq += RISCV_MAX_GENERIC_IRQ;

        /* Call the corresponding IRQ handler in _sw_isr_table */
        ite = (struct _isr_table_entry *)&_sw_isr_table[irq];
        ite->isr(ite->arg);

        /*
         * Claim IRQ
         */
        regs->icc = save_irq;
    }
}

/**
 *
 * @brief Initialize the BEAR Platform Level Interrupt Controller
 * @return N/A
 */
static int plic_bear_init(struct device *dev)
{
    ARG_UNUSED(dev);

    volatile uint32_t *en =
        (volatile uint32_t *)BEAR_PLIC_IEM_BASE;
    volatile uint32_t *pri =
        (volatile uint32_t *)BEAR_PLIC_PRI_BASE;
    volatile struct plic_ctx_regs_t *regs =
        (volatile struct plic_ctx_regs_t *)BEAR_PLIC_CTX_BASE;

    int i;

    /* Ensure that all interrupts are disabled initially */
    for (i = 0; i < PLIC_BEAR_EN_SIZE; i++) {
        *en = 0;
        en++;
    }

    /* Set priority of each interrupt line to 0 initially */
    for (i = 0; i < PLIC_BEAR_IRQS; i++) {
        *pri = 0;
        pri++;
    }

    /* Set threshold priority to 0 */
    regs->pri_thr = 0;

    /* Setup IRQ handler for PLIC driver */
    IRQ_CONNECT(RISCV_MACHINE_EXT_IRQ,
            0,
            plic_bear_irq_handler,
            NULL,
            0);

    /* Enable IRQ for PLIC driver */
    irq_enable(RISCV_MACHINE_EXT_IRQ);

    return 0;
}

SYS_INIT(plic_bear_init, PRE_KERNEL_1, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT);
