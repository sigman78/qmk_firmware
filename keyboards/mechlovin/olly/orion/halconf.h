// Copyright 2026 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/* Needed by the SPI/DMA WS2812 driver -- see config.h for why the underglow is
 * not bit-banged. */
#define HAL_USE_SPI TRUE

#include_next <halconf.h>
