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

#include "globalflags.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum {
  PPU_CONTROL,
  PPU_MASK,
  PPU_STATUS,
  PPU_OAMADDR,
  PPU_OAMDATA,
  PPU_SCROLL,
  PPU_PPUDATA,
  PPU_PPUADDR,
  PPU_OAMDMA
} PPURegisterType;

typedef struct {
  uint8_t control;
  uint8_t mask;
  uint8_t ppuStatus;
  uint8_t oamaddr;
  uint8_t oamdata;
  uint8_t scroll;
  uint8_t ppuaddr;
  uint8_t ppudata;
  uint8_t oamdma;
} PPURegisters;

enum PPUControlFlag {
  PPUCTRL_NAMETABLE1  = 0x1,
  PPUCTRL_NAMETABLE2  = 0x2,
  PPUCTRL_INCREMENT   = 0x4,
  PPUCTRL_SPRITEPATT  = 0x8,
  PPUCTRL_BKGPATT     = 0x10,
  PPUCTRL_SPRITESIZE  = 0x20,
  PPUCTRL_MASTERSLV   = 0x40,
  PPUCTRL_GENVBNMI    = 0x80
};

enum PPUMaskFlag {
  PPUMASK_GREYSCL     = 0x1,
  PPUMASK_BKGLEFT     = 0x2,
  PPUMASK_SPRITLEFT   = 0x4,
  PPUMASK_SHOWBKG     = 0x8,
  PPUMASK_SHOWSPRIT   = 0x10,
  PPUMASK_EMPHRED     = 0x20,
  PPUMASK_EMPHGREEN   = 0x40,
  PPUMASK_EMPHBLUE    = 0x80
};

enum PPUStatusFlag {
  PPUSTAT_OPEN0       = 0x1,
  PPUSTAT_OPEN1       = 0x2,
  PPUSTAT_OPEN2       = 0x4,
  PPUSTAT_OPEN3       = 0x8,
  PPUSTAT_OPEN4       = 0x10,
  PPUSTAT_SPRITEOVF   = 0x20,
  PPUSTAT_SPRITEZRO   = 0x40,
  PPUSTAT_VBLKSTART   = 0x80
};

/* PUBLIC METHODS - INTENDED FOR EXTERNAL USE */

/**
 * @brief Intialize PPU
 * 
 * @param vram the pointer to the VRAM
 * @param crom the pointer to the CHR ROM
 */
void ppu_init(uint8_t* crom, bool vmirror, uint8_t(*r)(uint16_t), void(*c)(uint32_t*));

/**
 * 
 * @brief Run the specified amount of cycles on the PPU
 * 
 * @param cycleCount the number of cycles
 */
void ppu_runCycles(uint32_t cycleCount);

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
uint8_t ppu_readRegister(PPURegisterType reg);

/**
 * @brief Directly set the value of the register
 * 
 * @param reg the register to write to
 * @param data the value to set the register to
 */
void ppu_writeRegister(PPURegisterType r, uint8_t data);

/* PRIVATE METHODS - NOT INTENDED FOR EXTERNAL USE */

/**
 * @brief Parse CHR ROM data for quicker reading
 */
void ppu_generateChrCache();

/**
 * @brief Draw a tile on the screen
 * 
 * @param bank the tile bank
 * @param tileID the tile ID
 * @param x the x position
 * @param y the y position
 */
static force_inline void ppu_drawTile(bool flipHorizontally, bool flipVertically, bool transparent, bool behindBackground, bool background, uint16_t tileID, uint8_t palette, uint16_t x, uint16_t y);

/**
 * @brief Set a pixel on the bitmap
 * 
 * @param color the RGB color of the pixel
 * @param x the x location on the bitmap
 * @param y the y location on the bitmap
 */
static force_inline void ppu_setPixel(uint32_t color, int16_t x, int16_t y);

/**
 * @brief Draw a scanline
 * 
 * @param y the scanline to draw
 */
static force_inline void ppu_drawScanline(uint8_t y);

/**
 * @brief Draw an entire frame
 */
static force_inline void ppu_drawFrame();

/**
 * @brief Perform write operation at mapped address.
 *        Note that read operations are simply done through
 *        direct array access.
 * 
 * @param address the address to write to
 * @param data the data to write
 */
static force_inline void ppu_writeMem(uint16_t address, uint8_t data);

#endif
