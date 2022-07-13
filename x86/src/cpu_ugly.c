/******************************
 *          WARNING
 ******************************
 * The following code is optimized to sacrifice readability and OOP principles
 * in exchange for raw performance. Viewer discretion is advised.
 */

/**
 * @file cpu.c
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

#include "cpu.h"

uint8_t reg_status = 0x24;
uint8_t reg_accumulator = 0x00;
uint8_t reg_x = 0x00;
uint8_t reg_y = 0x00;
uint16_t reg_pc = 0x0000;
uint8_t stackPointer = 0xFD;

uint64_t cycles = 4;

bool accumulatorAddrMode = false;
bool didPanic = false;
bool generateTrace = false;
bool diagnosticMode = false;

uint8_t* cpu_cpuRAM;
uint8_t* cpu_vidRAM;
uint8_t* cpu_paletteRAM;

void cpu_init() {
    cpu_reset();
    
    enum Instruction instruction = NOP;
    enum AddressingMode mode = ADM_IMPLIED;
    uint16_t address = 0x0000;
    uint8_t opcode = 0x00;
    uint64_t oldCycles = 0;
    bus_startTimeMonitor();
    while (instruction != BRK && reg_pc != 0xFFFD && !didPanic && cycles <= 1000000000) {
        oldCycles = cycles;
        opcode = bus_readCPU(reg_pc);
        reg_pc += 1;
        
        switch (opcode) {
            // LDA
            case 0xAD:
            {
                cycles += 4;
                instruction = LDA;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0xBD:
            {
                cycles += 4; 
                instruction = LDA;
                mode = ADM_ABS_X;
                break;
            }
            case 0xB9:
            {
                cycles += 4; 
                instruction = LDA;
                mode = ADM_ABS_Y;
                break;
            }
            case 0xA9:
            {
                cycles += 2; 
                instruction = LDA;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0xA5:
            {
                cycles += 3; 
                instruction = LDA;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0xA1:
            {
                cycles += 6; 
                instruction = LDA;
                mode = ADM_ZP_INDIRECT_X;
                break;
            }
            case 0xB5:
            {
                cycles += 4; 
                instruction = LDA;
                mode = ADM_ZP_X;
                break;
            }
            case 0xB1:
            {
                cycles += 5; 
                instruction = LDA;
                mode = ADM_ZP_INDIRECT_Y;
                break;
            }
            // LDX
            case 0xAE:
            {
                cycles += 4; 
                instruction = LDX;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0xBE:
            {
                cycles += 4; 
                instruction = LDX;
                mode = ADM_ABS_Y;
                break;
            }
            case 0xA2:
            {
                cycles += 2; 
                instruction = LDX;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0xA6:
            {
                cycles += 3; 
                instruction = LDX;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0xB6:
            {
                cycles += 4; 
                instruction = LDX;
                mode = ADM_ZP_Y;
                break;
            }
            // LDY
            case 0xAC:
            {
                cycles += 4; 
                instruction = LDY;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0xBC:
            {
                cycles += 4; 
                instruction = LDY;
                mode = ADM_ABS_X;
                break;
            }
            case 0xA0:
            {
                cycles += 2; 
                instruction = LDY;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0xA4:
            {
                cycles += 3; 
                instruction = LDY;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0xB4:
            {
                cycles += 4; 
                instruction = LDY;
                mode = ADM_ZP_X;
                break;
            }
            // STA
            case 0x8D:
            {
                cycles += 4; 
                instruction = STA;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x9D:
            {
                cycles += 5; 
                instruction = STA;
                mode = ADM_ABS_X;
                break;
            }
            case 0x99:
            {
                cycles += 5; 
                instruction = STA;
                mode = ADM_ABS_Y;
                break;
            }
            case 0x85:
            {
                cycles += 3; 
                instruction = STA;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x81:
            {
                cycles += 6; 
                instruction = STA;
                mode = ADM_ZP_INDIRECT_X;
                break;
            }
            case 0x95:
            {
                cycles += 4; 
                instruction = STA;
                mode = ADM_ZP_X;
                break;
            }
            case 0x91:
            {
                cycles += 6; 
                instruction = STA;
                mode = ADM_ZP_INDIRECT_Y;
                break;
            }
            // STX
            case 0x8E:
            {
                cycles += 4; 
                instruction = STX;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x86:
            {
                cycles += 3; 
                instruction = STX;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x96:
            {
                cycles += 4; 
                instruction = STX;
                mode = ADM_ZP_Y;
                break;
            }
            // STY
            case 0x8C:
            {
                cycles += 4; 
                instruction = STY;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x84:
            {
                cycles += 3; 
                instruction = STY;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x94:
            {
                cycles += 4; 
                instruction = STY;
                mode = ADM_ZP_X;
                break;
            }
            // ADC
            case 0x6D:
            {
                cycles += 4; 
                instruction = ADC;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x7D:
            {
                cycles += 4; 
                instruction = ADC;
                mode = ADM_ABS_X;
                break;
            }
            case 0x79:
            {
                cycles += 4; 
                instruction = ADC;
                mode = ADM_ABS_Y;
                break;
            }
            case 0x69:
            {
                cycles += 2; 
                instruction = ADC;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0x65:
            {
                cycles += 3; 
                instruction = ADC;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x61:
            {
                cycles += 6; 
                instruction = ADC;
                mode = ADM_ZP_INDIRECT_X;
                break;
            }
            case 0x75:
            {
                cycles += 4; 
                instruction = ADC;
                mode = ADM_ZP_X;
                break;
            }
            case 0x71:
            {
                cycles += 5; 
                instruction = ADC;
                mode = ADM_ZP_INDIRECT_Y;
                break;
            }
            // SBC
            case 0xED:
            {
                cycles += 4; 
                instruction = SBC;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0xFD:
            {
                cycles += 4; 
                instruction = SBC;
                mode = ADM_ABS_X;
                break;
            }
            case 0xF9:
            {
                cycles += 4; 
                instruction = SBC;
                mode = ADM_ABS_Y;
                break;
            }
            case 0xE9:
            {
                cycles += 2; 
                instruction = SBC;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0xE5:
            {
                cycles += 3; 
                instruction = SBC;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0xE1:
            {
                cycles += 6; 
                instruction = SBC;
                mode = ADM_ZP_INDIRECT_X;
                break;
            }
            case 0xF5:
            {
                cycles += 4; 
                instruction = SBC;
                mode = ADM_ZP_X;
                break;
            }
            case 0xF1:
            {
                cycles += 5; 
                instruction = SBC;
                mode = ADM_ZP_INDIRECT_Y;
                break;
            }
            // INC
            case 0xEE:
            {
                cycles += 6; 
                instruction = INC;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0xFE:
            {
                cycles += 7; 
                instruction = INC;
                mode = ADM_ABS_X;
                break;
            }
            case 0xE6:
            {
                cycles += 5; 
                instruction = INC;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0xF6:
            {
                cycles += 6; 
                instruction = INC;
                mode = ADM_ZP_X;
                break;
            }
            // INX
            case 0xE8:
            {
                cycles += 2; 
                instruction = INX;
                mode = ADM_IMPLIED;
                break;
            }
            // INY
            case 0xC8:
            {
                cycles += 2; 
                instruction = INY;
                mode = ADM_IMPLIED;
                break;
            }
            // DEC
            case 0xCE:
            {
                cycles += 6; 
                instruction = DEC;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0xDE:
            {
                cycles += 7; 
                instruction = DEC;
                mode = ADM_ABS_X;
                break;
            }
            case 0xC6:
            {
                cycles += 5; 
                instruction = DEC;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0xD6:
            {
                cycles += 6; 
                instruction = DEC;
                mode = ADM_ZP_X;
                break;
            }
            // DEX
            case 0xCA:
            {
                cycles += 2; 
                instruction = DEX;
                mode = ADM_IMPLIED;
                break;
            }
            // DEY
            case 0x88:
            {
                cycles += 2; 
                instruction = DEY;
                mode = ADM_IMPLIED;
                break;
            }
            // ASL
            case 0x0E:
            {
                cycles += 6; 
                instruction = ASL;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x1E:
            {
                cycles += 7; 
                instruction = ASL;
                mode = ADM_ABS_X;
                break;
            }
            case 0x0A:
            {
                cycles += 2; 
                instruction = ASL;
                mode = ADM_ACCUMULATOR;
                break;
            }
            case 0x06:
            {
                cycles += 5; 
                instruction = ASL;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x16:
            {
                cycles += 6; 
                instruction = ASL;
                mode = ADM_ZP_X;
                break;
            }
            // LSR
            case 0x4E:
            {
                cycles += 6; 
                instruction = LSR;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x5E:
            {
                cycles += 7; 
                instruction = LSR;
                mode = ADM_ABS_X;
                break;
            }
            case 0x4A:
            {
                cycles += 2; 
                instruction = LSR;
                mode = ADM_ACCUMULATOR;
                break;
            }
            case 0x46:
            {
                cycles += 5; 
                instruction = LSR;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x56:
            {
                cycles += 6; 
                instruction = LSR;
                mode = ADM_ZP_X;
                break;
            }
            // ROL
            case 0x2E:
            {
                cycles += 6; 
                instruction = ROL;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x3E:
            {
                cycles += 7; 
                instruction = ROL;
                mode = ADM_ABS_X;
                break;
            }
            case 0x2A:
            {
                cycles += 2; 
                instruction = ROL;
                mode = ADM_ACCUMULATOR;
                break;
            }
            case 0x26:
            {
                cycles += 5; 
                instruction = ROL;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x36:
            {
                cycles += 6; 
                instruction = ROL;
                mode = ADM_ZP_X;
                break;
            }
            // ROR
            case 0x6E:
            {
                cycles += 6; 
                instruction = ROR;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x7E:
            {
                cycles += 7; 
                instruction = ROR;
                mode = ADM_ABS_X;
                break;
            }
            case 0x6A:
            {
                cycles += 2; 
                instruction = ROR;
                mode = ADM_ACCUMULATOR;
                break;
            }
            case 0x66:
            {
                cycles += 5; 
                instruction = ROR;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x76:
            {
                cycles += 6; 
                instruction = ROR;
                mode = ADM_ZP_X;
                break;
            }
            // AND
            case 0x2D:
            {
                cycles += 4; 
                instruction = AND;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x3D:
            {
                cycles += 4; 
                instruction = AND;
                mode = ADM_ABS_X;
                break;
            }
            case 0x39:
            {
                cycles += 4; 
                instruction = AND;
                mode = ADM_ABS_Y;
                break;
            }
            case 0x29:
            {
                cycles += 2; 
                instruction = AND;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0x25:
            {
                cycles += 3; 
                instruction = AND;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x21:
            {
                cycles += 6; 
                instruction = AND;
                mode = ADM_ZP_INDIRECT_X;
                break;
            }
            case 0x35:
            {
                cycles += 4; 
                instruction = AND;
                mode = ADM_ZP_X;
                break;
            }
            case 0x31:
            {
                cycles += 5; 
                instruction = AND;
                mode = ADM_ZP_INDIRECT_Y;
                break;
            }
            // ORA
            case 0x0D:
            {
                cycles += 4; 
                instruction = ORA;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x1D:
            {
                cycles += 4; 
                instruction = ORA;
                mode = ADM_ABS_X;
                break;
            }
            case 0x19:
            {
                cycles += 4; 
                instruction = ORA;
                mode = ADM_ABS_Y;
                break;
            }
            case 0x09:
            {
                cycles += 2; 
                instruction = ORA;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0x05:
            {
                cycles += 3; 
                instruction = ORA;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x01:
            {
                cycles += 6; 
                instruction = ORA;
                mode = ADM_ZP_INDIRECT_X;
                break;
            }
            case 0x15:
            {
                cycles += 4; 
                instruction = ORA;
                mode = ADM_ZP_X;
                break;
            }
            case 0x11:
            {
                cycles += 5; 
                instruction = ORA;
                mode = ADM_ZP_INDIRECT_Y;
                break;
            }
            // EOR
            case 0x4D:
            {
                cycles += 4; 
                instruction = EOR;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x5D:
            {
                cycles += 4; 
                instruction = EOR;
                mode = ADM_ABS_X;
                break;
            }
            case 0x59:
            {
                cycles += 4; 
                instruction = EOR;
                mode = ADM_ABS_Y;
                break;
            }
            case 0x49:
            {
                cycles += 2; 
                instruction = EOR;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0x45:
            {
                cycles += 3; 
                instruction = EOR;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x41:
            {
                cycles += 6; 
                instruction = EOR;
                mode = ADM_ZP_INDIRECT_X;
                break;
            }
            case 0x55:
            {
                cycles += 4; 
                instruction = EOR;
                mode = ADM_ZP_X;
                break;
            }
            case 0x51:
            {
                cycles += 5; 
                instruction = EOR;
                mode = ADM_ZP_INDIRECT_Y;
                break;
            }
            // CMP
            case 0xCD:
            {
                cycles += 4; 
                instruction = CMP;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0xDD:
            {
                cycles += 4; 
                instruction = CMP;
                mode = ADM_ABS_X;
                break;
            }
            case 0xD9:
            {
                cycles += 4; 
                instruction = CMP;
                mode = ADM_ABS_Y;
                break;
            }
            case 0xC9:
            {
                cycles += 2; 
                instruction = CMP;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0xC5:
            {
                cycles += 3; 
                instruction = CMP;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0xC1:
            {
                cycles += 6; 
                instruction = CMP;
                mode = ADM_ZP_INDIRECT_X;
                break;
            }
            case 0xD5:
            {
                cycles += 4; 
                instruction = CMP;
                mode = ADM_ZP_X;
                break;
            }
            case 0xD1:
            {
                cycles += 5; 
                instruction = CMP;
                mode = ADM_ZP_INDIRECT_Y;
                break;
            }
            // CPX
            case 0xEC:
            {
                cycles += 4; 
                instruction = CPX;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0xE0:
            {
                cycles += 2; 
                instruction = CPX;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0xE4:
            {
                cycles += 3; 
                instruction = CPX;
                mode = ADM_ZEROPAGE;
                break;
            }
            // CPY
            case 0xCC:
            {
                cycles += 4; 
                instruction = CPY;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0xC0:
            {
                cycles += 2; 
                instruction = CPY;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0xC4:
            {
                cycles += 3; 
                instruction = CPY;
                mode = ADM_ZEROPAGE;
                break;
            }
            // BIT
            case 0x2C:
            {
                cycles += 4; 
                instruction = BIT;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x89:
            {
                cycles += 2; 
                instruction = BIT;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0x24:
            {
                cycles += 3; 
                instruction = BIT;
                mode = ADM_ZEROPAGE;
                break;
            }
            // BCC
            case 0x90:
            {
                cycles += 2; 
                instruction = BCC;
                mode = ADM_RELATIVE;
                break;
            }
            // BCS
            case 0xB0:
            {
                cycles += 2; 
                instruction = BCS;
                mode = ADM_RELATIVE;
                break;
            }
            // BNE
            case 0xD0:
            {
                cycles += 2; 
                instruction = BNE;
                mode = ADM_RELATIVE;
                break;
            }
            // BEQ
            case 0xF0:
            {
                cycles += 2; 
                instruction = BEQ;
                mode = ADM_RELATIVE;
                break;
            }
            // BPL
            case 0x10:
            {
                cycles += 2; 
                instruction = BPL;
                mode = ADM_RELATIVE;
                break;
            }
            // BMI
            case 0x30:
            {
                cycles += 2; 
                instruction = BMI;
                mode = ADM_RELATIVE;
                break;
            }
            // BVC
            case 0x50:
            {
                cycles += 2; 
                instruction = BVC;
                mode = ADM_RELATIVE;
                break;
            }
            // BVS
            case 0x70:
            {
                cycles += 2; 
                instruction = BVS;
                mode = ADM_RELATIVE;
                break;
            }
            // TAX
            case 0xAA:
            {
                cycles += 2; 
                instruction = TAX;
                mode = ADM_IMPLIED;
                break;
            }
            // TXA
            case 0x8A:
            {
                cycles += 2; 
                instruction = TXA;
                mode = ADM_IMPLIED;
                break;
            }
            // TAY
            case 0xA8:
            {
                cycles += 2; 
                instruction = TAY;
                mode = ADM_IMPLIED;
                break;
            }
            // TYA
            case 0x98:
            {
                cycles += 2; 
                instruction = TYA;
                mode = ADM_IMPLIED;
                break;
            }
            // TSX
            case 0xBA:
            {
                cycles += 2; 
                instruction = TSX;
                mode = ADM_IMPLIED;
                break;
            }
            // TXS
            case 0x9A:
            {
                cycles += 2; 
                instruction = TXS;
                mode = ADM_IMPLIED;
                break;
            }
            // PHA
            case 0x48:
            {
                cycles += 3; 
                instruction = PHA;
                mode = ADM_IMPLIED;
                break;
            }
            // PLA
            case 0x68:
            {
                cycles += 4; 
                instruction = PLA;
                mode = ADM_IMPLIED;
                break;
            }
            // PHP
            case 0x08:
            {
                cycles += 3; 
                instruction = PHP;
                mode = ADM_IMPLIED;
                break;
            }
            // PLP
            case 0x28:
            {
                cycles += 4; 
                instruction = PLP;
                mode = ADM_IMPLIED;
                break;
            }
            // JMP
            case 0x4C:
            {
                cycles += 3; 
                instruction = JMP;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x6C:
            {
                cycles += 5; 
                instruction = JMP;
                mode = ADM_ABS_INDIRECT;
                break;
            }
            // JSR
            case 0x20:
            {
                cycles += 6; 
                instruction = JSR;
                mode = ADM_ABSOLUTE;
                break;
            }
            // RTS
            case 0x60:
            {
                cycles += 6; 
                instruction = RTS;
                mode = ADM_IMPLIED;
                break;
            }
            // RTI
            case 0x40:
            {
                cycles += 6; 
                instruction = RTI;
                mode = ADM_IMPLIED;
                break;
            }
            // CLC
            case 0x18:
            {
                cycles += 2; 
                instruction = CLC;
                mode = ADM_IMPLIED;
                break;
            }
            // SEC
            case 0x38:
            {
                cycles += 2; 
                instruction = SEC;
                mode = ADM_IMPLIED;
                break;
            }
            // CLD
            case 0xD8:
            {
                cycles += 2; 
                instruction = CLD;
                mode = ADM_IMPLIED;
                break;
            }
            // SED
            case 0xF8:
            {
                cycles += 2; 
                instruction = SED;
                mode = ADM_IMPLIED;
                break;
            }
            // CLI
            case 0x58:
            {
                cycles += 2; 
                instruction = CLI;
                mode = ADM_IMPLIED;
                break;
            }
            // SEI
            case 0x78:
            {
                cycles += 2; 
                instruction = SEI;
                mode = ADM_IMPLIED;
                break;
            }
            // CLV
            case 0xB8:
            {
                cycles += 2; 
                instruction = CLV;
                mode = ADM_IMPLIED;
                break;
            }
            // BRK
            case 0x00:
            {
                cycles += 7; 
                instruction = BRK;
                mode = ADM_IMPLIED;
                break;
            }
            // NOP
            case 0xEA:
            {
                cycles += 2; 
                instruction = NOP;
                mode = ADM_IMPLIED;
                break;
            }
            // (ILLEGAL) ALR
            case 0x4B:
            {
                cycles += 2; 
                instruction = IL_ALR;
                mode = ADM_IMMEDIATE;
                break;
            }
            // (ILLEGAL) ANC
            case 0x0B:
            {
                cycles += 2; 
                instruction = IL_ANC;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0x2B:
            {
                cycles += 2; 
                instruction = IL_ANC;
                mode = ADM_IMMEDIATE;
                break;
            }
            // (ILLEGAL) ANE
            case 0x8B:
            {
                cycles += 2; 
                instruction = IL_ANE;
                mode = ADM_IMMEDIATE;
                break;
            }
            // (ILLEGAL) ARR
            case 0x6B:
            {
                cycles += 2; 
                instruction = IL_ARR;
                mode = ADM_IMMEDIATE;
                break;
            }
            // (ILLEGAL) DCP
            case 0xC7:
            {
                cycles += 5; 
                instruction = IL_DCP;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0xD7:
            {
                cycles += 6; 
                instruction = IL_DCP;
                mode = ADM_ZP_X;
                break;
            }
            case 0xCF:
            {
                cycles += 6; 
                instruction = IL_DCP;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0xDF:
            {
                cycles += 7; 
                instruction = IL_DCP;
                mode = ADM_ABS_X;
                break;
            }
            case 0xDB:
            {
                cycles += 7; 
                instruction = IL_DCP;
                mode = ADM_ABS_Y;
                break;
            }
            case 0xC3:
            {
                cycles += 8; 
                instruction = IL_DCP;
                mode = ADM_ZP_INDIRECT_X;
                break;
            }
            case 0xD3:
            {
                cycles += 8; 
                instruction = IL_DCP;
                mode = ADM_ZP_INDIRECT_Y;
                break;
            }
            // (ILLEGAL) ISC
            case 0xE7:
            {
                cycles += 5; 
                instruction = IL_ISC;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0xF7:
            {
                cycles += 6; 
                instruction = IL_ISC;
                mode = ADM_ZP_X;
                break;
            }
            case 0xEF:
            {
                cycles += 6; 
                instruction = IL_ISC;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0xFF:
            {
                cycles += 7; 
                instruction = IL_ISC;
                mode = ADM_ABS_X;
                break;
            }
            case 0xFB:
            {
                cycles += 7; 
                instruction = IL_ISC;
                mode = ADM_ABS_Y;
                break;
            }
            case 0xE3:
            {
                cycles += 8; 
                instruction = IL_ISC;
                mode = ADM_ZP_INDIRECT_X;
                break;
            }
            case 0xF3:
            {
                cycles += 4; 
                instruction = IL_ISC;
                mode = ADM_ZP_INDIRECT_Y;
                break;
            }
            // (ILLEGAL) LAS
            case 0xBB:
            {
                cycles += 4; 
                instruction = IL_LAS;
                mode = ADM_ABS_Y;
                break;
            }
            // (ILLEGAL) LAX
            case 0xA7:
            {
                cycles += 3; 
                instruction = IL_LAX;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0xB7:
            {
                cycles += 4; 
                instruction = IL_LAX;
                mode = ADM_ZP_Y;
                break;
            }
            case 0xAF:
            {
                cycles += 4; 
                instruction = IL_LAX;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0xBF:
            {
                cycles += 4; 
                instruction = IL_LAX;
                mode = ADM_ABS_Y;
                break;
            }
            case 0xA3:
            {
                cycles += 6; 
                instruction = IL_LAX;
                mode = ADM_ZP_INDIRECT_X;
                break;
            }
            case 0xB3:
            {
                cycles += 5; 
                instruction = IL_LAX;
                mode = ADM_ZP_INDIRECT_Y;
                break;
            }
            // (ILLEGAL) LXA
            case 0xAB:
            {
                cycles += 2; 
                instruction = IL_LXA;
                mode = ADM_IMMEDIATE;
                break;
            }
            // (ILLEGAL) RLA
            case 0x27:
            {
                cycles += 5; 
                instruction = IL_RLA;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x37:
            {
                cycles += 6; 
                instruction = IL_RLA;
                mode = ADM_ZP_X;
                break;
            }
            case 0x2F:
            {
                cycles += 6; 
                instruction = IL_RLA;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x3F:
            {
                cycles += 7; 
                instruction = IL_RLA;
                mode = ADM_ABS_X;
                break;
            }
            case 0x3B:
            {
                cycles += 7; 
                instruction = IL_RLA;
                mode = ADM_ABS_Y;
                break;
            }
            case 0x23:
            {
                cycles += 8; 
                instruction = IL_RLA;
                mode = ADM_ZP_INDIRECT_X;
                break;
            }
            case 0x33:
            {
                cycles += 8; 
                instruction = IL_RLA;
                mode = ADM_ZP_INDIRECT_Y;
                break;
            }
            // (ILLEGAL) RRA
            case 0x67:
            {
                cycles += 5; 
                instruction = IL_RRA;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x77:
            {
                cycles += 6; 
                instruction = IL_RRA;
                mode = ADM_ZP_X;
                break;
            }
            case 0x6F:
            {
                cycles += 6; 
                instruction = IL_RRA;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x7F:
            {
                cycles += 7; 
                instruction = IL_RRA;
                mode = ADM_ABS_X;
                break;
            }
            case 0x7B:
            {
                cycles += 7; 
                instruction = IL_RRA;
                mode = ADM_ABS_Y;
                break;
            }
            case 0x63:
            {
                cycles += 8; 
                instruction = IL_RRA;
                mode = ADM_ZP_INDIRECT_X;
                break;
            }
            case 0x73:
            {
                cycles += 8; 
                instruction = IL_RRA;
                mode = ADM_ZP_INDIRECT_Y;
                break;
            }
            // (ILLEGAL) SAX
            case 0x87:
            {
                cycles += 3; 
                instruction = IL_SAX;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x97:
            {
                cycles += 4; 
                instruction = IL_SAX;
                mode = ADM_ZP_Y;
                break;
            }
            case 0x8F:
            {
                cycles += 4; 
                instruction = IL_SAX;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x83:
            {
                cycles += 6; 
                instruction = IL_SAX;
                mode = ADM_ZP_INDIRECT_X;
                break;
            }
            // (ILLEGAL) SBX
            case 0xCB:
            {
                cycles += 2; 
                instruction = IL_SBX;
                mode = ADM_IMMEDIATE;
                break;
            }
            // (ILLEGAL) SHA
            case 0x9F:
            {
                cycles += 5; 
                instruction = IL_SHA;
                mode = ADM_ABS_Y;
                break;
            }
            case 0x93:
            {
                cycles += 6; 
                instruction = IL_SHA;
                mode = ADM_ZP_INDIRECT_Y;
                break;
            }
            // (ILLEGAL) SHX
            case 0x9E:
            {
                cycles += 5; 
                instruction = IL_SHX;
                mode = ADM_ABS_Y;
                break;
            }
            // (ILLEGAL) SHY
            case 0x9C:
            {
                cycles += 5; 
                instruction = IL_SHY;
                mode = ADM_ABS_Y;
                break;
            }
            // (ILLEGAL) SLO
            case 0x07:
            {
                cycles += 5; 
                instruction = IL_SLO;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x17:
            {
                cycles += 6; 
                instruction = IL_SLO;
                mode = ADM_ZP_X;
                break;
            }
            case 0x0F:
            {
                cycles += 6; 
                instruction = IL_SLO;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x1F:
            {
                cycles += 7; 
                instruction = IL_SLO;
                mode = ADM_ABS_X;
                break;
            }
            case 0x1B:
            {
                cycles += 7; 
                instruction = IL_SLO;
                mode = ADM_ABS_Y;
                break;
            }
            case 0x03:
            {
                cycles += 8; 
                instruction = IL_SLO;
                mode = ADM_ZP_INDIRECT_X;
                break;
            }
            case 0x13:
            {
                cycles += 8; 
                instruction = IL_SLO;
                mode = ADM_ZP_INDIRECT_Y;
                break;
            }
            // (ILLEGAL) SRE
            case 0x47:
            {
                cycles += 5; 
                instruction = IL_SRE;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x57:
            {
                cycles += 6; 
                instruction = IL_SRE;
                mode = ADM_ZP_X;
                break;
            }
            case 0x4F:
            {
                cycles += 6; 
                instruction = IL_SRE;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x5F:
            {
                cycles += 7; 
                instruction = IL_SRE;
                mode = ADM_ABS_X;
                break;
            }
            case 0x5B:
            {
                cycles += 7; 
                instruction = IL_SRE;
                mode = ADM_ABS_Y;
                break;
            }
            case 0x43:
            {
                cycles += 8; 
                instruction = IL_SRE;
                mode = ADM_ZP_INDIRECT_X;
                break;
            }
            case 0x53:
            {
                cycles += 8; 
                instruction = IL_SRE;
                mode = ADM_ZP_INDIRECT_Y;
                break;
            }
            // (ILLEGAL) TAS
            case 0x9B:
            {
                cycles += 5; 
                instruction = IL_TAS;
                mode = ADM_ABS_Y;
                break;
            }
            // (ILLEGAL) SBC
            case 0xEB:
            {
                cycles += 2; 
                instruction = IL_SBC;
                mode = ADM_IMMEDIATE;
                break;
            }
            // (ILLEGAL) NOP
            case 0x1A:
            {
                cycles += 2; 
                instruction = IL_NOP;
                mode = ADM_IMPLIED;
                break;
            }
            case 0x3A:
            {
                cycles += 2; 
                instruction = IL_NOP;
                mode = ADM_IMPLIED;
                break;
            }
            case 0x5A:
            {
                cycles += 2; 
                instruction = IL_NOP;
                mode = ADM_IMPLIED;
                break;
            }
            case 0x7A:
            {
                cycles += 2; 
                instruction = IL_NOP;
                mode = ADM_IMPLIED;
                break;
            }
            case 0xDA:
            {
                cycles += 2; 
                instruction = IL_NOP;
                mode = ADM_IMPLIED;
                break;
            }
            case 0xFA:
            {
                cycles += 2; 
                instruction = IL_NOP;
                mode = ADM_IMPLIED;
                break;
            }
            case 0x80:
            {
                cycles += 2; 
                instruction = IL_NOP;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0x82:
            {
                cycles += 2; 
                instruction = IL_NOP;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0xC2:
            {
                cycles += 2; 
                instruction = IL_NOP;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0xE2:
            {
                cycles += 2; 
                instruction = IL_NOP;
                mode = ADM_IMMEDIATE;
                break;
            }
            case 0x04:
            {
                cycles += 3; 
                instruction = IL_NOP;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x44:
            {
                cycles += 3; 
                instruction = IL_NOP;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x64:
            {
                cycles += 3; 
                instruction = IL_NOP;
                mode = ADM_ZEROPAGE;
                break;
            }
            case 0x14:
            {
                cycles += 4; 
                instruction = IL_NOP;
                mode = ADM_ZP_X;
                break;
            }
            case 0x34:
            {
                cycles += 4; 
                instruction = IL_NOP;
                mode = ADM_ZP_X;
                break;
            }
            case 0x54:
            {
                cycles += 4; 
                instruction = IL_NOP;
                mode = ADM_ZP_X;
                break;
            }
            case 0x74:
            {
                cycles += 4; 
                instruction = IL_NOP;
                mode = ADM_ZP_X;
                break;
            }
            case 0xD4:
            {
                cycles += 4; 
                instruction = IL_NOP;
                mode = ADM_ZP_X;
                break;
            }
            case 0xF4:
            {
                cycles += 4; 
                instruction = IL_NOP;
                mode = ADM_ZP_X;
                break;
            }
            case 0x0C:
            {
                cycles += 4; 
                instruction = IL_NOP;
                mode = ADM_ABSOLUTE;
                break;
            }
            case 0x1C:
            {
                cycles += 4; 
                instruction = IL_NOP;
                mode = ADM_ABS_X;
                break;
            }
            case 0x3C:
            {
                cycles += 4; 
                instruction = IL_NOP;
                mode = ADM_ABS_X;
                break;
            }
            case 0x5C:
            {
                cycles += 4; 
                instruction = IL_NOP;
                mode = ADM_ABS_X;
                break;
            }
            case 0x7C:
            {
                cycles += 4; 
                instruction = IL_NOP;
                mode = ADM_ABS_X;
                break;
            }
            case 0xDC:
            {
                cycles += 4; 
                instruction = IL_NOP;
                mode = ADM_ABS_X;
                break;
            }
            case 0xFC:
            {
                cycles += 4; 
                instruction = IL_NOP;
                mode = ADM_ABS_X;
                break;
            }
            // JAM
            case 0x02: 
            {
                instruction = IL_JAM;
                mode = ADM_IMPLIED;
                break;
            }
            case 0x12: 
            {
                instruction = IL_JAM;
                mode = ADM_IMPLIED;
                break;
            }
            case 0x22: 
            {
                instruction = IL_JAM;
                mode = ADM_IMPLIED;
                break;
            }
            case 0x32: 
            {
                instruction = IL_JAM;
                mode = ADM_IMPLIED;
                break;
            }
            case 0x42: 
            {
                instruction = IL_JAM;
                mode = ADM_IMPLIED;
                break;
            }
            case 0x52: 
            {
                instruction = IL_JAM;
                mode = ADM_IMPLIED;
                break;
            }
            case 0x62: 
            {
                instruction = IL_JAM;
                mode = ADM_IMPLIED;
                break;
            }
            case 0x72: 
            {
                instruction = IL_JAM;
                mode = ADM_IMPLIED;
                break;
            }
            case 0x92: 
            {
                instruction = IL_JAM;
                mode = ADM_IMPLIED;
                break;
            }
            case 0xB2: 
            {
                instruction = IL_JAM;
                mode = ADM_IMPLIED;
                break;
            }
            case 0xD2: 
            {
                instruction = IL_JAM;
                mode = ADM_IMPLIED;
                break;
            }
            case 0xF2: 
            {
                instruction = IL_JAM;
                mode = ADM_IMPLIED;
                break;
            }
            
            // Other
            default: 
                exc_panic_illegalInstruction(opcode);
                break;
        }

        switch (mode) {
            case ADM_ACCUMULATOR:
            {
                accumulatorAddrMode = true;
                address = 0x0000;
                break;
            }
            case ADM_IMPLIED:
            {
                address = 0x0000;
                break;
            }
            case ADM_IMMEDIATE:
            {
                reg_pc += 1;
                address = reg_pc - 1;
                break;
            }
            case ADM_ABSOLUTE:
            {
                address = bus_readCPU(reg_pc); // cycle
                reg_pc += 1;
                address = (((uint16_t) bus_readCPU(reg_pc)) << 8) | address; // cycle
                reg_pc += 1;

                break;
            }
            case ADM_ZEROPAGE:
            {
                address = bus_readCPU(reg_pc); // cycle
                reg_pc += 1;
                address %= 256;

                break;
            }
            case ADM_RELATIVE:
            {
                address = 0x0000;
                break;
            }
            case ADM_ABS_INDIRECT:
            {
                uint16_t initlow = bus_readCPU(reg_pc); // cycle
                reg_pc += 1;
                uint16_t inithigh = bus_readCPU(reg_pc); // cycle
                reg_pc += 1;
                inithigh = inithigh << 8;

                uint16_t low = (inithigh | initlow);
                uint16_t high = (inithigh | ((initlow + 1) % 256));
                low = bus_readCPU(low); // cycle
                high = bus_readCPU(high); // cycle
                high = high << 8;
                address = (high | low);

                break;
            }
            case ADM_ABS_X:
            {
                address = bus_readCPU(reg_pc); // cycle
                reg_pc += 1;
                address = (bus_readCPU(reg_pc) << 8) | address; // cycle
                reg_pc += 1;
                address += reg_x;
                
                break;
            }
            case ADM_ABS_Y:
            {
                address = bus_readCPU(reg_pc); // cycle
                reg_pc += 1;
                address = (bus_readCPU(reg_pc) << 8) | address; // cycle
                reg_pc += 1;
                address += reg_y; // cycle

                break;
            }
            case ADM_ZP_X:
            {
                address = bus_readCPU(reg_pc); // cycle
                reg_pc += 1;
                address += reg_x; // cycle
                address %= 256;

                break;
            }
            case ADM_ZP_Y:
            {
                address = bus_readCPU(reg_pc); // cycle
                reg_pc += 1;
                address += reg_y; // cycle
                address %= 256;

                break;
            }
            case ADM_ZP_INDIRECT_X:
            {
                address = bus_readCPU(reg_pc); // cycle
                reg_pc += 1;
                address += reg_x; // cycle
                address %= 256;

                uint16_t low = bus_readCPU(address); // cycle
                uint16_t high = bus_readCPU((address + 1) % 256); // cycle

                high = high << 8;
                address = (high | low);

                break;
            }
            case ADM_ZP_INDIRECT_Y:
            {
                address = bus_readCPU(reg_pc); // cycle
                reg_pc += 1;

                uint16_t low = bus_readCPU(address); // cycle
                uint16_t high = bus_readCPU((address + 1) % 256); // cycle

                high = high << 8;
                address = (high | low);
                address += reg_y; // cycle
                break;
            }
        }
        
        if (generateTrace) cpu_trace(instruction, mode, address);
        
        switch (instruction) {
            case LDA:
            {
                reg_accumulator = bus_readCPU(address);
                (reg_accumulator == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_accumulator & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case LDX:
            {
                reg_x = bus_readCPU(address);
                (reg_x == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_x & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case LDY:
            {
                reg_y = bus_readCPU(address);
                (reg_y == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_y & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case STA:
            {
                bus_writeCPU(address, reg_accumulator);
                break;
            }
            case STX:
            {
                bus_writeCPU(address, reg_x);
                break;
            }
            case STY:
            {
                bus_writeCPU(address, reg_y);
                break;
            }
            case ADC:
            {
                uint8_t memoryVal = bus_readCPU(address);

                uint16_t sum = reg_accumulator + memoryVal + ((uint16_t) cpu_getFlag(CPUSTAT_CARRY));

                cpu_setFlag(CPUSTAT_CARRY, sum > 0xFF);
                cpu_setFlag(CPUSTAT_OVERFLOW, (reg_accumulator ^ sum) & (memoryVal ^ sum) & 0x80);

                reg_accumulator = (uint8_t) sum;
                (reg_accumulator == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_accumulator & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case SBC:
            {
                uint8_t memoryVal = ~bus_readCPU(address);
                uint16_t sum = reg_accumulator + memoryVal + ((uint16_t) cpu_getFlag(CPUSTAT_CARRY));
                cpu_setFlag(CPUSTAT_CARRY, sum > 0xFF);
                cpu_setFlag(CPUSTAT_OVERFLOW, (reg_accumulator ^ sum) & (memoryVal ^ sum) & 0x80);
                reg_accumulator = (uint8_t) sum;
                (reg_accumulator == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_accumulator & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case INC:
            {
                uint8_t val = bus_readCPU(address) + 1;
                bus_writeCPU(address, val);
                (val == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((val & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case INX:
            {
                reg_x += 1;
                (reg_x == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_x & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case INY: 
            {
                reg_y += 1;
                (reg_y == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_y & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case DEC:
            {
                uint8_t val = bus_readCPU(address) - 1;
                bus_writeCPU(address, val);
                (val == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((val & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case DEX:
            {
                reg_x -= 1;
                (reg_x == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_x & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case DEY:
            {
                reg_y -= 1;
                (reg_y == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_y & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case ASL:
            {
                uint8_t storedVal = reg_accumulator;
                if (!accumulatorAddrMode) {
                    storedVal = bus_readCPU(address);
                }
                cpu_setFlag(CPUSTAT_CARRY, (storedVal >> 7) & 1);
                storedVal = storedVal << 1;
                (storedVal == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((storedVal & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                if (accumulatorAddrMode) {
                    reg_accumulator = storedVal;
                } else {
                    bus_writeCPU(address, storedVal);
                }
                accumulatorAddrMode = false;
                break;
            }
            case LSR:
            {
                uint8_t storedVal = reg_accumulator;
                if (!accumulatorAddrMode) {
                    storedVal = bus_readCPU(address);
                }
                cpu_setFlag(CPUSTAT_CARRY, storedVal & 1);
                storedVal = storedVal >> 1;
                (storedVal == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((storedVal & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                if (accumulatorAddrMode) {
                    reg_accumulator = storedVal;
                } else {
                    bus_writeCPU(address, storedVal);
                }
                accumulatorAddrMode = false;
                break;
            }
            case ROL:
            {
                uint8_t storedVal = reg_accumulator;
                if (!accumulatorAddrMode) {
                    storedVal = bus_readCPU(address);
                }
                bool oldCarry = cpu_getFlag(CPUSTAT_CARRY);
                cpu_setFlag(CPUSTAT_CARRY, (storedVal >> 7) & 1);
                storedVal = storedVal << 1;
                storedVal = storedVal | oldCarry;
                (storedVal == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((storedVal & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                if (accumulatorAddrMode) {
                    reg_accumulator = storedVal;
                } else {
                    bus_writeCPU(address, storedVal);
                }
                accumulatorAddrMode = false;
                break;
            }
            case ROR:
            {
                uint8_t storedVal = reg_accumulator;
                if (!accumulatorAddrMode) {
                    storedVal = bus_readCPU(address);
                }
                bool oldCarry = cpu_getFlag(CPUSTAT_CARRY);
                cpu_setFlag(CPUSTAT_CARRY, storedVal & 1);
                storedVal = storedVal >> 1;
                storedVal = storedVal | (oldCarry << 7);
                (storedVal == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((storedVal & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                if (accumulatorAddrMode) {
                    reg_accumulator = storedVal;
                } else {
                    bus_writeCPU(address, storedVal);
                }
                accumulatorAddrMode = false;
                break;
            }
            case AND:
            {
                reg_accumulator = bus_readCPU(address) & reg_accumulator;
                (reg_accumulator == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_accumulator & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case ORA:
            {
                reg_accumulator = bus_readCPU(address) | reg_accumulator;
                (reg_accumulator == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_accumulator & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case EOR:
            {
                reg_accumulator = bus_readCPU(address) ^ reg_accumulator;
                (reg_accumulator == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_accumulator & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case CMP:
            {
                uint8_t memVal = bus_readCPU(address);
                int8_t signedResult = ((int8_t) reg_accumulator) - ((int8_t) memVal);
                if (reg_accumulator < memVal) {
                    reg_status &= ~CPUSTAT_ZERO;
                    reg_status &= ~CPUSTAT_CARRY;
                } else if (reg_accumulator == memVal) {
                    reg_status |= CPUSTAT_ZERO;
                    reg_status |= CPUSTAT_CARRY;
                } else if (reg_accumulator > memVal) {
                    reg_status &= ~CPUSTAT_ZERO;
                    reg_status |= CPUSTAT_CARRY;
                }
                (signedResult < 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case CPX:
            {
                uint8_t memVal = bus_readCPU(address);
                int8_t signedResult = ((int8_t) reg_x) - ((int8_t) memVal);
                if (reg_x < memVal) {
                    reg_status &= ~CPUSTAT_ZERO;
                    reg_status &= ~CPUSTAT_CARRY;
                } else if (reg_x == memVal) {
                    reg_status |= CPUSTAT_ZERO;
                    reg_status |= CPUSTAT_CARRY;
                } else if (reg_x > memVal) {
                    cpu_setFlag(CPUSTAT_ZERO, 0);
                    cpu_setFlag(CPUSTAT_CARRY, 1);
                }
                (signedResult < 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case CPY:
            {
                uint8_t memVal = bus_readCPU(address);
                int8_t signedResult = ((int8_t) reg_y) - ((int8_t) memVal);
                if (reg_y < memVal) {
                    reg_status &= ~CPUSTAT_ZERO;
                    reg_status &= ~CPUSTAT_CARRY;
                } else if (reg_y == memVal) {
                    reg_status |= CPUSTAT_ZERO;
                    reg_status |= CPUSTAT_CARRY;
                } else if (reg_y > memVal) {
                    reg_status &= ~CPUSTAT_ZERO;
                    reg_status |= CPUSTAT_CARRY;
                }
                (signedResult < 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case BIT:
            {
                uint8_t memVal = bus_readCPU(address);
                ((reg_accumulator & memVal) == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((memVal & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                ((memVal & 0b01000000) != 0) ? (reg_status |= CPUSTAT_OVERFLOW) : (reg_status &= ~CPUSTAT_OVERFLOW);
                break;
            }
            case BCC:
            {
                cpu_branchHelper(0, CPUSTAT_CARRY);
                break;
            }
            case BCS:
            {
                cpu_branchHelper(1, CPUSTAT_CARRY);
                break;
            }
            case BNE:
            {
                cpu_branchHelper(0, CPUSTAT_ZERO);
                break;
            }
            case BEQ:
            {
                cpu_branchHelper(1, CPUSTAT_ZERO);
                break;
            }
            case BPL:
            {
                cpu_branchHelper(0, CPUSTAT_NEGATIVE);
                break;
            }
            case BMI:
            {
                cpu_branchHelper(1, CPUSTAT_NEGATIVE);
                break;
            }
            case BVC:
            {
                cpu_branchHelper(0, CPUSTAT_OVERFLOW);
                break;
            }
            case BVS:
            {
                cpu_branchHelper(1, CPUSTAT_OVERFLOW);
                break;
            }
            case TAX:
            {
                reg_x = reg_accumulator;
                (reg_x == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_x & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case TXA:
            {
                reg_accumulator = reg_x;
                // cycles += 1;
            (reg_accumulator == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_accumulator & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case TAY:
            {
                reg_y = reg_accumulator;
                (reg_y == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_y & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case TYA:
            {
                reg_accumulator = reg_y;
                (reg_accumulator == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_accumulator & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case TSX: 
            {
                reg_x = stackPointer;
                (reg_x == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_x & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case TXS:
            {
                stackPointer = reg_x;
                break;
            }
            case PLA:
            {
                reg_accumulator = cpu_stackPull();
                (reg_accumulator == 0) ? (reg_status |= CPUSTAT_ZERO) : (reg_status &= ~CPUSTAT_ZERO);
                ((reg_accumulator & 0b10000000) != 0) ? (reg_status |= CPUSTAT_NEGATIVE) : (reg_status &= ~CPUSTAT_NEGATIVE);
                break;
            }
            case PHA:
            {
                cpu_stackPush(reg_accumulator);
                break;
            }
            case PHP:
            {
                cpu_stackPush(reg_status);
                break;
            }
            case PLP:
            {
                reg_status = cpu_stackPull();
                reg_status |= CPUSTAT_BREAK2;
                reg_status &= ~CPUSTAT_BREAK;
                break;
            }
            case JMP:
            {
                reg_pc = address;
                break;
            }
            case JSR:
            {
                uint16_t returnAddress = reg_pc - 1;
                reg_pc = address;
                cpu_stackPush16(returnAddress);
                break;
            }
            case RTS:
            {
                reg_pc = cpu_stackPull16() + 1;
                break;
            }
            case RTI:
            {
                reg_status = cpu_stackPull();
                reg_pc = cpu_stackPull16();
                cpu_setFlag(CPUSTAT_BREAK2, true); // always ensure BREAK2 is set
                break;
            }
            case CLC:
            {
                reg_status &= ~CPUSTAT_CARRY;
                break;
            }
            case SEC:
            {
                reg_status |= CPUSTAT_CARRY;
                break;
            }
            case CLD:
            {
                reg_status &= ~CPUSTAT_DECIMAL;
                break;
            }
            case SED:
            {
                reg_status |= CPUSTAT_DECIMAL;
                break;
            }
            case CLI:
            {
                reg_status &= ~CPUSTAT_NO_INTRPT;
                break;
            }
            case SEI:
            {
                reg_status |= CPUSTAT_NO_INTRPT;
                break;
            }
            case CLV:
            {
                reg_status &= ~CPUSTAT_OVERFLOW;
                break;
            }
            case BRK:
            {
                reg_status |= CPUSTAT_BREAK;
                reg_status |= CPUSTAT_NO_INTRPT;
                break;
            }
            case NOP:
            {
                break;
            }
            case IL_ALR:
            {
                reg_accumulator = bus_readCPU(address) & reg_accumulator;
                cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
                uint8_t storedVal = reg_accumulator;
                if (!accumulatorAddrMode) {
                    storedVal = bus_readCPU(address);
                }
                cpu_setFlag(CPUSTAT_CARRY, storedVal & 1);
                storedVal = storedVal >> 1;

                cpu_setFlag(CPUSTAT_ZERO, storedVal == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (storedVal & 0b10000000) != 0);

                if (accumulatorAddrMode) {
                    reg_accumulator = storedVal;
                } else {
                    bus_writeCPU(address, storedVal);
                }

                accumulatorAddrMode = false;
                break;
            }
            case IL_ANC:
            {
                reg_accumulator = bus_readCPU(address) & reg_accumulator;
                cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
                cpu_setFlag(CPUSTAT_CARRY, (bus_readCPU(address) >> 7) & 1);
                break;
            }
            case IL_ANE:
            {
                reg_accumulator = reg_x & (rand() % 256);
                reg_accumulator = bus_readCPU(address) & reg_accumulator;
                cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
                break;
            }
            case IL_ARR:
            {
                reg_accumulator = bus_readCPU(address) & reg_accumulator;
                cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
                uint8_t storedVal = reg_accumulator;
                if (!accumulatorAddrMode) {
                    storedVal = bus_readCPU(address);
                }

                bool oldCarry = cpu_getFlag(CPUSTAT_CARRY);
                cpu_setFlag(CPUSTAT_CARRY, storedVal & 1);
                storedVal = storedVal >> 1;
                storedVal = storedVal | (oldCarry << 7);

                cpu_setFlag(CPUSTAT_ZERO, storedVal == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (storedVal & 0b10000000) != 0);

                if (accumulatorAddrMode) {
                    reg_accumulator = storedVal;
                } else {
                    bus_writeCPU(address, storedVal);
                }

                accumulatorAddrMode = false;
                break;
            }
            case IL_DCP:
            {
                uint8_t val = bus_readCPU(address) - 1;
                bus_writeCPU(address, val);

                cpu_setFlag(CPUSTAT_ZERO, val == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (val & 0b10000000) != 0);
                uint8_t memVal = bus_readCPU(address);
                int8_t signedResult = ((int8_t) reg_accumulator) - ((int8_t) memVal);
                if (reg_accumulator < memVal) {
                    cpu_setFlag(CPUSTAT_ZERO, 0);
                    cpu_setFlag(CPUSTAT_CARRY, 0);
                } else if (reg_accumulator == memVal) {
                    cpu_setFlag(CPUSTAT_ZERO, 1);
                    cpu_setFlag(CPUSTAT_CARRY, 1);
                } else if (reg_accumulator > memVal) {
                    cpu_setFlag(CPUSTAT_ZERO, 0);
                    cpu_setFlag(CPUSTAT_CARRY, 1);
                }

                cpu_setFlag(CPUSTAT_NEGATIVE, signedResult < 0);
                break;
            }
            case IL_ISC:
            {
                uint8_t val = bus_readCPU(address) + 1;
                bus_writeCPU(address, val);
                cpu_setFlag(CPUSTAT_ZERO, val == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (val & 0b10000000) != 0);
                uint8_t memoryVal = ~bus_readCPU(address);
                uint16_t sum = reg_accumulator + memoryVal + ((uint16_t) cpu_getFlag(CPUSTAT_CARRY));
                cpu_setFlag(CPUSTAT_CARRY, sum > 0xFF);
                cpu_setFlag(CPUSTAT_OVERFLOW, (reg_accumulator ^ sum) & (memoryVal ^ sum) & 0x80);
                reg_accumulator = (uint8_t) sum;
                cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
                break;
            }
            case IL_LAS:
            {
                reg_accumulator = bus_readCPU(address);
                cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
                reg_x = stackPointer;
                cpu_setFlag(CPUSTAT_ZERO, reg_x == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (reg_x & 0b10000000) != 0);
                break;
            }
            case IL_LAX:
            {
                reg_accumulator = bus_readCPU(address);
                reg_x = bus_readCPU(address);
                cpu_setFlag(CPUSTAT_ZERO, reg_x == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (reg_x & 0b10000000) != 0);
                break;
            }
            case IL_LXA:
            {
                reg_accumulator = (rand() % 256) & bus_readCPU(address);
                reg_x = reg_accumulator;
                break;
            }
            case IL_RLA:
            {
                uint8_t storedVal = reg_accumulator;
                if (!accumulatorAddrMode) {
                    storedVal = bus_readCPU(address);
                }
                bool oldCarry = cpu_getFlag(CPUSTAT_CARRY);
                cpu_setFlag(CPUSTAT_CARRY, (storedVal >> 7) & 1);
                storedVal = storedVal << 1;
                storedVal = storedVal | oldCarry;
                if (accumulatorAddrMode) {
                    reg_accumulator = storedVal;
                } else {
                    bus_writeCPU(address, storedVal);
                }
                accumulatorAddrMode = false;
                reg_accumulator = bus_readCPU(address) & reg_accumulator;
                cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
                break;
            }
            case IL_RRA:
            {
                uint8_t storedVal = reg_accumulator;
                if (!accumulatorAddrMode) {
                    storedVal = bus_readCPU(address);
                }
                bool oldCarry = cpu_getFlag(CPUSTAT_CARRY);
                cpu_setFlag(CPUSTAT_CARRY, storedVal & 1);
                storedVal = storedVal >> 1;
                storedVal = storedVal | (oldCarry << 7);
                cpu_setFlag(CPUSTAT_ZERO, storedVal == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (storedVal & 0b10000000) != 0);
                if (accumulatorAddrMode) {
                    reg_accumulator = storedVal;
                } else {
                    bus_writeCPU(address, storedVal);
                }
                accumulatorAddrMode = false;
                uint8_t memoryVal = bus_readCPU(address);
                uint16_t sum = reg_accumulator + memoryVal + ((uint16_t) cpu_getFlag(CPUSTAT_CARRY));
                cpu_setFlag(CPUSTAT_CARRY, sum > 0xFF);
                cpu_setFlag(CPUSTAT_OVERFLOW, (reg_accumulator ^ sum) & (memoryVal ^ sum) & 0x80);
                reg_accumulator = (uint8_t) sum;
                cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
                break;
            }
            case IL_SAX:
            {
                bus_writeCPU(address, reg_accumulator & reg_x);
                break;
            }
            case IL_SBX:
            {
                uint8_t memVal = bus_readCPU(address);
                uint8_t cmpVal = reg_accumulator & reg_x;
                int8_t signedResult = ((int8_t) cmpVal) - ((int8_t) memVal);
                if (cmpVal < memVal) {
                    cpu_setFlag(CPUSTAT_ZERO, 0);
                    cpu_setFlag(CPUSTAT_CARRY, 0);
                } else if (cmpVal == memVal) {
                    cpu_setFlag(CPUSTAT_ZERO, 1);
                    cpu_setFlag(CPUSTAT_CARRY, 1);
                } else if (cmpVal > memVal) {
                    cpu_setFlag(CPUSTAT_ZERO, 0);
                    cpu_setFlag(CPUSTAT_CARRY, 1);
                }
                cpu_setFlag(CPUSTAT_NEGATIVE, signedResult < 0);
                break;
            }
            case IL_SHA:
            {
                // programmed instability
                if (rand() % 4 == 0) {
                    bus_writeCPU(address, reg_accumulator & reg_x);
                } else {
                    bus_writeCPU(address, reg_accumulator & reg_x & ((bus_readCPU(address) >> 8) + 1));
                }
                break;
            }
            case IL_SHX:
            {
                // programmed instability
                if (rand() % 4 == 0) {
                    bus_writeCPU(address, reg_x);
                } else {
                    bus_writeCPU(address, reg_x & ((bus_readCPU(address) >> 8) + 1));
                }
                break;
            }
            case IL_SHY:
            {
                // programmed instability
                if (rand() % 4 == 0) {
                    bus_writeCPU(address, reg_y);
                } else {
                    bus_writeCPU(address, reg_y & ((bus_readCPU(address) >> 8) + 1));
                }
                break;
            }
            case IL_SLO:
            {
                uint8_t storedVal = reg_accumulator;
                if (!accumulatorAddrMode) {
                    storedVal = bus_readCPU(address);
                }
                cpu_setFlag(CPUSTAT_CARRY, (storedVal >> 7) & 1);
                storedVal = storedVal << 1;

                cpu_setFlag(CPUSTAT_ZERO, storedVal == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (storedVal & 0b10000000) != 0);

                if (accumulatorAddrMode) {
                    reg_accumulator = storedVal;
                } else {
                    bus_writeCPU(address, storedVal);
                }

                accumulatorAddrMode = false;
                reg_accumulator = bus_readCPU(address) | reg_accumulator;
                cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
                break;
            }
            case IL_SRE:
            {
                uint8_t storedVal = reg_accumulator;
                if (!accumulatorAddrMode) {
                    storedVal = bus_readCPU(address);
                }
                cpu_setFlag(CPUSTAT_CARRY, storedVal & 1);
                storedVal = storedVal >> 1;
                if (accumulatorAddrMode) {
                    reg_accumulator = storedVal;
                } else {
                    bus_writeCPU(address, storedVal);
                }
                accumulatorAddrMode = false;
                reg_accumulator = bus_readCPU(address) ^ reg_accumulator;
                cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
                break;
            }
            case IL_TAS:
            {
                stackPointer = reg_accumulator & reg_x;
                // programmed instability
                if (rand() % 4 == 0) {
                    bus_writeCPU(address, reg_accumulator & reg_x);
                } else {
                    bus_writeCPU(address, reg_accumulator & reg_x & ((bus_readCPU(address) >> 8) + 1));
                }
                break;
            }
            case IL_SBC:
            {
                uint8_t memoryVal = ~bus_readCPU(address);
                uint16_t sum = reg_accumulator + memoryVal + ((uint16_t) cpu_getFlag(CPUSTAT_CARRY));
                cpu_setFlag(CPUSTAT_CARRY, sum > 0xFF);
                cpu_setFlag(CPUSTAT_OVERFLOW, (reg_accumulator ^ sum) & (memoryVal ^ sum) & 0x80);
                reg_accumulator = (uint8_t) sum;
                cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
                cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
                break;
            }
            case IL_JAM:
            {
                cpu_setFlag(CPUSTAT_BREAK, 1);
                cpu_setFlag(CPUSTAT_NO_INTRPT, 1);
                break;
            }
            case IL_NOP:
            {
                break;
            }
        }
        bus_cpuReport(cycles - oldCycles);
    }

    bus_endTimeMonitor();
}

void cpu_vblankNMI() {
    cpu_stackPush16(reg_pc);
    cpu_stackPush(reg_status);
    cpu_setFlag(CPUSTAT_NO_INTRPT, true);
    // cycles += 2;
    reg_pc = bus_readCPUAddr(0xFFFA);
}

uint64_t cpu_getCycles() {
    return cycles;
}

void cpu_panic() {
    didPanic = true;
}

uint16_t cpu_fetchAddress(enum AddressingMode mode) {
    
    switch (mode) {
        case ADM_ACCUMULATOR:
        {
            accumulatorAddrMode = true;
            return 0x0000;
            break;
        }
        case ADM_IMPLIED:
        {
            return 0x0000;
            break;
        }
        case ADM_IMMEDIATE:
        {
            reg_pc += 1;
            return reg_pc - 1;
            break;
        }
        case ADM_ABSOLUTE:
        {
            uint16_t address = bus_readCPU(reg_pc); // cycle
            reg_pc += 1;
            address = (((uint16_t) bus_readCPU(reg_pc)) << 8) | address; // cycle
            reg_pc += 1;

            // cycles += 2;
            return address;
            break;
        }
        case ADM_ZEROPAGE:
        {
            uint16_t address = bus_readCPU(reg_pc); // cycle
            reg_pc += 1;
            address %= 256;

            // cycles += 1;
            return address;
            break;
        }
        case ADM_RELATIVE:
        {
            return 0x0000;
            break;
        }
        case ADM_ABS_INDIRECT:
        {
            uint16_t initlow = bus_readCPU(reg_pc); // cycle
            reg_pc += 1;
            uint16_t inithigh = bus_readCPU(reg_pc); // cycle
            reg_pc += 1;
            inithigh = inithigh << 8;

            uint16_t low = (inithigh | initlow);
            uint16_t high = (inithigh | ((initlow + 1) % 256));
            low = bus_readCPU(low); // cycle
            high = bus_readCPU(high); // cycle
            high = high << 8;
            uint16_t address = (high | low);

            // cycles += 4;
            return address;
            break;
        }
        case ADM_ABS_X:
        {
            uint16_t address = bus_readCPU(reg_pc); // cycle
            reg_pc += 1;
            address = (bus_readCPU(reg_pc) << 8) | address; // cycle
            reg_pc += 1;
            address += reg_x;
            
            // cycles += 3;
            return address;
            break;
        }
        case ADM_ABS_Y:
        {
            uint16_t address = bus_readCPU(reg_pc); // cycle
            reg_pc += 1;
            address = (bus_readCPU(reg_pc) << 8) | address; // cycle
            reg_pc += 1;
            address += reg_y; // cycle

            // cycles += 3;
            return address;
            break;
        }
        case ADM_ZP_X:
        {
            uint16_t address = bus_readCPU(reg_pc); // cycle
            reg_pc += 1;
            address += reg_x; // cycle
            address %= 256;

            // cycles += 2;
            return address;
            break;
        }
        case ADM_ZP_Y:
        {
            uint16_t address = bus_readCPU(reg_pc); // cycle
            reg_pc += 1;
            address += reg_y; // cycle
            address %= 256;

            // cycles += 2;
            return address;
            break;
        }
        case ADM_ZP_INDIRECT_X:
        {
            uint16_t address = bus_readCPU(reg_pc); // cycle
            reg_pc += 1;
            address += reg_x; // cycle
            address %= 256;

            uint16_t low = bus_readCPU(address); // cycle
            uint16_t high = bus_readCPU((address + 1) % 256); // cycle

            high = high << 8;
            address = (high | low);

            // cycles += 4;
            return address;

            break;
        }
        case ADM_ZP_INDIRECT_Y:
        {
            uint16_t address = bus_readCPU(reg_pc); // cycle
            reg_pc += 1;

            uint16_t low = bus_readCPU(address); // cycle
            uint16_t high = bus_readCPU((address + 1) % 256); // cycle

            high = high << 8;
            address = (high | low);
            address += reg_y; // cycle

            // cycles += 4;
            return address;

        }
    }
    return 0x0000;
}

void cpu_setFlag(enum CPUStatusFlag flag, bool enable) {
    if (enable) {
        reg_status |= flag;
    } else {
        reg_status &= ~flag;
    }
}

bool cpu_getFlag(enum CPUStatusFlag flag) {
    return (reg_status & flag) > 0;
}

void cpu_reset() {
    reg_accumulator = 0;
    reg_x = 0;
    reg_y = 0;
    reg_status = 0x24;
    reg_pc = bus_readCPUAddr(0xFFFC);
    if (diagnosticMode) reg_pc = 0xC000;

    stackPointer = 0xFD;
}

void cpu_stackPush(uint8_t val) {
    if (stackPointer == 0x00) {
        exc_panic_stackOverflow();
    } else {
        bus_writeCPU(0x100 + stackPointer, val);
        stackPointer -= 1;
        // cycles += 1;
    }
}

uint8_t cpu_stackPull() {
    if (stackPointer == 0xFF) {
        exc_panic_stackUnderflow();
    } else {
        stackPointer += 1;
        uint8_t val = bus_readCPU(0x100 + stackPointer);
        return val;
    }
    return 0;
}

void cpu_stackPush16(uint16_t val) {
    if (stackPointer == 0x01) {
        exc_panic_stackOverflow();
    } else {
        uint8_t high = (val >> 8) & 0x00FF;
        uint8_t low = (val >> 0) & 0x00FF;

        bus_writeCPU(0x100 + stackPointer, high);
        stackPointer -= 1;
        bus_writeCPU(0x100 + stackPointer, low);
        stackPointer -= 1;
    }
}

uint16_t cpu_stackPull16() {
    if (stackPointer == 0xFE) {
        exc_panic_stackUnderflow();
    } else {

        stackPointer += 1;
        uint8_t low = bus_readCPU(0x100 + stackPointer);

        stackPointer += 1;
        uint8_t high = bus_readCPU(0x100 + stackPointer);

        uint16_t addr = (((uint16_t) high) << 8) | ((uint16_t) low);

        return addr;
    }
    return 0;
}

void cpu_trace(enum Instruction instruction, enum AddressingMode mode, uint16_t address) {
    char* instructionCode = cpu_getInstructionString(instruction);
    uint8_t opcode = 0x00;
    uint8_t firstParam = 0x00;
    uint8_t secondParam = 0x00;
    char parameterString[64];
    char redirectString[64];
    uint16_t offsetAddress = 0x0000;
    uint16_t rewindPC = reg_pc - 1;
    int paramCount = 0;

    switch (mode) {
        case ADM_ACCUMULATOR:
        {
            rewindPC -= 0;
            opcode = bus_readCPU(rewindPC);
            sprintf(parameterString, "");
            sprintf(redirectString, "");
            paramCount = 0;
            break;
        }
        case ADM_IMPLIED:
        {
            rewindPC -= 0;
            opcode = bus_readCPU(rewindPC);
            sprintf(parameterString, "");
            sprintf(redirectString, "");
            paramCount = 0;
            break;
        }
        case ADM_IMMEDIATE:
        {
            rewindPC -= 1;
            opcode = bus_readCPU(rewindPC);
            uint8_t immediateVal = bus_readCPU(rewindPC + 1);
            sprintf(parameterString, "#$%02X", immediateVal);
            sprintf(redirectString, "");
            firstParam = bus_readCPU(rewindPC + 1);
            paramCount = 1;
            break;
        }
        case ADM_ABSOLUTE:
        {
            
            rewindPC -= 2;
            opcode = bus_readCPU(rewindPC);
            uint16_t absAddr = bus_readCPU(rewindPC + 1);
            absAddr = (((uint16_t) bus_readCPU(rewindPC + 2)) << 8) | absAddr;
            sprintf(parameterString, "$%04X", absAddr);
            firstParam = bus_readCPU(rewindPC + 1);
            secondParam = bus_readCPU(rewindPC + 2);
            paramCount = 2;
            sprintf(redirectString, "");
            break;
        }
        case ADM_ZEROPAGE:
        {
            rewindPC -= 1;
            opcode = bus_readCPU(rewindPC);
            uint8_t zpAddr = bus_readCPU(rewindPC + 1);
            sprintf(parameterString, "$%02X", zpAddr);
            firstParam = bus_readCPU(rewindPC + 1);
            paramCount = 1;
            sprintf(redirectString, "= %02X", bus_readCPU(zpAddr));
            break;
        }
        case ADM_RELATIVE:
        {
            rewindPC -= 0;
            opcode = bus_readCPU(rewindPC);
            int8_t offset = bus_readCPU(rewindPC + 1);
            uint16_t relAddr = rewindPC + offset;
            relAddr += 2;
            sprintf(parameterString, "$%04X", relAddr);
            firstParam = bus_readCPU(rewindPC + 1);
            paramCount = 1;
            sprintf(redirectString, "");
            break;
        }
        case ADM_ABS_INDIRECT:
        {
            rewindPC -= 2;
            opcode = bus_readCPU(rewindPC);
            uint16_t absAddr = bus_readCPU(rewindPC + 1);
            absAddr = (((uint16_t) bus_readCPU(rewindPC + 2)) << 8) | absAddr;
            sprintf(parameterString, "($%04X)", absAddr);
            firstParam = bus_readCPU(rewindPC + 1);
            secondParam = bus_readCPU(rewindPC + 2);
            paramCount = 2;
            sprintf(redirectString, "= %04X", address);
            break;
        }
        case ADM_ABS_X:
        {
            rewindPC -= 2;
            opcode = bus_readCPU(rewindPC);
            uint16_t absAddr = bus_readCPU(rewindPC + 1);
            absAddr = (((uint16_t) bus_readCPU(rewindPC + 2)) << 8) | absAddr;
            sprintf(parameterString, "$%04X,X", absAddr);
            firstParam = bus_readCPU(rewindPC + 1);
            secondParam = bus_readCPU(rewindPC + 2);
            paramCount = 2;
            sprintf(redirectString, "");
            break;
        }
        case ADM_ABS_Y:
        {
            rewindPC -= 2;
            opcode = bus_readCPU(rewindPC);
            uint16_t absAddr = bus_readCPU(rewindPC + 1);
            absAddr = (((uint16_t) bus_readCPU(rewindPC + 2)) << 8) | absAddr;
            sprintf(parameterString, "$%04X,Y", absAddr);
            firstParam = bus_readCPU(rewindPC + 1);
            secondParam = bus_readCPU(rewindPC + 2);
            paramCount = 2;
            sprintf(redirectString, "");
            break;
        }
        case ADM_ZP_X:
        {
            rewindPC -= 1;
            opcode = bus_readCPU(rewindPC);
            uint8_t zpAddr = bus_readCPU(rewindPC + 1);
            sprintf(parameterString, "$%02X,X", zpAddr);
            firstParam = bus_readCPU(rewindPC + 1);
            paramCount = 1;
            sprintf(redirectString, "");
            break;
        }
        case ADM_ZP_Y:
        {
            rewindPC -= 1;
            opcode = bus_readCPU(rewindPC);
            uint8_t zpAddr = bus_readCPU(rewindPC + 1);
            sprintf(parameterString, "$%02X,Y", zpAddr);
            firstParam = bus_readCPU(rewindPC + 1);
            paramCount = 1;
            sprintf(redirectString, "");
            break;
        }
        case ADM_ZP_INDIRECT_X:
        {
            rewindPC -= 1;
            opcode = bus_readCPU(rewindPC);
            uint8_t zpAddr = bus_readCPU(rewindPC + 1);
            sprintf(parameterString, "($%02X,X)", zpAddr);
            firstParam = bus_readCPU(rewindPC + 1);
            paramCount = 1;
            sprintf(redirectString, "@ %02X = %04X = %02X", (zpAddr + reg_x) % 256, address, bus_readCPU(address));
            break;
        }
        case ADM_ZP_INDIRECT_Y:
        {
            rewindPC -= 1;
            opcode = bus_readCPU(rewindPC);
            uint8_t zpAddr = bus_readCPU(rewindPC + 1);
            sprintf(parameterString, "($%02X),Y", zpAddr);
            firstParam = bus_readCPU(rewindPC + 1);
            paramCount = 1;
            sprintf(redirectString, "= %04X @ %04X = %02X", (((uint16_t) bus_readCPU(zpAddr + 1)) << 8) | ((uint16_t) bus_readCPU(zpAddr)), address, bus_readCPU(address));
            break;
        }
    }

    exc_trace(paramCount, redirectString, address, rewindPC, instructionCode, opcode, firstParam, secondParam, parameterString, offsetAddress, reg_accumulator, reg_status, reg_x, reg_y, stackPointer, cycles, 0, 0);
}

void cpu_branchHelper(bool desiredResult, enum CPUStatusFlag flag) {
    if (cpu_getFlag(flag) == desiredResult) {
        int8_t offset = (bus_readCPU(reg_pc));
        // cycles += 2;
        reg_pc += offset;
        reg_pc += 1;
    } else {
        reg_pc += 1;
    }
}

char* cpu_getInstructionString(enum Instruction instruction) {
    switch (instruction) {
        case LDA: return "LDA";
        case LDX: return "LDX";
        case LDY: return "LDY";
        case STA: return "STA";
        case STX: return "STX";
        case STY: return "STY";
        case ADC: return "ADC";
        case SBC: return "SBC";
        case INC: return "INC";
        case INX: return "INX";
        case INY: return "INY";
        case DEC: return "DEC";
        case DEX: return "DEX";
        case DEY: return "DEY";
        case ASL: return "ASL";
        case LSR: return "LSR";
        case ROL: return "ROL";
        case ROR: return "ROR";
        case AND: return "AND";
        case ORA: return "ORA";
        case EOR: return "EOR";
        case CMP: return "CMP";
        case CPX: return "CPX";
        case CPY: return "CPY";
        case BIT: return "BIT";
        case BCC: return "BCC";
        case BCS: return "BCS";
        case BNE: return "BNE";
        case BEQ: return "BEQ";
        case BPL: return "BPL";
        case BMI: return "BMI";
        case BVC: return "BVC";
        case BVS: return "BVS";
        case TAX: return "TAX";
        case TXA: return "TXA";
        case TAY: return "TAY";
        case TYA: return "TYA";
        case TSX: return "TSX";
        case TXS: return "TXS";
        case PLA: return "PLA";
        case PHA: return "PHA";
        case PHP: return "PHP";
        case PLP: return "PLP";
        case JMP: return "JMP";
        case JSR: return "JSR";
        case RTS: return "RTS";
        case RTI: return "RTI";
        case CLC: return "CLC";
        case SEC: return "SEC";
        case CLD: return "CLD";
        case SED: return "SED";
        case CLI: return "CLI";
        case SEI: return "SEI";
        case CLV: return "CLV";
        case BRK: return "BRK";
        case NOP: return "NOP";
        case IL_ALR: return "*ALR";
        case IL_ANC: return "*ANC";
        case IL_ANE: return "*ANE";
        case IL_ARR: return "*ARR";
        case IL_DCP: return "*DCP";
        case IL_ISC: return "*ISC";
        case IL_LAS: return "*LAS";
        case IL_LAX: return "*LAX";
        case IL_LXA: return "*LXA";
        case IL_RLA: return "*RLA";
        case IL_RRA: return "*RRA";
        case IL_SAX: return "*SAX";
        case IL_SBX: return "*SBX";
        case IL_SHA: return "*SHA";
        case IL_SHX: return "*SHX";
        case IL_SHY: return "*SHY";
        case IL_SLO: return "*SLO";
        case IL_SRE: return "*SRE";
        case IL_TAS: return "*TAS";
        case IL_SBC: return "*SBC";
        case IL_JAM: return "*JAM";
        case IL_NOP: return "*NOP";
    }
    return "???";
}
