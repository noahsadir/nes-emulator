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

#include "main.h"

void loadROM(char* path) {
    FILE* fp;
    uint8_t buffer[16];
    int n;
    fp = fopen(path, "rb");

    uint8_t prgROM[32768];
    uint8_t chrROM[8192];
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

    printf("\n *** LOAD %s *** \n\n", path);

    n = fread(buffer, 1, 16, fp);
    if (n == 16) {
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

        printf("PRG ROM Size: %d Bytes", prgROMSize);
        printf("\nCHR ROM Size: %d Bytes", chrROMSize);
        printf("\nVertical Mirroring: ");
        printf(verticalMirroring ? "true" : "false");
        printf("\nContains Battery PRG RAM: ");
        printf(containsBatteryPrgRam ? "true" : "false");
        printf("\nContains Trainer: ");
        printf(containsTrainer ? "true" : "false");
        printf("\nIgnore Mirroring Control: ");
        printf(ignoreMirroringControl ? "true" : "false");
        printf("\nMapper: %02x", mapper);
        printf("\nPRG RAM Size: %d Bytes", prgRAMSize);
        printf("\nTV System: ");
        printf(isPAL ? "PAL" : "NTSC");
        printf("\n");
    } else {
        printf(" ** INES READ ERROR (invalid header) **\n");
        exit(1);
    }

    if (mapper != 0) {
        printf(" ** UNSUPPORTED MAPPER **\n");
        exit(1);
    }

    if (containsTrainer) {
        fread(trainer, 1, 512, fp);
    }

    if (fread(prgROM, 1, prgROMSize, fp) == prgROMSize) {
        printf("PRG ROM Preview: \n[0x0000]: ");
        for (int i = 0; i < 16; i++) {
            printf("%02x ", prgROM[i]);
        }
        
        // Mirror small ROMs
        if (prgROMSize == 16384) {
            for (int i = 0; i < 16384; i++) {
                prgROM[i + 16384] = prgROM[i];
            }
        }

        printf("\n[0xFFF0]: ");

        for (int i = 32768 - 16; i < 32768; i++) {
            printf("%02X ", prgROM[i]);
        }

        printf("\n\n");
    } else {
        printf(" ** INES READ ERROR (invalid PRG ROM) **\n");
        exit(1);
    }

    if (fread(chrROM, 1, chrROMSize, fp) == chrROMSize) {
        printf("CHR ROM Preview: ");
        for (int i = 0; i < 16; i++) {
            printf("%02x ", chrROM[i]);
        }
        printf("\n\n");
    } else {
        printf(" ** INES READ ERROR (invalid CHR ROM) **\n");
        exit(1);
    }

    bus_loadPRGROM(prgROM, prgROMSize);
    bus_loadCHRROM(chrROM, chrROMSize);

    bus_initPPU();
    bus_initDisplay();
    bus_initCPU(); // MUST be initialized last, since it will begin a loop
    
}

int main(int argc, char* argv[]) {

    bool arg_genTrace = false;
    bool arg_limitClocks = true;
    uint8_t arg_scale = 0;

    if (argc >= 2) {
        loadROM(argv[1]);
    } else {
        printf("Please provide an iNES file.\n");
    }
    return 0;
}