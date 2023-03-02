/**
 * @file bus.h
 * @author Noah Sadir (development.noahsadir@gmail.com)
 * @brief Handle communications between simulated hardware
 * @version 1.0
 * @date 2022-07-03
 * 
 * @copyright Copyright (c) 2022 Noah Sadir
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

#ifndef BUS_H
#define BUS_H

#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>

#include "globalflags.h"
#include "cpu6502.h"
#include "ppu.h"
#include "io.h"
#include "joypad.h"

typedef enum {
  SYNC_SOUND,
  SYNC_REALTIME,
  SYNC_DISABLED
} SyncMode;

typedef enum {
  MIRRORING_HORIZONTAL  = 0,
  MIRRORING_VERTICAL    = 1
} MirroringType;

typedef enum {
  TV_NTSC  = 0,
  TV_PAL   = 1
} TVSystem;

typedef struct {
  uint8_t prgRomSize;
  uint8_t chrRomSize;
  MirroringType mirroringType;
  bool containsPrgRam;
  bool containsTrainer;
  bool ignoreMirroringControl;
  bool isVSUnisystem;
  bool isPlayChoice10;
  uint8_t mapperNumber;
  uint8_t prgRamSize;
  TVSystem tvSystem;
} HeaderINES;

typedef struct {
  HeaderINES header;
  uint8_t* trainer;
  uint8_t* prgRom;
  uint8_t* chrRom;
  uint8_t* instRom;
  uint8_t* pRom;
} INES;

/* INITIALIZATION METHODS */

void bus_init(FileBinary* bin);

bool bus_parseROM(FileBinary* bin);

/**
 * @brief Initialize the PPU
 */
void bus_initPPU();

/**
 * @brief Initialize the clock
 * 
 */
void bus_initClock();

/* DATA METHODS */

/**
 * @brief Perform read operation at mapped address
 * 
 * @param address the address to read
 * @return uint8_t the data from the specified address
 */
uint8_t bus_readCPU(uint16_t address);

/**
 * @brief Perform write operation at mapped address
 * 
 * @param address the address to write to
 * @param data the data to write
 */
void bus_writeCPU(uint16_t address, uint8_t data);

/**
 * @brief Write cartridge data based on mapper value
 * 
 * @param addr the address to write to
 * @param data the data to write
 */
void bus_cartridgeWrite(uint16_t addr, uint8_t data);

/**
 * @brief Read cartridge data based on mapper value
 * 
 * @param addr the address to read from
 * @return uint8_t the data contained at the address
 */
uint8_t bus_cartridgeRead(uint16_t addr);

/**
 * @brief Perform 16-bit read operation at mapped address
 * 
 * @param address the address to read
 * @return uint8_t the data from the specified address
 */
uint16_t bus_readCPUAddr(uint16_t address);

/**
 * @brief Perform 16-bit write operation at mapped address
 * 
 * @param address the address to write to
 * @param data the data to write
 */
void bus_writeCPUAddr(uint16_t address, uint16_t data);

/**
 * @brief Perform read operation at mapped address
 * 
 * @param address the address to read
 * @return uint8_t the data from the specified address
 */
uint8_t bus_readPPU(uint16_t address);

/**
 * @brief Perform write operation at mapped address
 * 
 * @param address the address to write to
 * @param data the data to write
 */
void bus_writePPU(uint16_t address, uint8_t data);

/**
 * @brief Set a button on the joypad
 * 
 * @param button the button to set
 */
void bus_setJoypad(JoypadButton button);

/**
 * @brief Unset a button on the joypad
 * 
 * @param button the button to unset
 */
void bus_unsetJoypad(JoypadButton button);

/* MONITORS */

void bus_frameIntervalReport();

/* CALLBACKS */

/**
 * @brief CPU has reported a successful execution of instruction
 * 
 * @param cycleCount the number of cycles elapsed
 */
void bus_cpuReport(uint8_t cycleCount);

/**
 * @brief PPU has reported a successful completion of a frame
 * 
 * @param bitmap the display bitmap
 */
void bus_ppuReport();

void bus_handleInput(NESInput input, bool enabled);

void bus_cpuKillReport();

/* TRIGGERS */

/**
 * @brief PPU generated an NMI
 */
void bus_triggerNMI();

/**
 * @brief Trigger a CPU Panic
 * 
 */
void bus_triggerCPUPanic();

#endif