/**
 * @file io.h
 * @author Noah Sadir (development.noahsadir@gmail.com)
 * @brief Link abstract emulator implementation to host hardware
 * @version 1.0
 * @date 2022-12-20
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

#ifndef IO_H
#define IO_H

#include "globalflags.h"
#include "font.h"

#include <SDL2/SDL.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>

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

typedef enum {
  INPUT_UP      = 0x0,
  INPUT_DOWN    = 0x1,
  INPUT_LEFT    = 0x2,
  INPUT_RIGHT   = 0x3,
  INPUT_A       = 0x4,
  INPUT_B       = 0x5,
  INPUT_SELECT  = 0x6,
  INPUT_START   = 0x7,
  INPUT_QUIT    = 0x8
} NESInput;

/**
 * @brief Initialize I/O
 */
void io_init(uint16_t scl);

/**
 * @brief Poll Joypad input
 * 
 * @param t the callback function
 */
void io_pollJoypad(void(*t)(NESInput, bool));

/**
 * @brief Update controller and display values
 * 
 * @param bitmap the screen bitmap of the PPU
 */
void io_update(char* overlay);

/**
 * @brief Print a string directly to the display
 * 
 * @param str 
 */
void io_printString(char* str, uint16_t x, uint8_t y);

/**
 * @brief Trigger panic
 * 
 * @param str the error message
 */
void io_panic(char* str);

/**
 * @brief Print a character to display
 * 
 * @param chr the character
 * @param x the x coordinate
 * @param y the y coordinate
 */
void io_printChar(char chr, uint16_t x, uint8_t y);

/**
 * @brief Draw all 4 nametables from VRAM
 *        Useful for debugging
 */
void io_drawDebugNametable();
#endif
