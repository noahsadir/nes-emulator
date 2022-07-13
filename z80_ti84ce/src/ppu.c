/**
 * @file ppu.c
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

#include "ppu.h"

uint8_t reg_control     = 0b00000000;
uint8_t reg_mask        = 0b00000000;
uint8_t reg_ppuStatus   = 0b00000000;
uint8_t reg_oamaddr     = 0x00;
uint8_t reg_oamdata     = 0x00;
uint8_t reg_scroll      = 0x00;
uint8_t reg_ppuaddr     = 0x00;
uint8_t reg_ppudata     = 0x00;
uint8_t reg_oamdma      = 0x00;

uint16_t addressBuffer = 0x0000;
uint8_t scrollX = 0;
uint8_t scrollY = 0;
uint8_t dataBuffer = 0x00;
bool addressLatch = false;
bool scrollLatch = false;
bool triggerSpriteZero = false;

uint8_t oamRAM[0x00FF];
uint8_t* vidRAM_Ptr;
uint8_t* chrROM_Ptr;

uint32_t ppuCycles = 0;
uint32_t ppuFrames = 0;
uint16_t scanline = 0;

void ppu_init(uint8_t* vram, uint8_t* crom) {
    gfx_SetDefaultPalette(gfx_8bpp);
    vidRAM_Ptr = vram;
    chrROM_Ptr = crom;
    //ppu_drawCHRROM(0);
    for (int i = 0; i < 61440; i++) {
        //bitmap[i] = 0;
    }
}

void ppu_runCycles(uint8_t cycleCount) {
    ppuCycles += cycleCount;
    if (ppuCycles >= 341) {
        ppu_scanline();
        ppuCycles -= 341;
    }
}

uint32_t ppu_getFrames() {
    return ppuFrames;
}

void ppu_scanline() {
    // render scanlines
    if (scanline >= 0 && scanline <= 239) {
        ppu_drawScanline(scanline);
    }

    // render all at once rather than by scanline
    if (scanline == 240) {
        //ppu_drawCHRROM(0);
        //ppu_drawRAMPalette();
        bus_ppuReport();
        ppuFrames += 1;
    }

    // start vblank
    if (scanline == 241) {
        ppu_setStatusFlag(PPUSTAT_VBLKSTART, true);
    }

    // end vblank
    if (scanline == 261) {
        ppu_setStatusFlag(PPUSTAT_VBLKSTART, false);
        ppu_setStatusFlag(PPUSTAT_SPRITEZRO, false);
        scanline = 0;
    } else {
        scanline += 1;
    }
}

void ppu_setPixel(uint32_t color, int16_t x, int16_t y) {
    if (x >= 0 && x < 256 && y >= 0 && y < 240) {
        uint16_t location = (y * 256) + x;
        if (color == 0) {
            gfx_SetColor(0);
        } else {
            gfx_SetColor(255);
        }
        
        gfx_SetPixel(x, y);
        //bitmap[location] = color;
    }
}

void ppu_drawTile(bool bank, uint16_t tileID, uint8_t palette, uint16_t x, uint16_t y) {
    tileID *= 16;
    for (uint16_t row = 0; row < 8; row++) {
        uint8_t high = bus_readPPU((0x1000 * bank) + tileID + row + 8);
        uint8_t low = bus_readPPU((0x1000 * bank) + tileID + row);
        for (uint16_t col = 0; col < 8; col++) {
            uint8_t color = ((high & 1) << 1) | (low & 1);
            high >>= 1;
            low >>= 1;
            ppu_setPixel(ppu_getColor(0, color), x + (8 - col) + (bank * 128), y + row);
            
        }
    }
}

void ppu_drawScanline(uint8_t y) {
    uint8_t fineX = scrollX % 8;
    uint8_t fineY = scrollY % 8;
    uint8_t coarseX = scrollX / 8;
    uint8_t coarseY = scrollY / 8;
    uint8_t tileRow = y / 8;

    uint8_t backgroundPixels[256];
    bool containsSpriteZero = false;

    uint16_t nametableOffset = (((((uint16_t) ppu_getControlFlag(PPUCTRL_NAMETABLE2)) << 1) | ((uint16_t) ppu_getControlFlag(PPUCTRL_NAMETABLE1))) * 0x0400);
    uint16_t bankOffset = 0x1000 * ppu_getControlFlag(PPUCTRL_BKGPATT);
    uint16_t spriteBankOffset = 0x1000 * ppu_getControlFlag(PPUCTRL_SPRITEPATT);

    // ensures 1-scanline delay for sprite zero hit
    if (triggerSpriteZero == true) {
        ppu_setStatusFlag(PPUSTAT_SPRITEZRO, true);
        triggerSpriteZero = false;
    }

    // indicate that sprite zero is present in the scanline
    if (ppu_getMaskFlag(PPUMASK_SHOWBKG) && ppu_getMaskFlag(PPUMASK_SHOWSPRIT) && oamRAM[0] <= y && oamRAM[0] > y - 8) {
        containsSpriteZero = true;
    }

    // i <3 maf
    if (ppu_getMaskFlag(PPUMASK_SHOWBKG)) {
        for (uint8_t tileCol = 0; tileCol < 33; tileCol++) {
            
            uint8_t adjCol = ((tileCol + coarseX) % 32);
            uint8_t adjRow = ((tileRow + coarseY) % 32);

            uint16_t nametablePos = 0x2000 + nametableOffset;
            nametablePos += (((tileCol + coarseX) / 32) * 0x0400);
            
            // cycle 2-3
            uint8_t nametableByte = bus_readPPU(nametablePos + adjCol + (adjRow * 32));

            // cycle 3-4
            uint8_t attributeByte = bus_readPPU(nametablePos + 0x03C0 + (adjCol / 4) + ((adjRow / 4) * 8));
            if ((adjCol / 2) % 2 == 1) {
                attributeByte >>= 2;
            }
            if ((adjRow / 2) % 2 == 1) {
                attributeByte >>= 4;
            }
            attributeByte &= 0b00000011;

            // cycle 5-6
            uint8_t chrLow = bus_readPPU(bankOffset + (nametableByte * 16) + (y % 8));

            // cycle 6-7
            uint8_t chrHigh = bus_readPPU(bankOffset + (nametableByte * 16) + (y % 8) + 8);

            for (int pixCol = 7; pixCol >= 0; pixCol--) {
                uint8_t colorID = ((chrHigh & 1) << 1) | (chrLow & 1);
                chrLow >>= 1;
                chrHigh >>= 1;

                if (tileCol < 32) {
                    backgroundPixels[(tileCol * 8) + pixCol] = colorID;
                }
                

                if (colorID == 0) {
                    ppu_setPixel(ppu_colors[bus_readPPU(0x3F00)], (tileCol * 8) + pixCol - fineX, y - fineY);
                } else {
                    ppu_setPixel(ppu_getColor(attributeByte, colorID), (tileCol * 8) + pixCol - fineX, y - fineY);
                }
            }
        }
    }

    if (ppu_getMaskFlag(PPUMASK_SHOWSPRIT)) {
        for (int i = 0; i < 256; i += 4) {
            
            if (oamRAM[i] > y - 8 && oamRAM[i] <= y) {
                uint8_t relY = y - oamRAM[i]; 
                bool flipHorizontally = (oamRAM[i + 2] >> 6) & 1;
                bool flipVertically = (oamRAM[i + 2] >> 7) & 1;
                bool isBehindBackground = (oamRAM[i + 2] >> 5) & 1;
                if (flipVertically) relY = 7 - relY;
                uint8_t sprLow = bus_readPPU(spriteBankOffset + (oamRAM[i + 1] * 16) + relY);
                uint8_t sprHigh = bus_readPPU(spriteBankOffset + (oamRAM[i + 1] * 16) + relY + 8);
                
                uint8_t paletteID = (oamRAM[i + 2]) & 0b00000011;
                for (int pixCol = 7; pixCol >= 0; pixCol--) {
                    uint8_t colorID = ((sprHigh & 1) << 1) | (sprLow & 1);
                    sprLow >>= 1;
                    sprHigh >>= 1;

                    if (colorID != 0) {
                        uint8_t relX = oamRAM[i + 3] + ((flipHorizontally) ? 7 - pixCol : pixCol);
                        
                        if (backgroundPixels[relX] == 0 || !isBehindBackground) {
                            ppu_setPixel(ppu_getColor(paletteID + 4, colorID), relX, y);
                        }

                        if (containsSpriteZero && i == 0) {
                            triggerSpriteZero = true;
                        }
                    }
                }
            }
        }
    }
}

/* Prints a screen centered string */
void PrintCentered(const char *str)
{
    gfx_PrintStringXY(str,
                      (LCD_WIDTH - gfx_GetStringWidth(str)) / 2,
                      (LCD_HEIGHT - 8) / 2);
}


void ppu_drawCHRROM(uint16_t bank) {
    for (uint16_t y = 0; y < 16; y++) {
        for (uint16_t x = 0; x < 16; x++) {
            ppu_drawTile(bank, (y * 16) + x, 0, x * 8, y * 8);
        }
    }
    if (bank == 0) {
        ppu_drawCHRROM(1);
    }
}

void ppu_drawRAMPalette() {
    for (int i = 0; i < 32; i++) {
        uint16_t x = ((i % 16) + ((i % 16) / 4)) + 1;
        uint16_t y = (i / 16) * 2;
        uint16_t location = (y * 256 * 8) + (x * 8);
        uint32_t color = ppu_colors[bus_readPPU(0x3F00 + i)];
        for (int j = 0; j < 64; j++) {
            location += 1;
            if (j % 8 == 0) {
                location += 248;
            }
            //bitmap[location] = color;
        }
    }
}

uint32_t ppu_getColor(uint8_t palette, uint8_t colorID) {
    return ppu_colors[bus_readPPU(0x3F00 + (palette << 2) + colorID)];
}

uint32_t* ppu_getDisplayBitmap() {
    return NULL;
}

void ppu_setStatusFlag(enum PPUStatusFlag flag, bool enable) {
    if (enable) {
        reg_ppuStatus |= flag;
    } else {
        reg_ppuStatus &= ~flag;
    }
}

bool ppu_getStatusFlag(enum PPUStatusFlag flag) {
    return (reg_ppuStatus & flag) > 0;
}

void ppu_setMaskFlag(enum PPUMaskFlag flag, bool enable) {
    if (enable) {
        reg_mask |= flag;
    } else {
        reg_mask &= ~flag;
    }
}

bool ppu_getMaskFlag(enum PPUMaskFlag flag) {
    return (reg_mask & flag) > 0;
}

void ppu_setControlFlag(enum PPUControlFlag flag, bool enable) {
    if (enable) {
        reg_control |= flag;
    } else {
        reg_control &= ~flag;
    }
}

bool ppu_getControlFlag(enum PPUControlFlag flag) {
    return (reg_control & flag) > 0;
}

uint8_t ppu_getRegister(enum PPURegister reg) {
    if (reg == PPU_CONTROL) {
        return reg_control;
    } else if (reg == PPU_MASK) {
        return reg_mask;
    } else if (reg == PPU_STATUS) {
        uint8_t stat = reg_ppuStatus;
        ppu_setStatusFlag(PPUSTAT_VBLKSTART, false);
        addressLatch = false;
        scrollLatch = false;
        return stat;
    } else if (reg == PPU_OAMADDR) {
        return reg_oamaddr;
    } else if (reg == PPU_OAMDATA) {
        return oamRAM[reg_oamaddr];
    } else if (reg == PPU_SCROLL) {
        return reg_scroll;
    } else if (reg == PPU_PPUDATA) {
        if (addressBuffer < 0x3F00) {
            reg_ppudata = dataBuffer;
            dataBuffer = bus_readPPU(addressBuffer);
        } else {
            reg_ppudata = bus_readPPU(addressBuffer);
        }
        addressBuffer += ppu_getControlFlag(PPUCTRL_INCREMENT) ? 32 : 1;
        return reg_ppudata;
    } else if (reg == PPU_PPUADDR) {
        return reg_ppuaddr;
    } else if (reg == PPU_OAMDMA) {
        return reg_oamdma;
    }
    return 0;
}

void ppu_setRegister(enum PPURegister reg, uint8_t data) {
    if (reg == PPU_CONTROL) {
        reg_control = data;
    } else if (reg == PPU_MASK) {
        reg_mask = data;
    } else if (reg == PPU_STATUS) {
        reg_ppuStatus = data;
        addressLatch = false;
        scrollLatch = false; 
        ppu_setStatusFlag(PPUSTAT_VBLKSTART, false);
    } else if (reg == PPU_OAMADDR) {
        reg_oamaddr = data;
    } else if (reg == PPU_OAMDATA) {
        reg_oamdata = data;
        oamRAM[reg_oamaddr] = reg_oamdata;
    } else if (reg == PPU_SCROLL) {
        if (scrollLatch) {
            scrollY = data;
            scrollLatch = false;
        } else {
            scrollX = data;
            scrollLatch = true;
        }
        reg_scroll = data;
    } else if (reg == PPU_PPUDATA) {
        reg_ppudata = data;
        bus_writePPU(addressBuffer, reg_ppudata);
        addressBuffer += ppu_getControlFlag(PPUCTRL_INCREMENT) ? 32 : 1;
    } else if (reg == PPU_PPUADDR) {
        if (addressLatch) {
            addressBuffer = ((uint16_t) reg_ppuaddr) << 8;
            addressBuffer |= (uint16_t) data;
            addressLatch = false;
        } else {
            reg_ppuaddr = data;
            addressLatch = true;
        }
    } else if (reg == PPU_OAMDMA) {
        uint16_t startAddr = ((uint16_t) data) << 8;
        for (int i = 0; i < 256; i++) {
            oamRAM[i] = bus_readCPU(startAddr + i);
        }
        reg_oamdma = data;
    }
}