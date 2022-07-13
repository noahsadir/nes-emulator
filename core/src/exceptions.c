/**
 * @file exceptions.c
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

#include "exceptions.h"

FILE* fp;

void exc_traceInit() {
    fp = fopen("./debug/output.log", "w");
    fp = fopen("./debug/output.log", "a");
}

void exc_panic(char* message) {
    printf("\n\n *** PANIC: %s *** \n\n", message);
    bus_triggerCPUPanic();
    //exit(1);
}

void exc_message(char* message) {
    printf("\n\n *** %s *** \n\n", message);
}

void exc_panic_invalidIO(uint16_t address) {
    printf("\n\n *** PANIC: INVALID CPU I/O (%04x) *** \n\n", address);
    bus_triggerCPUPanic();
    //exit(1);
}

void exc_panic_invalidPPUIO(uint16_t address) {
    printf("\n\n *** PANIC: INVALID PPU I/O (%04x) *** \n\n", address);
    bus_triggerCPUPanic();
    //exit(1);
}

void exc_panic_illegalInstruction(uint8_t opcode) {
    printf("\n\n *** PANIC: ILLEGAL INSTRUCTION (%02x) *** \n\n", opcode);
    bus_triggerCPUPanic();
    //exit(1);
}

void exc_panic_stackOverflow() {
    printf("\n\n *** PANIC: STACK OVERFLOW *** \n\n");
    bus_triggerCPUPanic();
}

void exc_panic_stackUnderflow() {
    printf("\n\n *** PANIC: STACK UNDERFLOW *** \n\n");
    bus_triggerCPUPanic();
}

void exc_trace(int paramCount, char* redirectString, uint16_t address, uint16_t pc, char* instructionCode, uint8_t opcode, uint8_t firstParam, uint8_t secondParam, char* parameterString, uint16_t offsetAddress, uint8_t reg_a, uint8_t reg_status, uint8_t reg_x, uint8_t reg_y, uint8_t stackPointer, uint64_t cpuCycles, uint64_t ppuCycles, uint64_t ppuBlanks) {
    
    if (fp == NULL)
    {
        //printf("Error opening file!\n");
        //exit(1);
        printf("");
        printf("%04X ", pc);
        printf("%-1s", "");
        printf("%02X ", opcode);

        
        if (paramCount >= 1) {
            printf("%02X ", firstParam);
        } else {
            printf("%-3s", "");
        }

        if (paramCount >= 2) {
            printf("%02X ", secondParam);
        } else {
            printf("%-3s", "");
        }

        printf("%-1s", "");
        printf("%-4s %-8s", instructionCode, parameterString);

        
        if (redirectString != NULL) {
            printf("%-25s", redirectString);
            //fprintf(fp, "%-20s", "");
        } else {
            printf("%-25s", "");
        }

        printf("A:%02X X:%02X Y:%02X P:%02X SP:%02X ", reg_a, reg_x, reg_y, reg_status, stackPointer);

        printf("PPU: %-3llu,%-3llu CYC:%llu", ppuBlanks, ppuCycles, cpuCycles);
        printf("\n");
        
    } else {
        setbuf(fp, NULL);
        
        fprintf(fp, "");
        fprintf(fp, "%04X ", pc);
        fprintf(fp, "%-1s", "");
        fprintf(fp, "%02X ", opcode);

        
        if (paramCount >= 1) {
            fprintf(fp, "%02X ", firstParam);
        } else {
            fprintf(fp, "%-3s", "");
        }

        if (paramCount >= 2) {
            fprintf(fp, "%02X ", secondParam);
        } else {
            fprintf(fp, "%-3s", "");
        }

        fprintf(fp, "%-1s", "");
        fprintf(fp, "%-4s %-8s", instructionCode, parameterString);

        
        if (redirectString != NULL) {
            fprintf(fp, "%-25s", redirectString);
            //fprintf(fp, "%-20s", "");
        } else {
            fprintf(fp, "%-25s", "");
        }

        fprintf(fp, "A:%02X X:%02X Y:%02X P:%02X SP:%02X ", reg_a, reg_x, reg_y, reg_status, stackPointer);

        fprintf(fp, "PPU: %-3llu,%-3llu CYC:%llu", ppuBlanks, ppuCycles, cpuCycles);
        fprintf(fp, "\n");
    }
}