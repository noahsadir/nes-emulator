/**
 * @file logging.h
 * @author Noah Sadir (development.noahsadir@gmail.com)
 * @brief Handles debugging features of emulator
 * @version 1.0
 * @date 2022-12-21
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

#ifndef LOGGING_H
#define LOGGING_H

#include "globalflags.h"

#if (LOGGING)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

/**
 * @brief Initialize logging functionality
 * 
 */
void logging_init();

/**
 * @brief Kill loggins functionality
 * 
 */
void logging_kill();

/**
 * @brief Save NES trace to file
 * 
 * @param trace the trace
 * @param cpuCycles the number of CPU Cycles
 */
void logging_saveNESTrace(char* trace, uint32_t cpuCycles);

/**
 * @brief Save disassembeled line to file
 * 
 * @param line the assembly instruction string
 */
void logging_saveDisassembly(char* line);

/**
 * @brief Convert bytecode to CPU trace
 * 
 * @param b the pointer to the bytecode object
 * @param line the output line
 */
void logging_bytecodeToTrace(CPURegisters reg, Bytecode* b, char* line, void(*memWrite)(uint16_t, uint8_t), uint8_t(*memRead)(uint16_t));

/**
 * @brief Convert bytecode to an assembly line
 * 
 * @param b the pointer to the bytecode object
 * @param line the output line
 */
void logging_bytecodeToAssembly(uint32_t pointer, Bytecode* b, char line[128], uint8_t flags);

/**
 * @brief Convert number to hex string
 * 
 * @param val the number to convert
 * @param size the size of the int (bits)
 * @param output the output string
 */
void logging_intToHexString(uint32_t val, uint8_t size, char* output);

#endif
