// Copyright 2026 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include_next <mcuconf.h>

/* WS2812 underglow on B15 is driven by SPI2 + DMA rather than bit-banged. */
#undef STM32_SPI_USE_SPI2
#define STM32_SPI_USE_SPI2 TRUE
