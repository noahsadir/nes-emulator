/**
 * @file cpu.h
 * @author Noah Sadir (development.noahsadir@gmail.com)
 * @brief An (almost) complete emulation of the MOS 6502 CPU
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

#ifndef CPU_H
#define CPU_H

#include "bus.h"

#include <stdint.h>
#include <stdbool.h>

enum CPUStatusFlag {
    CPUSTAT_CARRY       = 0b00000001,
    CPUSTAT_ZERO        = 0b00000010,
    CPUSTAT_NO_INTRPT   = 0b00000100,
    CPUSTAT_DECIMAL     = 0b00001000,
    CPUSTAT_BREAK       = 0b00010000,
    CPUSTAT_BREAK2      = 0b00100000,
    CPUSTAT_OVERFLOW    = 0b01000000,
    CPUSTAT_NEGATIVE    = 0b10000000
};

enum AddressingMode {
    ADM_ACCUMULATOR,
    ADM_IMPLIED,
    ADM_IMMEDIATE,
    ADM_ABSOLUTE,
    ADM_ZEROPAGE,
    ADM_RELATIVE,
    ADM_ABS_INDIRECT,
    ADM_ABS_X,
    ADM_ABS_Y,
    ADM_ZP_X,
    ADM_ZP_Y,
    ADM_ZP_INDIRECT_X,
    ADM_ZP_INDIRECT_Y
};

enum Instruction {
    LDA, LDX, LDY, STA, STX, STY, ADC, SBC, INC, INX, INY, DEC, DEX, DEY, ASL,
    LSR, ROL, ROR, AND, ORA, EOR, CMP, CPX, CPY, BIT, BCC, BCS, BNE, BEQ, BPL,
    BMI, BVC, BVS, TAX, TXA, TAY, TYA, TSX, TXS, PLA, PHA, PHP, PLP, JMP, JSR,
    RTS, RTI, CLC, SEC, CLD, SED, CLI, SEI, CLV, BRK, NOP,
    IL_ALR, IL_ANC, IL_ANE, IL_ARR, IL_DCP, IL_ISC, IL_LAS, IL_LAX, IL_LXA,
    IL_RLA, IL_RRA, IL_SAX, IL_SBX, IL_SHA, IL_SHX, IL_SHY, IL_SLO, IL_SRE,
    IL_TAS, IL_SBC, IL_JAM, IL_NOP
};

/* PUBLIC METHODS - INTENDED FOR EXTERNAL USE */

/**
 * @brief Start the CPU
 */
void cpu_init();

/**
 * @brief Get the number of cycles performed by the CPU
 * 
 * @return uint32_t the number of cycles
 */
uint32_t cpu_getCycles();

/**
 * @brief CPU recieved an NMI
 */
void cpu_vblankNMI();

/**
 * @brief aaaaaaaaaaaaaaah
 */
void cpu_panic();

/**
 * @brief Perform reset interrupt
 */
void cpu_reset();

/* PRIVATE METHODS - NOT INTENDED FOR EXTERNAL USE */

/**
 * @brief Execute an instruction
 * 
 * @return uint8_t the number of cycles elapsed
 */
uint8_t cpu_execute();

/*
 * No need for documentation; you know what these do.
 * Reminder: https://www.masswerk.at/6502/6502_instruction_set.html
 */ 
uint8_t cpu_lda(uint16_t address);
uint8_t cpu_ldx(uint16_t address);
uint8_t cpu_ldy(uint16_t address);
uint8_t cpu_sta(uint16_t address);
uint8_t cpu_stx(uint16_t address);
uint8_t cpu_sty(uint16_t address);
uint8_t cpu_adc(uint16_t address);
uint8_t cpu_sbc(uint16_t address);
uint8_t cpu_inc(uint16_t address);
uint8_t cpu_inx();
uint8_t cpu_iny();
uint8_t cpu_dec(uint16_t address);
uint8_t cpu_dex();
uint8_t cpu_dey();
uint8_t cpu_asl(uint16_t address);
uint8_t cpu_lsr(uint16_t address);
uint8_t cpu_rol(uint16_t address);
uint8_t cpu_ror(uint16_t address);
uint8_t cpu_and(uint16_t address);
uint8_t cpu_ora(uint16_t address);
uint8_t cpu_eor(uint16_t address);
uint8_t cpu_cmp(uint16_t address);
uint8_t cpu_cpx(uint16_t address);
uint8_t cpu_cpy(uint16_t address);
uint8_t cpu_bit(uint16_t address);
uint8_t cpu_bcc();
uint8_t cpu_bcs();
uint8_t cpu_bne();
uint8_t cpu_beq();
uint8_t cpu_bpl();
uint8_t cpu_bmi();
uint8_t cpu_bvc();
uint8_t cpu_bvs();
uint8_t cpu_tax();
uint8_t cpu_txa();
uint8_t cpu_tay();
uint8_t cpu_tya();
uint8_t cpu_tsx();
uint8_t cpu_txs();
uint8_t cpu_pla();
uint8_t cpu_pha();
uint8_t cpu_php();
uint8_t cpu_plp();
uint8_t cpu_jmp(uint16_t address);
uint8_t cpu_jsr(uint16_t address);
uint8_t cpu_rts();
uint8_t cpu_rti();
uint8_t cpu_clc();
uint8_t cpu_sec();
uint8_t cpu_cld();
uint8_t cpu_sed();
uint8_t cpu_cli();
uint8_t cpu_sei();
uint8_t cpu_clv();
uint8_t cpu_brk();
uint8_t cpu_nop();
uint8_t cpu_illegal_alr(uint16_t address);
uint8_t cpu_illegal_anc(uint16_t address);
uint8_t cpu_illegal_ane(uint16_t address);
uint8_t cpu_illegal_arr(uint16_t address);
uint8_t cpu_illegal_dcp(uint16_t address);
uint8_t cpu_illegal_isc(uint16_t address);
uint8_t cpu_illegal_las(uint16_t address);
uint8_t cpu_illegal_lax(uint16_t address);
uint8_t cpu_illegal_lxa(uint16_t address);
uint8_t cpu_illegal_rla(uint16_t address);
uint8_t cpu_illegal_rra(uint16_t address);
uint8_t cpu_illegal_sax(uint16_t address);
uint8_t cpu_illegal_sbx(uint16_t address);
uint8_t cpu_illegal_sha(uint16_t address);
uint8_t cpu_illegal_shx(uint16_t address);
uint8_t cpu_illegal_shy(uint16_t address);
uint8_t cpu_illegal_slo(uint16_t address);
uint8_t cpu_illegal_sre(uint16_t address);
uint8_t cpu_illegal_tas(uint16_t address);
uint8_t cpu_illegal_sbc(uint16_t address);
uint8_t cpu_illegal_nop(uint16_t address);
uint8_t cpu_illegal_jam();

/**
 * @brief Helper method for branch instructions
 * 
 * @param desiredResult the value to branch on
 * @param flag the flag to check
 */
void cpu_branchHelper(bool desiredResult, enum CPUStatusFlag flagVal);

/**
 * @brief Get the instruction string
 * 
 * @param instruction the CPU instruction
 * @return char* the resulting string
 */
char* cpu_getInstructionString(enum Instruction instruction);

/**
 * @brief Trace the CPU instruction
 * 
 * @param instruction the instruction type
 * @param mode the addressing mode
 * @param address the derived address
 */
void cpu_trace(enum Instruction instruction, enum AddressingMode mode, uint16_t address);

/**
 * @brief Get address using specified mode
 * 
 * @param mode the mode to fetch
 * @return uint16_t the resulting address
 */
uint16_t cpu_fetchAddress(enum AddressingMode mode);

/**
 * @brief Set a status flag of CPU
 * 
 * @param flag the flag to set
 * @param val the value of the flag
 */
void cpu_setFlag(enum CPUStatusFlag flag, bool enable);

/**
 * @brief Get a status flag of CPU
 * 
 * @param flag the flag to get
 * @return the value of the flag
 */
bool cpu_getFlag(enum CPUStatusFlag flag);

/**
 * @brief Push value onto stack
 * 
 * @param val the value to push
 */
void cpu_stackPush(uint8_t val);

/**
 * @brief Pull value from stack
 * 
 * @return uint8_t the value pulled
 */
uint8_t cpu_stackPull();

/**
 * @brief Push 16-bit value onto stack
 * 
 * @param val the value to push
 */
void cpu_stackPush16(uint16_t val);

/**
 * @brief Pull value from stack
 * 
 * @return uint16_t the value pulled
 */
uint16_t cpu_stackPull16();

#endif