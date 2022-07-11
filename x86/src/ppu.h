/**
 * @file ppu.h
 * @author Noah Sadir (development.noahsadir@gmail.com)
 * @brief A basic emulation of the NES PPU
 * @version 1.0
 * @date 2022-07-04
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

#ifndef PPU_H
#define PPU_H

#include "bus.h"
#include "chars.h"

enum PPURegister {
    PPU_CONTROL,
    PPU_MASK,
    PPU_STATUS,
    PPU_OAMADDR,
    PPU_OAMDATA,
    PPU_SCROLL,
    PPU_PPUDATA,
    PPU_PPUADDR,
    PPU_OAMDMA
};

enum PPUControlFlag {
    PPUCTRL_NAMETABLE1  = 0b00000001,
    PPUCTRL_NAMETABLE2  = 0b00000010,
    PPUCTRL_INCREMENT   = 0b00000100,
    PPUCTRL_SPRITEPATT  = 0b00001000,
    PPUCTRL_BKGPATT     = 0b00010000,
    PPUCTRL_SPRITESIZE  = 0b00100000,
    PPUCTRL_MASTERSLV   = 0b01000000,
    PPUCTRL_GENVBNMI    = 0b10000000
};

enum PPUMaskFlag {
    PPUMASK_GREYSCL     = 0b00000001,
    PPUMASK_BKGLEFT     = 0b00000010,
    PPUMASK_SPRITLEFT   = 0b00000100,
    PPUMASK_SHOWBKG     = 0b00001000,
    PPUMASK_SHOWSPRIT   = 0b00010000,
    PPUMASK_EMPHRED     = 0b00100000,
    PPUMASK_EMPHGREEN   = 0b01000000,
    PPUMASK_EMPHBLUE    = 0b10000000
};

enum PPUStatusFlag {
    PPUSTAT_OPEN0       = 0b00000001,
    PPUSTAT_OPEN1       = 0b00000010,
    PPUSTAT_OPEN2       = 0b00000100,
    PPUSTAT_OPEN3       = 0b00001000,
    PPUSTAT_OPEN4       = 0b00010000,
    PPUSTAT_SPRITEOVF   = 0b00100000,
    PPUSTAT_SPRITEZRO   = 0b01000000,
    PPUSTAT_VBLKSTART   = 0b10000000
};

// taken from https://github.com/toblu302/NESlig/blob/master/src/ppu2C02.h
static const uint32_t ppu_colors[64] =
{
0x757575, 0x271B8F, 0x0000AB, 0x47009F, 0x8F0077, 0xAB0013, 0xA70000, 0x7F0B00,
0x432F00, 0x004700, 0x005100, 0x003F17, 0x1B3F5F, 0x000000, 0x000000, 0x000000,
0xBCBCBC, 0x0073EF, 0x233BEF, 0x8300F3, 0xBF00BF, 0xE7005B, 0xDB2B00, 0xCB4F0F,
0x8B7300, 0x009700, 0x00AB00, 0x00933B, 0x00838B, 0x000000, 0x000000, 0x000000,
0xFFFFFF, 0x3FBFFF, 0x5F97FF, 0xA78BFD, 0xF77BFF, 0xFF77B7, 0xFF7763, 0xFF9B3B,
0xF3BF3F, 0x83D313, 0x4FDF4B, 0x58F898, 0x00EBDB, 0x000000, 0x000000, 0x000000,
0xFFFFFF, 0xABE7FF, 0xC7D7FF, 0xD7CBFF, 0xFFC7FF, 0xFFC7DB, 0xFFBFB3, 0xFFDBAB,
0xFFE7A3, 0xE3FFA3, 0xABF3BF, 0xB3FFCF, 0x9FFFF3, 0x000000, 0x000000, 0x000000
};

static const uint32_t ppu_colors2[64] = {
    0x808080, 0x003DA6, 0x0012B0, 0x440096, 0xA1005E,
    0xC70028, 0xBA0600, 0x8C1700, 0x5C2F00, 0x104500,
    0x054A00, 0x00472E, 0x004166, 0x000000, 0x050505,
    0x050505, 0xC7C7C7, 0x0077FF, 0x2155FF, 0x8237FA,
    0xEB2FB5, 0xFF2950, 0xFF2200, 0xD63200, 0xC46200,
    0x358000, 0x058F00, 0x008A55, 0x0099CC, 0x212121,
    0x090909, 0x090909, 0xFFFFFF, 0x0FD7FF, 0x69A2FF,
    0xD480FF, 0xFF45F3, 0xFF618B, 0xFF8833, 0xFF9C12,
    0xFABC20, 0x9FE30E, 0x2BF035, 0x0CF0A4, 0x05FBFF,
    0x5E5E5E, 0x0D0D0D, 0x0D0D0D, 0xFFFFFF, 0xA6FCFF,
    0xB3ECFF, 0xDAABEB, 0xFFA8F9, 0xFFABB3, 0xFFD2B0,
    0xFFEFA6, 0xFFF79C, 0xD7E895, 0xA6EDAF, 0xA2F2DA,
    0x99FFFC, 0xDDDDDD, 0x111111, 0x111111
};

/* PUBLIC METHODS - INTENDED FOR EXTERNAL USE */

/**
 * @brief Intialize PPU
 * 
 * @param vram the pointer to the VRAM
 * @param crom the pointer to the CHR ROM
 */
void ppu_init(uint8_t* vram, uint8_t* crom);

/**
 * 
 * @brief Run the specified amount of cycles on the PPU
 * 
 * @param cycleCount the number of cycles
 */
void ppu_runCycles(uint8_t cycleCount);

/**
 * @brief Set a status flag of PPU
 * 
 * @param flag the flag to set
 * @param val the value of the flag
 */
void ppu_setStatusFlag(enum PPUStatusFlag flag, bool enable);

/**
 * @brief Get a status flag of PPU
 * 
 * @param flag the flag to get
 * @return the value of the flag
 */
bool ppu_getStatusFlag(enum PPUStatusFlag flag);

/**
 * @brief Set a mask flag of PPU
 * 
 * @param flag the flag to set
 * @param val the value of the flag
 */
void ppu_setMaskFlag(enum PPUMaskFlag flag, bool enable);

/**
 * @brief Get a mask flag of PPU
 * 
 * @param flag the flag to get
 * @return the value of the flag
 */
bool ppu_getMaskFlag(enum PPUMaskFlag flag);

/**
 * @brief Set a control flag of PPU
 * 
 * @param flag the flag to set
 * @param val the value of the flag
 */
void ppu_setControlFlag(enum PPUControlFlag flag, bool enable);

/**
 * @brief Get a control flag of PPU
 * 
 * @param flag the flag to get
 * @return the value of the flag
 */
bool ppu_getControlFlag(enum PPUControlFlag flag);

/**
 * @brief Get raw value of register
 * 
 * @param reg the register to read
 * @return uint8_t the value of the register
 */
uint8_t ppu_getRegister(enum PPURegister reg);

/**
 * @brief Directly set the value of the register
 * 
 * @param reg the register to write to
 * @param data the value to set the register to
 */
void ppu_setRegister(enum PPURegister reg, uint8_t data);

/**
 * @brief Get number of frames generated
 * 
 * @return uint64_t the number of frames
 */
uint64_t ppu_getFrames();

/**
 * @brief Get a 256 x 240 bitmap of the display
 * 
 * @return uint32_t* the bitmap array
 */
uint32_t* ppu_getDisplayBitmap();

/* PRIVATE METHODS - NOT INTENDED FOR EXTERNAL USE */

/**
 * @brief Render a scanline
 */
void ppu_scanline();

/**
 * @brief Draw the CHR ROM
 */
void ppu_drawCHRROM(uint16_t bank);

/**
 * @brief Draw the RAM Palette
 */
void ppu_drawRAMPalette();

/**
 * @brief Get the RGB color of a pixel
 * 
 * @param palette the RAM palette
 * @param colorID the color ID
 * @return uint32_t the RGB color
 */
uint32_t ppu_getColor(uint8_t palette, uint8_t colorID);


/**
 * @brief Draw a tile on the screen
 * 
 * @param bank the tile bank
 * @param tileID the tile ID
 * @param x the x position
 * @param y the y position
 */
void ppu_drawTile(bool bank, uint16_t tileID, uint8_t palette, uint16_t x, uint16_t y);

/**
 * @brief Set a pixel on the bitmap
 * 
 * @param color the RGB color of the pixel
 * @param x the x location on the bitmap
 * @param y the y location on the bitmap
 */
void ppu_setPixel(uint32_t color, int16_t x, int16_t y);

/**
 * @brief Draw a scanline
 * 
 * @param y the scanline to draw
 */
void ppu_drawScanline(uint8_t y);

#endif