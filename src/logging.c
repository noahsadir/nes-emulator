/**
 * @file logging.c
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

#include "logging.h"

#if (LOGGING)
FILE* fp;
FILE* asmfp;

const char* mnemonicStr[80] = {
  "??? ", "ADC ", "AND ", "ASL ", "BCC ",
  "BCS ", "BEQ ", "BIT ", "BMI ", "BNE ",
  "BPL ", "BRK ", "BVC ", "BVS ", "CLC ",
  "CLD ", "CLI ", "CLV ", "CMP ", "CPX ",
  "CPY ", "DEC ", "DEX ", "DEY ", "EOR ",
  "INC ", "INX ", "INY ", "JMP ", "JSR ",
  "LDA ", "LDX ", "LDY ", "LSR ", "NOP ",
  "ORA ", "PHA ", "PHP ", "PLA ", "PLP ",
  "ROL ", "ROR ", "RTI ", "RTS ", "SBC ",
  "SEC ", "SED ", "SEI ", "STA ", "STX ",
  "STY ", "TAX ", "TAY ", "TSX ", "TXA ",
  "TXS ", "TYA ", "ALR ", "ANC ", "ANC ",
  "ANE ", "ARR ", "DCP ", "ISC ", "LAS ",
  "LAX ", "LXA ", "RLA ", "RRA ", "SAX ",
  "SBX ", "SHA ", "SHX ", "SHY ", "SLO ",
  "SRE ", "TAS ", "USBC ", "NOP ", "JAM "
};

const char* hexStr[16] = { 
  "0", "1", "2", "3", "4", "5", "6", "7",
  "8", "9", "A", "B", "C", "D", "E", "F"
};

void logging_init() {
  fp = fopen("./debug/output.log", "w");
  fp = fopen("./debug/output.log", "a");
  asmfp = fopen("./debug/rom.asm", "w");
  asmfp = fopen("./debug/rom.asm", "a");
}

void logging_kill() {
  fclose(fp);
  fclose(asmfp);
}

void logging_saveNESTrace(char* trace, uint32_t cpuCycles) {
  fprintf(fp, "%s CYC: %d\n", trace, cpuCycles);
}

void logging_saveDisassembly(char* line) {
  fprintf(asmfp, "%s\n", line);
}

void logging_intToHexString(uint32_t val, uint8_t size, char* output) {
  strcpy(output, "");
  for (int i = size - 4; i >= 0; i -= 4) {
    strcat(output, hexStr[(val >> i) & BIT_FILL_4]);
  }
}

void logging_bytecodeToTrace(CPURegisters reg, Bytecode* b, char* line, void(*memWrite)(uint16_t, uint8_t), uint8_t(*memRead)(uint16_t)) {

  strcpy(line, "");
  char pcStr[5];
  logging_intToHexString(reg.pc, 16, pcStr);
  strcat(line, pcStr);
  strcat(line, "  ");

  if (b->count >= 1) {
    strcat(line, hexStr[b->data[0] >> 4]);
    strcat(line, hexStr[b->data[0] & BIT_FILL_4]);
    strcat(line, " ");
  } else {
    strcat(line, "   ");
  }

  if (b->count >= 2) {
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    strcat(line, " ");
  } else {
    strcat(line, "   ");
  }

  if (b->count >= 3) {
    strcat(line, hexStr[b->data[2] >> 4]);
    strcat(line, hexStr[b->data[2] & BIT_FILL_4]);
    strcat(line, " ");
  } else {
    strcat(line, "   ");
  }

  switch (b->mnemonic) {
    case I_ILL_ALR: case I_ILL_ANC2: case I_ILL_ANC: case I_ILL_ANE: case I_ILL_ARR:
    case I_ILL_DCP: case I_ILL_ISC: case I_ILL_JAM: case I_ILL_LAS: case I_ILL_LAX:
    case I_ILL_LXA: case I_ILL_NOP: case I_ILL_RLA: case I_ILL_RRA: case I_ILL_SAX:
    case I_ILL_SBX: case I_ILL_SHA: case I_ILL_SHX: case I_ILL_SHY: case I_ILL_SLO:
    case I_ILL_SRE: case I_ILL_TAS: case I_ILL_USBC:
    {
      strcat(line, "*");
      break;
    }
    default:
    {
      strcat(line, " ");
      break;
    }
  }

  strcat(line, mnemonicStr[b->mnemonic]);
  char hex[8];
  if (b->addressingMode == AM_IMMEDIATE) {
    strcat(line, "#$");
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
  } else if (b->addressingMode == AM_ABSOLUTE) {
    strcat(line, "$");
    strcat(line, hexStr[b->data[2] >> 4]);
    strcat(line, hexStr[b->data[2] & BIT_FILL_4]);
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    if (b->mnemonic != I_JSR && b->mnemonic != I_JMP) {
      strcat(line, " = ");
      uint16_t val = ((uint16_t)b->data[2] << 8) | (uint16_t)b->data[1];
      logging_intToHexString(memRead(val), 8, hex);
      strcat(line, hex);
    }
  } else if (b->addressingMode == AM_ZERO_PAGE) {
    strcat(line, "$");
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    strcat(line, " = ");
    uint16_t val = (uint16_t)b->data[1];
    logging_intToHexString(memRead(val), 8, hex);
    strcat(line, hex);
  } else if (b->addressingMode == AM_RELATIVE) {
    strcat(line, "$");
    logging_intToHexString(reg.pc + (int8_t)(b->data[1] + 2), 16, hex);
    strcat(line, hex);
  } else if (b->addressingMode == AM_ABS_INDIRECT) {
    strcat(line, "($");
    strcat(line, hexStr[b->data[2] >> 4]);
    strcat(line, hexStr[b->data[2] & BIT_FILL_4]);
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    strcat(line, ") = ");
    uint16_t addr = ((uint16_t)b->data[2] << 8) | (uint16_t)b->data[1];
    uint16_t addrinc = addr;
    if ((addrinc & 0x00FF) == 0x00FF) {
      addrinc &= 0xFF00;
    } else {
      addrinc += 1;
    }
    uint16_t val = ((uint16_t)memRead(addrinc) << 8) | (uint16_t)memRead(addr);
    logging_intToHexString(val, 16, hex);
    strcat(line, hex);
  } else if (b->addressingMode == AM_ABS_X) {
    strcat(line, "$");
    strcat(line, hexStr[b->data[2] >> 4]);
    strcat(line, hexStr[b->data[2] & BIT_FILL_4]);
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    strcat(line, ",X @ ");
    uint16_t val = ((uint16_t)b->data[2] << 8) | (uint16_t)b->data[1];
    logging_intToHexString(val + reg.x, 16, hex);
    strcat(line, hex);
    strcat(line, " = ");
    logging_intToHexString(memRead(val + reg.x), 8, hex);
    strcat(line, hex);
  } else if (b->addressingMode == AM_ABS_Y) {
    strcat(line, "$");
    strcat(line, hexStr[b->data[2] >> 4]);
    strcat(line, hexStr[b->data[2] & BIT_FILL_4]);
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    strcat(line, ",Y @ ");
    uint16_t val = ((uint16_t)b->data[2] << 8) | (uint16_t)b->data[1];
    logging_intToHexString(val + reg.y, 16, hex);
    strcat(line, hex);
    strcat(line, " = ");
    logging_intToHexString(memRead(val + reg.y), 8, hex);
    strcat(line, hex);
  } else if (b->addressingMode == AM_ZP_X) {
    strcat(line, "$");
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    strcat(line, ",X @ ");
    uint8_t val = (uint16_t)b->data[1] + reg.x;
    logging_intToHexString(val, 8, hex);
    strcat(line, hex);
    strcat(line, " = ");
    logging_intToHexString(memRead(val), 8, hex);
    strcat(line, hex);
  } else if (b->addressingMode == AM_ZP_Y) {
    strcat(line, "$");
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    strcat(line, ",Y @ ");
    uint8_t val = (uint16_t)b->data[1] + reg.y;
    logging_intToHexString(val, 8, hex);
    strcat(line, hex);
    strcat(line, " = ");
    logging_intToHexString(memRead(val), 8, hex);
    strcat(line, hex);
  } else if (b->addressingMode == AM_ZP_X_INDIRECT) {
    strcat(line, "($");
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    strcat(line, ",X) @ ");
    logging_intToHexString(b->data[1] + reg.x, 8, hex);
    strcat(line, hex);
    strcat(line, " = ");
    uint8_t zpVal = b->data[1] + reg.x;
    uint8_t zpValInc = zpVal + 1;
    uint16_t addr = ((uint16_t)memRead(zpValInc) << 8) | (uint16_t)memRead(zpVal);
    logging_intToHexString(addr, 16, hex);
    strcat(line, hex);
    strcat(line, " = ");
    logging_intToHexString(memRead(addr), 8, hex);
    strcat(line, hex);
  } else if (b->addressingMode == AM_ZP_INDIRECT_Y) {
    strcat(line, "($");
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    strcat(line, "),Y = ");
    uint8_t zpVal = b->data[1];
    uint8_t zpValInc = zpVal + 1;
    uint16_t addr = ((uint16_t)memRead(zpValInc) << 8) | (uint16_t)memRead(zpVal);
    logging_intToHexString(addr, 16, hex);
    strcat(line, hex);
    addr += reg.y;
    strcat(line, " @ ");
    logging_intToHexString(addr, 16, hex);
    strcat(line, hex);
    strcat(line, " = ");
    logging_intToHexString(memRead(addr), 8, hex);
    strcat(line, hex);
  } else if (b->addressingMode == AM_ACCUMULATOR) {
    strcat(line, "A");
  }
  
  while (strlen(line) < 48) {
    strcat(line, " ");
  }

  char regStr[5];
  logging_intToHexString(reg.a, 8, regStr);
  strcat(line, "A:");
  strcat(line, regStr);
  logging_intToHexString(reg.x, 8, regStr);
  strcat(line, " X:");
  strcat(line, regStr);
  logging_intToHexString(reg.y, 8, regStr);
  strcat(line, " Y:");
  strcat(line, regStr);
  logging_intToHexString(reg.p, 8, regStr);
  strcat(line, " P:");
  strcat(line, regStr);
  logging_intToHexString(reg.s, 8, regStr);
  strcat(line, " SP:");
  strcat(line, regStr);

}

void logging_bytecodeToAssembly(uint32_t pointer, Bytecode* b, char line[128], uint8_t flags) {
  strcpy(line, "");

  if (flags & DASM_SHOW_ADDR) {
    strcat(line, hexStr[(pointer >> 28) & BIT_FILL_4]);
    strcat(line, hexStr[(pointer >> 24) & BIT_FILL_4]);
    strcat(line, hexStr[(pointer >> 20) & BIT_FILL_4]);
    strcat(line, hexStr[(pointer >> 16) & BIT_FILL_4]);
    strcat(line, hexStr[(pointer >> 12) & BIT_FILL_4]);
    strcat(line, hexStr[(pointer >> 8) & BIT_FILL_4]);
    strcat(line, hexStr[(pointer >> 4) & BIT_FILL_4]);
    strcat(line, hexStr[(pointer >> 0) & BIT_FILL_4]);
    strcat(line, " - ");
  }

  if (flags & DASM_SHOW_MEM) {
    if (b->count >= 1) {
      strcat(line, hexStr[b->data[0] >> 4]);
      strcat(line, hexStr[b->data[0] & BIT_FILL_4]);
      strcat(line, " ");
    } else {
      strcat(line, "   ");
    }

    if (b->count >= 2) {
      strcat(line, hexStr[b->data[1] >> 4]);
      strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
      strcat(line, " ");
    } else {
      strcat(line, "   ");
    }

    if (b->count >= 3) {
      strcat(line, hexStr[b->data[2] >> 4]);
      strcat(line, hexStr[b->data[2] & BIT_FILL_4]);
      strcat(line, " ");
    } else {
      strcat(line, "   ");
    }
    strcat(line, " : ");
  }

  strcat(line, mnemonicStr[b->mnemonic]);

  if (b->addressingMode == AM_IMMEDIATE) {
    strcat(line, "#$");
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
  } else if (b->addressingMode == AM_ABSOLUTE) {
    strcat(line, "$");
    strcat(line, hexStr[b->data[2] >> 4]);
    strcat(line, hexStr[b->data[2] & BIT_FILL_4]);
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
  } else if (b->addressingMode == AM_ZERO_PAGE) {
    strcat(line, "$");
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
  } else if (b->addressingMode == AM_RELATIVE) {
    strcat(line, "$");
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
  } else if (b->addressingMode == AM_ABS_INDIRECT) {
    strcat(line, "($");
    strcat(line, hexStr[b->data[2] >> 4]);
    strcat(line, hexStr[b->data[2] & BIT_FILL_4]);
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    strcat(line, ")");
  } else if (b->addressingMode == AM_ABS_X) {
    strcat(line, "$");
    strcat(line, hexStr[b->data[2] >> 4]);
    strcat(line, hexStr[b->data[2] & BIT_FILL_4]);
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    strcat(line, ",X");
  } else if (b->addressingMode == AM_ABS_Y) {
    strcat(line, "$");
    strcat(line, hexStr[b->data[2] >> 4]);
    strcat(line, hexStr[b->data[2] & BIT_FILL_4]);
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    strcat(line, ",Y");
  } else if (b->addressingMode == AM_ZP_X) {
    strcat(line, "$");
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    strcat(line, ",X");
  } else if (b->addressingMode == AM_ZP_Y) {
    strcat(line, "$");
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    strcat(line, ",Y");
  } else if (b->addressingMode == AM_ZP_X_INDIRECT) {
    strcat(line, "($");
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    strcat(line, ",X)");
  } else if (b->addressingMode == AM_ZP_INDIRECT_Y) {
    strcat(line, "($");
    strcat(line, hexStr[b->data[1] >> 4]);
    strcat(line, hexStr[b->data[1] & BIT_FILL_4]);
    strcat(line, "),Y");
  }
}

#else

// dummy functions for when logging is disabled

void logging_init() {
  return;
}

void logging_kill() {
  return;
}

void logging_saveDisassembly(char* line) {
  return;
}

void logging_saveNESTrace(char* trace, uint32_t cpuCycles) {
  return;
}

void logging_bytecodeToTrace(CPURegisters reg, Bytecode* b, char* line, void(*memWrite)(uint16_t, uint8_t), uint8_t(*memRead)(uint16_t)) {
  return;
}

void logging_bytecodeToAssembly(uint32_t pointer, Bytecode* b, char line[128], uint8_t flags) {
  return;
}

void logging_intToHexString(uint32_t val, uint8_t size, char* output) {
  return;
}

#endif
