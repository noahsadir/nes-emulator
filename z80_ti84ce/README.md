# NES Emulator for TI-84 CE

This is a crude port of my NES emulator to the TI-84 CE, mainly done out of curiosity. It does actually manage to run, thanks to the [CE C/C++ Toolchain](https://ce-programming.github.io/toolchain/). However, it runs at approximately 20 seconds per frame (0.05 FPS!), thus making it nowhere near playable.

Currently, in order to run a ROM, the binary must be represented as a `uint8_t` array, located in a separate header file (see `rom.h`). Though it may be possible to convert the iNES rom file to a valid TI-84 format like other emulators, it's simply not worth the effort.

## Compile

- Configure `rom.h` with your desired ROM
- [Follow these instructions](https://ce-programming.github.io/toolchain/static/getting-started.html) in order to generate an executable which can be installed on the calculator.
