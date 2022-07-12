# NES Emulator

A **very** basic NES Emulator written entirely in C, with portability in mind.

#### Quick Note

I mostly wrote this for fun, and it certainly isn't near feature complete.

#### Another Note

The details below pertain to the x86 emulator. I may attempt to port it to the TI-84 CE, but it may not be feasible due to the hardware limitations and the sheer number of optimizations which need to be done.

#### Dependencies
- SDL2

#### References and Helpful Links

Most of the code is original, however much of the logic was derived from the Nesdev Wiki, along with some logic from the tutorial linked below.

[Nesdev Wiki](https://www.nesdev.org/wiki/Nesdev_Wiki)
- Good source for logic and debugging

[NES Rust Tutorial](https://bugzmanov.github.io/nes_ebook/)
- NES emulation basics & project structure

[6502 Instruction Set](https://www.masswerk.at/6502/6502_instruction_set.html)
- Very comprehensive outline of 6502 instruction set

[6502 Assembly Wiki](https://en.wikibooks.org/wiki/6502_Assembly)
- More digestible breakdown of 6502 assembly & instruction set

#### Documentation

The source files have some inline comments, but most of the documentation is located in the header files.

## Hardware

### CPU

From my own testing, the CPU implementation is fairly robust. The only thing missing is the implementation of the decimal flag, which I may implement in the future. Additionally, it is NOT cycle accurate but it isn't too far off.

The CPU was written with readability in mind. Thus, there is quite a bit of repeated code for similar instructions (e.g. INX, INY, DEX, DEY).

After each instruction, the CPU calls `bus_cpuReport(uint8_t)`, witht the parameter being the number of cycles elapsed.

#### Todo
- Cycle accuracy
- Decimal Flag

### PPU

The current PPU implementation supports only the most basic of games. The scanline rendering is somewhat rough around the edges and needs some work.

After each frame, the PPU calls `bus_ppuReport(uint32_t*)` with a array of size 61440, where each element is an RGB color value representing a pixel of the 256x240 display. The `bus.c` class is in charge of rendering the display.

Simply returning a bitmap array allows the PPU's code to remain mostly unchanged if a different rendering technique/library is used. It also helps isolate display bugs that may occur as a result.

#### Todo
- Fix rightmost artifacts (i.e. actually scroll properly)
- Fix vertical scroll artifacts
- Improve sprite zero hit accuracy
- Improve scanline accuracy

### APU

Not implemented yet.

#### Todo
- Implement

### Bus

This was originally supposed to represent the CPU bus, but ended up being the communication layer between all of the emulated hardware components. So it's really more of a motherboard implementation (which includes the busses, of course).

Since I wanted to maximize portability, most of the platform specific dependencies are done through this class. Addtionally, the hardware classes should not communicate with each other, but rather pass communications through `bus.c`.

#### Todo
- Support for basic mappers

### Joypad

The emulator supports one joypad whose buttons are mapped to the keyboard.

Currently, the mapping is hardcoded in `bus.c`.

Again, due to the focus on portability, the joypad does not interact with the keyboard directly. Rather, the bus feeds keyboard input into the joypad.

## Communication Flow

The following diagram shows how each of the classes communicate with each other.


```
          |<----------|
          |-------->[APU]
          |
          |  |<-------|
          |  |----->[PPU]
          |  |     
[MAIN]--->[BUS]<--->[CPU]
          |  |   
          |  |----->[JOYPAD]
          |  |<--------|
          |
          |-------->[EXCEPTIONS]
          |<-----------|
```
