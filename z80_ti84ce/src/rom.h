/**
 * @file rom.h
 * @author Noah Sadir (development.noahsadir@gmail.com)
 * @brief Contains the binary for an iNES ROM file
 * @version 1.0
 * @date 2022-07-13
 * 
 * @copyright Copyright (c) 2022 Noah Sadir
 * 
 */

#ifndef ROM_H
#define ROM_H

/**
 * Specify the size of the ROM in bytes here
 */
#define ROM_SIZE 0

#include <stdint.h>

static const uint16_t rom_size = ROM_SIZE;

/**
 * The binary for the ROM
 * 
 * CONVERSION STEPS
 * -------------------------
 * - Obtain a hex dump of the iNES file (with spacing/delimiters between bytes)
 * - Use string replacement (replace " " with ", 0x") to convert to array format
 * - Copy string into brackets
 * - Make comma adjustments so that it compiles
 * - Set ROM_SIZE macro to the proper size
 */
static const uint8_t rom_binary[ROM_SIZE] = {

};

#endif