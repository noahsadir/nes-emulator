# NES Emulator (Core)

This folder contains the source code necessary for the emulator itself.

## Compile

To generate an executable, simply run the command `make` in this directory.

Alternatively, the program can be compiled and linked separately using the `make compile` and `make link` command.

An executable named `emulator` should generate in the `./bin/` folder.

## Run

To run the program, simply run the following command:

```
$ ./bin/emulator [INES_FILE]
```

### Key Mappings

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

