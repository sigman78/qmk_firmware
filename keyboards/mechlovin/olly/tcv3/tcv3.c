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

void keyboard_post_init_kb(void) {
#ifdef RGBLIGHT_ENABLE
    rgblight_sethsv_at(255, 255, 255, 0);
#endif
#ifdef LED_MATRIX_ENABLE
    /* Center/marker LED in the six-LED layer display. */
    led_matrix_set_value(107, 0xFF);
#endif
    keyboard_post_init_user();
}

bool led_matrix_indicators_kb(void) {
    if (!led_matrix_indicators_user()) {
        return false;
    }

    led_t host_leds = host_keyboard_led_state();
    if (host_leds.caps_lock) {
        led_matrix_set_value(55, 0xFF); /* Caps Lock switch LED */
    }
    led_matrix_set_value(101, host_leds.num_lock ? 0xFF : 0x00);
    led_matrix_set_value(102, host_leds.caps_lock ? 0xFF : 0x00);
    led_matrix_set_value(103, host_leds.scroll_lock ? 0xFF : 0x00);
    led_matrix_set_value(107, 0xFF);
    return true;
}

layer_state_t layer_state_set_kb(layer_state_t state) {
    state         = layer_state_set_user(state);
    uint8_t layer = get_highest_layer(state);

    led_matrix_set_value(104, layer == 0 ? 0xFF : 0x00);
    led_matrix_set_value(105, layer == 1 ? 0xFF : 0x00);
    led_matrix_set_value(106, layer == 2 ? 0xFF : 0x00);
    led_matrix_set_value(108, layer == 3 ? 0xFF : 0x00);

    return state;
}
