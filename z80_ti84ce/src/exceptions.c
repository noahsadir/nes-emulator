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

int line = 0;
void exc_traceInit() {

}

void exc_panic(char* message) {
    printf(" *** PANIC: %s *** ", message);
    bus_triggerCPUPanic();
    //exit(1);
}

void exc_message(char* message) {
    printf(" *** %s *** ", message);
}

void exc_panic_invalidIO(uint16_t address) {
    printf(" *** PANIC: INVALID CPU I/O (%04x) *** ", address);
    bus_triggerCPUPanic();
    //exit(1);
}

void exc_panic_invalidPPUIO(uint16_t address) {
    printf(" *** PANIC: INVALID PPU I/O (%04x) *** ", address);
    bus_triggerCPUPanic();
    //exit(1);
}

void exc_panic_illegalInstruction(uint8_t opcode) {
    printf(" *** PANIC: ILLEGAL INSTRUCTION (%02x) *** ", opcode);
    bus_triggerCPUPanic();
    //exit(1);
}

void exc_panic_stackOverflow() {
    printf(" *** PANIC: STACK OVERFLOW *** ");
    bus_triggerCPUPanic();
}

void exc_panic_stackUnderflow() {
    printf(" *** PANIC: STACK UNDERFLOW *** ");
    bus_triggerCPUPanic();
}

void exc_trace(int paramCount, char* redirectString, uint16_t address, uint16_t pc, char* instructionCode, uint8_t opcode, uint8_t firstParam, uint8_t secondParam, char* parameterString, uint16_t offsetAddress, uint8_t reg_a, uint8_t reg_status, uint8_t reg_x, uint8_t reg_y, uint8_t stackPointer, uint64_t cpuCycles, uint64_t ppuCycles, uint64_t ppuBlanks) {
    
    if (line % 100 == 0) {
        printf("%04X: ", line);
        printf("%04X", pc);
        printf("%-1s", "");

        /*
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
        }*/

        printf("%-1s", "");
        printf("%-4s %-8s", instructionCode, parameterString);
        printf("%-1s", "");
    }
    line += 1;
    

    /*
    if (redirectString != NULL) {
        printf("%-25s", redirectString);
        //fprintf(fp, "%-20s", "");
    } else {
        printf("%-25s", "");
    }

    printf("A:%02X X:%02X Y:%02X P:%02X SP:%02X ", reg_a, reg_x, reg_y, reg_status, stackPointer);

    printf("PPU: %-3llu,%-3llu CYC:%llu", ppuBlanks, ppuCycles, cpuCycles);
    printf("\n");
    */
}