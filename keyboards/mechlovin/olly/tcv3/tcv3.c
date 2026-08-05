/* Copyright 2023 Mechlovin'
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 */

#include "quantum.h"

void board_init(void) {
    /* PA15 is a matrix row, so release the STM32F103 debug pins. */
    AFIO->MAPR = (AFIO->MAPR & ~AFIO_MAPR_SWJ_CFG_Msk) | AFIO_MAPR_SWJ_CFG_DISABLE;
}

const is31fl3731_led_t PROGMEM g_is31fl3731_leds[IS31FL3731_LED_COUNT] = {
    {0, C1_1},  {0, C2_1},  {0, C3_1},               {0, C5_1},  {0, C6_1},  {0, C7_1},  {0, C8_1},  {0, C9_1},  {0, C1_9},  {0, C2_9},  {0, C3_9},  {0, C4_9},  {0, C5_9},  {0, C6_9},  {0, C7_9},  {0, C8_9},  {0, C9_9},
    {0, C1_2},  {0, C2_2},  {0, C3_2},  {0, C4_2},  {0, C5_2},  {0, C6_2},  {0, C7_2},  {0, C8_2},  {0, C9_2},  {0, C1_10}, {0, C2_10}, {0, C3_10}, {0, C4_10}, {0, C5_10}, {0, C6_10}, {0, C7_10}, {0, C8_10}, {0, C9_10},
    {0, C1_3},  {0, C2_3},  {0, C3_3},  {0, C4_3},  {0, C5_3},  {0, C6_3},  {0, C7_3},  {0, C8_3},  {0, C9_3},  {0, C1_11}, {0, C2_11}, {0, C3_11}, {0, C4_11}, {0, C5_11}, {0, C6_11}, {0, C7_11}, {0, C8_11}, {0, C9_11},
    {0, C1_4},  {0, C2_4},  {0, C3_4},  {0, C4_4},  {0, C5_4},  {0, C6_4},  {0, C7_4},  {0, C8_4},  {0, C9_4},  {0, C1_12}, {0, C2_12}, {0, C3_12}, {0, C4_12}, {0, C5_12}, {0, C6_12}, {0, C7_12}, {0, C8_12}, {0, C9_12},
    {0, C1_5},  {0, C2_5},  {0, C3_5},  {0, C4_5},  {0, C5_5},  {0, C6_5},  {0, C7_5},  {0, C8_5},  {0, C9_5},  {0, C1_13}, {0, C2_13}, {0, C3_13}, {0, C4_13}, {0, C5_13}, {0, C6_13}, {0, C7_13}, {0, C8_13}, {0, C9_13},
    {0, C1_6},  {0, C2_6},  {0, C3_6},  {0, C4_6},  {0, C5_6},                                           {0, C5_8},                                           {0, C4_14}, {0, C5_14}, {0, C6_14}, {0, C7_14}, {0, C8_14}, {0, C9_14},

    /* Lock indicators: Num, Caps, Scroll. */
    {0, C6_6}, {0, C8_6}, {0, C7_6},

    /* Six layer-indicator LEDs. */
    {0, C5_15}, {0, C4_15}, {0, C4_16}, {0, C6_15}, {0, C6_16}, {0, C5_16},
};

#ifdef LED_MATRIX_ENABLE

/* Dimmed, fading status indicators -- see config.h for the tunables.
 *
 * Unlike the Olly Orion, which drives its indicators straight off GPIO and
 * needs a software PWM engine to dim them, these sit on the IS31FL3731 and
 * already have 8-bit hardware PWM per LED. Dimming is just writing a smaller
 * value; all that is needed on top is a ramp so state changes fade. */

enum tcv3_indicator {
    TCV3_IND_CAPS_KEY, /* Caps Lock *switch* LED, part of the key matrix */
    TCV3_IND_NUM_LOCK,
    TCV3_IND_CAPS_LOCK,
    TCV3_IND_SCROLL_LOCK,
    TCV3_IND_LAYER_0,
    TCV3_IND_LAYER_1,
    TCV3_IND_LAYER_2,
    TCV3_IND_LAYER_3,
    TCV3_IND_MARKER, /* centre marker of the six-LED layer display */
    TCV3_IND_COUNT,
};

static const uint8_t tcv3_indicator_led[TCV3_IND_COUNT] = {
    [TCV3_IND_CAPS_KEY]    = 55,
    [TCV3_IND_NUM_LOCK]    = 101,
    [TCV3_IND_CAPS_LOCK]   = 102,
    [TCV3_IND_SCROLL_LOCK] = 103,
    [TCV3_IND_LAYER_0]     = 104,
    [TCV3_IND_LAYER_1]     = 105,
    [TCV3_IND_LAYER_2]     = 106,
    [TCV3_IND_LAYER_3]     = 108,
    [TCV3_IND_MARKER]      = 107,
};

static uint8_t tcv3_indicator_level[TCV3_IND_COUNT];
static uint8_t tcv3_indicator_target[TCV3_IND_COUNT];

/* Perceptual level (0-255) to PWM duty. Squaring roughly cancels the eye's
 * response, so a linear ramp of `level` reads as a linear fade. */
static inline uint8_t tcv3_indicator_gamma(uint8_t level) {
    return (uint8_t)(((uint16_t)level * level) / 255U);
}

static inline void tcv3_indicator_set(uint8_t index, bool on) {
    tcv3_indicator_target[index] = on ? TCV3_LED_ON_LEVEL : 0;
}

void housekeeping_task_kb(void) {
    static uint32_t last_fade = 0;

    if (timer_elapsed32(last_fade) >= TCV3_LED_FADE_INTERVAL_MS) {
        last_fade = timer_read32();

        for (uint8_t i = 0; i < TCV3_IND_COUNT; i++) {
            uint8_t level  = tcv3_indicator_level[i];
            uint8_t target = tcv3_indicator_target[i];

            if (level == target) {
                continue;
            }
            /* Asymmetric: the downward ramp is slower, so an indicator going
             * out decays like an incandescent rather than snapping off. */
            if (target > level) {
                level = (target - level > TCV3_LED_FADE_STEP) ? level + TCV3_LED_FADE_STEP : target;
            } else {
                level = (level - target > TCV3_LED_FADE_OUT_STEP) ? level - TCV3_LED_FADE_OUT_STEP : target;
            }
            tcv3_indicator_level[i] = level;
        }
    }
    /* No housekeeping_task_user() call -- housekeeping_task() invokes the _kb
     * and _user hooks separately, unlike the *_init_kb hooks. */
}

#endif // LED_MATRIX_ENABLE

void keyboard_post_init_kb(void) {
#ifdef RGBLIGHT_ENABLE
    rgblight_sethsv_at(255, 255, 255, 0);
#endif
#ifdef LED_MATRIX_ENABLE
    /* Light the default layer and the marker at boot so they fade up rather
     * than waiting for the first layer change. */
    tcv3_indicator_set(TCV3_IND_MARKER, true);
    tcv3_indicator_set(TCV3_IND_LAYER_0, get_highest_layer(layer_state) == 0);
#endif
    keyboard_post_init_user();
}

#ifdef LED_MATRIX_ENABLE

bool led_matrix_indicators_kb(void) {
    if (!led_matrix_indicators_user()) {
        return false;
    }

    led_t host_leds = host_keyboard_led_state();
    tcv3_indicator_set(TCV3_IND_CAPS_KEY, host_leds.caps_lock);
    tcv3_indicator_set(TCV3_IND_NUM_LOCK, host_leds.num_lock);
    tcv3_indicator_set(TCV3_IND_CAPS_LOCK, host_leds.caps_lock);
    tcv3_indicator_set(TCV3_IND_SCROLL_LOCK, host_leds.scroll_lock);
    tcv3_indicator_set(TCV3_IND_MARKER, true);

    for (uint8_t i = 0; i < TCV3_IND_COUNT; i++) {
        uint8_t level = tcv3_indicator_level[i];

        /* The Caps Lock switch LED is a normal key LED: only override it while
         * it has something to show, otherwise hand it back to the running
         * matrix effect instead of pinning it to black. The dedicated
         * indicators have no effect behind them, so they are always written. */
        if (i == TCV3_IND_CAPS_KEY && level == 0) {
            continue;
        }
        led_matrix_set_value(tcv3_indicator_led[i], tcv3_indicator_gamma(level));
    }
    return true;
}

#endif // LED_MATRIX_ENABLE

layer_state_t layer_state_set_kb(layer_state_t state) {
    state = layer_state_set_user(state);
#ifdef LED_MATRIX_ENABLE
    uint8_t layer = get_highest_layer(state);

    tcv3_indicator_set(TCV3_IND_LAYER_0, layer == 0);
    tcv3_indicator_set(TCV3_IND_LAYER_1, layer == 1);
    tcv3_indicator_set(TCV3_IND_LAYER_2, layer == 2);
    tcv3_indicator_set(TCV3_IND_LAYER_3, layer == 3);
#endif
    return state;
}
