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

PPURegisters ppureg;

uint16_t addressBuffer = 0x0000;
uint8_t scrollX = 0;
uint8_t scrollY = 0;
uint8_t dataBuffer = 0x00;
bool addressLatch = false;
bool scrollLatch = false;
bool triggerSpriteZero = false;
bool verticalMirroring = false;
bool didRenderFrame = false;

uint8_t oamRAM[0x00FF];
uint8_t vidRAM[0x2000];
uint8_t paletteRAM[32];
uint8_t* chrROM;
uint32_t bitmap[DISPLAY_BITMAP_SIZE];
uint32_t debugbmp[DISPLAY_PIXEL_SIZE];
uint8_t chrCache[512][64];

uint64_t ppuCycles = 0;
uint64_t ppuFrames = 0;
uint16_t scanline = 0;
uint16_t spriteZeroScanline = 0;
uint16_t initialScrollX = 0;
uint16_t initialScrollY = 0;
uint16_t scanlineCycleCounter = 0;

void(*callback)(uint32_t*);
uint8_t(*readCPUDirect)(uint16_t);

void ppu_init(uint8_t* crom, bool vmirror, uint8_t(*r)(uint16_t), void(*c)(uint32_t*)) {
  callback = c;
  chrROM = crom;
  readCPUDirect = r;
  verticalMirroring = vmirror;
  ppureg.control     = 0x00;
  ppureg.mask        = 0x00;
  ppureg.ppuStatus   = 0x00;
  ppureg.oamaddr     = 0x00;
  ppureg.oamdata     = 0x00;
  ppureg.scroll      = 0x00;
  ppureg.ppuaddr     = 0x00;
  ppureg.ppudata     = 0x00;
  ppureg.oamdma      = 0x00;
  ppu_generateChrCache();
  for (int i = 0; i < 61440; i++) {
    bitmap[i] = 0;
  }
}

void ppu_runCycles(uint32_t cycleCount) {
  #if (PPU_IMMEDIATE_CATCHUP)
  ppuCycles += cycleCount;
  if (ppuCycles >= 341) {
    // render scanlines
    if (scanline >= 0 && scanline <= 239) {
      ppu_drawScanline(scanline);
    }

    // render all at once rather than by scanline
    if (scanline == 240) {
      callback(bitmap);
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
    ppuCycles -= 341;
  }
  #else
  ppuCycles += cycleCount;
  scanlineCycleCounter += cycleCount;

  // ensures 1-scanline delay for sprite zero hit
  if (triggerSpriteZero == true) {
    ppu_setStatusFlag(PPUSTAT_SPRITEZRO, true);
    triggerSpriteZero = false;
  }
  
  if (ppuCycles >= PPU_FRAME_CYCLES) {
    // end of frame; reset vblank and sprite zero hit
    ppureg.ppuStatus = 0x00;
    ppuCycles -= PPU_FRAME_CYCLES;
    didRenderFrame = false;
    scrollX = 0;
    scrollY = 0;
    initialScrollX = 0;
    initialScrollY = 0;
  } else if (!didRenderFrame && ppuCycles >= PPU_SCANLINE_CYCLES * DISPLAY_HEIGHT) {
    // render all scanlines all at once and start vblank
    ppu_drawFrame();
    callback(bitmap);
    didRenderFrame = true;
    ppu_setStatusFlag(PPUSTAT_VBLKSTART, true);
  }

  // check for sprite zero hit
  uint16_t sl = ppuCycles / 341;
  if (!triggerSpriteZero && ppu_getMaskFlag(PPUMASK_SHOWBKG) && ppu_getMaskFlag(PPUMASK_SHOWSPRIT) && oamRAM[0] <= sl && oamRAM[0] > sl - 8) {
    triggerSpriteZero = true;
    spriteZeroScanline = sl;
  }

  // store initial scroll values in case they're changed mid-scroll
  if (sl == 0) {
    initialScrollX = scrollX;
    initialScrollY = scrollY;
  }
  #endif
}

uint64_t ppu_getFrames() {
  return ppuFrames;
}

static force_inline void ppu_setPixel(uint32_t color, int16_t x, int16_t y) {
  if (x >= 0 && x < 256 && y >= 0 && y < 240) bitmap[(y * 256) + x] = color;
}

static force_inline void ppu_drawTile(bool flipHorizontally, bool flipVertically, bool transparent, bool behindBackground, bool background, uint16_t tileID, uint8_t palette, uint16_t x, uint16_t y) {
  for (uint16_t row = 0; row < 8; row++) {
    for (uint16_t col = 0; col < 8; col++) {
      uint8_t color = chrCache[tileID][(row * 8) + col];
      if ((transparent && color == 0)) continue;
      if (behindBackground) {
        // before drawing behind-background sprite, check if this pixel
        // is same as "clear" color (palette entry 0)
        // not 100% accurate, but good enough for now
        int16_t bmpX = x + (flipHorizontally ? col : (7 - col));
        int16_t bmpY = y + (flipVertically ? (7 - row) : row);
        if (bitmap[(bmpY * 256) + bmpX] == colors[paletteRAM[0]]) {
          ppu_setPixel(colors[paletteRAM[(palette << 2) + color]], bmpX, bmpY);
        }
      } else if (background && color == 0) {
        ppu_setPixel(colors[paletteRAM[0]], x + (flipHorizontally ? col : (7 - col)), y + (flipVertically ? (7 - row) : row));
      } else {
        ppu_setPixel(colors[paletteRAM[(palette << 2) + color]], x + (flipHorizontally ? col : (7 - col)), y + (flipVertically ? (7 - row) : row));
      }
    }
  }
}

void ppu_drawCHRROM(uint16_t bank) {
  for (uint16_t y = 0; y < 16; y++) {
    for (uint16_t x = 0; x < 16; x++) {
      ppu_drawTile(false, false, false, false, false, (y * 16) + x + (bank * 128), 0, x * 8, y * 8);
    }
  }
  if (bank == 0) {
    ppu_drawCHRROM(1);
  }
}

void ppu_generateChrCache() {
  // Storage of character data is very memory efficient, but also
  // computationally expensive to decode.
  // Since memory is cheap and abundant now, we can store it in a manner
  // that's easier to read from on-the-fly
  for (uint16_t tileID = 0; tileID < 512; tileID++) {
    for (uint16_t row = 0; row < 8; row++) {
      uint8_t high = chrROM[((tileID * 16) + row + 8) & 0x3FFF];
      uint8_t low = chrROM[((tileID * 16) + row) & 0x3FFF];
      for (uint16_t col = 0; col < 8; col++) {
        uint8_t color = ((high & 1) << 1) | (low & 1);
        high >>= 1;
        low >>= 1;
        chrCache[tileID][(row * 8) + col] = color;
      }
    }
  }
}

static force_inline void ppu_drawFrame() {
  // decode nametable & attribute table
  uint8_t nametables[4][960];
  uint8_t attributeTables[4][960];

  uint16_t bankOffset = 256 * ppu_getControlFlag(PPUCTRL_BKGPATT);
  uint16_t spriteBankOffset = 256 * ppu_getControlFlag(PPUCTRL_SPRITEPATT);

  uint8_t nametableID = (((uint16_t) ppu_getControlFlag(PPUCTRL_NAMETABLE2)) << 1) | ((uint16_t) ppu_getControlFlag(PPUCTRL_NAMETABLE1));
  
  bool showBackground = ppu_getMaskFlag(PPUMASK_SHOWBKG);
  bool showSprite = ppu_getMaskFlag(PPUMASK_SHOWSPRIT);

  // render all nametables for simplicity's sake
  for (int n = 0; n < 4; n++) {
    for (int i = 0; i < 960; i++) {
      uint8_t ntRow = i / 32;
      uint8_t ntCol = i % 32;

      // https://www.nesdev.org/wiki/PPU_attribute_tables
      // There might be a simpler way to get palette value for tile
      // but this is the best I could manage.
      uint8_t attrByte = vidRAM[(0x400 * n) + 960 + ((ntRow / 4) * 8) + (ntCol / 4)];
      uint8_t quadrantPalette = (attrByte >> ((((ntRow / 2) % 2) * 4) + (((ntCol / 2) % 2) * 2))) & BIT_FILL_2;
      attributeTables[n][i] = quadrantPalette;
      nametables[n][i] = vidRAM[(0x400 * n) + (ntRow * 32) + ntCol];
    }
  }

  // render visible background
  uint16_t ntRow, ntCol, ntPos, scrolledNametable;
  for (int row = 0; row <= 30; row++) { // load 31 rows to account for scroll
    // determine scroll value based on sprite zero hit location
    uint8_t fineX = ((row * 8) > spriteZeroScanline ? scrollX : initialScrollX) % 8;
    uint8_t fineY = ((row * 8) > spriteZeroScanline ? scrollY : initialScrollY) % 8;
    uint8_t coarseX = ((row * 8) > spriteZeroScanline ? scrollX : initialScrollX) / 8;
    uint8_t coarseY = ((row * 8) > spriteZeroScanline ? scrollY : initialScrollY) / 8;

    for (int col = 0; col <= 32; col++) { // load 33 rows to account for scroll
      // adjust row & tile for scroll
      ntCol = col + coarseX;
      ntRow = row + coarseY;

      // account for nametable & scroll overflow
      scrolledNametable = nametableID;
      if (ntCol >= 32) scrolledNametable += 1;
      if (ntRow >= 32) scrolledNametable += 2;
      scrolledNametable %= 4;
      ntPos = ((ntRow % 32) * 32) + (ntCol % 32);

      // ignore above calculations if before sprite zero hit
      if ((row * 8) <= spriteZeroScanline) scrolledNametable = 0; // a little hacky to just set nametable to 0

      if (showBackground) ppu_drawTile(false, false, false, false, true, bankOffset + nametables[scrolledNametable][ntPos], attributeTables[scrolledNametable][ntPos], (col * 8) - fineX, (row * 8) - fineY);
    }
  }

  // draw sprites from OAM
  for (int i = 0; i < 256; i += 4) {
    uint8_t relY = oamRAM[i];
    uint8_t relX = oamRAM[i + 3];
    bool flipHorizontally = (oamRAM[i + 2] >> 6) & 1;
    bool flipVertically = (oamRAM[i + 2] >> 7) & 1;
    bool isBehindBackground = (oamRAM[i + 2] >> 5) & 1;
    uint8_t paletteID = (oamRAM[i + 2]) & BIT_FILL_2;
    // only draw sprite if visible and within visible bounds
    if (relY < 0xEF && relY != 0x00 && showSprite) ppu_drawTile(flipHorizontally, flipVertically, true, isBehindBackground, false, spriteBankOffset + oamRAM[i + 1], paletteID + 4, relX, relY);
  }
}

static force_inline void ppu_drawScanline(uint8_t y) {
  uint8_t fineX = scrollX % 8;
  uint8_t fineY = scrollY % 8;
  uint8_t coarseX = scrollX / 8;
  uint8_t coarseY = scrollY / 8;
  uint8_t tileRow = y / 8;

  uint16_t nametableID = (((((uint16_t) ppu_getControlFlag(PPUCTRL_NAMETABLE2)) << 1) | ((uint16_t) ppu_getControlFlag(PPUCTRL_NAMETABLE1))) * 0x0400);
  uint16_t bankOffset = 256 * ppu_getControlFlag(PPUCTRL_BKGPATT);
  uint16_t spriteBankOffset = 256 * ppu_getControlFlag(PPUCTRL_SPRITEPATT);
  
  bool containsSpriteZero = false;
  bool showBackground = ppu_getMaskFlag(PPUMASK_SHOWBKG);
  bool showSprite = ppu_getMaskFlag(PPUMASK_SHOWSPRIT);

  // ensures 1-scanline delay for sprite zero hit
  if (triggerSpriteZero == true) {
    ppu_setStatusFlag(PPUSTAT_SPRITEZRO, true);
    triggerSpriteZero = false;
  }

  // indicate that sprite zero is present in the scanline
  if (showBackground && showSprite && oamRAM[0] <= y && oamRAM[0] > y - 8) {
    containsSpriteZero = true;
  }

  // draw background
  for (uint8_t tileCol = 0; tileCol < 33; tileCol++) {
    uint8_t adjCol = ((tileCol + coarseX) % 32);
    uint8_t adjRow = ((tileRow + coarseY) % 32);

    uint16_t nametablePos = nametableID + (((tileCol + coarseX) / 32) * 0x0400);
    
    // cycle 2-3
    uint8_t nametableByte = vidRAM[nametablePos + adjCol + (adjRow * 32)];

    // cycle 3-4
    uint8_t attributeByte = vidRAM[nametablePos + 0x03C0 + (adjCol / 4) + ((adjRow / 4) * 8)];
    if ((adjCol / 2) % 2 == 1) {
        attributeByte >>= 2;
    }
    if ((adjRow / 2) % 2 == 1) {
        attributeByte >>= 4;
    }
    attributeByte &= BIT_FILL_2;

    uint8_t* tile = chrCache[bankOffset + nametableByte];
    for (int pixCol = 0; pixCol < 8; pixCol++) {
      uint8_t colorID = tile[((y % 8) * 8) + (7 - pixCol)];
      if (showBackground) ppu_setPixel(colors[paletteRAM[colorID ? ((attributeByte << 2) + colorID) : 0]], (tileCol * 8) + pixCol - fineX, y - fineY);
    }
  }

  // draw sprites from OAM
  for (int i = 0; i < 256; i += 4) {
    // skip this sprite if not present in scanline
    if (!(oamRAM[i] > y - 8 && oamRAM[i] <= y)) continue;

    // parse sprite data
    uint8_t relY = y - oamRAM[i]; 
    bool flipHorizontally = (oamRAM[i + 2] >> 6) & 1;
    bool flipVertically = (oamRAM[i + 2] >> 7) & 1;
    bool isBehindBackground = (oamRAM[i + 2] >> 5) & 1;
    if (flipVertically) relY = 7 - relY;
    uint8_t paletteID = (oamRAM[i + 2]) & BIT_FILL_2;
    uint8_t* tile = chrCache[spriteBankOffset + oamRAM[i + 1]];

    // only draw sprite if visible and within visible bounds
    for (int pixCol = 7; pixCol >= 0; pixCol--) {
      uint8_t colorID = tile[(relY * 8) + (7 - pixCol)];
      if (colorID == 0) continue; // skip drawing this pixel if transparent
      uint8_t relX = oamRAM[i + 3] + ((flipHorizontally) ? 7 - pixCol : pixCol);

      // only draw if sprite is in foreground (or if background is clear)
      if (bitmap[(y * 256) + relX] == colors[paletteRAM[0]] || !isBehindBackground) ppu_setPixel(colors[paletteRAM[((paletteID + 4) << 2) + colorID]], relX, y);
      
      // encountered sprite zero hit; trigger it at next scanline
      if (containsSpriteZero && i == 0) triggerSpriteZero = true;
    }
  }
}

void ppu_setStatusFlag(enum PPUStatusFlag flag, bool enable) {
  if (enable) {
    ppureg.ppuStatus |= flag;
  } else {
    ppureg.ppuStatus &= ~flag;
  }
}

bool ppu_getStatusFlag(enum PPUStatusFlag flag) {
  return (ppureg.ppuStatus & flag) > 0;
}

void ppu_setMaskFlag(enum PPUMaskFlag flag, bool enable) {
  if (enable) {
    ppureg.mask |= flag;
  } else {
    ppureg.mask &= ~flag;
  }
}

bool ppu_getMaskFlag(enum PPUMaskFlag flag) {
  return (ppureg.mask & flag) > 0;
}

void ppu_setControlFlag(enum PPUControlFlag flag, bool enable) {
  if (enable) {
    ppureg.control |= flag;
  } else {
    ppureg.control &= ~flag;
  }
}

bool ppu_getControlFlag(enum PPUControlFlag flag) {
  return (ppureg.control & flag) > 0;
}

uint8_t ppu_getRegister(PPURegisterType r) {
  switch (r) {
    case PPU_CONTROL: return ppureg.control;
    case PPU_MASK: return ppureg.mask;
    case PPU_STATUS: {
      uint8_t stat = ppureg.ppuStatus;
      ppu_setStatusFlag(PPUSTAT_VBLKSTART, false);
      addressLatch = false;
      scrollLatch = false;
      return stat;
    }
    case PPU_OAMADDR: return ppureg.oamaddr;
    case PPU_OAMDATA: return oamRAM[ppureg.oamaddr];
    case PPU_SCROLL: return ppureg.scroll;
    case PPU_PPUDATA: {
      if (addressBuffer < 0x2000) { // chr
        ppureg.ppudata = dataBuffer;
        dataBuffer = chrROM[addressBuffer & 0x3FFF];
      } else if (addressBuffer < 0x3F00) { // nametable
        ppureg.ppudata = dataBuffer;
        dataBuffer = vidRAM[addressBuffer - 0x2000];
      } else { // palette
        ppureg.ppudata = paletteRAM[addressBuffer - 0x3F00];
      }
      addressBuffer += ppu_getControlFlag(PPUCTRL_INCREMENT) ? 32 : 1;
      return ppureg.ppudata;
    }
    case PPU_PPUADDR: return ppureg.ppuaddr;
    case PPU_OAMDMA: return ppureg.oamdma;
  }
  return 0;
}

void ppu_setRegister(PPURegisterType r, uint8_t data) {
  switch (r) {
    case PPU_CONTROL: {
      ppureg.control = data;
      break;
    }
    case PPU_MASK: {
      ppureg.mask = data;
      break;
    }
    case PPU_STATUS: {
      ppureg.ppuStatus = data;
      addressLatch = false;
      scrollLatch = false;
      ppu_setStatusFlag(PPUSTAT_VBLKSTART, false);
      break;
    }
    case PPU_OAMADDR: {
      ppureg.oamaddr = data;
      break;
    }
    case PPU_OAMDATA: {
      ppureg.oamdata = data;
      oamRAM[ppureg.oamaddr] = ppureg.oamdata;
      break;
    }
    case PPU_SCROLL: {
      scrollLatch ? (scrollY = data) : (scrollX = data);
      scrollLatch = !scrollLatch;
      ppureg.scroll = data;
      break;
    }
    case PPU_PPUDATA: {
      ppureg.ppudata = data;
      ppu_writeMem(addressBuffer, ppureg.ppudata);
      addressBuffer += ppu_getControlFlag(PPUCTRL_INCREMENT) ? 32 : 1;
      break;
    }
    case PPU_PPUADDR: {
      if (addressLatch) {
        addressBuffer = ((uint16_t) ppureg.ppuaddr) << 8;
        addressBuffer |= (uint16_t) data;
        addressLatch = false;
      } else {
        ppureg.ppuaddr = data;
        addressLatch = true;
      }
      break;
    }
    case PPU_OAMDMA: {
      uint16_t startAddr = ((uint16_t) data) << 8;
      for (int i = 0; i < 256; i++) {
        oamRAM[i] = readCPUDirect(startAddr + i);
      }
      break;
    }
  }
}

static force_inline uint8_t ppu_readMem(uint16_t address) {
  if (address < 0x2000) { // chr rom
    return chrROM[address & 0x3FFF];
  } else if (address < 0x3F00) { // vram
    return vidRAM[address & 0x1FFF];
  } else if (address < 0x4000) {
    address &= 0x1F;
    if (address == 0x0010 || address == 0x0014 || address == 0x0018 || address == 0x001C) {
      address -= 0x0010;
    }
    return paletteRAM[address];
  }
  return 0x00;
}

static force_inline void ppu_writeMem(uint16_t address, uint8_t data) {
  address = address & 0x3FFF;
  if (address < 0x2000) { // chr rom
    // read only -- invalid operation
  } else if (address < 0x3F00) {
    // I'm sure this incredibly convoluted mirroring makes more sense in
    // the real hardware implementation. But boy is this a mess in software!

    // NOTE: VRAM addresses 0x2000-0x3FFF, but for simplicity we'll subtract 0x2000
    //       and flatten 0x3000-0x3FFF onto 0x2000-0x2FFF
    uint16_t primaryAddr = address & 0x0FFF;
    uint16_t secondaryAddr = primaryAddr;

    // Per https://www.nesdev.org/wiki/PPU_nametables,
    // Vertical mirroring: $2000 equals $2800 and $2400 equals $2C00 (e.g. Super Mario Bros.)
    // Horizontal mirroring: $2000 equals $2400 and $2800 equals $2C00 (e.g. Kid Icarus)
    if (verticalMirroring) {
      if ((primaryAddr / 0x400) & 3) { // (if quadrant >= 2)
        secondaryAddr = primaryAddr - 0x0800;
      } else {
        secondaryAddr = primaryAddr + 0x0800;
      }
    } else {
      if ((primaryAddr / 0x400) & 1) { // (if quadrant is odd)
        secondaryAddr = primaryAddr - 0x0400;
      } else {
        secondaryAddr = primaryAddr + 0x0400;
      }
    }
    // Data is mirrored to 4 addresses on VRAM
    vidRAM[primaryAddr] = data;
    vidRAM[secondaryAddr] = data;
    // 0x2000-0x2FFF mirrors 0x3000-0x3FFF
    vidRAM[primaryAddr + 0x1000] = data;
    vidRAM[secondaryAddr + 0x1000] = data;
  } else if (address < 0x4000) {
    address &= 0x1F;
    if (address == 0x0010 || address == 0x0014 || address == 0x0018 || address == 0x001C) {
      address -= 0x0010;
    }
    paletteRAM[address] = data;
  }
}
