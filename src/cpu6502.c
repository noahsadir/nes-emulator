/**
 * @file cpu6502.c
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

#include "cpu6502.h"

CPURegisters reg;
CPUClockMode clockMode = CPUCLOCK_SUSPENDED;

void(*memWrite)(uint16_t, uint8_t);
uint8_t(*memRead)(uint16_t);

BytecodeProgram* prgBytecode;

uint8_t cpuerrno = 0;

void cpu6502_init(void(*w)(uint16_t, uint8_t), uint8_t(*r)(uint16_t)) {
  memWrite = w;
  memRead = r;

  reg.p = 0x24;
  reg.a = 0x00;
  reg.x = 0x00;
  reg.y = 0x00;
  reg.s = 0xFD;
  reg.pc = cpu6502_read16(0xFFFC);

  static BytecodeProgram prog;
  prgBytecode = &prog;
  prog.bytecodeCount = 0;
  for (int i = 0; i < 65536; i++) {
    prog.addrMap[i] = 0x0000;
  }
  prog.bytecodes = malloc(sizeof(Bytecode));

  #if (CPU_DEBUG)
  reg.pc = 0xC000;
  #endif
}

void cpu6502_step(char* traceStr, void(*c)(uint8_t)) {
  if (prgBytecode->addrMap[reg.pc] == 0x0000) {
    // bytecode not compiled yet
    Bytecode bytecode;
    cpu6502_parseOpcode(memRead(reg.pc), &bytecode);
    for (int i = 0; i < bytecode.count; i++) {
      bytecode.data[i] = memRead(reg.pc + i);
    }
    prgBytecode->bytecodeCount += 1;
    prgBytecode->bytecodes = realloc(prgBytecode->bytecodes, sizeof(Bytecode) * (prgBytecode->bytecodeCount + 1));
    prgBytecode->bytecodes[prgBytecode->bytecodeCount - 1] = bytecode;
    prgBytecode->addrMap[reg.pc] = prgBytecode->bytecodeCount - 1;
  }

  Bytecode* b = &prgBytecode->bytecodes[prgBytecode->addrMap[reg.pc]];
  if (traceStr != NULL) {
    logging_bytecodeToTrace(reg, b, traceStr, memWrite, memRead);
  }
  c(cpu6502_execute(b));
}

void cpu6502_nmi() {
  cpu6502_stackPush((reg.pc >> 8) & BIT_FILL_8);
  cpu6502_stackPush(reg.pc & BIT_FILL_8);
  cpu6502_stackPush(reg.p);
  cpu6502_setFlag(CPUSTAT_NO_INTRPT, true);
  reg.pc = ((uint16_t)memRead(0xFFFB) << 8) | (uint16_t)memRead(0xFFFA);
}

static force_inline void cpu6502_stackPush(uint8_t val) {
  if (reg.s == 0x00) {
    // overflow
  } else {
    memWrite(0x100 + reg.s, val);
    reg.s -= 1;
  }
}

static force_inline uint8_t cpu6502_stackPull() {
  if (reg.s == 0xFF) {
    // underflow
  } else {
    reg.s += 1;
    uint8_t val = memRead(0x100 + reg.s);
    return val;
  }
  return 0;
}

static force_inline bool cpu6502_shouldBranch(bool desiredResult, CPUStatusFlag flag) {
  return (((reg.p & flag) > 0) == desiredResult);
}

static force_inline void cpu6502_setFlag(CPUStatusFlag flag, bool enabled) {
  if (enabled) {
    reg.p |= flag;
  } else {
    reg.p &= ~flag;
  }
}

CPUClockMode cpu6502_getClockMode() {
  return clockMode;
}

void cpu6502_setClockMode(CPUClockMode mode) {
  clockMode = mode;
}

static force_inline uint16_t cpu6502_read16(uint16_t addr) {
  return (((uint16_t)memRead(addr + 1) << 8) | (uint16_t)memRead(addr));
}

void cpu6502_dasm(uint8_t* prgData, uint32_t prgSize, void(*c)(char c[128]), uint8_t flags) {
  uint32_t pointer = 0x0000;
  char line[128];
  Bytecode bytecode;
  while (pointer < prgSize) {
    cpu6502_parseOpcode(prgData[pointer], &bytecode);
    for (int i = 0; i < bytecode.count; i++) {
      bytecode.data[i] = prgData[pointer];
      pointer += 1;
    }
    logging_bytecodeToAssembly(pointer, &bytecode, line, flags);
    c(line);
  }
}

void cpu6502_loadBytecodeProgram(uint8_t* prgData, uint32_t prgSize) {
  // populate entries
  uint32_t pointer = 0x0000;
  while (pointer < prgSize) {
    Bytecode bytecode;
    cpu6502_parseOpcode(prgData[pointer], &bytecode);
    int offset = 0;
    for (int i = 0; i < bytecode.count; i++) {
      bytecode.data[i] = prgData[pointer + i];
      offset += 1;
    }
    
    prgBytecode->bytecodeCount += 1;
    prgBytecode->bytecodes = realloc(prgBytecode->bytecodes, sizeof(Bytecode) * (prgBytecode->bytecodeCount + 1));
    prgBytecode->bytecodes[prgBytecode->bytecodeCount - 1] = bytecode;
    
    // account for mirroring
    for (int i = pointer; i < 0xFFFF; i += prgSize) {
      prgBytecode->addrMap[i] = prgBytecode->bytecodeCount - 1;
    }
    
    pointer += offset;
  }
}

static force_inline void cpu6502_parseOpcode(uint8_t opcode, Bytecode* b) {
  if (b == NULL) return;
  b->mnemonic = I_UNSET;
  b->addressingMode = AM_UNSET;
  b->count = 1;
  // get mnemonic
  switch (opcode) {
    case 0xAD: case 0xBD: case 0xB9: case 0xA9: case 0xA5: case 0xA1: case 0xB5: case 0xB1:
    {
      b->mnemonic = I_LDA;
      break;
    }
    case 0xAE: case 0xBE: case 0xA2: case 0xA6: case 0xB6:
    {
      b->mnemonic = I_LDX;
      break;
    }
    case 0xAC: case 0xBC: case 0xA0: case 0xA4: case 0xB4:
    {
      b->mnemonic = I_LDY;
      break;
    }
    case 0x8D: case 0x9D: case 0x99: case 0x85: case 0x81: case 0x95: case 0x91:
    {
      b->mnemonic = I_STA;
      break;
    }
    case 0x8E: case 0x86: case 0x96:
    {
      b->mnemonic = I_STX;
      break;
    }
    case 0x8C: case 0x84: case 0x94:
    {
      b->mnemonic = I_STY;
      break;
    }
    case 0x6D: case 0x7D: case 0x79: case 0x69: case 0x65: case 0x61: case 0x75: case 0x71:
    {
      b->mnemonic = I_ADC;
      break;
    }
    case 0xED: case 0xFD: case 0xF9: case 0xE9: case 0xE5: case 0xE1: case 0xF5: case 0xF1:
    {
      b->mnemonic = I_SBC;
      break;
    }
    case 0xEE: case 0xFE: case 0xE6: case 0xF6:
    {
      b->mnemonic = I_INC;
      break;
    }
    case 0xE8:
    {
      b->mnemonic = I_INX;
      break;
    }
    case 0xC8:
    {
      b->mnemonic = I_INY;
      break;
    }
    case 0xCE: case 0xDE: case 0xC6: case 0xD6:
    {
      b->mnemonic = I_DEC;
      break;
    }
    case 0xCA:
    {
      b->mnemonic = I_DEX;
      break;
    }
    case 0x88:
    {
      b->mnemonic = I_DEY;
      break;
    }
    case 0x0E: case 0x1E: case 0x0A: case 0x06: case 0x16:
    {
      b->mnemonic = I_ASL;
      break;
    }
    case 0x4E: case 0x5E: case 0x4A: case 0x46: case 0x56:
    {
      b->mnemonic = I_LSR;
      break;
    }
    case 0x2E: case 0x3E: case 0x2A: case 0x26: case 0x36:
    {
      b->mnemonic = I_ROL;
      break;
    }
    case 0x6E: case 0x7E: case 0x6A: case 0x66: case 0x76:
    {
      b->mnemonic = I_ROR;
      break;
    }
    case 0x2D: case 0x3D: case 0x39: case 0x29: case 0x25: case 0x21: case 0x35: case 0x31:
    {
      b->mnemonic = I_AND;
      break;
    }
    case 0x0D: case 0x1D: case 0x19: case 0x09: case 0x05: case 0x01: case 0x15: case 0x11:
    {
      b->mnemonic = I_ORA;
      break;
    }
    case 0x4D: case 0x5D: case 0x59: case 0x49: case 0x45: case 0x41: case 0x55: case 0x51:
    {
      b->mnemonic = I_EOR;
      break;
    }
    case 0xCD: case 0xDD: case 0xD9: case 0xC9: case 0xC5: case 0xC1: case 0xD5: case 0xD1:
    {
      b->mnemonic = I_CMP;
      break;
    }
    case 0xEC: case 0xE0: case 0xE4:
    {
      b->mnemonic = I_CPX;
      break;
    }
    case 0xCC: case 0xC0: case 0xC4:
    {
      b->mnemonic = I_CPY;
      break;
    }
    case 0x2C: case 0x89: case 0x24:
    {
      b->mnemonic = I_BIT;
      break;
    }
    case 0x90:
    {
      b->mnemonic = I_BCC;
      break;
    }
    case 0xB0:
    {
      b->mnemonic = I_BCS;
      break;
    }
    case 0xD0:
    {
      b->mnemonic = I_BNE;
      break;
    }
    case 0xF0:
    {
      b->mnemonic = I_BEQ;
      break;
    }
    case 0x10:
    {
      b->mnemonic = I_BPL;
      break;
    }
    case 0x30:
    {
      b->mnemonic = I_BMI;
      break;
    }
    case 0x50:
    {
      b->mnemonic = I_BVC;
      break;
    }
    case 0x70:
    {
      b->mnemonic = I_BVS;
      break;
    }
    case 0xAA:
    {
      b->mnemonic = I_TAX;
      break;
    }
    case 0x8A:
    {
      b->mnemonic = I_TXA;
      break;
    }
    case 0xA8:
    {
      b->mnemonic = I_TAY;
      break;
    }
    case 0x98:
    {
      b->mnemonic = I_TYA;
      break;
    }
    case 0xBA:
    {
      b->mnemonic = I_TSX;
      break;
    }
    case 0x9A:
    {
      b->mnemonic = I_TXS;
      break;
    }
    case 0x48:
    {
      b->mnemonic = I_PHA;
      break;
    }
    case 0x68:
    {
      b->mnemonic = I_PLA;
      break;
    }
    case 0x08:
    {
      b->mnemonic = I_PHP;
      break;
    }
    case 0x28:
    {
      b->mnemonic = I_PLP;
      break;
    }
    case 0x4C: case 0x6C:
    {
      b->mnemonic = I_JMP;
      break;
    }
    case 0x20:
    {
      b->mnemonic = I_JSR;
      break;
    }
    case 0x60:
    {
      b->mnemonic = I_RTS;
      break;
    }
    case 0x40:
    {
      b->mnemonic = I_RTI;
      break;
    }
    case 0x18:
    {
      b->mnemonic = I_CLC;
      break;
    }
    case 0x38:
    {
      b->mnemonic = I_SEC;
      break;
    }
    case 0xD8:
    {
      b->mnemonic = I_CLD;
      break;
    }
    case 0xF8:
    {
      b->mnemonic = I_SED;
      break;
    }
    case 0x58:
    {
      b->mnemonic = I_CLI;
      break;
    }
    case 0x78:
    {
      b->mnemonic = I_SEI;
      break;
    }
    case 0xB8:
    {
      b->mnemonic = I_CLV;
      break;
    }
    case 0x00:
    {
      b->mnemonic = I_BRK;
      break;
    }
    case 0xEA:
    {
      b->mnemonic = I_NOP;
      break;
    }
    case 0x4B:
    {
      b->mnemonic = I_ILL_ALR;
      break;
    }
    case 0x0B:
    {
      b->mnemonic = I_ILL_ANC;
      break;
    }
    case 0x2B:
    {
      b->mnemonic = I_ILL_ANC2;
      break;
    }
    case 0x8B:
    {
      b->mnemonic = I_ILL_ANE;
      break;
    }
    case 0x6B:
    {
      b->mnemonic = I_ILL_ARR;
      break;
    }
    case 0xC7: case 0xD7: case 0xCF: case 0xDF: case 0xDB: case 0xC3: case 0xD3:
    {
      b->mnemonic = I_ILL_DCP;
      break;
    }
    case 0xE7: case 0xF7: case 0xEF: case 0xFF: case 0xFB: case 0xE3: case 0xF3:
    {
      b->mnemonic = I_ILL_ISC;
      break;
    }
    case 0xBB:
    {
      b->mnemonic = I_ILL_LAS;
      break;
    }
    case 0xA7: case 0xB7: case 0xAF: case 0xBF: case 0xA3: case 0xB3:
    {
      b->mnemonic = I_ILL_LAX;
      break;
    }
    case 0xAB:
    {
      b->mnemonic = I_ILL_LXA;
      break;
    }
    case 0x27: case 0x37: case 0x2F: case 0x3F: case 0x3B: case 0x23: case 0x33:
    {
      b->mnemonic = I_ILL_RLA;
      break;
    }
    case 0x67: case 0x77: case 0x6F: case 0x7F: case 0x7B: case 0x63: case 0x73:
    {
      b->mnemonic = I_ILL_RRA;
      break;
    }
    case 0x87: case 0x97: case 0x8F: case 0x83:
    {
      b->mnemonic = I_ILL_SAX;
      break;
    }
    case 0xCB:
    {
      b->mnemonic = I_ILL_SBX;
      break;
    }
    case 0x9F: case 0x93:
    {
      b->mnemonic = I_ILL_SHA;
      break;
    }
    case 0x9E:
    {
      b->mnemonic = I_ILL_SHX;
      break;
    }
    case 0x9C:
    {
      b->mnemonic = I_ILL_SHY;
      break;
    }
    case 0x07: case 0x17: case 0x0F: case 0x1F: case 0x1B: case 0x03: case 0x13:
    {
      b->mnemonic = I_ILL_SLO;
      break;
    }
    case 0x47: case 0x57: case 0x4F: case 0x5F: case 0x5B: case 0x43: case 0x53:
    {
      b->mnemonic = I_ILL_SRE;
      break;
    }
    case 0x9B:
    {
      b->mnemonic = I_ILL_TAS;
      break;
    }
    case 0xEB:
    {
      b->mnemonic = I_ILL_USBC;
      break;
    }
    case 0x1A: case 0x3A: case 0x5A: case 0x7A: case 0xDA: case 0xFA: case 0x80:
    case 0x82: case 0xC2: case 0xE2: case 0x04: case 0x44: case 0x64:
    case 0x14: case 0x34: case 0x54: case 0x74: case 0xD4: case 0xF4: case 0x0C:
    case 0x1C: case 0x3C: case 0x5C: case 0x7C: case 0xDC: case 0xFC:
    {
      b->mnemonic = I_ILL_NOP;
      break;
    }
    case 0x02: case 0x12: case 0x22: case 0x32: case 0x42: case 0x52: case 0x62:
    case 0x72: case 0x92: case 0xB2: case 0xD2: case 0xF2:
    {
      b->mnemonic = I_ILL_JAM;
      break;
    }
    default:
    {
      b->mnemonic = I_UNSET;
      break;
    }
  }

  switch (opcode)
  {
    case 0xAD: case 0xAE: case 0xAC: case 0x8D: case 0x8E: case 0x8C: case 0x6D: case 0xED:
    case 0xEE: case 0xCE: case 0x0E: case 0x4E: case 0x2E: case 0x6E: case 0x2D: case 0x0D:
    case 0x4D: case 0xCD: case 0xEC: case 0xCC: case 0x2C: case 0x4C: case 0x20: case 0xCF:
    case 0xEF: case 0xAF: case 0x2F: case 0x6F: case 0x8F: case 0x0F: case 0x4F: case 0x0C:
    {
      b->addressingMode = AM_ABSOLUTE;
      b->count = 3;
      break;
    }
    case 0xBD: case 0xBC: case 0x9D: case 0x7D: case 0xFD: case 0xFE: case 0xDE: case 0x1E:
    case 0x5E: case 0x3E: case 0x7E: case 0x3D: case 0x1D: case 0x5D: case 0xDD: case 0xDF:
    case 0xFF: case 0x3F: case 0x7F: case 0x9C: case 0x1F: case 0x5F: case 0x1C: case 0x3C:
    case 0x5C: case 0x7C: case 0xDC: case 0xFC:
    {
      b->addressingMode = AM_ABS_X;
      b->count = 3;
      break;
    }
    case 0xB9: case 0xBE: case 0x99: case 0x79: case 0xF9: case 0x39: case 0x19: case 0x59:
    case 0xD9: case 0xDB: case 0xFB: case 0xBF: case 0x3B: case 0x7B: case 0x9F: case 0x9E: 
    case 0x1B: case 0x5B: case 0x9B:
    {
      b->addressingMode = AM_ABS_Y;
      b->count = 3;
      break;
    }
    case 0xA9: case 0xA2: case 0xA0: case 0x69: case 0xE9: case 0x29: case 0x09: case 0x49:
    case 0xC9: case 0xE0: case 0xC0: case 0x89: case 0x4B: case 0x0B: case 0x2B: case 0x8B:
    case 0x6B: case 0xAB: case 0xCB: case 0xEB: case 0x80: case 0x82: case 0xC2:
    case 0xE2:
    {
      b->addressingMode = AM_IMMEDIATE;
      b->count = 2;
      break;
    }
    case 0xA5: case 0xA6: case 0xA4: case 0x85: case 0x86: case 0x84: case 0x65: case 0xE5:
    case 0xE6: case 0xC6: case 0x06: case 0x46: case 0x26: case 0x66: case 0x25: case 0x05:
    case 0x45: case 0xC5: case 0xE4: case 0xC4: case 0x24: case 0xC7: case 0xE7: case 0xA7: 
    case 0x27: case 0x67: case 0x87: case 0x07: case 0x47: case 0x04: case 0x44: case 0x64:
    {
      b->addressingMode = AM_ZERO_PAGE;
      b->count = 2;
      break;
    }
    case 0xA1: case 0x81: case 0x61: case 0xE1: case 0x21: case 0x01: case 0x41: case 0xC1:
    case 0xC3: case 0xE3: case 0xA3: case 0x23: case 0x63: case 0x83: case 0x03: case 0x43:
    {
      b->addressingMode = AM_ZP_X_INDIRECT;
      b->count = 2;
      break;
    }
    case 0xB5: case 0xB4: case 0x95: case 0x94: case 0x75: case 0xF5: case 0xF6: case 0xD6: 
    case 0x16: case 0x56: case 0x36: case 0x76: case 0x35: case 0x15: case 0x55: case 0xD5:
    case 0xD7: case 0xF7: case 0x37: case 0x77: case 0x17: case 0x57: case 0x14: case 0x34:
    case 0x54: case 0x74: case 0xD4: case 0xF4:
    {
      b->addressingMode = AM_ZP_X;
      b->count = 2;
      break;
    }
    case 0xB1: case 0x91: case 0x71: case 0xF1: case 0x31: case 0x11: case 0x51: case 0xD1:
    case 0xD3: case 0xF3: case 0xB3: case 0x33: case 0x73: case 0x93: case 0x13: case 0x53:
    {
      b->addressingMode = AM_ZP_INDIRECT_Y;
      b->count = 2;
      break;
    }
    case 0xB6: case 0x96: case 0x97: case 0xB7:
    {
      b->addressingMode = AM_ZP_Y;
      b->count = 2;
      break;
    }
    case 0x0A: case 0x4A: case 0x2A: case 0x6A: 
    {
      b->addressingMode = AM_ACCUMULATOR;
      b->count = 1;
      break;
    }
    case 0x90: case 0xB0: case 0xD0: case 0xF0: case 0x10: case 0x30: case 0x50: case 0x70:
    {
      b->addressingMode = AM_RELATIVE;
      b->count = 2;
      break;
    }
    case 0xE8: case 0xC8: case 0xCA: case 0x88: case 0xAA: case 0x8A: case 0xA8: case 0x98:
    case 0xBA: case 0x9A: case 0x48: case 0x68: case 0x08: case 0x28: case 0x60: case 0x40:
    case 0x18: case 0x38: case 0xD8: case 0xF8: case 0x58: case 0x78: case 0xB8: case 0x00:
    case 0xEA:
    {
      b->addressingMode = AM_IMPLIED;
      b->count = 1;
      break;
    }
    case 0x6C:
    {
      b->addressingMode = AM_ABS_INDIRECT;
      b->count = 3;
      break;
    }
    default:
    {
      b->addressingMode = AM_UNSET;
      b->count = 1;
      break;
    }
  }
}

static force_inline uint8_t cpu6502_execute(Bytecode* b) {
  uint16_t val;
  uint8_t cycleCount = 2;
  switch (b->addressingMode) {
    case AM_IMMEDIATE:
    {
      val = reg.pc + 1;
      break;
    }
    case AM_ABSOLUTE: 
    {
      cycleCount += 2;
      val = ((uint16_t)b->data[2] << 8) | (uint16_t)b->data[1];
      break;
    }
    case AM_ZERO_PAGE: 
    {
      cycleCount += 1;
      val = (uint16_t)b->data[1];
      break;
    }
    case AM_ABS_INDIRECT:
    {
      uint16_t pointerAddr = ((uint16_t)b->data[2] << 8) | (uint16_t)b->data[1];
      uint16_t pointerAddrInc = pointerAddr;
      if ((pointerAddrInc & 0x00FF) == 0x00FF) {
        pointerAddrInc &= 0xFF00;
      } else {
        pointerAddrInc += 1;
      }
      val = ((uint16_t)memRead(pointerAddrInc) << 8) | (uint16_t)memRead(pointerAddr);
      break;
    }
    case AM_ABS_X:
    {
      cycleCount += 2;
      val = (((uint16_t)b->data[2] << 8) | (uint16_t)b->data[1]) + (uint16_t)reg.x;
      break;
    }
    case AM_ABS_Y:
    {
      cycleCount += 2;
      val = (((uint16_t)b->data[2] << 8) | (uint16_t)b->data[1]) + (uint16_t)reg.y;
      break;
    }
    case AM_ZP_X:
    {
      cycleCount += 2;
      uint8_t zpVal = b->data[1] + reg.x;
      val = (uint16_t)zpVal;
      break;
    }
    case AM_ZP_Y:
    {
      cycleCount += 2;
      uint8_t zpVal = b->data[1] + reg.y;
      val = (uint16_t)zpVal;
      break;
    }
    case AM_ZP_X_INDIRECT:
    {
      cycleCount += 4;
      uint8_t zpVal = b->data[1] + reg.x;
      uint8_t zpValInc = zpVal + 1;
      val = ((uint16_t)memRead(zpValInc) << 8) | (uint16_t)memRead(zpVal);
      break;
    }
    case AM_ZP_INDIRECT_Y:
    {
      cycleCount += 3;
      uint8_t zpVal = b->data[1];
      uint8_t zpValInc = zpVal + 1;
      val = ((uint16_t)memRead(zpValInc) << 8) | (uint16_t)memRead(zpVal);
      val += reg.y;
      break;
    }
    case AM_RELATIVE:
    {
      int8_t offset = b->data[1];
      val = reg.pc + offset + 2;
      break;
    }
    default:
    {
      cycleCount += 1;
      val = 0;
      break;
    }
  }

  switch(b->mnemonic) {
    case I_LDA:
    {
      reg.a = memRead(val);
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_LDX:
    {
      reg.x = memRead(val);
      cpu6502_setFlag(CPUSTAT_ZERO, reg.x == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.x & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_LDY:
    {
      reg.y = memRead(val);
      cpu6502_setFlag(CPUSTAT_ZERO, reg.y == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.y & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_STA:
    {
      memWrite(val, reg.a);
      reg.pc += b->count;
      break;
    }
    case I_STX:
    {
      memWrite(val, reg.x);
      reg.pc += b->count;
      break;
    }
    case I_STY:
    {
      memWrite(val, reg.y);
      reg.pc += b->count;
      break;
    }
    case I_ADC:
    {
      uint8_t memoryVal = memRead(val);
      uint16_t sum = reg.a + memoryVal + ((uint16_t)((reg.p & CPUSTAT_CARRY) > 0));
      cpu6502_setFlag(CPUSTAT_CARRY, sum > 0xFF);
      cpu6502_setFlag(CPUSTAT_OVERFLOW, (reg.a ^ sum) & (memoryVal ^ sum) & 0x80);
      reg.a = (uint8_t) sum;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_SBC:
    {
      uint8_t memoryVal = ~memRead(val);
      uint16_t sum = reg.a + memoryVal + ((uint16_t)((reg.p & CPUSTAT_CARRY) > 0));
      cpu6502_setFlag(CPUSTAT_CARRY, sum > 0xFF);
      cpu6502_setFlag(CPUSTAT_OVERFLOW, (reg.a ^ sum) & (memoryVal ^ sum) & 0x80);
      reg.a = (uint8_t) sum;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_INC:
    {
      uint8_t incval = memRead(val) + 1;
      memWrite(val, incval);
      cpu6502_setFlag(CPUSTAT_ZERO, incval == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (incval & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_INX:
    {
      reg.x += 1;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.x == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.x & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_INY:
    {
      reg.y += 1;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.y == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.y & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_DEC:
    {
      uint8_t decval = memRead(val) - 1;
      memWrite(val, decval);
      cpu6502_setFlag(CPUSTAT_ZERO, decval == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (decval & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_DEX:
    {
      reg.x -= 1;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.x == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.x & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_DEY:
    {
      reg.y -= 1;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.y == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.y & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_ASL:
    {
      uint8_t storedVal = reg.a;
      if (!(b->addressingMode == AM_ACCUMULATOR)) {
        storedVal = memRead(val);
      }

      cpu6502_setFlag(CPUSTAT_CARRY, (storedVal >> 7) & 1);
      storedVal = storedVal << 1;

      cpu6502_setFlag(CPUSTAT_ZERO, storedVal == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (storedVal & BIT_MASK_8) != 0);

      if (b->addressingMode == AM_ACCUMULATOR) {
        reg.a = storedVal;
      } else {
        memWrite(val, storedVal);
      }
      reg.pc += b->count;
      break;
    }
    case I_LSR:
    {
      uint8_t storedVal = reg.a;
      if (!(b->addressingMode == AM_ACCUMULATOR)) {
        storedVal = memRead(val);
      }

      cpu6502_setFlag(CPUSTAT_CARRY, storedVal & 1);
      storedVal = storedVal >> 1;

      cpu6502_setFlag(CPUSTAT_ZERO, storedVal == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (storedVal & BIT_MASK_8) != 0);

      if (b->addressingMode == AM_ACCUMULATOR) {
        reg.a = storedVal;
      } else {
        memWrite(val, storedVal);
      }
      reg.pc += b->count;
      break;
    }
    case I_ROL:
    {
      uint8_t storedVal = reg.a;
      if (!(b->addressingMode == AM_ACCUMULATOR)) {
        storedVal = memRead(val);
      }

      bool oldCarry = ((reg.p & CPUSTAT_CARRY) > 0);
      cpu6502_setFlag(CPUSTAT_CARRY, (storedVal >> 7) & 1);
      storedVal = storedVal << 1;
      storedVal = storedVal | oldCarry;

      cpu6502_setFlag(CPUSTAT_ZERO, storedVal == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (storedVal & BIT_MASK_8) != 0);

      if (b->addressingMode == AM_ACCUMULATOR) {
        reg.a = storedVal;
      } else {
        memWrite(val, storedVal);
      }
      reg.pc += b->count;
      break;
    }
    case I_ROR:
    {
      uint8_t storedVal = reg.a;
      if (!(b->addressingMode == AM_ACCUMULATOR)) {
        storedVal = memRead(val);
      }

      bool oldCarry = ((reg.p & CPUSTAT_CARRY) > 0);
      cpu6502_setFlag(CPUSTAT_CARRY, storedVal & 1);
      storedVal = storedVal >> 1;
      storedVal = storedVal | (oldCarry << 7);

      cpu6502_setFlag(CPUSTAT_ZERO, storedVal == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (storedVal & BIT_MASK_8) != 0);

      if (b->addressingMode == AM_ACCUMULATOR) {
        reg.a = storedVal;
      } else {
        memWrite(val, storedVal);
      }
      reg.pc += b->count;
      break;
    }
    case I_AND:
    {
      reg.a = memRead(val) & reg.a;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_ORA:
    {
      reg.a = memRead(val) | reg.a;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_EOR:
    {
      reg.a = memRead(val) ^ reg.a;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_CMP:
    {
      uint8_t memVal = memRead(val);

      int8_t signedResult = ((int8_t) reg.a) - ((int8_t) memVal);
      if (reg.a < memVal) {
        cpu6502_setFlag(CPUSTAT_ZERO, 0);
        cpu6502_setFlag(CPUSTAT_CARRY, 0);
      } else if (reg.a == memVal) {
        cpu6502_setFlag(CPUSTAT_ZERO, 1);
        cpu6502_setFlag(CPUSTAT_CARRY, 1);
      } else if (reg.a > memVal) {
        cpu6502_setFlag(CPUSTAT_ZERO, 0);
        cpu6502_setFlag(CPUSTAT_CARRY, 1);
      }

      cpu6502_setFlag(CPUSTAT_NEGATIVE, signedResult < 0);
      reg.pc += b->count;
      break;
    }
    case I_CPX:
    {
      uint8_t memVal = memRead(val);

      int8_t signedResult = ((int8_t) reg.x) - ((int8_t) memVal);
      if (reg.x < memVal) {
        cpu6502_setFlag(CPUSTAT_ZERO, 0);
        cpu6502_setFlag(CPUSTAT_CARRY, 0);
      } else if (reg.x == memVal) {
        cpu6502_setFlag(CPUSTAT_ZERO, 1);
        cpu6502_setFlag(CPUSTAT_CARRY, 1);
      } else if (reg.x > memVal) {
        cpu6502_setFlag(CPUSTAT_ZERO, 0);
        cpu6502_setFlag(CPUSTAT_CARRY, 1);
      }

      cpu6502_setFlag(CPUSTAT_NEGATIVE, signedResult < 0);
      reg.pc += b->count;
      break;
    }
    case I_CPY:
    {
      uint8_t memVal = memRead(val);

      int8_t signedResult = ((int8_t) reg.y) - ((int8_t) memVal);
      if (reg.y < memVal) {
        cpu6502_setFlag(CPUSTAT_ZERO, 0);
        cpu6502_setFlag(CPUSTAT_CARRY, 0);
      } else if (reg.y == memVal) {
        cpu6502_setFlag(CPUSTAT_ZERO, 1);
        cpu6502_setFlag(CPUSTAT_CARRY, 1);
      } else if (reg.y > memVal) {
        cpu6502_setFlag(CPUSTAT_ZERO, 0);
        cpu6502_setFlag(CPUSTAT_CARRY, 1);
      }

      cpu6502_setFlag(CPUSTAT_NEGATIVE, signedResult < 0);
      reg.pc += b->count;
      break;
    }
    case I_BIT:
    {
      uint8_t memVal = memRead(val);
      cpu6502_setFlag(CPUSTAT_ZERO, (reg.a & memVal) == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (memVal & BIT_MASK_8) != 0);
      cpu6502_setFlag(CPUSTAT_OVERFLOW, (memVal & BIT_MASK_7) != 0);
      reg.pc += b->count;
      break;
    }
    case I_BCC:
    {
      cpu6502_shouldBranch(0, CPUSTAT_CARRY) ? (reg.pc = val) : (reg.pc += b->count);
      break;
    }
    case I_BCS:
    {
      cpu6502_shouldBranch(1, CPUSTAT_CARRY) ? (reg.pc = val) : (reg.pc += b->count);
      break;
    }
    case I_BNE:
    {
      cpu6502_shouldBranch(0, CPUSTAT_ZERO) ? (reg.pc = val) : (reg.pc += b->count);
      break;
    }
    case I_BEQ:
    {
      cpu6502_shouldBranch(1, CPUSTAT_ZERO) ? (reg.pc = val) : (reg.pc += b->count);
      break;
    }
    case I_BPL:
    {
      cpu6502_shouldBranch(0, CPUSTAT_NEGATIVE) ? (reg.pc = val) : (reg.pc += b->count);
      break;
    }
    case I_BMI:
    {
      cpu6502_shouldBranch(1, CPUSTAT_NEGATIVE) ? (reg.pc = val) : (reg.pc += b->count);
      break;
    }
    case I_BVC:
    {
      cpu6502_shouldBranch(0, CPUSTAT_OVERFLOW) ? (reg.pc = val) : (reg.pc += b->count);
      break;
    }
    case I_BVS:
    {
      cpu6502_shouldBranch(1, CPUSTAT_OVERFLOW) ? (reg.pc = val) : (reg.pc += b->count);
      break;
    }
    case I_TAX:
    {
      reg.x = reg.a;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.x == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.x & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_TXA:
    {
      reg.a = reg.x;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_TAY:
    {
      reg.y = reg.a;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.y == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.y & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_TYA:
    {
      reg.a = reg.y;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_TSX:
    {
      reg.x = reg.s;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.x == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.x & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_TXS:
    {
      reg.s = reg.x;
      reg.pc += b->count;
      break;
    }
    case I_PLA:
    {
      reg.a = cpu6502_stackPull();
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_PHA:
    {
      cpu6502_stackPush(reg.a);
      reg.pc += b->count;
      break;
    }
    case I_PLP:
    {
      reg.p = cpu6502_stackPull();
      cpu6502_setFlag(CPUSTAT_BREAK2, true);
      cpu6502_setFlag(CPUSTAT_BREAK, false);
      reg.pc += b->count;
      break;
    }
    case I_PHP:
    {
      uint8_t regPVal = reg.p | CPUSTAT_BREAK;
      cpu6502_stackPush(regPVal);
      reg.pc += b->count;
      break;
    }
    case I_JMP:
    {
      reg.pc = val;
      break;
    }
    case I_JSR:
    {
      uint16_t retAddr = reg.pc + 2;
      reg.pc = val;
      cpu6502_stackPush(retAddr >> 8);
      cpu6502_stackPush(retAddr & BIT_FILL_8);
      break;
    }
    case I_RTS:
    {
      uint16_t low = cpu6502_stackPull();
      uint16_t high = cpu6502_stackPull();
      reg.pc = ((high << 8) | low) + 1;
      break;
    }
    case I_RTI:
    {
      reg.p = cpu6502_stackPull();
      uint16_t low = cpu6502_stackPull();
      uint16_t high = cpu6502_stackPull();
      reg.pc = ((high << 8) | low);
      cpu6502_setFlag(CPUSTAT_BREAK2, true);
      break;
    }
    case I_CLC:
    {
      cpu6502_setFlag(CPUSTAT_CARRY, 0);
      reg.pc += b->count;
      break;
    }
    case I_SEC:
    {
      cpu6502_setFlag(CPUSTAT_CARRY, 1);
      reg.pc += b->count;
      break;
    }
    case I_CLD:
    {
      cpu6502_setFlag(CPUSTAT_DECIMAL, 0);
      reg.pc += b->count;
      break;
    }
    case I_SED:
    {
      cpu6502_setFlag(CPUSTAT_DECIMAL, 1);
      reg.pc += b->count;
      break;
    }
    case I_CLI:
    {
      cpu6502_setFlag(CPUSTAT_NO_INTRPT, 0);
      reg.pc += b->count;
      break;
    }
    case I_SEI:
    {
      cpu6502_setFlag(CPUSTAT_NO_INTRPT, 1);
      reg.pc += b->count;
      break;
    }
    case I_CLV:
    {
      cpu6502_setFlag(CPUSTAT_OVERFLOW, 0);
      reg.pc += b->count;
      break;
    }
    case I_BRK:
    {
      cpu6502_setFlag(CPUSTAT_BREAK, 1);
      cpu6502_setFlag(CPUSTAT_NO_INTRPT, 1);
      reg.pc += b->count;
      break;
    }
    case I_NOP:
    {
      reg.pc += b->count;
      break;
    }
    // illegal instructions
    case I_ILL_ALR: // AND + LSR
    {
      // AND
      reg.a = memRead(val) & reg.a;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);

      // LSR
      uint8_t storedVal = reg.a;
      if (!(b->addressingMode == AM_ACCUMULATOR)) {
        storedVal = memRead(val);
      }

      cpu6502_setFlag(CPUSTAT_CARRY, storedVal & 1);
      storedVal = storedVal >> 1;

      cpu6502_setFlag(CPUSTAT_ZERO, storedVal == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (storedVal & BIT_MASK_8) != 0);

      if (b->addressingMode == AM_ACCUMULATOR) {
        reg.a = storedVal;
      } else {
        memWrite(val, storedVal);
      }
      reg.pc += b->count;
      break;
    }
    case I_ILL_ANC: // AND + (C<-ASL)
    {
      reg.pc += b->count;
      break;
    }
    case I_ILL_ANC2: // AND + (C<-ROL)
    {
      reg.pc += b->count;
      break;
    }
    case I_ILL_ANE: // (* AND X) + AND
    {
      reg.a = (rand() % 0xFF) & reg.x;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);

      reg.a = memRead(val) & reg.a;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_ILL_ARR: // AND + ROR
    {
      // AND
      reg.a = memRead(val) & reg.a;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      
      // ROR
      uint8_t storedVal = reg.a;
      if (!(b->addressingMode == AM_ACCUMULATOR)) {
        storedVal = memRead(val);
      }

      bool oldCarry = ((reg.p & CPUSTAT_CARRY) > 0);
      cpu6502_setFlag(CPUSTAT_CARRY, storedVal & 1);
      storedVal = storedVal >> 1;
      storedVal = storedVal | (oldCarry << 7);

      cpu6502_setFlag(CPUSTAT_ZERO, storedVal == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (storedVal & BIT_MASK_8) != 0);

      if (b->addressingMode == AM_ACCUMULATOR) {
        reg.a = storedVal;
      } else {
        memWrite(val, storedVal);
      }
      reg.pc += b->count;
      break;
    }
    case I_ILL_DCP: // DEC + CMP
    {
      // DEC
      uint8_t decval = memRead(val) - 1;
      memWrite(val, decval);
      cpu6502_setFlag(CPUSTAT_ZERO, decval == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (decval & BIT_MASK_8) != 0);
      
      // CMP
      uint8_t memVal = memRead(val);

      int8_t signedResult = ((int8_t) reg.a) - ((int8_t) memVal);
      if (reg.a < memVal) {
        cpu6502_setFlag(CPUSTAT_ZERO, 0);
        cpu6502_setFlag(CPUSTAT_CARRY, 0);
      } else if (reg.a == memVal) {
        cpu6502_setFlag(CPUSTAT_ZERO, 1);
        cpu6502_setFlag(CPUSTAT_CARRY, 1);
      } else if (reg.a > memVal) {
        cpu6502_setFlag(CPUSTAT_ZERO, 0);
        cpu6502_setFlag(CPUSTAT_CARRY, 1);
      }

      cpu6502_setFlag(CPUSTAT_NEGATIVE, signedResult < 0);
      reg.pc += b->count;
      break;
    }
    case I_ILL_ISC:
    {
      // INC
      uint8_t incval = memRead(val) + 1;
      memWrite(val, incval);
      cpu6502_setFlag(CPUSTAT_ZERO, incval == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (incval & BIT_MASK_8) != 0);
      
      // SBC
      uint8_t memoryVal = ~memRead(val);
      uint16_t sum = reg.a + memoryVal + ((uint16_t)((reg.p & CPUSTAT_CARRY) > 0));
      cpu6502_setFlag(CPUSTAT_CARRY, sum > 0xFF);
      cpu6502_setFlag(CPUSTAT_OVERFLOW, (reg.a ^ sum) & (memoryVal ^ sum) & 0x80);
      reg.a = (uint8_t) sum;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_ILL_LAS:
    {
      // LDA
      reg.a = memRead(val);
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      
      // TSX
      reg.x = reg.s;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.x == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.x & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_ILL_LAX:
    {
      // LDA
      reg.a = memRead(val);
      reg.x = reg.a;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.x == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.x & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_ILL_LXA:
    {
      // (magic) AND
      uint8_t calcVal = (rand() % 0xFF) & reg.a;
      reg.a = calcVal;
      reg.x = calcVal;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_ILL_RLA:
    {
      // ROL
      uint8_t storedVal = reg.a;
      if (!(b->addressingMode == AM_ACCUMULATOR)) {
        storedVal = memRead(val);
      }

      bool oldCarry = ((reg.p & CPUSTAT_CARRY) > 0);
      cpu6502_setFlag(CPUSTAT_CARRY, (storedVal >> 7) & 1);
      storedVal = storedVal << 1;
      storedVal = storedVal | oldCarry;

      cpu6502_setFlag(CPUSTAT_ZERO, storedVal == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (storedVal & BIT_MASK_8) != 0);

      if (b->addressingMode == AM_ACCUMULATOR) {
        reg.a = storedVal;
      } else {
        memWrite(val, storedVal);
      }
      
      // AND
      reg.a = memRead(val) & reg.a;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_ILL_RRA:
    {
      uint8_t storedVal = reg.a;
      if (!(b->addressingMode == AM_ACCUMULATOR)) {
        storedVal = memRead(val);
      }

      bool oldCarry = ((reg.p & CPUSTAT_CARRY) > 0);
      cpu6502_setFlag(CPUSTAT_CARRY, storedVal & 1);
      storedVal = storedVal >> 1;
      storedVal = storedVal | (oldCarry << 7);

      cpu6502_setFlag(CPUSTAT_ZERO, storedVal == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (storedVal & BIT_MASK_8) != 0);

      if (b->addressingMode == AM_ACCUMULATOR) {
        reg.a = storedVal;
      } else {
        memWrite(val, storedVal);
      }

      uint8_t memoryVal = memRead(val);
      uint16_t sum = reg.a + memoryVal + ((uint16_t)((reg.p & CPUSTAT_CARRY) > 0));
      cpu6502_setFlag(CPUSTAT_CARRY, sum > 0xFF);
      cpu6502_setFlag(CPUSTAT_OVERFLOW, (reg.a ^ sum) & (memoryVal ^ sum) & 0x80);
      reg.a = (uint8_t) sum;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_ILL_SAX:
    {
      memWrite(val, reg.a & reg.x);
      reg.pc += b->count;
      break;
    }
    case I_ILL_SBX:
    {
      // CMP
      reg.x -= 1;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.x == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.x & BIT_MASK_8) != 0);
      
      // DEX
      reg.x -= 1;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.x == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.x & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_ILL_SHA:
    {
      memWrite(val, reg.a & reg.x & (uint8_t)(((val & 0xF0) >> 0x0F) + 1));
      reg.pc += b->count;
      break;
    }
    case I_ILL_SHX:
    {
      memWrite(val, reg.x & (uint8_t)(((val & 0xF0) >> 0x0F) + 1));
      reg.pc += b->count;
      break;
    }
    case I_ILL_SHY:
    {
      memWrite(val, reg.y & (uint8_t)(((val & 0xF0) >> 0x0F) + 1));
      reg.pc += b->count;
      break;
    }
    case I_ILL_SLO:
    {
      // SLO
      uint8_t storedVal = reg.a;
      if (!(b->addressingMode == AM_ACCUMULATOR)) {
        storedVal = memRead(val);
      }

      cpu6502_setFlag(CPUSTAT_CARRY, (storedVal >> 7) & 1);
      storedVal = storedVal << 1;

      cpu6502_setFlag(CPUSTAT_ZERO, storedVal == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (storedVal & BIT_MASK_8) != 0);

      if (b->addressingMode == AM_ACCUMULATOR) {
        reg.a = storedVal;
      } else {
        memWrite(val, storedVal);
      }
      
      // ORA
      reg.a = memRead(val) | reg.a;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_ILL_SRE:
    {
      // LSR
      uint8_t storedVal = reg.a;
      if (!(b->addressingMode == AM_ACCUMULATOR)) {
        storedVal = memRead(val);
      }

      cpu6502_setFlag(CPUSTAT_CARRY, storedVal & 1);
      storedVal = storedVal >> 1;

      cpu6502_setFlag(CPUSTAT_ZERO, storedVal == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (storedVal & BIT_MASK_8) != 0);

      if (b->addressingMode == AM_ACCUMULATOR) {
        reg.a = storedVal;
      } else {
        memWrite(val, storedVal);
      }

      // EOR
      reg.a = memRead(val) ^ reg.a;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_ILL_TAS:
    {
      reg.s = reg.a & reg.x;
      memWrite(val, reg.a & reg.x & (uint8_t)(((val & 0xF0) >> 0x0F) + 1));
      reg.pc += b->count;
      break;
    }
    case I_ILL_USBC:
    {
      uint8_t memoryVal = ~memRead(val);
      uint16_t sum = reg.a + memoryVal + ((uint16_t)((reg.p & CPUSTAT_CARRY) > 0));
      cpu6502_setFlag(CPUSTAT_CARRY, sum > 0xFF);
      cpu6502_setFlag(CPUSTAT_OVERFLOW, (reg.a ^ sum) & (memoryVal ^ sum) & 0x80);
      reg.a = (uint8_t) sum;
      cpu6502_setFlag(CPUSTAT_ZERO, reg.a == 0);
      cpu6502_setFlag(CPUSTAT_NEGATIVE, (reg.a & BIT_MASK_8) != 0);
      reg.pc += b->count;
      break;
    }
    case I_ILL_NOP:
    {
      reg.pc += b->count;
      break;
    }
    case I_ILL_JAM:
    {
      cpuerrno = 1;
      clockMode = CPUCLOCK_HALT;
      reg.pc += b->count;
      break;
    }
    default:
    {
      cpuerrno = 1;
      clockMode = CPUCLOCK_HALT;
      reg.pc += b->count;
      break;
    }
  }
  return cycleCount;
}

uint8_t cpu6502_getErrno() {
  return cpuerrno;
}
