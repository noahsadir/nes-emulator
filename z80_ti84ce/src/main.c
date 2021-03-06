/**
 * @file main.c
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
#include "rom.h"
#include <tice.h>
#include <stdio.h>
#include <graphx.h>

uint8_t* tifile_bin;
uint32_t tifile_size = 0;
uint32_t tifile_index = 0;

void tifile_init(uint8_t* binary, uint32_t size) {
    tifile_bin = binary;
    tifile_size = size;
    tifile_index = 0;
}

int main() {
    os_ClrHome();
    tifile_init(rom_binary, rom_size);

    uint8_t buffer[16];

    // calculator seems to have issues with large statically allocated arrays
    uint8_t* prgROM = malloc(32768 * sizeof(uint8_t));
    uint8_t* chrROM = malloc(8192 * sizeof(uint8_t));
    uint8_t trainer[512];

    uint16_t prgROMSize;
    uint16_t chrROMSize;
    uint16_t prgRAMSize;

    bool verticalMirroring;
    bool containsBatteryPrgRam;
    bool containsTrainer;
    bool ignoreMirroringControl;
    uint8_t mapper;

    bool vsUnisystem;
    bool playChoice10;
    bool isPAL;

    for (int i = 0; i < 16; i++) {
        buffer[i] = tifile_bin[tifile_index];
        tifile_index += 1;
    }

    int n = 16;
    // flags 4
    prgROMSize = 16384 * buffer[4];

    // flags 5
    chrROMSize = 8192 * buffer[5];

    // flags 6
    verticalMirroring = (buffer[6] >> 0) & 1;
    containsBatteryPrgRam = (buffer[6] >> 1) & 1;
    containsTrainer = (buffer[6] >> 2) & 1;
    ignoreMirroringControl = (buffer[6] >> 3) & 1;
    mapper = buffer[6] >> 4;

    // flags 7
    vsUnisystem = (buffer[7] >> 0) & 1;
    playChoice10 = (buffer[7] >> 1) & 1;
    mapper = ((buffer[7] >> 4) << 4) | mapper;

    // flags 8
    prgRAMSize = 8192 * buffer[8];

    // flags 9
    isPAL = (buffer[9] >> 0) & 1;

    if (containsTrainer) {
        for (int i = 0; i < 512; i++) {
            trainer[i] = tifile_bin[tifile_index];
            tifile_index += 1;
        }
    }

    /**/

    if (tifile_size - tifile_index >= prgROMSize) {
        for (int i = 0; i < prgROMSize; i++) {
            prgROM[i] = tifile_bin[tifile_index];
            tifile_index += 1;
        }
        
        // Mirror small ROMs
        if (prgROMSize == 16384) {
            for (int i = 0; i < 16384; i++) {
                prgROM[i + 16384] = prgROM[i];
            }
        }
    } else {
        exc_panic("INES READ ERROR (invalid PRG ROM)");
    }

    
    if (tifile_size - tifile_index >= chrROMSize) {
        for (int i = 0; i < chrROMSize; i++) {
            chrROM[i] = tifile_bin[tifile_index];
            tifile_index += 1;
        }
    } else {
        exc_panic("INES READ ERROR (invalid CHR ROM)");
    }

    bus_loadPRGROM(prgROM, prgROMSize);
    bus_loadCHRROM(chrROM, chrROMSize);

    gfx_Begin();
    bus_initPPU();
    bus_initCPU();

    while (!os_GetCSC());
    gfx_End();
    free(prgROM);
    free(chrROM);
    return 0;
}