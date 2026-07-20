/* Copyright 2023 Mechlovin'
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#pragma once

/* The IS31FL3731 AD pin is tied to SDA (7-bit address 0x76). */
#define IS31FL3731_I2C_ADDRESS_1 IS31FL3731_I2C_ADDRESS_SDA

/* I2C1 uses the STM32F103 default pins: PB6=SCL, PB7=SDA. */
#define I2C1_CLOCK_SPEED 400000
#define I2C1_DUTY_CYCLE FAST_DUTY_CYCLE_2
