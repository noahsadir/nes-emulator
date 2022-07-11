/**
 * @file exceptions.h
 * @author Noah Sadir (development.noahsadir@gmail.com)
 * @brief Display feedback for debugging and troubleshooting.
 * @version 1.0
 * @date 2022-07-03
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

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "bus.h"

void exc_traceInit();

void exc_panic(char* message);

void exc_panic_invalidIO(uint16_t address);

void exc_panic_invalidPPUIO(uint16_t address);

void exc_panic_illegalInstruction(uint8_t opcode);

void exc_trace(int paramCount, char* redirectString, uint16_t address,
    uint16_t pc, char* instructionCode, uint8_t opcode, uint8_t firstParam,
    uint8_t secondParam, char* parameterString, uint16_t offsetAddress,
    uint8_t reg_a, uint8_t reg_status, uint8_t reg_x, uint8_t reg_y,
    uint8_t stackPointer, uint64_t cpuCycles, uint64_t ppuCycles,
    uint64_t ppuBlanks);

void exc_message(char* message);

void exc_panic_stackOverflow();

void exc_panic_stackUnderflow();

#endif