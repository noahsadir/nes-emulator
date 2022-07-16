/**
 * @file io.c
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

#include "io.h"

SDL_Window* window;
SDL_Surface* surface;

struct JoypadMapping keyMap;

uint16_t width = 256;
uint16_t height = 240;
uint16_t scale = 1;

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

void io_update(uint32_t* bitmap) {

    // read key events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == keyMap.up) {
                bus_setJoypad(JP_UP);
            } else if (event.key.keysym.sym == keyMap.down) {
                bus_setJoypad(JP_DOWN);
            } else if (event.key.keysym.sym == keyMap.left) {
                bus_setJoypad(JP_LEFT);
            } else if (event.key.keysym.sym == keyMap.right) {
                bus_setJoypad(JP_RIGHT);
            } else if (event.key.keysym.sym == keyMap.a) {
                bus_setJoypad(JP_BTN_A);
            } else if (event.key.keysym.sym == keyMap.b) {
                bus_setJoypad(JP_BTN_B);
            } else if (event.key.keysym.sym == keyMap.select) {
                bus_setJoypad(JP_SELECT);
            } else if (event.key.keysym.sym == keyMap.start) {
                bus_setJoypad(JP_START);
            }
        }

        if (event.type == SDL_KEYUP) {
            if (event.key.keysym.sym == keyMap.up) {
                bus_unsetJoypad(JP_UP);
            } else if (event.key.keysym.sym == keyMap.down) {
                bus_unsetJoypad(JP_DOWN);
            } else if (event.key.keysym.sym == keyMap.left) {
                bus_unsetJoypad(JP_LEFT);
            } else if (event.key.keysym.sym == keyMap.right) {
                bus_unsetJoypad(JP_RIGHT);
            } else if (event.key.keysym.sym == keyMap.a) {
                bus_unsetJoypad(JP_BTN_A);
            } else if (event.key.keysym.sym == keyMap.b) {
                bus_unsetJoypad(JP_BTN_B);
            } else if (event.key.keysym.sym == keyMap.select) {
                bus_unsetJoypad(JP_SELECT);
            } else if (event.key.keysym.sym == keyMap.start) {
                bus_unsetJoypad(JP_START);
            }
        }

        if (event.type == SDL_QUIT) {
            cpu_panic();
        }
    }

    // update display
    SDL_FreeSurface(surface);
    uint32_t* pixels = (uint32_t*)surface->pixels;
    SDL_LockSurface(surface);

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

    SDL_UnlockSurface(surface);
    SDL_UpdateWindowSurface(window);
}