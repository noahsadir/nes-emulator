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
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <sys/time.h>
#include <unistd.h>

#include "cpu.h"
#include "ppu.h"
#include "joypad.h"
#include "exceptions.h"

struct JoypadMapping {
    SDL_KeyCode up;
    SDL_KeyCode down;
    SDL_KeyCode left;
    SDL_KeyCode right;
    SDL_KeyCode a;
    SDL_KeyCode b;
    SDL_KeyCode select;
    SDL_KeyCode start;
};

/* INITIALIZATION METHODS */

/**
 * @brief Initialize the CPU
 */
void bus_initCPU();

/**
 * @brief Initialize the PPU
 */
void bus_initPPU();

/**
 * @brief Initialize the display
 * 
 */
void bus_initDisplay();

/**
 * @brief Set location of the PRG ROM
 * 
 * @param romData_PTR the pointer to the PRG ROM
 * @param romSize the size of the PRG ROM
 */
void bus_loadPRGROM(uint8_t* romData_PTR, uint16_t romSize);

/**
 * @brief Set location of the CHR ROM
 * 
 * @param romData_PTR the pointer to the PRG ROM
 * @param romSize the size of the PRG ROM
 */
void bus_loadCHRROM(uint8_t* romData_PTR, uint16_t romSize);

/* I/O METHODS */

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

/* MONITORS */

/**
 * @brief Start monitoring time.
 */
void bus_startTimeMonitor();

/**
 * @brief Report time (in microseconds) since call to
 *        bus_startTimeMonitor()
 * 
 * @return uint64_t the elapsed time
 */
uint64_t bus_endTimeMonitor();

/**
 * @brief Report time (in microseconds) since call to
 *        bus_startTimeMonitor()
 * 
 * @return uint64_t the elapsed time
 */
uint64_t bus_pollTimeMonitor();

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
void bus_ppuReport(uint32_t* bitmap);

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