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

uint64_t cycles = 0;

bool accumulatorAddrMode = false;
bool didPanic = false;
bool generateTrace = false;

void cpu_init() {
    cpu_reset();
    
    enum Instruction instruction = NOP;
    
    bus_startTimeMonitor();
    while (instruction != BRK && reg_pc != 0xFFFD && !didPanic) {
        uint64_t oldCycles = cycles;
        instruction = cpu_execute();
        bus_cpuReport(cycles - oldCycles);
    }

    bus_endTimeMonitor();
}

void cpu_vblankNMI() {
    cpu_stackPush16(reg_pc);
    cpu_stackPush(reg_status);
    cpu_setFlag(CPUSTAT_NO_INTRPT, true);
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
        }
        case ADM_IMPLIED:
        {
            return 0x0000;
        }
        case ADM_IMMEDIATE:
        {
            reg_pc += 1;
            return reg_pc - 1;
        }
        case ADM_ABSOLUTE:
        {
            uint16_t address = bus_readCPU(reg_pc);
            reg_pc += 1;
            address = (((uint16_t) bus_readCPU(reg_pc)) << 8) | address;
            reg_pc += 1;
            return address;
        }
        case ADM_ZEROPAGE:
        {
            uint16_t address = bus_readCPU(reg_pc);
            reg_pc += 1;
            address %= 256;
            return address;
        }
        case ADM_RELATIVE:
        {
            return 0x0000;
        }
        case ADM_ABS_INDIRECT:
        {
            uint16_t initlow = bus_readCPU(reg_pc);
            reg_pc += 1;
            uint16_t inithigh = bus_readCPU(reg_pc);
            reg_pc += 1;
            inithigh = inithigh << 8;

            uint16_t low = (inithigh | initlow);
            uint16_t high = (inithigh | ((initlow + 1) % 256));
            low = bus_readCPU(low);
            high = bus_readCPU(high);
            high = high << 8;
            uint16_t address = (high | low);
            return address;
        }
        case ADM_ABS_X:
        {
            uint16_t address = bus_readCPU(reg_pc);
            reg_pc += 1;
            address = (bus_readCPU(reg_pc) << 8) | address;
            reg_pc += 1;
            address += reg_x;
            return address;
        }
        case ADM_ABS_Y:
        {
            uint16_t address = bus_readCPU(reg_pc);
            reg_pc += 1;
            address = (bus_readCPU(reg_pc) << 8) | address;
            reg_pc += 1;
            address += reg_y;
            return address;
        }
        case ADM_ZP_X:
        {
            uint16_t address = bus_readCPU(reg_pc);
            reg_pc += 1;
            address += reg_x;
            address %= 256;
            return address;
        }
        case ADM_ZP_Y:
        {
            uint16_t address = bus_readCPU(reg_pc);
            reg_pc += 1;
            address += reg_y;
            address %= 256;
            return address;
        }
        case ADM_ZP_INDIRECT_X:
        {
            uint16_t address = bus_readCPU(reg_pc);
            reg_pc += 1;
            address += reg_x;
            address %= 256;

            uint16_t low = bus_readCPU(address);
            uint16_t high = bus_readCPU((address + 1) % 256);

            high = high << 8;
            address = (high | low);

            return address;

        }
        case ADM_ZP_INDIRECT_Y:
        {
            uint16_t address = bus_readCPU(reg_pc);
            reg_pc += 1;

            uint16_t low = bus_readCPU(address);
            uint16_t high = bus_readCPU((address + 1) % 256);

            high = high << 8;
            address = (high | low);
            address += reg_y;

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
    stackPointer = 0xFD;
}

void cpu_stackPush(uint8_t val) {
    if (stackPointer == 0x00) {
        exc_panic_stackOverflow();
    } else {
        bus_writeCPU(0x100 + stackPointer, val);
        stackPointer -= 1;
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

uint8_t cpu_instruction(enum Instruction instruction, enum AddressingMode mode) {
    uint16_t address = cpu_fetchAddress(mode);
    
    if (generateTrace) cpu_trace(instruction, mode, address);
    
    switch (instruction) {
        case LDA: return cpu_lda(address);
        case LDX: return cpu_ldx(address);
        case LDY: return cpu_ldy(address);
        case STA: return cpu_sta(address);
        case STX: return cpu_stx(address);
        case STY: return cpu_sty(address);
        case ADC: return cpu_adc(address);
        case SBC: return cpu_sbc(address);
        case INC: return cpu_inc(address);
        case INX: return cpu_inx();
        case INY: return cpu_iny();
        case DEC: return cpu_dec(address);
        case DEX: return cpu_dex();
        case DEY: return cpu_dey();
        case ASL: return cpu_asl(address);
        case LSR: return cpu_lsr(address);
        case ROL: return cpu_rol(address);
        case ROR: return cpu_ror(address);
        case AND: return cpu_and(address);
        case ORA: return cpu_ora(address);
        case EOR: return cpu_eor(address);
        case CMP: return cpu_cmp(address);
        case CPX: return cpu_cpx(address);
        case CPY: return cpu_cpy(address);
        case BIT: return cpu_bit(address);
        case BCC: return cpu_bcc();
        case BCS: return cpu_bcs();
        case BNE: return cpu_bne();
        case BEQ: return cpu_beq();
        case BPL: return cpu_bpl();
        case BMI: return cpu_bmi();
        case BVC: return cpu_bvc();
        case BVS: return cpu_bvs();
        case TAX: return cpu_tax();
        case TXA: return cpu_txa();
        case TAY: return cpu_tay();
        case TYA: return cpu_tya();
        case TSX: return cpu_tsx();
        case TXS: return cpu_txs();
        case PLA: return cpu_pla();
        case PHA: return cpu_pha();
        case PHP: return cpu_php();
        case PLP: return cpu_plp();
        case JMP: return cpu_jmp(address);
        case JSR: return cpu_jsr(address);
        case RTS: return cpu_rts();
        case RTI: return cpu_rti();
        case CLC: return cpu_clc();
        case SEC: return cpu_sec();
        case CLD: return cpu_cld();
        case SED: return cpu_sed();
        case CLI: return cpu_cli();
        case SEI: return cpu_sei();
        case CLV: return cpu_clv();
        case BRK: return cpu_brk();
        case NOP: return cpu_nop();
        case IL_ALR: return cpu_illegal_alr(address);
        case IL_ANC: return cpu_illegal_anc(address);
        case IL_ANE: return cpu_illegal_ane(address);
        case IL_ARR: return cpu_illegal_arr(address);
        case IL_DCP: return cpu_illegal_dcp(address);
        case IL_ISC: return cpu_illegal_isc(address);
        case IL_LAS: return cpu_illegal_las(address);
        case IL_LAX: return cpu_illegal_lax(address);
        case IL_LXA: return cpu_illegal_lxa(address);
        case IL_RLA: return cpu_illegal_rla(address);
        case IL_RRA: return cpu_illegal_rra(address);
        case IL_SAX: return cpu_illegal_sax(address);
        case IL_SBX: return cpu_illegal_sbx(address);
        case IL_SHA: return cpu_illegal_sha(address);
        case IL_SHX: return cpu_illegal_shx(address);
        case IL_SHY: return cpu_illegal_shy(address);
        case IL_SLO: return cpu_illegal_slo(address);
        case IL_SRE: return cpu_illegal_sre(address);
        case IL_TAS: return cpu_illegal_tas(address);
        case IL_SBC: return cpu_illegal_sbc(address);
        case IL_JAM: return cpu_illegal_jam();
        case IL_NOP: return cpu_illegal_nop(address);
    }
    return NOP;
}

uint8_t cpu_execute() {

    uint8_t opcode = bus_readCPU(reg_pc);
    reg_pc += 1;
    
    switch (opcode) {
        // LDA
        case 0xAD: cycles += 4; return cpu_instruction(LDA, ADM_ABSOLUTE);
        case 0xBD: cycles += 4; return cpu_instruction(LDA, ADM_ABS_X);
        case 0xB9: cycles += 4; return cpu_instruction(LDA, ADM_ABS_Y);
        case 0xA9: cycles += 2; return cpu_instruction(LDA, ADM_IMMEDIATE);
        case 0xA5: cycles += 3; return cpu_instruction(LDA, ADM_ZEROPAGE);
        case 0xA1: cycles += 6; return cpu_instruction(LDA, ADM_ZP_INDIRECT_X);
        case 0xB5: cycles += 4; return cpu_instruction(LDA, ADM_ZP_X);
        case 0xB1: cycles += 5; return cpu_instruction(LDA, ADM_ZP_INDIRECT_Y);
        // LDX
        case 0xAE: cycles += 4; return cpu_instruction(LDX, ADM_ABSOLUTE);
        case 0xBE: cycles += 4; return cpu_instruction(LDX, ADM_ABS_Y);
        case 0xA2: cycles += 2; return cpu_instruction(LDX, ADM_IMMEDIATE);
        case 0xA6: cycles += 3; return cpu_instruction(LDX, ADM_ZEROPAGE);
        case 0xB6: cycles += 4; return cpu_instruction(LDX, ADM_ZP_Y);
        // LDY
        case 0xAC: cycles += 4; return cpu_instruction(LDY, ADM_ABSOLUTE);
        case 0xBC: cycles += 4; return cpu_instruction(LDY, ADM_ABS_X);
        case 0xA0: cycles += 2; return cpu_instruction(LDY, ADM_IMMEDIATE);
        case 0xA4: cycles += 3; return cpu_instruction(LDY, ADM_ZEROPAGE);
        case 0xB4: cycles += 4; return cpu_instruction(LDY, ADM_ZP_X);
        // STA
        case 0x8D: cycles += 4; return cpu_instruction(STA, ADM_ABSOLUTE);
        case 0x9D: cycles += 5; return cpu_instruction(STA, ADM_ABS_X);
        case 0x99: cycles += 5; return cpu_instruction(STA, ADM_ABS_Y);
        case 0x85: cycles += 3; return cpu_instruction(STA, ADM_ZEROPAGE);
        case 0x81: cycles += 6; return cpu_instruction(STA, ADM_ZP_INDIRECT_X);
        case 0x95: cycles += 4; return cpu_instruction(STA, ADM_ZP_X);
        case 0x91: cycles += 6; return cpu_instruction(STA, ADM_ZP_INDIRECT_Y);
        // STX
        case 0x8E: cycles += 4; return cpu_instruction(STX, ADM_ABSOLUTE);
        case 0x86: cycles += 3; return cpu_instruction(STX, ADM_ZEROPAGE);
        case 0x96: cycles += 4; return cpu_instruction(STX, ADM_ZP_Y);
        // STY
        case 0x8C: cycles += 4; return cpu_instruction(STY, ADM_ABSOLUTE);
        case 0x84: cycles += 3; return cpu_instruction(STY, ADM_ZEROPAGE);
        case 0x94: cycles += 4; return cpu_instruction(STY, ADM_ZP_X);
        // ADC
        case 0x6D: cycles += 4; return cpu_instruction(ADC, ADM_ABSOLUTE);
        case 0x7D: cycles += 4; return cpu_instruction(ADC, ADM_ABS_X);
        case 0x79: cycles += 4; return cpu_instruction(ADC, ADM_ABS_Y);
        case 0x69: cycles += 2; return cpu_instruction(ADC, ADM_IMMEDIATE);
        case 0x65: cycles += 3; return cpu_instruction(ADC, ADM_ZEROPAGE);
        case 0x61: cycles += 6; return cpu_instruction(ADC, ADM_ZP_INDIRECT_X);
        case 0x75: cycles += 4; return cpu_instruction(ADC, ADM_ZP_X);
        case 0x71: cycles += 5; return cpu_instruction(ADC, ADM_ZP_INDIRECT_Y);
        // SBC
        case 0xED: cycles += 4; return cpu_instruction(SBC, ADM_ABSOLUTE);
        case 0xFD: cycles += 4; return cpu_instruction(SBC, ADM_ABS_X);
        case 0xF9: cycles += 4; return cpu_instruction(SBC, ADM_ABS_Y);
        case 0xE9: cycles += 2; return cpu_instruction(SBC, ADM_IMMEDIATE);
        case 0xE5: cycles += 3; return cpu_instruction(SBC, ADM_ZEROPAGE);
        case 0xE1: cycles += 6; return cpu_instruction(SBC, ADM_ZP_INDIRECT_X);
        case 0xF5: cycles += 4; return cpu_instruction(SBC, ADM_ZP_X);
        case 0xF1: cycles += 5; return cpu_instruction(SBC, ADM_ZP_INDIRECT_Y);
        // INC
        case 0xEE: cycles += 6; return cpu_instruction(INC, ADM_ABSOLUTE);
        case 0xFE: cycles += 7; return cpu_instruction(INC, ADM_ABS_X);
        case 0xE6: cycles += 5; return cpu_instruction(INC, ADM_ZEROPAGE);
        case 0xF6: cycles += 6; return cpu_instruction(INC, ADM_ZP_X);
        // INX
        case 0xE8: cycles += 2; return cpu_instruction(INX, ADM_IMPLIED);
        // INY
        case 0xC8: cycles += 2; return cpu_instruction(INY, ADM_IMPLIED);
        // DEC
        case 0xCE: cycles += 6; return cpu_instruction(DEC, ADM_ABSOLUTE);
        case 0xDE: cycles += 7; return cpu_instruction(DEC, ADM_ABS_X);
        case 0xC6: cycles += 5; return cpu_instruction(DEC, ADM_ZEROPAGE);
        case 0xD6: cycles += 6; return cpu_instruction(DEC, ADM_ZP_X);
        // DEX
        case 0xCA: cycles += 2; return cpu_instruction(DEX, ADM_IMPLIED);
        // DEY
        case 0x88: cycles += 2; return cpu_instruction(DEY, ADM_IMPLIED);
        // ASL
        case 0x0E: cycles += 6; return cpu_instruction(ASL, ADM_ABSOLUTE);
        case 0x1E: cycles += 7; return cpu_instruction(ASL, ADM_ABS_X);
        case 0x0A: cycles += 2; return cpu_instruction(ASL, ADM_ACCUMULATOR);
        case 0x06: cycles += 5; return cpu_instruction(ASL, ADM_ZEROPAGE);
        case 0x16: cycles += 6; return cpu_instruction(ASL, ADM_ZP_X);
        // LSR
        case 0x4E: cycles += 6; return cpu_instruction(LSR, ADM_ABSOLUTE);
        case 0x5E: cycles += 7; return cpu_instruction(LSR, ADM_ABS_X);
        case 0x4A: cycles += 2; return cpu_instruction(LSR, ADM_ACCUMULATOR);
        case 0x46: cycles += 5; return cpu_instruction(LSR, ADM_ZEROPAGE);
        case 0x56: cycles += 6; return cpu_instruction(LSR, ADM_ZP_X);
        // ROL
        case 0x2E: cycles += 6; return cpu_instruction(ROL, ADM_ABSOLUTE);
        case 0x3E: cycles += 7; return cpu_instruction(ROL, ADM_ABS_X);
        case 0x2A: cycles += 2; return cpu_instruction(ROL, ADM_ACCUMULATOR);
        case 0x26: cycles += 5; return cpu_instruction(ROL, ADM_ZEROPAGE);
        case 0x36: cycles += 6; return cpu_instruction(ROL, ADM_ZP_X);
        // ROR
        case 0x6E: cycles += 6; return cpu_instruction(ROR, ADM_ABSOLUTE);
        case 0x7E: cycles += 7; return cpu_instruction(ROR, ADM_ABS_X);
        case 0x6A: cycles += 2; return cpu_instruction(ROR, ADM_ACCUMULATOR);
        case 0x66: cycles += 5; return cpu_instruction(ROR, ADM_ZEROPAGE);
        case 0x76: cycles += 6; return cpu_instruction(ROR, ADM_ZP_X);
        // AND
        case 0x2D: cycles += 4; return cpu_instruction(AND, ADM_ABSOLUTE);
        case 0x3D: cycles += 4; return cpu_instruction(AND, ADM_ABS_X);
        case 0x39: cycles += 4; return cpu_instruction(AND, ADM_ABS_Y);
        case 0x29: cycles += 2; return cpu_instruction(AND, ADM_IMMEDIATE);
        case 0x25: cycles += 3; return cpu_instruction(AND, ADM_ZEROPAGE);
        case 0x21: cycles += 6; return cpu_instruction(AND, ADM_ZP_INDIRECT_X);
        case 0x35: cycles += 4; return cpu_instruction(AND, ADM_ZP_X);
        case 0x31: cycles += 5; return cpu_instruction(AND, ADM_ZP_INDIRECT_Y);
        // ORA
        case 0x0D: cycles += 4; return cpu_instruction(ORA, ADM_ABSOLUTE);
        case 0x1D: cycles += 4; return cpu_instruction(ORA, ADM_ABS_X);
        case 0x19: cycles += 4; return cpu_instruction(ORA, ADM_ABS_Y);
        case 0x09: cycles += 2; return cpu_instruction(ORA, ADM_IMMEDIATE);
        case 0x05: cycles += 3; return cpu_instruction(ORA, ADM_ZEROPAGE);
        case 0x01: cycles += 6; return cpu_instruction(ORA, ADM_ZP_INDIRECT_X);
        case 0x15: cycles += 4; return cpu_instruction(ORA, ADM_ZP_X);
        case 0x11: cycles += 5; return cpu_instruction(ORA, ADM_ZP_INDIRECT_Y);
        // EOR
        case 0x4D: cycles += 4; return cpu_instruction(EOR, ADM_ABSOLUTE);
        case 0x5D: cycles += 4; return cpu_instruction(EOR, ADM_ABS_X);
        case 0x59: cycles += 4; return cpu_instruction(EOR, ADM_ABS_Y);
        case 0x49: cycles += 2; return cpu_instruction(EOR, ADM_IMMEDIATE);
        case 0x45: cycles += 3; return cpu_instruction(EOR, ADM_ZEROPAGE);
        case 0x41: cycles += 6; return cpu_instruction(EOR, ADM_ZP_INDIRECT_X);
        case 0x55: cycles += 4; return cpu_instruction(EOR, ADM_ZP_X);
        case 0x51: cycles += 5; return cpu_instruction(EOR, ADM_ZP_INDIRECT_Y);
        // CMP
        case 0xCD: cycles += 4; return cpu_instruction(CMP, ADM_ABSOLUTE);
        case 0xDD: cycles += 4; return cpu_instruction(CMP, ADM_ABS_X);
        case 0xD9: cycles += 4; return cpu_instruction(CMP, ADM_ABS_Y);
        case 0xC9: cycles += 2; return cpu_instruction(CMP, ADM_IMMEDIATE);
        case 0xC5: cycles += 3; return cpu_instruction(CMP, ADM_ZEROPAGE);
        case 0xC1: cycles += 6; return cpu_instruction(CMP, ADM_ZP_INDIRECT_X);
        case 0xD5: cycles += 4; return cpu_instruction(CMP, ADM_ZP_X);
        case 0xD1: cycles += 5; return cpu_instruction(CMP, ADM_ZP_INDIRECT_Y);
        // CPX
        case 0xEC: cycles += 4; return cpu_instruction(CPX, ADM_ABSOLUTE);
        case 0xE0: cycles += 2; return cpu_instruction(CPX, ADM_IMMEDIATE);
        case 0xE4: cycles += 3; return cpu_instruction(CPX, ADM_ZEROPAGE);
        // CPY
        case 0xCC: cycles += 4; return cpu_instruction(CPY, ADM_ABSOLUTE);
        case 0xC0: cycles += 2; return cpu_instruction(CPY, ADM_IMMEDIATE);
        case 0xC4: cycles += 3; return cpu_instruction(CPY, ADM_ZEROPAGE);
        // BIT
        case 0x2C: cycles += 4; return cpu_instruction(BIT, ADM_ABSOLUTE);
        case 0x89: cycles += 2; return cpu_instruction(BIT, ADM_IMMEDIATE);
        case 0x24: cycles += 3; return cpu_instruction(BIT, ADM_ZEROPAGE);
        // BCC
        case 0x90: cycles += 2; return cpu_instruction(BCC, ADM_RELATIVE);
        // BCS
        case 0xB0: cycles += 2; return cpu_instruction(BCS, ADM_RELATIVE);
        // BNE
        case 0xD0: cycles += 2; return cpu_instruction(BNE, ADM_RELATIVE);
        // BEQ
        case 0xF0: cycles += 2; return cpu_instruction(BEQ, ADM_RELATIVE);
        // BPL
        case 0x10: cycles += 2; return cpu_instruction(BPL, ADM_RELATIVE);
        // BMI
        case 0x30: cycles += 2; return cpu_instruction(BMI, ADM_RELATIVE);
        // BVC
        case 0x50: cycles += 2; return cpu_instruction(BVC, ADM_RELATIVE);
        // BVS
        case 0x70: cycles += 2; return cpu_instruction(BVS, ADM_RELATIVE);
        // TAX
        case 0xAA: cycles += 2; return cpu_instruction(TAX, ADM_IMPLIED);
        // TXA
        case 0x8A: cycles += 2; return cpu_instruction(TXA, ADM_IMPLIED);
        // TAY
        case 0xA8: cycles += 2; return cpu_instruction(TAY, ADM_IMPLIED);
        // TYA
        case 0x98: cycles += 2; return cpu_instruction(TYA, ADM_IMPLIED);
        // TSX
        case 0xBA: cycles += 2; return cpu_instruction(TSX, ADM_IMPLIED);
        // TXS
        case 0x9A: cycles += 2; return cpu_instruction(TXS, ADM_IMPLIED);
        // PHA
        case 0x48: cycles += 3; return cpu_instruction(PHA, ADM_IMPLIED);
        // PLA
        case 0x68: cycles += 4; return cpu_instruction(PLA, ADM_IMPLIED);
        // PHP
        case 0x08: cycles += 3; return cpu_instruction(PHP, ADM_IMPLIED);
        // PLP
        case 0x28: cycles += 4; return cpu_instruction(PLP, ADM_IMPLIED);
        // JMP
        case 0x4C: cycles += 3; return cpu_instruction(JMP, ADM_ABSOLUTE);
        case 0x6C: cycles += 5; return cpu_instruction(JMP, ADM_ABS_INDIRECT);
        // JSR
        case 0x20: cycles += 6; return cpu_instruction(JSR, ADM_ABSOLUTE);
        // RTS
        case 0x60: cycles += 6; return cpu_instruction(RTS, ADM_IMPLIED);
        // RTI
        case 0x40: cycles += 6; return cpu_instruction(RTI, ADM_IMPLIED);
        // CLC
        case 0x18: cycles += 2; return cpu_instruction(CLC, ADM_IMPLIED);
        // SEC
        case 0x38: cycles += 2; return cpu_instruction(SEC, ADM_IMPLIED);
        // CLD
        case 0xD8: cycles += 2; return cpu_instruction(CLD, ADM_IMPLIED);
        // SED
        case 0xF8: cycles += 2; return cpu_instruction(SED, ADM_IMPLIED);
        // CLI
        case 0x58: cycles += 2; return cpu_instruction(CLI, ADM_IMPLIED);
        // SEI
        case 0x78: cycles += 2; return cpu_instruction(SEI, ADM_IMPLIED);
        // CLV
        case 0xB8: cycles += 2; return cpu_instruction(CLV, ADM_IMPLIED);
        // BRK
        case 0x00: cycles += 7; return cpu_instruction(BRK, ADM_IMPLIED);
        // NOP
        case 0xEA: cycles += 2; return cpu_instruction(NOP, ADM_IMPLIED);
        // (ILLEGAL) ALR
        case 0x4B: cycles += 2; return cpu_instruction(IL_ALR, ADM_IMMEDIATE);
        // (ILLEGAL) ANC
        case 0x0B: cycles += 2; return cpu_instruction(IL_ANC, ADM_IMMEDIATE);
        case 0x2B: cycles += 2; return cpu_instruction(IL_ANC, ADM_IMMEDIATE);
        // (ILLEGAL) ANE
        case 0x8B: cycles += 2; return cpu_instruction(IL_ANE, ADM_IMMEDIATE);
        // (ILLEGAL) ARR
        case 0x6B: cycles += 2; return cpu_instruction(IL_ARR, ADM_IMMEDIATE);
        // (ILLEGAL) DCP
        case 0xC7: cycles += 5; return cpu_instruction(IL_DCP, ADM_ZEROPAGE);
        case 0xD7: cycles += 6; return cpu_instruction(IL_DCP, ADM_ZP_X);
        case 0xCF: cycles += 6; return cpu_instruction(IL_DCP, ADM_ABSOLUTE);
        case 0xDF: cycles += 7; return cpu_instruction(IL_DCP, ADM_ABS_X);
        case 0xDB: cycles += 7; return cpu_instruction(IL_DCP, ADM_ABS_Y);
        case 0xC3: cycles += 8; return cpu_instruction(IL_DCP, ADM_ZP_INDIRECT_X);
        case 0xD3: cycles += 8; return cpu_instruction(IL_DCP, ADM_ZP_INDIRECT_Y);
        // (ILLEGAL) ISC
        case 0xE7: cycles += 5; return cpu_instruction(IL_ISC, ADM_ZEROPAGE);
        case 0xF7: cycles += 6; return cpu_instruction(IL_ISC, ADM_ZP_X);
        case 0xEF: cycles += 6; return cpu_instruction(IL_ISC, ADM_ABSOLUTE);
        case 0xFF: cycles += 7; return cpu_instruction(IL_ISC, ADM_ABS_X);
        case 0xFB: cycles += 7; return cpu_instruction(IL_ISC, ADM_ABS_Y);
        case 0xE3: cycles += 8; return cpu_instruction(IL_ISC, ADM_ZP_INDIRECT_X);
        case 0xF3: cycles += 4; return cpu_instruction(IL_ISC, ADM_ZP_INDIRECT_Y);
        // (ILLEGAL) LAS
        case 0xBB: cycles += 4; return cpu_instruction(IL_LAS, ADM_ABS_Y);
        // (ILLEGAL) LAX
        case 0xA7: cycles += 3; return cpu_instruction(IL_LAX, ADM_ZEROPAGE);
        case 0xB7: cycles += 4; return cpu_instruction(IL_LAX, ADM_ZP_Y);
        case 0xAF: cycles += 4; return cpu_instruction(IL_LAX, ADM_ABSOLUTE);
        case 0xBF: cycles += 4; return cpu_instruction(IL_LAX, ADM_ABS_Y);
        case 0xA3: cycles += 6; return cpu_instruction(IL_LAX, ADM_ZP_INDIRECT_X);
        case 0xB3: cycles += 5; return cpu_instruction(IL_LAX, ADM_ZP_INDIRECT_Y);
        // (ILLEGAL) LXA
        case 0xAB: cycles += 2; return cpu_instruction(IL_LXA, ADM_IMMEDIATE);
        // (ILLEGAL) RLA
        case 0x27: cycles += 5; return cpu_instruction(IL_RLA, ADM_ZEROPAGE);
        case 0x37: cycles += 6; return cpu_instruction(IL_RLA, ADM_ZP_X);
        case 0x2F: cycles += 6; return cpu_instruction(IL_RLA, ADM_ABSOLUTE);
        case 0x3F: cycles += 7; return cpu_instruction(IL_RLA, ADM_ABS_X);
        case 0x3B: cycles += 7; return cpu_instruction(IL_RLA, ADM_ABS_Y);
        case 0x23: cycles += 8; return cpu_instruction(IL_RLA, ADM_ZP_INDIRECT_X);
        case 0x33: cycles += 8; return cpu_instruction(IL_RLA, ADM_ZP_INDIRECT_Y);
        // (ILLEGAL) RRA
        case 0x67: cycles += 5; return cpu_instruction(IL_RRA, ADM_ZEROPAGE);
        case 0x77: cycles += 6; return cpu_instruction(IL_RRA, ADM_ZP_X);
        case 0x6F: cycles += 6; return cpu_instruction(IL_RRA, ADM_ABSOLUTE);
        case 0x7F: cycles += 7; return cpu_instruction(IL_RRA, ADM_ABS_X);
        case 0x7B: cycles += 7; return cpu_instruction(IL_RRA, ADM_ABS_Y);
        case 0x63: cycles += 8; return cpu_instruction(IL_RRA, ADM_ZP_INDIRECT_X);
        case 0x73: cycles += 8; return cpu_instruction(IL_RRA, ADM_ZP_INDIRECT_Y);
        // (ILLEGAL) SAX
        case 0x87: cycles += 3; return cpu_instruction(IL_SAX, ADM_ZEROPAGE);
        case 0x97: cycles += 4; return cpu_instruction(IL_SAX, ADM_ZP_Y);
        case 0x8F: cycles += 4; return cpu_instruction(IL_SAX, ADM_ABSOLUTE);
        case 0x83: cycles += 6; return cpu_instruction(IL_SAX, ADM_ZP_INDIRECT_X);
        // (ILLEGAL) SBX
        case 0xCB: cycles += 2; return cpu_instruction(IL_SBX, ADM_IMMEDIATE);
        // (ILLEGAL) SHA
        case 0x9F: cycles += 5; return cpu_instruction(IL_SHA, ADM_ABS_Y);
        case 0x93: cycles += 6; return cpu_instruction(IL_SHA, ADM_ZP_INDIRECT_Y);
        // (ILLEGAL) SHX
        case 0x9E: cycles += 5; return cpu_instruction(IL_SHX, ADM_ABS_Y);
        // (ILLEGAL) SHY
        case 0x9C: cycles += 5; return cpu_instruction(IL_SHY, ADM_ABS_Y);
        // (ILLEGAL) SLO
        case 0x07: cycles += 5; return cpu_instruction(IL_SLO, ADM_ZEROPAGE);
        case 0x17: cycles += 6; return cpu_instruction(IL_SLO, ADM_ZP_X);
        case 0x0F: cycles += 6; return cpu_instruction(IL_SLO, ADM_ABSOLUTE);
        case 0x1F: cycles += 7; return cpu_instruction(IL_SLO, ADM_ABS_X);
        case 0x1B: cycles += 7; return cpu_instruction(IL_SLO, ADM_ABS_Y);
        case 0x03: cycles += 8; return cpu_instruction(IL_SLO, ADM_ZP_INDIRECT_X);
        case 0x13: cycles += 8; return cpu_instruction(IL_SLO, ADM_ZP_INDIRECT_Y);
        // (ILLEGAL) SRE
        case 0x47: cycles += 5; return cpu_instruction(IL_SRE, ADM_ZEROPAGE);
        case 0x57: cycles += 6; return cpu_instruction(IL_SRE, ADM_ZP_X);
        case 0x4F: cycles += 6; return cpu_instruction(IL_SRE, ADM_ABSOLUTE);
        case 0x5F: cycles += 7; return cpu_instruction(IL_SRE, ADM_ABS_X);
        case 0x5B: cycles += 7; return cpu_instruction(IL_SRE, ADM_ABS_Y);
        case 0x43: cycles += 8; return cpu_instruction(IL_SRE, ADM_ZP_INDIRECT_X);
        case 0x53: cycles += 8; return cpu_instruction(IL_SRE, ADM_ZP_INDIRECT_Y);
        // (ILLEGAL) TAS
        case 0x9B: cycles += 5; return cpu_instruction(IL_TAS, ADM_ABS_Y);
        // (ILLEGAL) SBC
        case 0xEB: cycles += 2; return cpu_instruction(IL_SBC, ADM_IMMEDIATE);
        // (ILLEGAL) NOP
        case 0x1A: cycles += 2; return cpu_instruction(IL_NOP, ADM_IMPLIED);
        case 0x3A: cycles += 2; return cpu_instruction(IL_NOP, ADM_IMPLIED);
        case 0x5A: cycles += 2; return cpu_instruction(IL_NOP, ADM_IMPLIED);
        case 0x7A: cycles += 2; return cpu_instruction(IL_NOP, ADM_IMPLIED);
        case 0xDA: cycles += 2; return cpu_instruction(IL_NOP, ADM_IMPLIED);
        case 0xFA: cycles += 2; return cpu_instruction(IL_NOP, ADM_IMPLIED);
        case 0x80: cycles += 2; return cpu_instruction(IL_NOP, ADM_IMMEDIATE);
        case 0x82: cycles += 2; return cpu_instruction(IL_NOP, ADM_IMMEDIATE);
        case 0xC2: cycles += 2; return cpu_instruction(IL_NOP, ADM_IMMEDIATE);
        case 0xE2: cycles += 2; return cpu_instruction(IL_NOP, ADM_IMMEDIATE);
        case 0x04: cycles += 3; return cpu_instruction(IL_NOP, ADM_ZEROPAGE);
        case 0x44: cycles += 3; return cpu_instruction(IL_NOP, ADM_ZEROPAGE);
        case 0x64: cycles += 3; return cpu_instruction(IL_NOP, ADM_ZEROPAGE);
        case 0x14: cycles += 4; return cpu_instruction(IL_NOP, ADM_ZP_X);
        case 0x34: cycles += 4; return cpu_instruction(IL_NOP, ADM_ZP_X);
        case 0x54: cycles += 4; return cpu_instruction(IL_NOP, ADM_ZP_X);
        case 0x74: cycles += 4; return cpu_instruction(IL_NOP, ADM_ZP_X);
        case 0xD4: cycles += 4; return cpu_instruction(IL_NOP, ADM_ZP_X);
        case 0xF4: cycles += 4; return cpu_instruction(IL_NOP, ADM_ZP_X);
        case 0x0C: cycles += 4; return cpu_instruction(IL_NOP, ADM_ABSOLUTE);
        case 0x1C: cycles += 4; return cpu_instruction(IL_NOP, ADM_ABS_X);
        case 0x3C: cycles += 4; return cpu_instruction(IL_NOP, ADM_ABS_X);
        case 0x5C: cycles += 4; return cpu_instruction(IL_NOP, ADM_ABS_X);
        case 0x7C: cycles += 4; return cpu_instruction(IL_NOP, ADM_ABS_X);
        case 0xDC: cycles += 4; return cpu_instruction(IL_NOP, ADM_ABS_X);
        case 0xFC: cycles += 4; return cpu_instruction(IL_NOP, ADM_ABS_X);
        // JAM
        case 0x02: return cpu_instruction(IL_JAM, ADM_IMPLIED);
        case 0x12: return cpu_instruction(IL_JAM, ADM_IMPLIED);
        case 0x22: return cpu_instruction(IL_JAM, ADM_IMPLIED);
        case 0x32: return cpu_instruction(IL_JAM, ADM_IMPLIED);
        case 0x42: return cpu_instruction(IL_JAM, ADM_IMPLIED);
        case 0x52: return cpu_instruction(IL_JAM, ADM_IMPLIED);
        case 0x62: return cpu_instruction(IL_JAM, ADM_IMPLIED);
        case 0x72: return cpu_instruction(IL_JAM, ADM_IMPLIED);
        case 0x92: return cpu_instruction(IL_JAM, ADM_IMPLIED);
        case 0xB2: return cpu_instruction(IL_JAM, ADM_IMPLIED);
        case 0xD2: return cpu_instruction(IL_JAM, ADM_IMPLIED);
        case 0xF2: return cpu_instruction(IL_JAM, ADM_IMPLIED);
        
        // Other
        default: 
            exc_panic_illegalInstruction(opcode);
            break;
    }

    return 0;
}

uint8_t cpu_lda(uint16_t address) {
    reg_accumulator = bus_readCPU(address);
    cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
    return LDA;
}

uint8_t cpu_ldx(uint16_t address) {
    reg_x = bus_readCPU(address);
    cpu_setFlag(CPUSTAT_ZERO, reg_x == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_x & 0b10000000) != 0);
    return LDX;
}

uint8_t cpu_ldy(uint16_t address) {
    reg_y = bus_readCPU(address);
    cpu_setFlag(CPUSTAT_ZERO, reg_y == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_y & 0b10000000) != 0);
    return LDY;
}

uint8_t cpu_sta(uint16_t address) {
    bus_writeCPU(address, reg_accumulator);
    return STA;
}

uint8_t cpu_stx(uint16_t address) {
    bus_writeCPU(address, reg_x);
    return STX;
}

uint8_t cpu_sty(uint16_t address) {
    bus_writeCPU(address, reg_y);
    return STY;
}

uint8_t cpu_adc(uint16_t address) {
    uint8_t memoryVal = bus_readCPU(address);
    uint16_t sum = reg_accumulator + memoryVal + ((uint16_t) cpu_getFlag(CPUSTAT_CARRY));
    cpu_setFlag(CPUSTAT_CARRY, sum > 0xFF);
    cpu_setFlag(CPUSTAT_OVERFLOW, (reg_accumulator ^ sum) & (memoryVal ^ sum) & 0x80);
    reg_accumulator = (uint8_t) sum;
    cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
    return ADC;
}

uint8_t cpu_sbc(uint16_t address) {
    uint8_t memoryVal = ~bus_readCPU(address);
    uint16_t sum = reg_accumulator + memoryVal + ((uint16_t) cpu_getFlag(CPUSTAT_CARRY));
    cpu_setFlag(CPUSTAT_CARRY, sum > 0xFF);
    cpu_setFlag(CPUSTAT_OVERFLOW, (reg_accumulator ^ sum) & (memoryVal ^ sum) & 0x80);
    reg_accumulator = (uint8_t) sum;
    cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
    return SBC;
}

uint8_t cpu_inc(uint16_t address) {
    uint8_t val = bus_readCPU(address) + 1;
    bus_writeCPU(address, val);
    cpu_setFlag(CPUSTAT_ZERO, val == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (val & 0b10000000) != 0);
    return INC;
}

uint8_t cpu_inx() {
    reg_x += 1;
    cpu_setFlag(CPUSTAT_ZERO, reg_x == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_x & 0b10000000) != 0);
    return INX;
}

uint8_t cpu_iny() {
    reg_y += 1;
    cpu_setFlag(CPUSTAT_ZERO, reg_y == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_y & 0b10000000) != 0);
    return INY;
}

uint8_t cpu_dec(uint16_t address) {
    uint8_t val = bus_readCPU(address) - 1;
    bus_writeCPU(address, val);
    cpu_setFlag(CPUSTAT_ZERO, val == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (val & 0b10000000) != 0);
    return DEC;
}

uint8_t cpu_dex() {
    reg_x -= 1;    
    cpu_setFlag(CPUSTAT_ZERO, reg_x == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_x & 0b10000000) != 0);
    return DEX;
}

uint8_t cpu_dey() {
    reg_y -= 1;
    cpu_setFlag(CPUSTAT_ZERO, reg_y == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_y & 0b10000000) != 0);
    return DEY;
}

uint8_t cpu_asl(uint16_t address) {
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
    return ASL;
}

uint8_t cpu_lsr(uint16_t address) {
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
    return LSR;
}

uint8_t cpu_rol(uint16_t address) {
    uint8_t storedVal = reg_accumulator;
    if (!accumulatorAddrMode) {
        storedVal = bus_readCPU(address);
    }
    

    bool oldCarry = cpu_getFlag(CPUSTAT_CARRY);
    cpu_setFlag(CPUSTAT_CARRY, (storedVal >> 7) & 1);
    storedVal = storedVal << 1;
    storedVal = storedVal | oldCarry;

    cpu_setFlag(CPUSTAT_ZERO, storedVal == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (storedVal & 0b10000000) != 0);

    if (accumulatorAddrMode) {
        reg_accumulator = storedVal;
    } else {
        bus_writeCPU(address, storedVal);
    }

    accumulatorAddrMode = false;
    return ROL;
}

uint8_t cpu_ror(uint16_t address) {
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
    return ROR;
}

uint8_t cpu_and(uint16_t address) {
    reg_accumulator = bus_readCPU(address) & reg_accumulator;
    cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
    return AND;
}

uint8_t cpu_ora(uint16_t address) {
    reg_accumulator = bus_readCPU(address) | reg_accumulator;
    cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
    return ORA;
}

uint8_t cpu_eor(uint16_t address) {
    reg_accumulator = bus_readCPU(address) ^ reg_accumulator;
    
    cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
    return EOR;
}

uint8_t cpu_cmp(uint16_t address) {
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
    return CMP;
}

uint8_t cpu_cpx(uint16_t address) {
    uint8_t memVal = bus_readCPU(address);

    int8_t signedResult = ((int8_t) reg_x) - ((int8_t) memVal);
    if (reg_x < memVal) {
        cpu_setFlag(CPUSTAT_ZERO, 0);
        cpu_setFlag(CPUSTAT_CARRY, 0);
    } else if (reg_x == memVal) {
        cpu_setFlag(CPUSTAT_ZERO, 1);
        cpu_setFlag(CPUSTAT_CARRY, 1);
    } else if (reg_x > memVal) {
        cpu_setFlag(CPUSTAT_ZERO, 0);
        cpu_setFlag(CPUSTAT_CARRY, 1);
    }

    cpu_setFlag(CPUSTAT_NEGATIVE, signedResult < 0);
    return CPX;
}

uint8_t cpu_cpy(uint16_t address) {
    uint8_t memVal = bus_readCPU(address);

    int8_t signedResult = ((int8_t) reg_y) - ((int8_t) memVal);
    if (reg_y < memVal) {
        cpu_setFlag(CPUSTAT_ZERO, 0);
        cpu_setFlag(CPUSTAT_CARRY, 0);
    } else if (reg_y == memVal) {
        cpu_setFlag(CPUSTAT_ZERO, 1);
        cpu_setFlag(CPUSTAT_CARRY, 1);
    } else if (reg_y > memVal) {
        cpu_setFlag(CPUSTAT_ZERO, 0);
        cpu_setFlag(CPUSTAT_CARRY, 1);
    }

    cpu_setFlag(CPUSTAT_NEGATIVE, signedResult < 0);
    return CPY;
}

uint8_t cpu_bit(uint16_t address) {
    uint8_t memVal = bus_readCPU(address);
    cpu_setFlag(CPUSTAT_ZERO, (reg_accumulator & memVal) == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (memVal & 0b10000000) != 0);
    cpu_setFlag(CPUSTAT_OVERFLOW, (memVal & 0b01000000) != 0);
    return BIT;
}

void cpu_branchHelper(bool desiredResult, enum CPUStatusFlag flag) {
    if (cpu_getFlag(flag) == desiredResult) {
        int8_t offset = (bus_readCPU(reg_pc));
        reg_pc += offset;
        reg_pc += 1;
    } else {
        reg_pc += 1;
    }
}

uint8_t cpu_bcc() {
    cpu_branchHelper(0, CPUSTAT_CARRY);
    return BCC;
}

uint8_t cpu_bcs() {
    cpu_branchHelper(1, CPUSTAT_CARRY);
    return BCS;
}

uint8_t cpu_bne() {
    cpu_branchHelper(0, CPUSTAT_ZERO);
    return BNE;
}

uint8_t cpu_beq() {
    cpu_branchHelper(1, CPUSTAT_ZERO);
    return BEQ;
}

uint8_t cpu_bpl() {
    cpu_branchHelper(0, CPUSTAT_NEGATIVE);
    return BPL;
}

uint8_t cpu_bmi() {
    cpu_branchHelper(1, CPUSTAT_NEGATIVE);
    return BMI;
}

uint8_t cpu_bvc() {
    cpu_branchHelper(0, CPUSTAT_OVERFLOW);
    return BVC;
}

uint8_t cpu_bvs() {
    cpu_branchHelper(1, CPUSTAT_OVERFLOW);
    return BVS;
}

uint8_t cpu_tax() {
    reg_x = reg_accumulator;
    cpu_setFlag(CPUSTAT_ZERO, reg_x == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_x & 0b10000000) != 0);
    return TAX;
}

uint8_t cpu_txa() {
    reg_accumulator = reg_x;
    cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
    return TXA;
}

uint8_t cpu_tay() {
    reg_y = reg_accumulator;
    cpu_setFlag(CPUSTAT_ZERO, reg_y == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_y & 0b10000000) != 0);
    return TAY;
}

uint8_t cpu_tya() {
    reg_accumulator = reg_y;
    cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
    return TYA;
}

uint8_t cpu_tsx() {
    reg_x = stackPointer;
    cpu_setFlag(CPUSTAT_ZERO, reg_x == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_x & 0b10000000) != 0);
    return TSX;
}

uint8_t cpu_txs() {
    stackPointer = reg_x;
    return TXS;
}

uint8_t cpu_pla() {
    reg_accumulator = cpu_stackPull();
    cpu_setFlag(CPUSTAT_ZERO, reg_accumulator == 0);
    cpu_setFlag(CPUSTAT_NEGATIVE, (reg_accumulator & 0b10000000) != 0);
    return PLA;
}

uint8_t cpu_pha() {
    cpu_stackPush(reg_accumulator);
    return PHA;
}

uint8_t cpu_php() {
    cpu_stackPush(reg_status);
    return PHP;
}

uint8_t cpu_plp() {
    reg_status = cpu_stackPull();
    cpu_setFlag(CPUSTAT_BREAK2, true);
    cpu_setFlag(CPUSTAT_BREAK, false);
    return PLP;
}

uint8_t cpu_jmp(uint16_t address) {
    reg_pc = address;
    return JMP;
}

uint8_t cpu_jsr(uint16_t address) {
    
    uint16_t returnAddress = reg_pc - 1;
    reg_pc = address;
    cpu_stackPush16(returnAddress);
    return JSR;
}

uint8_t cpu_rts() {
    reg_pc = cpu_stackPull16() + 1;
    return RTS;
}

uint8_t cpu_rti() {
    reg_status = cpu_stackPull();
    reg_pc = cpu_stackPull16();
    cpu_setFlag(CPUSTAT_BREAK2, true); // always ensure BREAK2 is set
    return RTI;
}

uint8_t cpu_clc() {
    cpu_setFlag(CPUSTAT_CARRY, 0);
    return CLC;
}

uint8_t cpu_sec() {
    cpu_setFlag(CPUSTAT_CARRY, 1);
    return SEC;
}

uint8_t cpu_cld() {
    cpu_setFlag(CPUSTAT_DECIMAL, 0);
    return CLD;
}

uint8_t cpu_sed() {
    cpu_setFlag(CPUSTAT_DECIMAL, 1);
    return SED;
}

uint8_t cpu_cli() {
    cpu_setFlag(CPUSTAT_NO_INTRPT, 0);
    return CLI;
}

uint8_t cpu_sei() {
    cpu_setFlag(CPUSTAT_NO_INTRPT, 1);
    return SEI;
}

uint8_t cpu_clv() {
    cpu_setFlag(CPUSTAT_OVERFLOW, 0);
    return CLV;
}

uint8_t cpu_brk() {
    cpu_setFlag(CPUSTAT_BREAK, 1);
    cpu_setFlag(CPUSTAT_NO_INTRPT, 1);
    return BRK;
}

uint8_t cpu_nop() {
    return NOP;
}

uint8_t cpu_illegal_alr(uint16_t address) {
    cpu_and(address);
    cpu_lsr(address);
    return IL_ALR;
}

uint8_t cpu_illegal_anc(uint16_t address) {
    cpu_and(address);
    cpu_setFlag(CPUSTAT_CARRY, (bus_readCPU(address) >> 7) & 1);
    return IL_ANC;
}

uint8_t cpu_illegal_ane(uint16_t address) {
    reg_accumulator = reg_x & (rand() % 256);
    cpu_and(address);
    return IL_ANE;
}

uint8_t cpu_illegal_arr(uint16_t address) {
    cpu_and(address);
    cpu_ror(address); 
    return IL_ARR;
}

uint8_t cpu_illegal_dcp(uint16_t address) {
    cpu_dec(address);
    cpu_cmp(address);
    return IL_DCP;
}

uint8_t cpu_illegal_isc(uint16_t address) {
    cpu_inc(address);
    cpu_sbc(address);
    return IL_ISC;
}

uint8_t cpu_illegal_las(uint16_t address) {
    cpu_lda(address);
    cpu_tsx();
    return IL_LAS;
}

uint8_t cpu_illegal_lax(uint16_t address) {
    cpu_lda(address);
    cpu_ldx(address);
    return IL_LAX;
}

uint8_t cpu_illegal_lxa(uint16_t address) {
    reg_accumulator = (rand() % 256) & bus_readCPU(address);
    reg_x = reg_accumulator;
    
    return IL_LXA;
}

uint8_t cpu_illegal_rla(uint16_t address) {
    cpu_rol(address);
    cpu_and(address);
    return IL_RLA;
}

uint8_t cpu_illegal_rra(uint16_t address) {
    cpu_ror(address);
    cpu_adc(address);
    return IL_RRA;
}

uint8_t cpu_illegal_sax(uint16_t address) {
    bus_writeCPU(address, reg_accumulator & reg_x);
    
    return IL_SAX;
}

uint8_t cpu_illegal_sbx(uint16_t address) {
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
    return IL_SBX;
}

uint8_t cpu_illegal_sha(uint16_t address) {
    // programmed instability
    if (rand() % 4 == 0) {
        bus_writeCPU(address, reg_accumulator & reg_x);
    } else {
        bus_writeCPU(address, reg_accumulator & reg_x & ((bus_readCPU(address) >> 8) + 1));
    }
    
    return IL_SHA;
}

uint8_t cpu_illegal_shx(uint16_t address) {
    // programmed instability
    if (rand() % 4 == 0) {
        bus_writeCPU(address, reg_x);
    } else {
        bus_writeCPU(address, reg_x & ((bus_readCPU(address) >> 8) + 1));
    }
    
    return IL_SHX;
}

uint8_t cpu_illegal_shy(uint16_t address) {
    // programmed instability
    if (rand() % 4 == 0) {
        bus_writeCPU(address, reg_y);
    } else {
        bus_writeCPU(address, reg_y & ((bus_readCPU(address) >> 8) + 1));
    }
    
    return IL_SHY;
}

uint8_t cpu_illegal_slo(uint16_t address) {
    cpu_asl(address);
    cpu_ora(address);
    return IL_SLO;
}

uint8_t cpu_illegal_sre(uint16_t address) {
    cpu_lsr(address);
    cpu_eor(address);
    return IL_SRE;
}

uint8_t cpu_illegal_tas(uint16_t address) {
    stackPointer = reg_accumulator & reg_x;
    // programmed instability
    if (rand() % 4 == 0) {
        bus_writeCPU(address, reg_accumulator & reg_x);
    } else {
        bus_writeCPU(address, reg_accumulator & reg_x & ((bus_readCPU(address) >> 8) + 1));
    }
    
    return IL_TAS;
}

uint8_t cpu_illegal_sbc(uint16_t address) {
    cpu_sbc(address);
    return IL_SBC;
}

uint8_t cpu_illegal_jam() {
    cpu_brk(); // not really but I doubt it'll affect any games
    return IL_JAM;
}

uint8_t cpu_illegal_nop(uint16_t address) {
    return IL_NOP;
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
