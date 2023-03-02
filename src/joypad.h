/**
 * @file joypad.h
 * @author Noah Sadir (development.noahsadir@gmail.com)
 * @brief Joypad for NES Emulator
 * @version 1.0
 * @date 2022-07-02
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

#ifndef JOYPAD_H
#define JOYPAD_H

#include "globalflags.h"
#include <stdint.h>

typedef enum {
    JP_RIGHT   = 0b10000000,
    JP_LEFT    = 0b01000000,
    JP_DOWN    = 0b00100000,
    JP_UP      = 0b00010000,
    JP_START   = 0b00001000,
    JP_SELECT  = 0b00000100,
    JP_BTN_B   = 0b00000010,
    JP_BTN_A   = 0b00000001,
    JP_NULL    = 0b00000000
} JoypadButton;

/**
 * @brief Read button value from joypad
 * 
 * @return true if button is set, false if unset
 */
bool joypad_read();

/**
 * @brief Set the value of the strobe flag
 * 
 * @param val the value of the strobe flag (0 = off, 1+ = on)
 */
void joypad_write(uint8_t val);

/**
 * @brief Set a button on the joypad
 * 
 * @param button the button to set
 */
void joypad_setButton(JoypadButton button);

/**
 * @brief Unset a button on the joypad
 * 
 * @param button the button to unset
 */
void joypad_unsetButton(JoypadButton button);

#endif