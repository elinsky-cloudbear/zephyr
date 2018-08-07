/*
 * Copyright (c) 2017 Alexander Kozlov <alexander.kozlov@cloudbear.ru>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file SoC configuration macros for the BEAR SoC
 */

#ifndef __BEAR_BASE_SOC_H_
#define __BEAR_BASE_SOC_H_

#include <soc_ext.h>
#include <soc_common.h>

#include <misc/util.h>

// lib-c hooks required RAM defined variables
#define RISCV_RAM_BASE             CONFIG_RISCV_RAM_BASE_ADDR
#define RISCV_RAM_SIZE             KB(CONFIG_RISCV_RAM_SIZE_KB)

#endif /* __BEAR_BASE_SOC_H_ */
