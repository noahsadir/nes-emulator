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
#define PERFORMANCE_DEBUG FALSE

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
 * @brief Limit clock speed to original NES (1.789773 MHz)
 */
#define LIMIT_CLOCK_SPEED TRUE

/**
 * @brief Set the display scale.
 */
#define DISPLAY_SCALE 2

/**
 * @brief Allow PPU to catch up with CPU after every instruction.
 *        Otherwise, if faslse, catch up every frame
 *        NOTE: This also affects PPU-will operate at scanline-level granularity
 *              if enabled, or just frame-level granularity if not
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

#define DISPLAY_PIXEL_SIZE ((DISPLAY_WIDTH * DISPLAY_SCALE) * (DISPLAY_HEIGHT * DISPLAY_SCALE))

#define DISPLAY_FRAMERATE 60

#define DISPLAY_FRAME_USEC 16667

#define CPU_FREQUENCY 1789773

#define CPU_FRAME_CYCLES 29780

#define PPU_FRAME_CYCLES 89340

#define PPU_SCANLINE_CYCLES 341

// manually define background bank for debug nametable
#define DBG_BKG_BANK 0

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

// Taken from NES Rust Tutorial (see readme:references)
static const uint32_t colors2[64] = {
  0x808080, 0x003DA6, 0x0012B0, 0x440096, 0xA1005E,
  0xC70028, 0xBA0600, 0x8C1700, 0x5C2F00, 0x104500,
  0x054A00, 0x00472E, 0x004166, 0x000000, 0x050505,
  0x050505, 0xC7C7C7, 0x0077FF, 0x2155FF, 0x8237FA,
  0xEB2FB5, 0xFF2950, 0xFF2200, 0xD63200, 0xC46200,
  0x358000, 0x058F00, 0x008A55, 0x0099CC, 0x212121,
  0x090909, 0x090909, 0xFFFFFF, 0x0FD7FF, 0x69A2FF,
  0xD480FF, 0xFF45F3, 0xFF618B, 0xFF8833, 0xFF9C12,
  0xFABC20, 0x9FE30E, 0x2BF035, 0x0CF0A4, 0x05FBFF,
  0x5E5E5E, 0x0D0D0D, 0x0D0D0D, 0xFFFFFF, 0xA6FCFF,
  0xB3ECFF, 0xDAABEB, 0xFFA8F9, 0xFFABB3, 0xFFD2B0,
  0xFFEFA6, 0xFFF79C, 0xD7E895, 0xA6EDAF, 0xA2F2DA,
  0x99FFFC, 0xDDDDDD, 0x111111, 0x111111
};


extern uint32_t bitmap[DISPLAY_BITMAP_SIZE];
extern uint8_t oamRAM[0x00FF];
extern uint8_t vidRAM[0x2000];
extern uint8_t paletteRAM[32];
extern uint8_t chrCache[512][64];
extern uint32_t debugbmp[DISPLAY_PIXEL_SIZE];

#endif
