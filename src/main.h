/**
 * @file main.h
 * @author Noah Sadir (development.noahsadir@gmail.com)
 * @brief Main driver program for NES emulator
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

#ifndef MAIN_H
#define MAIN_H

#include "globalflags.h"
#include "bus.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Load a ROM from the specified path
 * 
 * @param path the file path
 * @param fp the file pointer
 * @return true if the file was opened successfully
 * @return false upon failure
 */
bool main_loadROM(char* path, FileBinary* binary);

/**
 * @brief Compare generated vs correct CPU traces.
 */
void main_compareCPUTraces();

/**
 * @brief Main entry point of the program.
 *
 * @param argc the number of args
 * @param argv the contents of args
 * @return int the exit status
 */
int main(int argc, char* argv[]);

#endif
