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

/* Dimmed, fading status indicators (lock LEDs + the layer display).
 *
 * These live on the IS31FL3731, which already does 8-bit hardware PWM per LED,
 * so dimming is simply writing a smaller value -- no software PWM needed. Only
 * the ramp is ours. All values can be overridden from a keymap's config.h.
 */

/* Brightness of an indicator in the "on" state, 0-255 perceptual. Gamma
 * corrected before it reaches the driver, so 120 lands on a duty of ~22%
 * instead of the stock always-full 0xFF. */
#ifndef TCV3_LED_ON_LEVEL
#    define TCV3_LED_ON_LEVEL 120
#endif

/* Fade ramp: move TCV3_LED_FADE_STEP units of the 0-255 scale every
 * TCV3_LED_FADE_INTERVAL_MS. Fade-out uses a smaller step, so switching off
 * decays more slowly than switching on -- the afterglow of an incandescent
 * indicator. The duration ratio is 8/5 = 1.6x; at the default on-level that is
 * roughly 75 ms in and 120 ms out. Both scale with brightness, since the ramp
 * travels 0..TCV3_LED_ON_LEVEL. Set the two steps equal for a symmetric fade. */
#ifndef TCV3_LED_FADE_INTERVAL_MS
#    define TCV3_LED_FADE_INTERVAL_MS 5
#endif
#ifndef TCV3_LED_FADE_STEP
#    define TCV3_LED_FADE_STEP 8
#endif
#ifndef TCV3_LED_FADE_OUT_STEP
#    define TCV3_LED_FADE_OUT_STEP 5
#endif
