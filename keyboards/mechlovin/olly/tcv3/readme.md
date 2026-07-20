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
