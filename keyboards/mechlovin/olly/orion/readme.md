# Olly Orion

![Olly Orion](https://i.imgur.com/midAJSplh.jpeg)

The Olly Orion is designed as a drop-in replacement PCB for the Duck Orion V2; V2.5 and V3 TKL custom keyboards. 

* Keyboard Maintainer: [Mechlovin' Studio](https://github.com/mechlovin)
* Hardware Supported: Olly Orion, APM32F103.
* Hardware Availability: [Mechlovin' Studio](https://mechlovin.studio), [GB](https://www.reddit.com/r/mechmarket/comments/z1i5g3/gb_mechlovin_olly_orion_octagon_duck_orion/).

Make example for this keyboard (after setting up your build environment):

    make mechlovin/olly/orion:default

Flashing example for this keyboard:

    make mechlovin/olly/orion/default:flash

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

**Reset Key:** 3 ways to put the IF87.5 into bootloader:
- By keycode: Tap RESET keycode.
- By bootmagic: hold ESC key while plugging in.
- By hardware: Push reset button on bottom of the PCB while the PCB is plugged in.

## Status LED brightness and fading

The three lock indicators (Caps/Num/Scroll) and the five layer LEDs are driven by a
software PWM engine rather than plain on/off GPIO, so they sit at a reduced brightness
and fade smoothly when they change state. Which LED lights under which condition is
unchanged; only brightness and the transition differ.

Num Lock and Scroll Lock sit on `A13`/`A14` (SWDIO/SWCLK), which have no timer
alternate function, so hardware PWM is not possible for them — all eight LEDs use the
same software path instead.

The defaults live in `keyboards/mechlovin/olly/orion/config.h` and can be overridden
from a keymap's own `config.h`:

| Define | Default | Meaning |
|---|---|---|
| `ORION_LED_ON_LEVEL` | `120` | Brightness of an LED in the "on" state, 0–255 perceptual. Gamma corrected onto the PWM duty range; 120 gives ~22% duty. Use `255` for full brightness. With VIA this is only the power-on default. |
| `ORION_LED_FADE_INTERVAL_MS` | `5` | How often the fade ramp advances. |
| `ORION_LED_FADE_STEP` | `8` | Levels per step when fading **in**. |
| `ORION_LED_FADE_OUT_STEP` | `5` | Levels per step when fading **out**. Smaller than the fade-in step so LEDs decay like an incandescent; the ratio 8/5 makes switching off 1.6× longer than switching on. Set equal to `ORION_LED_FADE_STEP` for a symmetric fade. |
| `ORION_LED_PWM_LEVELS` | `32` | PWM steps per frame. Must be a power of two. |
| `ORION_LED_TICK_US` | `160` | PWM tick period. 160 µs × 32 levels ≈ 195 Hz frame rate. |
| `ORION_LED_ACTIVE_LOW` | `0` | Set to `1` if the LEDs are lit by driving the pin low. |

At the default on-level that gives roughly a 75 ms fade in and 120 ms fade out. Both
scale with brightness, because the ramp travels `0..ORION_LED_ON_LEVEL`.

To get the stock look back, set `ORION_LED_ON_LEVEL` to `255` and both fade steps to
`255`.

### Adjusting brightness from VIA

The `via` keymap exposes a **Status LEDs → Indicators → Brightness** slider (0–255) that
changes the indicator brightness live and stores it in EEPROM, so it survives a replug.
It uses the keyboard-specific VIA channel (`id_custom_channel`, value id 1) and two bytes
of `VIA_EEPROM_CUSTOM_CONFIG_SIZE` — a magic byte plus the level. The magic byte is what
lets a never-written EEPROM fall back to `ORION_LED_ON_LEVEL`, since `0x00` and `0xFF`
are both legitimate brightness values.

The matching VIA definition is `orion_via.json` in this folder; load it via VIA's Design
tab (Settings → Show Design tab).

## VIA

    make mechlovin/olly/orion:via

The stm32duino bootloader leaves only 56K of flash (`0x08002000`-`0x08010000`), and the
base firmware already fills ~80% of it, so the `via` keymap enables `LTO_ENABLE` to make
room. With LTO the VIA build is actually smaller than the non-LTO default one.

Emulated EEPROM is 1K (wear levelling over MCU flash), of which VIA's dynamic keymaps
take 816 bytes at the default 4 layers (4 x 6 rows x 17 cols x 2). That leaves little
room for macros, and it is why `DYNAMIC_KEYMAP_LAYER_COUNT` is not raised to 5 to match
the five layer LEDs -- 5 layers would need 1020 bytes and fail the build. Raising
`WEAR_LEVELING_BACKING_SIZE` to 4096 would buy the space at the cost of 2K more flash.
