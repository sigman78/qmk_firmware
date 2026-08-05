# Olly TCV3

The Olly TCV3 is a drop-in replacement PCB for the Duck TCV3 keyboard.

* Keyboard Maintainer: [Mechlovin' Studio](https://github.com/mechlovin)
* Hardware Supported: Olly TCV3, STM32F103-compatible MCU.
* Hardware Availability: [Mechlovin' Studio](https://mechlovin.studio).

Make example for this keyboard (after setting up your build environment):

    qmk compile -kb mechlovin/olly/tcv3 -km default

Flashing example for this keyboard:

    qmk flash -kb mechlovin/olly/tcv3 -km default

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

**Reset Key:** three ways to put the PCB into its bootloader:
- By keycode: Tap the `QK_BOOT` keycode.
- By bootmagic: hold ESC key while plugging in.
- By hardware: Push reset button on bottom of the PCB while the PCB is plugged in.

## Status LED brightness and fading

The lock indicators and the layer display sit at a reduced brightness and fade
smoothly when they change state. Which indicator lights under which condition is
unchanged; only brightness and the transition differ.

These LEDs are on the IS31FL3731, which already provides 8-bit hardware PWM per
LED, so dimming is just writing a smaller value — no software PWM is involved.
(The sister board `mechlovin/olly/orion` drives its indicators straight off GPIO
and does need a soft-PWM engine for the same effect.)

Defaults live in `config.h` and can be overridden from a keymap's own `config.h`:

| Define | Default | Meaning |
|---|---|---|
| `TCV3_LED_ON_LEVEL` | `120` | Brightness of an indicator in the "on" state, 0–255 perceptual. Gamma corrected before it reaches the driver; 120 gives ~22% duty. Use `255` for the stock full brightness. |
| `TCV3_LED_FADE_INTERVAL_MS` | `5` | How often the fade ramp advances. |
| `TCV3_LED_FADE_STEP` | `8` | Levels per step when fading **in**. |
| `TCV3_LED_FADE_OUT_STEP` | `5` | Levels per step when fading **out**. Smaller than the fade-in step so indicators decay like an incandescent; the ratio 8/5 makes switching off 1.6× longer than switching on. Set equal to `TCV3_LED_FADE_STEP` for a symmetric fade. |

At the default on-level that is roughly a 75 ms fade in and 120 ms fade out. Both
scale with brightness, because the ramp travels `0..TCV3_LED_ON_LEVEL`.

The Caps Lock *switch* LED is treated differently from the dedicated indicators:
it is only overridden while it has something to show, so once it has finished
fading out the running LED matrix effect takes it back rather than it being
pinned dark.
