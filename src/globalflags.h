#ifndef GLOBALFLAGS_H
#define GLOBALFLAGS_H

#define TRUE 1
#define FALSE 0

#define force_inline __attribute__((always_inline)) inline

/**
 * @brief Platform value
 *
 * VALUES
 * ------
 * 0 : macOS
 * 1 : Zilog Z80 (TI Calculator)
 */
#define PLATFORM 0

/**
 * @brief Toggle verbose exceptions.
 */
#define VERBOSE_EXC FALSE

/**
 * @brief Generate CPU trace.
 */
#define LOGGING FALSE

/**
 * @brief Record & display perfomance metrics.
 */
#define PERFORMANCE_DEBUG TRUE

/**
 * @brief Run in headless mode (PPU will not render)
 */
#define HEADLESS FALSE

/**
 * @brief Minimum number of microseconds which must occur
 *        before the display is re-rendered.
 *        NOTE: PPU will continue to render scanlines
 */
#define MIN_DRAW_INTERVAL 16667

/**
 * @brief Set the display scale.
 */
#define DISPLAY_SCALE 2

/**
 * @brief Allow PPU to catch up with CPU after every instruction.
 *        Otherwise, if faslse, catch up every frame
 */
#define PPU_IMMEDIATE_CATCHUP TRUE

/**
 * @brief Determine how the CPU should handle programs
 *        CPUEMU_INTERPRET_DIRECT - Decode instruction every time 
 *                                  it is encountered.
 *        CPUEMU_INTERPRET_CACHED - Store encountered instructions as
 *                                  bytecode for quicker decoding.
 *       
 */
#define EMU_MODE CPUEMU_INTERPRET_CACHED

// magic numbers

#define DISPLAY_WIDTH 256

#define DISPLAY_HEIGHT 240

#define DISPLAY_BITMAP_SIZE (DISPLAY_WIDTH * DISPLAY_HEIGHT)

#define DISPLAY_FRAMERATE 60

#define DISPLAY_FRAME_USEC 16667

#define CPU_FREQUENCY 1789773

#define CPU_FRAME_CYCLES 29780

#define PPU_FRAME_CYCLES 89340

#define PPU_SCANLINE_CYCLES 341

#define BIT_FILL_0 0x0
#define BIT_FILL_1 0x1
#define BIT_FILL_2 0x3
#define BIT_FILL_3 0x7
#define BIT_FILL_4 0xF
#define BIT_FILL_5 0x1F
#define BIT_FILL_6 0x3F
#define BIT_FILL_7 0x7F
#define BIT_FILL_8 0xFF

#define BIT_MASK_1 0x1
#define BIT_MASK_2 0x2
#define BIT_MASK_3 0x4
#define BIT_MASK_4 0x8
#define BIT_MASK_5 0x10
#define BIT_MASK_6 0x20
#define BIT_MASK_7 0x40
#define BIT_MASK_8 0x80

#include <stdint.h>

typedef enum { false = 0, true = 1} bool;

typedef struct {
  uint8_t* data;
  uint32_t bytes;
} FileBinary;

typedef enum {
  CPUEMU_INTERPRET_DIRECT,
  CPUEMU_INTERPRET_CACHED,
  CPUEMU_DISASSEMBLE
} CPUEmulationMode;

typedef enum {
  CPUCLOCK_SUSPENDED,
  CPUCLOCK_STEP_MANUAL,
  CPUCLOCK_HALT
} CPUClockMode;

typedef enum {
  DASM_MINIMAL  = 0x0,
  DASM_SHOW_MEM = 0x1,
  DASM_SHOW_ADDR = 0x2
} CPUDasmFlags;

typedef enum {
    CPUSTAT_CARRY       = BIT_MASK_1,
    CPUSTAT_ZERO        = BIT_MASK_2,
    CPUSTAT_NO_INTRPT   = BIT_MASK_3,
    CPUSTAT_DECIMAL     = BIT_MASK_4,
    CPUSTAT_BREAK       = BIT_MASK_5,
    CPUSTAT_BREAK2      = BIT_MASK_6,
    CPUSTAT_OVERFLOW    = BIT_MASK_7,
    CPUSTAT_NEGATIVE    = BIT_MASK_8,
} CPUStatusFlag;

typedef enum {
  AM_UNSET          = 0,
  AM_ACCUMULATOR    = 1,
  AM_IMPLIED        = 2,
  AM_IMMEDIATE      = 3,
  AM_ABSOLUTE       = 4,
  AM_ZERO_PAGE      = 5,
  AM_RELATIVE       = 6,
  AM_ABS_INDIRECT   = 7,
  AM_ABS_X          = 8,
  AM_ABS_Y          = 9,
  AM_ZP_X           = 10,
  AM_ZP_Y           = 11,
  AM_ZP_X_INDIRECT  = 12,
  AM_ZP_INDIRECT_Y  = 13
} CPUAddressingMode;

typedef enum {
  I_UNSET = 0,
  I_ADC   = 1,
  I_AND   = 2,
  I_ASL   = 3,
  I_BCC   = 4,
  I_BCS   = 5,
  I_BEQ   = 6,
  I_BIT   = 7,
  I_BMI   = 8,
  I_BNE   = 9,
  I_BPL   = 10,
  I_BRK   = 11,
  I_BVC   = 12,
  I_BVS   = 13,
  I_CLC   = 14,
  I_CLD   = 15,
  I_CLI   = 16,
  I_CLV   = 17,
  I_CMP   = 18,
  I_CPX   = 19,
  I_CPY   = 20,
  I_DEC   = 21,
  I_DEX   = 22,
  I_DEY   = 23,
  I_EOR   = 24,
  I_INC   = 25,
  I_INX   = 26,
  I_INY   = 27,
  I_JMP   = 28,
  I_JSR   = 29,
  I_LDA   = 30,
  I_LDX   = 31,
  I_LDY   = 32,
  I_LSR   = 33,
  I_NOP   = 34,
  I_ORA   = 35,
  I_PHA   = 36,
  I_PHP   = 37,
  I_PLA   = 38,
  I_PLP   = 39,
  I_ROL   = 40,
  I_ROR   = 41,
  I_RTI   = 42,
  I_RTS   = 43,
  I_SBC   = 44,
  I_SEC   = 45,
  I_SED   = 46,
  I_SEI   = 47,
  I_STA   = 48,
  I_STX   = 49,
  I_STY   = 50,
  I_TAX   = 51,
  I_TAY   = 52,
  I_TSX   = 53,
  I_TXA   = 54,
  I_TXS   = 55,
  I_TYA   = 56,
  I_ILL_ALR   = 57,
  I_ILL_ANC   = 58,
  I_ILL_ANC2  = 59,
  I_ILL_ANE   = 60,
  I_ILL_ARR   = 61,
  I_ILL_DCP   = 62,
  I_ILL_ISC   = 63,
  I_ILL_LAS   = 64,
  I_ILL_LAX   = 65,
  I_ILL_LXA   = 66,
  I_ILL_RLA   = 67,
  I_ILL_RRA   = 68,
  I_ILL_SAX   = 69,
  I_ILL_SBX   = 70,
  I_ILL_SHA   = 71,
  I_ILL_SHX   = 72,
  I_ILL_SHY   = 73,
  I_ILL_SLO   = 74,
  I_ILL_SRE   = 75,
  I_ILL_TAS   = 76,
  I_ILL_USBC  = 77,
  I_ILL_NOP   = 78,
  I_ILL_JAM   = 79
} CPUMnemonic;

typedef struct {
  CPUMnemonic mnemonic;
  CPUAddressingMode addressingMode;
  uint8_t count;
  uint8_t data[3];
} Bytecode;

typedef struct {
  Bytecode* bytecodes;
  uint16_t addrMap[65536];
  uint16_t bytecodeCount;
} BytecodeProgram;

typedef struct {
  uint8_t a;
  uint8_t x;
  uint8_t y;
  uint16_t pc;
  uint8_t s;
  uint8_t p;
} CPURegisters;

extern uint32_t bitmap[DISPLAY_BITMAP_SIZE];

static const uint32_t colors[64] =
{
0x757575, 0x271B8F, 0x0000AB, 0x47009F, 0x8F0077, 0xAB0013, 0xA70000, 0x7F0B00,
0x432F00, 0x004700, 0x005100, 0x003F17, 0x1B3F5F, 0x000000, 0x000000, 0x000000,
0xBCBCBC, 0x0073EF, 0x233BEF, 0x8300F3, 0xBF00BF, 0xE7005B, 0xDB2B00, 0xCB4F0F,
0x8B7300, 0x009700, 0x00AB00, 0x00933B, 0x00838B, 0x000000, 0x000000, 0x000000,
0xFFFFFF, 0x3FBFFF, 0x5F97FF, 0xA78BFD, 0xF77BFF, 0xFF77B7, 0xFF7763, 0xFF9B3B,
0xF3BF3F, 0x83D313, 0x4FDF4B, 0x58F898, 0x00EBDB, 0x000000, 0x000000, 0x000000,
0xFFFFFF, 0xABE7FF, 0xC7D7FF, 0xD7CBFF, 0xFFC7FF, 0xFFC7DB, 0xFFBFB3, 0xFFDBAB,
0xFFE7A3, 0xE3FFA3, 0xABF3BF, 0xB3FFCF, 0x9FFFF3, 0x000000, 0x000000, 0x000000
};

#endif
