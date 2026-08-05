// Copyright 2026 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/* Dimmable status LEDs (soft PWM).
 *
 * The 3 lock indicators (B10/A13/A14) and the 5 layer LEDs (B5-B9) are driven
 * by a software PWM engine in orion.c instead of plain on/off GPIO, so they can
 * sit at a reduced brightness and fade between states.
 *
 * A13/A14 are SWDIO/SWCLK and have no timer alternate function, so hardware PWM
 * is impossible for num/scroll lock -- hence soft PWM for all eight, keeping a
 * single code path.
 *
 * Every value below can be overridden from a keymap's own config.h.
 */

/* Number of PWM steps per frame. Must be a power of two. */
#ifndef ORION_LED_PWM_LEVELS
#    define ORION_LED_PWM_LEVELS 32
#endif

/* PWM tick period in microseconds. 160us * 32 levels ~= 195 Hz frame rate.
 * The ChibiOS system tick is 100 kHz here, so this quantises to exactly 16
 * ticks. Raise it (or lower ORION_LED_PWM_LEVELS) if the WS2812 underglow
 * shows artefacts; anything above ~100 Hz frame rate stays flicker-free. */
#ifndef ORION_LED_TICK_US
#    define ORION_LED_TICK_US 160
#endif

/* Brightness of an LED in the "on" state, 0-255 perceptual. Gamma corrected
 * onto the PWM duty range, so the default 144 lands on a duty of 10/31 (~32%)
 * -- noticeably dimmer than the stock always-full-on behaviour. */
#ifndef ORION_LED_ON_LEVEL
#    define ORION_LED_ON_LEVEL 144
#endif

/* Fade ramp: ORION_LED_FADE_STEP units of the 0-255 scale every
 * ORION_LED_FADE_INTERVAL_MS. Defaults give a ~96 ms fade at the default
 * on-level (144 / 12 = 12 steps * 8 ms). */
#ifndef ORION_LED_FADE_INTERVAL_MS
#    define ORION_LED_FADE_INTERVAL_MS 8
#endif
#ifndef ORION_LED_FADE_STEP
#    define ORION_LED_FADE_STEP 12
#endif

/* Set to 1 if the LEDs sink current through the MCU pin (lit when driven low).
 * Stock firmware drove these pins high for "on", so the default is active-high. */
#ifndef ORION_LED_ACTIVE_LOW
#    define ORION_LED_ACTIVE_LOW 0
#endif
