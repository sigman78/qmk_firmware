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
 * onto the PWM duty range, so the default 120 lands on a duty of 6/31 (~22%)
 * -- noticeably dimmer than the stock always-full-on behaviour.
 *
 * With VIA enabled this is only the power-on default; the live value is
 * adjustable from VIA's "Status LEDs" menu and stored in EEPROM. */
#ifndef ORION_LED_ON_LEVEL
#    define ORION_LED_ON_LEVEL 120
#endif

/* Optional: expose the brightness as a slider in VIA (Status LEDs > Indicators).
 *
 * OFF BY DEFAULT, and think before turning it on. It needs two bytes of VIA
 * custom config, which pushes DYNAMIC_KEYMAP_EEPROM_START up by two -- and
 * VIA's validity magic is derived from the product string, so it does NOT
 * notice the move. An EEPROM written by a build without this option is then
 * read back one keycode out of step: alphas mostly land on other alphas and
 * look fine, while modifiers, backspace and arrows come out as nonsense.
 *
 * So enabling this REQUIRES clearing the EEPROM afterwards. Leaving it off
 * keeps the stock VIA layout, and brightness stays a compile-time constant. */
#ifdef ORION_LED_VIA_BRIGHTNESS
#    define VIA_EEPROM_CUSTOM_CONFIG_SIZE 2
#endif

/* Fade ramp: move ORION_LED_FADE_STEP units of the 0-255 scale every
 * ORION_LED_FADE_INTERVAL_MS.
 *
 * Fade-out uses a smaller step than fade-in, so switching off decays more
 * slowly than switching on -- the afterglow of an incandescent indicator
 * rather than a symmetric ramp. The duration ratio is
 * ORION_LED_FADE_STEP / ORION_LED_FADE_OUT_STEP = 8/5 = 1.6x.
 *
 * At the default on-level of 120 that works out to ~75 ms in, ~120 ms out.
 * Both scale with brightness, since the ramp travels 0..ORION_LED_ON_LEVEL.
 * Set ORION_LED_FADE_OUT_STEP equal to ORION_LED_FADE_STEP for symmetry. */
#ifndef ORION_LED_FADE_INTERVAL_MS
#    define ORION_LED_FADE_INTERVAL_MS 5
#endif
#ifndef ORION_LED_FADE_STEP
#    define ORION_LED_FADE_STEP 8
#endif
#ifndef ORION_LED_FADE_OUT_STEP
#    define ORION_LED_FADE_OUT_STEP 5
#endif

/* Set to 1 if the LEDs sink current through the MCU pin (lit when driven low).
 * Stock firmware drove these pins high for "on", so the default is active-high. */
#ifndef ORION_LED_ACTIVE_LOW
#    define ORION_LED_ACTIVE_LOW 0
#endif
