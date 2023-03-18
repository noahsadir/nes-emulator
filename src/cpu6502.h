/**
 * @file cpu6502.h
 * @author Noah Sadir (development.noahsadir@gmail.com)
 * @brief Portable implementation of MOS 6502
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

#ifndef CPU6502_H
#define CPU6502_H

#include "globalflags.h"
#include "logging.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define CPU_DEBUG FALSE

/**
 * @brief Initialize the CPU
 * 
 * @param w the pointer to the write function
 * @param r the pointer to the read function
 * @param mode the emulation mode
 */
void cpu6502_init(void(*w)(uint16_t, uint8_t), uint8_t(*r)(uint16_t), CPUEmulationMode mode);

/**
 * @brief Execute a CPU instruction
 * 
 * @param traceStr the trace string (or NULL if tracing disabled)
 * @param c the callback function which contains one argument:
 *          - (uint8_t) clocks : the number of clocks elapsed since last step
 */
void cpu6502_step(char* traceStr, void(*c)(uint8_t));

/**
 * @brief Trigger NMI
 */
void cpu6502_nmi();

/**
 * @brief Push a value to the stack
 * 
 * @param val the value to push
 */
static force_inline void cpu6502_stackPush(uint8_t val);

/**
 * @brief Pull a value from the stack
 * 
 * @return uint8_t the value pulled
 */
static force_inline uint8_t cpu6502_stackPull();

/**
 * @brief Set a CPU flag
 * 
 * @param flag the CPU flag
 * @param enabled the value of the flag
 */
static force_inline void cpu6502_setFlag(CPUStatusFlag flag, bool enabled);

/**
 * @brief Perform a branch
 * 
 * @param desiredResult the branch condition
 * @param flag the flag to check condition for
 */
static force_inline bool cpu6502_shouldBranch(bool desiredResult, CPUStatusFlag flag);

/**
 * @brief Execute a bytecode instruction
 * 
 * @param b the bytecode pointer
 * @return uint8_t the number of clocks elapsed
 */
static force_inline uint8_t cpu6502_execute(Bytecode* b);

/**
 * @brief Set the clock mode of the CPU
 * 
 * @param mode the clock mode
 */
void cpu6502_setClockMode(CPUClockMode mode);

/**
 * @brief Get the clock mode of the CPU
 * 
 * @return CPUClockMode the clock mode
 */
CPUClockMode cpu6502_getClockMode();

/**
 * @brief Read 16-bit value at address
 * 
 * @param addr the address
 * @return uint16_t the return value
 */
static force_inline uint16_t cpu6502_read16(uint16_t addr);

/**
 * @brief Disassembly program code
 * 
 * @param prgData the program binary
 * @param prgSize the size of the program (in bytes)
 * @param c the callback which runs for each line generated
 */
void cpu6502_dasm(uint8_t* prgData, uint32_t prgSize, void(*c)(char c[128]), uint8_t flags);

/**
 * @brief Recompile machine code into bytecode
 * 
 * @param prgData the program binary
 * @param prgSize the program size (in bytes)
 * @param bprog the pointer to the bytecode program object
 */
void cpu6502_loadBytecodeProgram(uint8_t* prgData, uint32_t prgSize);

/**
 * @brief Determine the mnemonic, addressing mode, and size of
 *        an instruction given its opcode.
 *
 * @param opcode the opcode
 * @param b the pointer to the bytecode object
 */
static force_inline void cpu6502_parseOpcode(uint8_t opcode, Bytecode* b);

/**
 * @brief Get error number of CPU
 * 
 * ERROR CODES
 * -----------
 * 0 - None
 * 1 - Illegal Instruction
 * @return uint8_t the error number
 */
uint8_t cpu6502_getErrno();

#endif
