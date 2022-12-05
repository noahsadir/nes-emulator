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
#include "cpu.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#if (DEBUG_MODE)

/**
 * @brief Initialize CPU trace functionality
 */
void exc_traceInit();

/**
 * @brief Generate generic panic message
 * 
 * @param message the panic message
 */
void exc_panic(char* message);

/**
 * @brief Generate a panic for invalid I/O operations to
 *        the CPU address space
 * 
 * @param address the invalid address
 */
void exc_panic_invalidIO(uint16_t address);

/**
 * @brief Generate a panic for invalid I/O operations to
 *        the PPU address space
 * 
 * @param address the invalid address
 */
void exc_panic_invalidPPUIO(uint16_t address);

/**
 * @brief Generate a panic for invalid CPU instructions
 * 
 * @param address the invalid opcode
 */
void exc_panic_illegalInstruction(uint8_t opcode);

/**
 * @brief Generate a CPU trace
 *        Obviously this would be much easier to implement in the CPU class
 *        itself but I've added it here so that the CPU doesn't depend on file I/O
 * 
 * Too many parameters, most are self-explanatory
 */
void exc_trace(Trace* trace);

void exc_message(char* message);

void exc_panic_stackOverflow();

void exc_panic_stackUnderflow();

#endif

#endif