/**
 * @file chars.h
 * @author Noah Sadir (development.noahsadir@gmail.com)
 * @brief Characters A-Z and 0-9 in NES sprite format.
 *        Useful for outputting custom messages, such as for debugging.
 * @version 1.0
 * @date 2022-07-06
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
#ifndef CHARS_H
#define CHARS_H

static const uint8_t tile_a[16] = {
    0b00111000,
    0b01111100,
    0b11000010,
    0b11000010,
    0b11111110,
    0b11000010,
    0b11000010,
    0b00000000,

    0,0,0,0,0,0,0,0
};

static const uint8_t tile_b[16] = {
    0b11111100,
    0b11000010,
    0b11000010,
    0b11111100,
    0b11000010,
    0b11000010,
    0b11111100,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_c[16] = {
    0b00111110,
    0b01100000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b01100000,
    0b00111110,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_d[16] = {
    0b11111100,
    0b11000010,
    0b11000010,
    0b11000010,
    0b11000010,
    0b11000010,
    0b11111100,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_e[16] = {
    0b11111111,
    0b11000000,
    0b11000000,
    0b11111111,
    0b11000000,
    0b11000000,
    0b11111111,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_f[16] = {
    0b11111111,
    0b11000000,
    0b11000000,
    0b11111111,
    0b11000000,
    0b11000000,
    0b11000000,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_g[16] = {
    0b00111110,
    0b01100000,
    0b11000000,
    0b11001110,
    0b11000010,
    0b01100010,
    0b00111110,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_h[16] = {
    0b11000110,
    0b11000110,
    0b11000110,
    0b11111110,
    0b11000110,
    0b11000110,
    0b11000110,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_i[16] = {
    0b11111110,
    0b00010000,
    0b00010000,
    0b00010000,
    0b00010000,
    0b00010000,
    0b11111110,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_j[16] = {
    0b11111110,
    0b00000110,
    0b00000110,
    0b00000110,
    0b00000110,
    0b10001100,
    0b11111000,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_k[16] = {
    0b11000110,
    0b11001100,
    0b11011000,
    0b11110000,
    0b11011000,
    0b11001100,
    0b11000110,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_l[16] = {
    0b11000000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b11000000,
    0b11111111,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_m[16] = {
    0b11000110,
    0b11101110,
    0b11111110,
    0b11010110,
    0b11000110,
    0b11000110,
    0b11000110,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_n[16] = {
    0b11000110,
    0b11100110,
    0b11010110,
    0b11010110,
    0b11001110,
    0b11001110,
    0b11000110,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_o[16] = {
    0b01111100,
    0b11000110,
    0b10000010,
    0b10000010,
    0b10000010,
    0b11000110,
    0b01111100,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_p[16] = {
    0b11111100,
    0b11000010,
    0b11000010,
    0b11111100,
    0b11000000,
    0b11000000,
    0b11000000,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_q[16] = {
    0b01111100,
    0b11000110,
    0b10000010,
    0b10000110,
    0b11001100,
    0b01111100,
    0b00000110,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_r[16] = {
    0b11111100,
    0b11000110,
    0b11000110,
    0b11111100,
    0b11011000,
    0b11001100,
    0b11000110,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_s[16] = {
    0b01111110,
    0b11000000,
    0b11000000,
    0b01111100,
    0b00000110,
    0b00000110,
    0b11111100,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_t[16] = {
    0b11111110,
    0b00010000,
    0b00010000,
    0b00010000,
    0b00010000,
    0b00010000,
    0b00010000,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_u[16] = {
    0b10000010,
    0b10000010,
    0b10000010,
    0b10000010,
    0b10000010,
    0b11000110,
    0b01111100,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_v[16] = {
    0b10000010,
    0b11000110,
    0b01000100,
    0b01101100,
    0b00101000,
    0b00111000,
    0b00010000,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_w[16] = {
    0b11000110,
    0b11000110,
    0b11000110,
    0b11010110,
    0b11111110,
    0b11101110,
    0b11000110,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_x[16] = {
    0b10000010,
    0b11000110,
    0b01101100,
    0b00111000,
    0b01101100,
    0b11000110,
    0b10000010,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_y[16] = {
    0b10000010,
    0b11000110,
    0b01101100,
    0b00111000,
    0b00010000,
    0b00010000,
    0b00010000,
    0b00000000,

    0,0,0,0,0,0,0,0
};

static const uint8_t tile_z[16] = {
    0b11111110,
    0b00001100,
    0b00011000,
    0b00110000,
    0b01100000,
    0b11000000,
    0b11111110,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_0[16] = {
    0b01111100,
    0b11000110,
    0b10001010,
    0b10010010,
    0b10100010,
    0b11000110,
    0b01111100,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_1[16] = {
    0b01110000,
    0b11010000,
    0b10010000,
    0b00010000,
    0b00010000,
    0b00010000,
    0b11111110,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_2[16] = {
    0b01111100,
    0b10000110,
    0b00001100,
    0b00011000,
    0b00110000,
    0b01100000,
    0b11111110,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_3[16] = {
    0b11111110,
    0b00000100,
    0b00001000,
    0b00000100,
    0b00000010,
    0b00000010,
    0b11111100,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_4[16] = {
    0b00000110,
    0b00001010,
    0b00010010,
    0b00100010,
    0b01111110,
    0b00000010,
    0b00000010,
    0b00000000,

    0,0,0,0,0,0,0,0
};

static const uint8_t tile_5[16] = {
    0b11111110,
    0b11000000,
    0b11000000,
    0b01111100,
    0b00000110,
    0b00000110,
    0b11111100,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_6[16] = {
    0b01111100,
    0b11000010,
    0b10000000,
    0b11111100,
    0b10000010,
    0b10000010,
    0b01111100,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_7[16] = {
    0b11111110,
    0b00000110,
    0b00001100,
    0b00011000,
    0b00110000,
    0b01100000,
    0b11000000,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_8[16] = {
    0b01111100,
    0b10000010,
    0b10000010,
    0b01111100,
    0b10000010,
    0b10000010,
    0b01111100,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

static const uint8_t tile_9[16] = {
    0b01111100,
    0b10000010,
    0b10000010,
    0b01111110,
    0b00000010,
    0b00000010,
    0b11111100,
    0b00000000,
    
    0,0,0,0,0,0,0,0
};

#endif