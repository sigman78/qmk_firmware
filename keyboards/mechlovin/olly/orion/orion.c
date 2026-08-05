/* Copyright 2022 Mechlovin' Studio
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "quantum.h"
#include <ch.h>

/* Soft-PWM status LEDs -- see config.h for the tunables. */

#if (ORION_LED_PWM_LEVELS & (ORION_LED_PWM_LEVELS - 1)) != 0
#    error "ORION_LED_PWM_LEVELS must be a power of two"
#endif

#define ORION_LED_PWM_MAX (ORION_LED_PWM_LEVELS - 1)

#if ORION_LED_ACTIVE_LOW
#    define ORION_LED_PIN_LEVEL(on) (!(on))
#else
#    define ORION_LED_PIN_LEVEL(on) (on)
#endif

enum orion_led_index {
    ORION_LED_CAPS_LOCK,
    ORION_LED_NUM_LOCK,
    ORION_LED_SCROLL_LOCK,
    ORION_LED_LAYER_0,
    ORION_LED_LAYER_1,
    ORION_LED_LAYER_2,
    ORION_LED_LAYER_3,
    ORION_LED_LAYER_4,
    ORION_LED_COUNT,
};

static const pin_t orion_led_pins[ORION_LED_COUNT] = {
    [ORION_LED_CAPS_LOCK]   = B10,
    [ORION_LED_NUM_LOCK]    = A13, // SWDIO, freed by board_init()
    [ORION_LED_SCROLL_LOCK] = A14, // SWCLK, freed by board_init()
    [ORION_LED_LAYER_0]     = B7,
    [ORION_LED_LAYER_1]     = B6,
    [ORION_LED_LAYER_2]     = B5,
    [ORION_LED_LAYER_3]     = B8,
    [ORION_LED_LAYER_4]     = B9,
};

static uint8_t          orion_led_level[ORION_LED_COUNT];  // 0-255, ramped towards target
static uint8_t          orion_led_target[ORION_LED_COUNT]; // 0-255, set by the LED/layer hooks
static volatile uint8_t orion_led_duty[ORION_LED_COUNT];   // 0-ORION_LED_PWM_MAX, read by the ISR

static virtual_timer_t orion_led_vt;
static uint8_t         orion_led_phase;

/* Perceptual level (0-255) to PWM duty. The squared response roughly cancels
 * the eye's logarithmic one, so a linear ramp of `level` looks like a linear
 * fade. Endpoints are exact: 0 -> 0, 255 -> ORION_LED_PWM_MAX. */
static inline uint8_t orion_led_gamma(uint8_t level) {
    return (uint8_t)(((uint32_t)level * level * ORION_LED_PWM_MAX) / (255UL * 255UL));
}

static inline void orion_led_set(uint8_t index, bool on) {
    orion_led_target[index] = on ? ORION_LED_ON_LEVEL : 0;
}

static void orion_led_set_layers(layer_state_t state) {
    orion_led_set(ORION_LED_LAYER_0, layer_state_cmp(state, 0));
    orion_led_set(ORION_LED_LAYER_1, layer_state_cmp(state, 1));
    orion_led_set(ORION_LED_LAYER_2, layer_state_cmp(state, 2));
    orion_led_set(ORION_LED_LAYER_3, layer_state_cmp(state, 3));
    orion_led_set(ORION_LED_LAYER_4, layer_state_cmp(state, 4));
}

/* ISR context, invoked outside the kernel lock by the virtual timer list.
 * Keep it to the eight pin writes -- no loops, no floats, no QMK API beyond
 * gpio_write_pin(). */
static void orion_led_pwm_cb(virtual_timer_t *vtp, void *arg) {
    (void)vtp;
    (void)arg;

    uint8_t phase   = (orion_led_phase + 1) & ORION_LED_PWM_MAX;
    orion_led_phase = phase;

    for (uint8_t i = 0; i < ORION_LED_COUNT; i++) {
        gpio_write_pin(orion_led_pins[i], ORION_LED_PIN_LEVEL(orion_led_duty[i] > phase));
    }
}

void board_init(void) {
   //JTAG-DP Disabled and SW-DP Enabled
   AFIO->MAPR = (AFIO->MAPR & ~AFIO_MAPR_SWJ_CFG_Msk) | AFIO_MAPR_SWJ_CFG_DISABLE;
}

void keyboard_pre_init_kb(void) {
    for (uint8_t i = 0; i < ORION_LED_COUNT; i++) {
        gpio_set_pin_output(orion_led_pins[i]);
        gpio_write_pin(orion_led_pins[i], ORION_LED_PIN_LEVEL(false));
    }
    keyboard_pre_init_user();
}

void keyboard_post_init_kb(void) {
    /* Light the default layer on boot; stock only updated the layer LEDs on a
     * layer *change*, leaving L0 dark until you toggled away and back. */
    orion_led_set_layers(layer_state);

    chVTObjectInit(&orion_led_vt);
    chVTSetContinuous(&orion_led_vt, TIME_US2I(ORION_LED_TICK_US), orion_led_pwm_cb, NULL);

    keyboard_post_init_user();
}

void housekeeping_task_kb(void) {
    static uint32_t last_fade = 0;

    if (timer_elapsed32(last_fade) >= ORION_LED_FADE_INTERVAL_MS) {
        last_fade = timer_read32();

        for (uint8_t i = 0; i < ORION_LED_COUNT; i++) {
            uint8_t level  = orion_led_level[i];
            uint8_t target = orion_led_target[i];

            if (level == target) {
                continue;
            }
            if (target > level) {
                level = (target - level > ORION_LED_FADE_STEP) ? level + ORION_LED_FADE_STEP : target;
            } else {
                level = (level - target > ORION_LED_FADE_STEP) ? level - ORION_LED_FADE_STEP : target;
            }

            orion_led_level[i] = level;
            orion_led_duty[i]  = orion_led_gamma(level);
        }
    }
    /* No housekeeping_task_user() call here -- unlike the *_init_kb hooks,
     * housekeeping_task() invokes the _kb and _user hooks separately. */
}

bool led_update_kb(led_t led_state) {
    if (led_update_user(led_state)) {
        orion_led_set(ORION_LED_CAPS_LOCK, led_state.caps_lock);
        orion_led_set(ORION_LED_NUM_LOCK, led_state.num_lock);
        orion_led_set(ORION_LED_SCROLL_LOCK, led_state.scroll_lock);
    }
    /* Always false: the soft-PWM engine owns these pins, so led_update_ports()
     * must never drive them digitally. */
    return false;
}

layer_state_t layer_state_set_kb(layer_state_t state) {
    state = layer_state_set_user(state);
    orion_led_set_layers(state);
    return state;
}
