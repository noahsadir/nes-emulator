/**
 * @file bus.c
 *
 * Copyright (c) 2022 Noah Sadir
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "bus.h"

uint8_t cpuRAM[2048];
uint8_t* prgRAM = NULL;

INES cartridge;
char trace[150];

bool cpuPaused = false;
bool audioEnabled = false;

SyncMode syncMode = SYNC_REALTIME;

int32_t cyclesUntilDelay = 0;
int32_t cyclesUntilSample = 0;
int32_t cyclesUntilSecond = 0;
uint64_t frameIntervalCount = 0;
uint32_t cpuTimeCount = 0;

#if (PERFORMANCE_DEBUG)
uint32_t framesElapsed = 0;
uint32_t framesPerSec = 0;
double usecsElapsed = 0;
uint32_t cyclesPerSec = 0;
uint32_t freqHertz;
uint32_t framerate;
struct timeval t1, t2;
struct timeval pt1, pt2;
struct timeval pd1, pd2;
#endif

void bus_init(FileBinary* bin) {
  #if (PERFORMANCE_DEBUG)
  gettimeofday(&t1, NULL);
  gettimeofday(&pd1, NULL);
  #endif

  io_init(DISPLAY_SCALE);

  // panic if issues with rom
  if (bin == NULL) {
    io_panic("(I/O) 0x00 NO_ROM");
    while (true) io_pollJoypad(&bus_handleInput);
  }

  if (!bus_parseROM(bin)) {
    io_panic("(I/O) 0x01 INVALID_ROM");
    while (true) io_pollJoypad(&bus_handleInput);
  }

  #if (HEADLESS)
  // looks like somebody chopped off the PPU!
  io_printString("DISPLAY OFF", 88, 64);
  #endif

  CPUEmulationMode mode = EMU_MODE; // perhaps this could be set dynamically?

  // initialize hardware
  ppu_init(cartridge.chrRom, cartridge.header.mirroringType == MIRRORING_VERTICAL, &bus_readCPU, &bus_ppuReport);
  cpu6502_init(&bus_writeCPU, &bus_readCPU, mode);
  //bus_initClock();

  // determine if emulator should run in disassembly mode or not
  if (mode == CPUEMU_DISASSEMBLE) {
    //cpu6502_dasm(cartridge.prgRom, rom.header.prgRomSize * 16384, &nes_handleDisassemblyLine, DASM_MINIMAL);
  } else if (mode == CPUEMU_INTERPRET_DIRECT || mode == CPUEMU_INTERPRET_CACHED) {
    while (cpu6502_getClockMode() != CPUCLOCK_HALT) {
      cpu6502_step(trace, &bus_cpuReport);
    }
  }

  // CPU should never halt until shut off
  switch (cpu6502_getErrno())
  {
    case 0: io_panic("(CPU) 0x00 UNEXPECTED_HALT"); break;
    case 1: io_panic("(CPU) 0x01 ILLEGAL_INSTR"); break;
    default: io_panic("(CPU) 0x?? UNKNOWN"); break;
  }
  cpu6502_setClockMode(CPUCLOCK_HALT);
  while (true) io_pollJoypad(&bus_handleInput); // wait for user to exit, essentially
}

bool bus_parseROM(FileBinary* bin) {
  uint32_t pos = 0;
  if (bin->bytes < 16) return false;

  // parse header
  HeaderINES header;
  header.prgRomSize = bin->data[4];
  header.chrRomSize = bin->data[5];
  uint8_t flags6 = bin->data[6];
  header.mirroringType = (flags6 & BIT_FILL_1) ? MIRRORING_VERTICAL : MIRRORING_HORIZONTAL;
  header.containsPrgRam = ((flags6 >> 1) & BIT_FILL_1);
  header.containsTrainer = ((flags6 >> 2) & BIT_FILL_1);
  header.ignoreMirroringControl = ((flags6 >> 3) & BIT_FILL_1);
  header.mapperNumber = (flags6 >> 4) & BIT_FILL_4;
  uint8_t flags7 = bin->data[7];
  header.isVSUnisystem = (flags7 & BIT_FILL_1);
  header.isPlayChoice10 = ((flags7 >> 1) & BIT_FILL_1);
  header.mapperNumber |= (flags7 & 0xF0);
  header.prgRamSize = bin->data[8];
  header.tvSystem = (bin->data[9] & BIT_FILL_1) ? TV_PAL : TV_NTSC;
  cartridge.header = header;
  pos = 16;

  // load trainer, if present
  if (header.containsTrainer) {
    cartridge.trainer = malloc(sizeof(uint8_t) * 512);
    for (int i = 0; i < 512; i++) {
      cartridge.trainer[i] = bin->data[pos];
      pos += 1;
    }
  }

  // load prg rom (or fail if size mismatch)
  if ((pos + (header.prgRomSize * 16384)) > bin->bytes) return false;
  cartridge.prgRom = malloc(sizeof(uint8_t) * (header.prgRomSize * 16384));
  for (int i = 0; i < (header.prgRomSize * 16384); i++) {
    cartridge.prgRom[i] = bin->data[pos];
    pos += 1;
  }

  // load chr rom (or fail if size mismatch)
  if ((pos + (header.chrRomSize * 8192)) > bin->bytes) return false;
  cartridge.chrRom = malloc(sizeof(uint8_t) * (header.chrRomSize * 8192));
  for (int i = 0; i < (header.chrRomSize * 8192); i++) {
    cartridge.chrRom[i] = bin->data[pos];
    pos += 1;
  }

  if (header.containsPrgRam) {
    prgRAM = malloc(sizeof(uint8_t) * (header.prgRamSize * 8192));
  }

  return true;
}

void bus_writeCPU(uint16_t addr, uint8_t data) {
  if (addr <= 0x1FFF) {
    cpuRAM[addr] = data;
  } else if (addr <= 0x0FFF) {
    cpuRAM[addr - 0x0800] = data;
  } else if (addr <= 0x17FF) {
    cpuRAM[addr - 0x1000] = data;
  } else if (addr <= 0x1FFF) {
    cpuRAM[addr - 0x1800] = data;
  } else if (addr <= 0x3FFF) {
    addr = (addr & 0x0007) | 0x2000;
    if (addr == 0x2000) { // ppu control
      ppu_setRegister(PPU_CONTROL, data);
    } else if (addr == 0x2001) { // ppu mask
      ppu_setRegister(PPU_MASK, data);
    } else if (addr == 0x2002) { // ppu status
      ppu_setRegister(PPU_STATUS, data);
    } else if (addr == 0x2003) { // ppu oam address
      ppu_setRegister(PPU_OAMADDR, data);
    } else if (addr == 0x2004) { // ppu oam data
      ppu_setRegister(PPU_OAMDATA, data);
    } else if (addr == 0x2005) { // ppu scroll
      ppu_setRegister(PPU_SCROLL, data);
    } else if (addr == 0x2006) { // ppu address
      ppu_setRegister(PPU_PPUADDR, data);
    } else if (addr == 0x2007) { // ppu data
      ppu_setRegister(PPU_PPUDATA, data);
    }
  } else if (addr <= 0x4017) {
    if (addr == 0x4014) {
      ppu_setRegister(PPU_OAMDMA, data);
    } else if (addr == 0x4016) {
      joypad_write(data);
    }
    // apu & i/o regs
  } else if (addr <= 0x401F) {
    // disabled apu & i/o behavior
  } else {
    bus_cartridgeWrite(addr, data);
  }
}

uint8_t bus_readCPU(uint16_t addr) {
  if (addr <= 0x07FF) {
    return cpuRAM[addr];
  } else if (addr <= 0x0FFF) {
    return cpuRAM[addr - 0x0800];
  } else if (addr <= 0x17FF) {
    return cpuRAM[addr - 0x1000];
  } else if (addr <= 0x1FFF) {
    return cpuRAM[addr - 0x1800];
  } else if (addr <= 0x3FFF) {
    addr = (addr & 0x0007) | 0x2000;
    if (addr == 0x2000) {
      return ppu_getRegister(PPU_CONTROL);
    } else if (addr == 0x2001) {
      return ppu_getRegister(PPU_MASK);
    } else if (addr == 0x2002) {
      return ppu_getRegister(PPU_STATUS);
    } else if (addr == 0x2003) {
      return ppu_getRegister(PPU_OAMADDR);
    } else if (addr == 0x2004) {
      return ppu_getRegister(PPU_OAMDATA);
    } else if (addr == 0x2005) {
      return ppu_getRegister(PPU_SCROLL);
    } else if (addr == 0x2006) {
      return ppu_getRegister(PPU_PPUADDR);
    } else if (addr == 0x2007) {
      return ppu_getRegister(PPU_PPUDATA);
    }
  } else if (addr <= 0x4017) {
    if (addr == 0x4014) {
      return ppu_getRegister(PPU_OAMDMA);
    } else if (addr == 0x4016) {
      return joypad_read();
    }
    return 0; // apu & i/o regs
  } else if (addr <= 0x401F) {
    return 0; // disabled apu & i/o behavior
  } else {
    return bus_cartridgeRead(addr);
  }
  return 0;
}

uint16_t bus_readCPUAddr(uint16_t addr) {
  return ((uint16_t)bus_readCPU(addr + 1) << 8) | (uint16_t)bus_readCPU(addr);
}

void bus_writeCPUAddr(uint16_t address, uint16_t data) {
  if (address < 0x2000) { // cpu ram
    cpuRAM[address] = (data << 8) >> 8;
    cpuRAM[address + 1] = data >> 8;
  }
}

void bus_cartridgeWrite(uint16_t addr, uint8_t data) {
  if (cartridge.header.mapperNumber == 0) {
    // mapper 0
    if (addr < 0x6000) {
      // invalid write
    } else if (addr <= 0x7FFF) {
      if (cartridge.header.containsPrgRam) {
        prgRAM[addr - 0x6000] = data;
      } else {
        // invalid write
      }
    } else {
      // invalid write
    }
  } else {
    io_panic("(I/O) 0x02 UNSUPPORTED_MAPPER");
    while (true) io_pollJoypad(&bus_handleInput);
  }
}

uint8_t bus_cartridgeRead(uint16_t addr) {
  if (cartridge.header.mapperNumber == 0) {
    // mapper 0
    if (addr < 0x6000) {
      //exceptions_invalidMemoryRead(addr);
    } else if (addr <= 0x7FFF) {
      if (cartridge.header.containsPrgRam) {
        return prgRAM[addr - 0x6000];
      } else {
        //exceptions_invalidMemoryRead(addr);
      }
    } else if (addr <= 0xBFFF) {
      return cartridge.prgRom[addr - 0x8000];
    } else if (addr <= 0xFFFF) {
      if (cartridge.header.prgRomSize == 1) {
        return cartridge.prgRom[addr - 0xC000];
      } else if (cartridge.header.prgRomSize == 2) {
        return cartridge.prgRom[addr - 0x8000];
      } else {
        //exceptions_invalidMemoryRead(addr);
      }
    }
  } else {
    io_panic("(I/O) 0x02 UNSUPPORTED_MAPPER");
    while (true) io_pollJoypad(&bus_handleInput);
  }
  return 0;
}

void bus_setJoypad(JoypadButton button) {
    joypad_setButton(button);
}

void bus_unsetJoypad(JoypadButton button) {
    joypad_unsetButton(button);
}

void bus_initClock() {

  struct timeval tv1, tv2;
  gettimeofday(&tv1, NULL);
  uint64_t elapsed;

  while (cpu6502_getClockMode() != CPUCLOCK_HALT)
  {
    gettimeofday(&tv2, NULL);
    elapsed = ((tv2.tv_sec - tv1.tv_sec) * 1000000) + (tv2.tv_usec - tv1.tv_usec);

    if (elapsed >= DISPLAY_FRAME_USEC) {
      bus_frameIntervalReport();
      tv1 = tv2;
      elapsed -= DISPLAY_FRAME_USEC;
    }

    if (!cpuPaused) {
      cpu6502_step(NULL, &bus_cpuReport);
    }
  }

  io_panic("(BUS) 0x00 CPU_HALT");
  while (true) io_pollJoypad(&bus_handleInput);
}

void bus_initPPU() {
    #if (!HEADLESS)
    ppu_init(cartridge.chrRom, cartridge.header.mirroringType == MIRRORING_VERTICAL, &bus_readCPU, &bus_ppuReport);
    #endif
}

void bus_cpuReport(uint8_t cycleCount) {
  cyclesPerSec += cycleCount;
  // update PPU
  #if (!HEADLESS)
  // run 3x the number of cycles on the PPU
  ppu_runCycles(cycleCount * 3);

  // determine if necessary to generate NMI
  if (ppu_getControlFlag(PPUCTRL_GENVBNMI)
    && ppu_getStatusFlag(PPUSTAT_VBLKSTART)) {
    bus_triggerNMI();
    ppu_setStatusFlag(PPUSTAT_VBLKSTART, false);
  }
  #endif

  // update cycle counters
  cyclesUntilDelay -= cycleCount;
  cyclesUntilSample -= cycleCount;
  cyclesUntilSecond -= cycleCount;

  // pause CPU until next frame interval
  if (cyclesUntilDelay <= 0) {
      if (syncMode != SYNC_DISABLED) cpuPaused = true;
      cyclesUntilDelay += CPU_FRAME_CLOCKS;
  }

  // CPU second has elapsed (CPU second = 1789773 clocks)
  if (cyclesUntilSecond <= 0) {
      cpuTimeCount += 1;
      cyclesUntilSecond = CPU_FREQUENCY;
  }

  // fill audio buffer
  if (audioEnabled && cyclesUntilSample <= 0) {
      // TODO: Queue audio sample
      cyclesUntilSample = 40;
  }

  #if (DEBUG_MODE)
  exc_trace(&trace);
  #endif
}

void bus_frameIntervalReport() {
    frameIntervalCount += 1;
    if (syncMode == SYNC_REALTIME) cpuPaused = false;
}

void bus_ppuReport() {
  char str[32];
  str[0] = '\0';
  #if (PERFORMANCE_DEBUG)
  gettimeofday(&pd2, NULL); // poll delay
  uint32_t delayCounter = (pd2.tv_sec - pd1.tv_sec) * 1000000.0;
  delayCounter += (pd2.tv_usec - pd1.tv_usec);
  framesElapsed += 1;
  if (delayCounter >= 1000000) {
    freqHertz = cyclesPerSec;
    framerate = framesElapsed;
    framesElapsed = 0;
    cyclesPerSec = 0;
    gettimeofday(&pd1, NULL);
  }
  uint32_t fq = freqHertz;
  uint32_t fr = framerate;
  for (int i = 8; i >= 0; i--) {
    if (i < 3) {
      str[i] = '0' + (fq % 10);
    } else if (i < 6) {
      str[i + 1] = '0' + (fq % 10);
      str[i] = '.';
    }
    fq /= 10;
  }
  str[7] = '\0';
  strcat(str, " MHz (");
  for (int i = 16; i >= 13; i--) {
    str[i] = '0' + (fr % 10);
    fr /= 10;
  }
  str[17] = '\0';
  strcat(str, " FPS)");
  #endif

  #if (!HEADLESS)
  io_update(str);
  #endif
  io_pollJoypad(&bus_handleInput);
}

void bus_handleInput(NESInput input, bool enabled) {
    if (input == INPUT_QUIT) exit(0);
    JoypadButton jpMappings[9] = {JP_UP, JP_DOWN, JP_LEFT, JP_RIGHT, JP_BTN_A, JP_BTN_B, JP_SELECT, JP_START, JP_NULL};
    if (enabled) {
        joypad_setButton(jpMappings[input]);
    } else {
        joypad_unsetButton(jpMappings[input]);
    }
}

void bus_triggerNMI() {
    cpu6502_nmi();
}

void bus_triggerCPUPanic() {
    io_panic("(BUS) 0x01 TRIGGER_PANIC");
    while (true) io_pollJoypad(&bus_handleInput);
}

void bus_cpuKillReport() {
    uint32_t realTimeCount = frameIntervalCount / DISPLAY_FRAMERATE;

    double floatingCPUTime = (double) cpuTimeCount;
    double floatingRealTime = (double) realTimeCount;
    double realClockSpeed = (((double)CPU_FREQUENCY) * (floatingCPUTime / floatingRealTime)) / 1000000;

    printf("** EMULATION STOPPED **\n");
    printf("\n");
    printf("CPU Time Elapsed: %d seconds\n", cpuTimeCount);
    printf("Real Time Elapsed: %d seconds\n", realTimeCount);
    printf("\n");
    printf("Real Clock Speed: %.4f MHz\n", realClockSpeed);
    printf("\n");
}