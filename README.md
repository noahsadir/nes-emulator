# NES Emulator

A **very** basic NES Emulator written entirely in C, with portability in mind.

#### 03/01/2023 Update

I'm currently rewriting large portions of this emulator to make it less hacky.

Because of this, there may be a disconnect between the documentation and the source code, along with lots of other quirks.

#### Quick Note

I mostly wrote this for fun, and it certainly isn't near feature complete.

#### Another Note

~The details below pertain to the core emulator. The Z80 port was done out of sheer curiosity and is not being actively worked on.~

I removed the separate Z80 port, but may add support for other platforms in a more unified manner. This includes a TI-84 CE port that will presumably run at <1 FPS.

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

The CPU code is essentially blind to the rest of the emulator and could theoretically be used for other 6502-based systems.

It also has the ability to disassemble programs!

#### Todo
- Cycle accuracy (account for page crosses)
- Decimal Flag
- Illegal instructions

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

### Bus (aka NES System)

`bus.c` essentially acts as a bus in the sense that it connects all the components and allows them to interact with each other.

Since I wanted to maximize portability, most of the platform specific dependencies are done through this class. Addtionally, the hardware classes should not communicate with each other, but rather pass communications through `bus.c`.

Additionally, `bus.c` performs most of the NES-specific code so that other components, such as I/O and CPU, could theoretically be used to emulate other systems.

#### Todo
- Support for basic mappers

### Joypad

The emulator supports one joypad whose buttons are mapped to the keyboard.

Currently, the mapping is hardcoded in `io.c`.

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
          |  |----->[I/O]
          |  |<--------|
          |
          |-------->[EXCEPTIONS]
          |<-----------|
```
