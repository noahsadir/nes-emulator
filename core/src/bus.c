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
uint8_t vidRAM[2048];
uint8_t paletteRAM[32];

uint8_t* prgROM;
uint8_t* chrROM;

// config
bool verticalMirroring = false;
bool panicOnUnimplemented = false;
bool audioEnabled = false;
bool graphicsEnabled = true;
bool cpuPaused = false;
SyncMode syncMode = SYNC_REALTIME;

int32_t cyclesUntilDelay = 0;
int32_t cyclesUntilSample = 0;
int32_t cyclesUntilSecond = 0;
uint64_t frameIntervalCount = 0;
uint32_t cpuTimeCount = 0;

Trace trace;

uint8_t bus_readCPU(uint16_t address) {
    if (address < 0x2000) { // cpu ram
        address = address % 0x800; // mirrored
        return cpuRAM[address];
    } else if (address < 0x4000) {
        address = ((address - 0x2000) % 8) + 0x2000;
        if (address == 0x2000) { // ppu control
            return ppu_getRegister(PPU_CONTROL);
        } else if (address == 0x2001) { // ppu mask
            return ppu_getRegister(PPU_MASK);
        } else if (address == 0x2002) { // ppu status
            return ppu_getRegister(PPU_STATUS);
        } else if (address == 0x2003) { // ppu oam address
            return ppu_getRegister(PPU_OAMADDR);
        } else if (address == 0x2004) { // ppu oam data
            return ppu_getRegister(PPU_OAMDATA);
        } else if (address == 0x2005) { // ppu scroll
            return ppu_getRegister(PPU_SCROLL);
        } else if (address == 0x2006) { // ppu address
            return ppu_getRegister(PPU_PPUADDR);
        } else if (address == 0x2007) { // ppu data
            return ppu_getRegister(PPU_PPUDATA);
        }
    } else if (address < 0x4020) { // io registers
        if (address >= 0x4000 && address <= 0x4013) { // apu

        } else if (address == 0x4014) { // ppu oam data
            return ppu_getRegister(PPU_OAMDMA);
        } else if (address == 0x4015) { // apu status

        } else if (address == 0x4016) { // controller
            return (uint8_t) joypad_read();
        } else if (address == 0x4017) { // apu frame counter

        } else {
            
        }
    } else if (address < 0x6000) { // expansion rom
        #if (DEBUG_MODE)
        if (panicOnUnimplemented) exc_panic_invalidIO(address);
        #endif
    } else if (address < 0x8000) { // save ram
        #if (DEBUG_MODE)
        if (panicOnUnimplemented) exc_panic_invalidIO(address);
        #endif
    } else if (address < 0xFFFF) { // prg rom
        address -= 0x8000;
        return prgROM[address];
    }

    return 0x00;
}

void bus_writeCPU(uint16_t address, uint8_t data) {
    if (address < 0x2000) { // cpu ram
        cpuRAM[address] = data;
    } else if (address < 0x4000) { // io registers
        address = ((address - 0x2000) % 8) + 0x2000;
        if (address == 0x2000) { // ppu control
            ppu_setRegister(PPU_CONTROL, data);
        } else if (address == 0x2001) { // ppu mask
            ppu_setRegister(PPU_MASK, data);
        } else if (address == 0x2002) { // ppu status
            ppu_setRegister(PPU_STATUS, data);
        } else if (address == 0x2003) { // ppu oam address
            ppu_setRegister(PPU_OAMADDR, data);
        } else if (address == 0x2004) { // ppu oam data
            ppu_setRegister(PPU_OAMDATA, data);
        } else if (address == 0x2005) { // ppu scroll
            ppu_setRegister(PPU_SCROLL, data);
        } else if (address == 0x2006) { // ppu address
            ppu_setRegister(PPU_PPUADDR, data);
        } else if (address == 0x2007) { // ppu data
            ppu_setRegister(PPU_PPUDATA, data);
        }
    } else if (address < 0x4020) { // i/o registers
        if (address >= 0x4000 && address <= 0x4013) { // apu

        } else if (address == 0x4014) { // ppu oam dma
            ppu_setRegister(PPU_OAMDMA, data);
        } else if (address == 0x4015) { // apu status

        } else if (address == 0x4016) { // controller
            joypad_write(data);
        } else if (address == 0x4017) { // apu frame counter

        } else {
            
        }    
    } else if (address < 0x6000) { // expansion rom
        #if (DEBUG_MODE)
        if (panicOnUnimplemented) exc_panic_invalidIO(address);
        #endif
    } else if (address < 0x8000) { // save ram
        #if (DEBUG_MODE)
        if (panicOnUnimplemented) exc_panic_invalidIO(address);
        #endif
    } else if (address < 0xFFFF) { // prg rom
        #if (DEBUG_MODE)
        if (panicOnUnimplemented) exc_panic_invalidIO(address);
        #endif
    }
}

uint16_t bus_readCPUAddr(uint16_t address) {
    if (address < 0x2000) { // cpu ram
        address = address % 0x800; // mirrored
        return ((uint16_t) cpuRAM[address] << 8) | ((uint16_t) cpuRAM[address]);
    } else if (address < 0x4020) { // io registers
        #if (DEBUG_MODE)
        if (panicOnUnimplemented) exc_panic_invalidIO(address);
        #endif
    } else if (address < 0x6000) { // expansion rom
        #if (DEBUG_MODE)
        if (panicOnUnimplemented) exc_panic_invalidIO(address);
        #endif
    } else if (address < 0x8000) { // save ram
        #if (DEBUG_MODE)
        if (panicOnUnimplemented) exc_panic_invalidIO(address);
        #endif
    } else if (address < 0xFFFF) { // prg rom
        address -= 0x8000;
        return (((uint16_t) prgROM[address + 1]) << 8) | ((uint16_t) prgROM[address]);
    }

    return 0x0000;
}

void bus_writeCPUAddr(uint16_t address, uint16_t data) {
    if (address < 0x2000) { // cpu ram
        cpuRAM[address] = (data << 8) >> 8;
        cpuRAM[address + 1] = data >> 8;
    } else {
        #if (DEBUG_MODE)
        if (panicOnUnimplemented) exc_panic_invalidIO(address);
        #endif
    }
}

uint8_t bus_readPPU(uint16_t address) {
    address = address % 0x4000;
    if (address < 0x2000) { // chr rom
        return chrROM[address];
    } else if (address < 0x3F00) { // vram
        address = (address % 0x2000) + 0x2000;
        if (verticalMirroring) {
            if (address < 0x2400) {
                address = address - 0x2000;
            } else if (address < 0x2800) {
                address = address - 0x2400;
            } else if (address < 0x2C00) {
                address = address - 0x2400;
            } else if (address < 0x2FFF) {
                address = address - 0x2800;
            }
        } else {
            if (address < 0x2400) {
                address = address - 0x2000;
            } else if (address < 0x2800) {
                address = address - 0x2000;
            } else if (address < 0x2C00) {
                address = address - 0x2800;
            } else if (address < 0x3000) {
                address = address - 0x2800;
            }
        }
        return vidRAM[address];
    } else if (address < 0x4000) {
        address &= 0b0011111;
        if (address == 0x0010 || address == 0x0014 || address == 0x0018 || address == 0x001C) {
            address -= 0x0010;
        }
        return paletteRAM[address];
    }

    return 0x00;
}

void bus_writePPU(uint16_t address, uint8_t data) {
    address = address % 0x4000;
    if (address < 0x2000) { // chr rom
        #if (DEBUG_MODE)
        if (panicOnUnimplemented) exc_panic_invalidPPUIO(address);
        #endif
    } else if (address < 0x3F00) { // vram
        address = (address % 0x2000) + 0x2000;
        if (verticalMirroring) {
            if (address < 0x2400) {
                address = address - 0x2000;
            } else if (address < 0x2800) {
                address = address - 0x2400;
            } else if (address < 0x2C00) {
                address = address - 0x2400;
            } else if (address < 0x2FFF) {
                address = address - 0x2800;
            }
        } else {
            if (address < 0x2400) {
                address = address - 0x2000;
            } else if (address < 0x2800) {
                address = address - 0x2000;
            } else if (address < 0x2C00) {
                address = address - 0x2800;
            } else if (address < 0x3000) {
                address = address - 0x2800;
            }
        }
        vidRAM[address] = data;
    } else if (address < 0x4000) {
        address &= 0b00011111;
        if (address == 0x0010 || address == 0x0014 || address == 0x0018 || address == 0x001C) {
            address -= 0x0010;
        }
        paletteRAM[address] = data;
    }
}

void bus_setJoypad(enum JoypadButton button) {
    joypad_setButton(button);
}

void bus_unsetJoypad(enum JoypadButton button) {
    joypad_unsetButton(button);
}

void bus_loadPRGROM(uint8_t* romData_PTR, uint16_t romSize) {
    prgROM = romData_PTR;
}

void bus_loadCHRROM(uint8_t* romData_PTR, uint16_t romSize) {
    chrROM = romData_PTR;
}

void bus_initCPU() {
    #if (DEBUG_MODE)
    exc_traceInit();
    #endif
    cpu_init(&bus_writeCPU, &bus_readCPU, &trace);
    bus_initClock();
}

void bus_initClock() {
    Instruction inst = NOP;
    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL);
    uint64_t elapsed;
    while (inst != BRK)
    {
        gettimeofday(&tv2, NULL);
        elapsed = ((tv2.tv_sec - tv1.tv_sec) * 1000000) + (tv2.tv_usec - tv1.tv_usec);
        if (elapsed >= USEC_PER_FRAME) {
            bus_frameIntervalReport();
            tv1 = tv2;
            elapsed -= USEC_PER_FRAME;
        }

        if (!cpuPaused) {
            inst = cpu_execute(&bus_cpuReport);
        }
    }
    bus_triggerCPUPanic();
}

void bus_initPPU() {
    #if (DEBUG_MODE)
    graphicsEnabled = false;
    #endif
    if (graphicsEnabled) {
        ppu_init(vidRAM, chrROM);
    }
}

void bus_initDisplay() {
    io_init(3);
}

void bus_cpuReport(uint8_t cycleCount) {
    // update PPU
    if (graphicsEnabled) {
        // run 3x the number of cycles on the PPU
        ppu_runCycles(cycleCount * 3);

        // determine if necessary to generate NMI
        if (ppu_getControlFlag(PPUCTRL_GENVBNMI)
            && ppu_getStatusFlag(PPUSTAT_VBLKSTART)) {
            bus_triggerNMI();
            ppu_setStatusFlag(PPUSTAT_VBLKSTART, false);
        }
    }

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
        cyclesUntilSecond = CPU_CLOCK_SPEED;
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

void bus_ppuReport(uint32_t* bitmap) {
    if (graphicsEnabled) io_update(bitmap);
}

void bus_triggerNMI() {
    cpu_nmi();
}

void bus_triggerCPUPanic() {
    bus_cpuKillReport();
    cpu_panic();
}

void bus_cpuKillReport() {
    uint32_t realTimeCount = frameIntervalCount / FRAMERATE;

    double floatingCPUTime = (double) cpuTimeCount;
    double floatingRealTime = (double) realTimeCount;
    double realClockSpeed = (((double)CPU_CLOCK_SPEED) * (floatingCPUTime / floatingRealTime)) / 1000000;

    printf("** EMULATION STOPPED **\n");
    printf("\n");
    printf("CPU Time Elapsed: %d seconds\n", cpuTimeCount);
    printf("Real Time Elapsed: %d seconds\n", realTimeCount);
    printf("\n");
    printf("Real Clock Speed: %.4f MHz\n", realClockSpeed);
    printf("\n");
}