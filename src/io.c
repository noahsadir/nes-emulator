/**
 * @file io.c
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

#include "io.h"

SDL_Window* window;
SDL_Surface* surface;

struct JoypadMapping keyMap;

uint16_t width = 256;
uint16_t height = 240;
uint16_t scale = 1;

uint32_t panicbmp[DISPLAY_BITMAP_SIZE];

bool didPanic = false;

void io_init(uint16_t scl) {
  scale = scl;
  SDL_Init(SDL_INIT_VIDEO);
  window = SDL_CreateWindow("Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width * scale, height * scale, SDL_WINDOW_SHOWN);
  surface = SDL_GetWindowSurface(window);
  keyMap.up = SDLK_w;
  keyMap.down = SDLK_s;
  keyMap.left = SDLK_a;
  keyMap.right = SDLK_d;
  keyMap.a = SDLK_SPACE;
  keyMap.b = SDLK_RSHIFT;
  keyMap.start = SDLK_RETURN;
  keyMap.select = SDLK_p;
}

void io_pollJoypad(void(*toggle)(NESInput, bool)) {
  // read key events
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_KEYDOWN) {
      if (event.key.keysym.sym == keyMap.up) {
        toggle(INPUT_UP, true);
      }
      if (event.key.keysym.sym == keyMap.down) {
        toggle(INPUT_DOWN, true);
      }
      if (event.key.keysym.sym == keyMap.left) {
        toggle(INPUT_LEFT, true);
      }
      if (event.key.keysym.sym == keyMap.right) {
        toggle(INPUT_RIGHT, true);
      }
      if (event.key.keysym.sym == keyMap.a) {
        toggle(INPUT_A, true);
      }
      if (event.key.keysym.sym == keyMap.b) {
        toggle(INPUT_B, true);
      }
      if (event.key.keysym.sym == keyMap.select) {
        toggle(INPUT_SELECT, true);
      }
      if (event.key.keysym.sym == keyMap.start) {
        toggle(INPUT_START, true);
      }
    }

    if (event.type == SDL_KEYUP) {
      if (event.key.keysym.sym == keyMap.up) {
        toggle(INPUT_UP, false);
      }
      if (event.key.keysym.sym == keyMap.down) {
        toggle(INPUT_DOWN, false);
      }
      if (event.key.keysym.sym == keyMap.left) {
        toggle(INPUT_LEFT, false);
      }
      if (event.key.keysym.sym == keyMap.right) {
        toggle(INPUT_RIGHT, false);
      }
      if (event.key.keysym.sym == keyMap.a) {
        toggle(INPUT_A, false);
      }
      if (event.key.keysym.sym == keyMap.b) {
        toggle(INPUT_B, false);
      }
      if (event.key.keysym.sym == keyMap.select) {
        toggle(INPUT_SELECT, false);
      }
      if (event.key.keysym.sym == keyMap.start) {
        toggle(INPUT_START, false);
      }
    }

    if (event.type == SDL_QUIT) {
      toggle(INPUT_QUIT, true);
    }
  }
}

void io_panic(char* str) {
  didPanic = true;
  for (int i = 0; i < DISPLAY_HEIGHT * DISPLAY_WIDTH; i++) {
    panicbmp[i] = colors[0x0F];
  }
  int len = 0;
  while (str[len] != '\0') len += 1;
  io_printString("PANIC!", 104, 64);
  if (len < 32) {
    io_printString(str, 128 - ((len / 2) * 8), 80);
  } else {
    io_printString(str, 0, 80);
  }
}

void io_printString(char* str, uint8_t x, uint8_t y) {
  while (*str != '\0') {
    if (x >= 248 || *str == '\n') {
      x = 0;
      y += 8;
    }

    if (*str != '\n') {
      io_printChar(*str, x, y);
      x += 8;
      str += 1;
    }
  }
  io_update(NULL);
}

void io_printChar(char chr, uint8_t x, uint8_t y) {
  int pos = (y * DISPLAY_WIDTH) + x;
  uint64_t charPix = font[chr & 127];
  int8_t shift = 63;
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 8; col++) {
      if (didPanic) {
        if (y == 64) {
          panicbmp[pos] = ((charPix >> shift) & 1) ? 0xFFFFFF : colors[0x06];
        } else {
          panicbmp[pos] = ((charPix >> shift) & 1) ? 0xFFFFFF : colors[0x0F];
        }
      } else {
        bitmap[pos] = ((charPix >> shift) & 1) ? 0xFFFFFF : 0x000000;
      }
      shift -= 1;
      pos += 1;
    }
    pos -= 8;
    pos += DISPLAY_WIDTH;
  }
}

void io_update(char* overlay) {
  if (overlay != NULL) {
    io_printString(overlay, 4, 4);
    return;
  }
  // update display
  SDL_FreeSurface(surface);
  uint32_t* pixels = (uint32_t*)surface->pixels;
  SDL_LockSurface(surface);

  if (didPanic) {
    for (int i = 0; i < 256 * 240; i++) {
      int x = i % 256;
      int y = i / 256;
      int scaleSquared = scale * scale;
      for (int subpix = 0; subpix < scaleSquared; subpix++) {
        int px = subpix % scale;
        int py = subpix / scale;
        pixels[(x * scale) + (y * scaleSquared * width) + px + (py * width * scale)] = panicbmp[i];
      }
    }
  } else {
    for (int i = 0; i < 256 * 240; i++) {
      int x = i % 256;
      int y = i / 256;
      int scaleSquared = scale * scale;
      for (int subpix = 0; subpix < scaleSquared; subpix++) {
        int px = subpix % scale;
        int py = subpix / scale;
        pixels[(x * scale) + (y * scaleSquared * width) + px + (py * width * scale)] = bitmap[i];
      }
    }
  }
  

  SDL_UnlockSurface(surface);
  SDL_UpdateWindowSurface(window);
}
