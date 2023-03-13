# How to get this thing working

## Compile

To generate an executable, simply run the command `make` in this directory.

Alternatively, the program can be compiled and linked separately using the `make compile` and `make link` command.

An executable named `emulator` should generate in the `./bin/` folder.

## Run

To run the program, simply run the following command:

```
$ ./bin/emulator [INES_FILE]
```

## Panics

If something goes wrong during the emulation, a panic screen will display.

The following exceptions may be encountered:

| Type | Code | Exception ID       | Description
|------|------|--------------------|------------
| CPU  | 0x00 | UNEXPECTED_HALT    | CPU halted unexpectedly
| CPU  | 0x01 | ILLEGAL_INSTR      | CPU encountered an unimplemented instruction
| CPU  | 0xFF | UNKNOWN            | An unknown CPU error occurred
| I/O  | 0x00 | NO_ROM             | No ROM was provided to the program
| I/O  | 0x01 | INVALID_ROM        | The provided ROM could not be decoded
| I/O  | 0x02 | UNSUPPORTED_MAPPER | The ROM was decoded, however its mapper is not yet implemented
| I/O  | 0xFF | UNKNOWN            | An unknown I/O error occurred
| SYS  | 0x00 | TRIGGER_PANIC      | A panic was manually invoked
| SYS  | 0xFF | UNKNOWN            | An unknown system error occurred

## Keyboard Mappings

The key mappings can be changed in the `bus_initDisplay()` method of `bus.c`

| Key    | Joypad Button |
|--------|---------------|
| W      | Up            |
| S      | Down          |
| A      | Left          |
| D      | Right         |
| Return | Start         |
| P      | Select        |
| Space  | A             |
| RShift | B             |

## Tested Configurations

This program has been verified to build and run sucessfully on the following configurations:

#### Configuration 1

| Component     | Version |
|---------------|---------|
| macOS (Intel) | 12.3.1  |
| Clang         | 13.1.6  |
| SDL2          | 2.0.22  |
| Make          | 3.81    |

