#include "bus.h"

uint8_t cpuRAM[2048];
uint8_t vidRAM[2048];
uint8_t paletteRAM[32];

uint8_t* prgROM;
uint8_t* chrROM;

struct timespec t1, t2;
struct timeval total1, total2;

double elapsedTime;
uint64_t totalTime = 0;
uint64_t dataPoints = 0;
bool verticalMirroring = false;
bool panicOnUnimplemented = false;
bool limitClockSpeed = true;

uint64_t msecCounter = 0;
uint64_t clocksPerMsec = 0;

SDL_Window* window;
SDL_Surface* surface;

struct JoypadMapping keyMap;

uint8_t bus_readCPU(uint16_t address) {
    if (address < 0x2000) { // cpu ram
        address = address % 0x800; // mirrored
        return cpuRAM[address];
    } else if (address < 0x4000) {
        address = ((address - 0x2000) % 8) + 0x2000;
        if (address == 0x2000) { // ppu control
            return ppu_getRegister(PPU_CONTROL);
        } else if (address == 0x2001) { // ppu mask
            return ppu_getRegister(PPU_MASK);
        } else if (address == 0x2002) { // ppu status
            return ppu_getRegister(PPU_STATUS);
        } else if (address == 0x2003) { // ppu oam address
            return ppu_getRegister(PPU_OAMADDR);
        } else if (address == 0x2004) { // ppu oam data
            return ppu_getRegister(PPU_OAMDATA);
        } else if (address == 0x2005) { // ppu scroll
            return ppu_getRegister(PPU_SCROLL);
        } else if (address == 0x2006) { // ppu address
            return ppu_getRegister(PPU_PPUADDR);
        } else if (address == 0x2007) { // ppu data
            return ppu_getRegister(PPU_PPUDATA);
        }
    } else if (address < 0x4020) { // io registers
        if (address >= 0x4000 && address <= 0x4013) { // apu

        } else if (address == 0x4014) { // ppu oam data
            return ppu_getRegister(PPU_OAMDMA);
        } else if (address == 0x4015) { // apu status

        } else if (address == 0x4016) { // controller
            return (uint8_t) joypad_read();
        } else if (address == 0x4017) { // apu frame counter

        } else {
            
        }
    } else if (address < 0x6000) { // expansion rom
        exc_panic_invalidIO(address);
    } else if (address < 0x8000) { // save ram
        exc_panic_invalidIO(address);
    } else if (address < 0xFFFF) { // prg rom
        address -= 0x8000;
        return prgROM[address];
    }

    return 0x00;
}

void bus_writeCPU(uint16_t address, uint8_t data) {
    if (address < 0x2000) { // cpu ram
        cpuRAM[address] = data;
    } else if (address < 0x4000) { // io registers
        address = ((address - 0x2000) % 8) + 0x2000;
        if (address == 0x2000) { // ppu control
            ppu_setRegister(PPU_CONTROL, data);
        } else if (address == 0x2001) { // ppu mask
            ppu_setRegister(PPU_MASK, data);
        } else if (address == 0x2002) { // ppu status
            ppu_setRegister(PPU_STATUS, data);
        } else if (address == 0x2003) { // ppu oam address
            ppu_setRegister(PPU_OAMADDR, data);
        } else if (address == 0x2004) { // ppu oam data
            ppu_setRegister(PPU_OAMDATA, data);
        } else if (address == 0x2005) { // ppu scroll
            ppu_setRegister(PPU_SCROLL, data);
        } else if (address == 0x2006) { // ppu address
            ppu_setRegister(PPU_PPUADDR, data);
        } else if (address == 0x2007) { // ppu data
            ppu_setRegister(PPU_PPUDATA, data);
        }
    } else if (address < 0x4020) { // i/o registers
        if (address >= 0x4000 && address <= 0x4013) { // apu

        } else if (address == 0x4014) { // ppu oam dma
            ppu_setRegister(PPU_OAMDMA, data);
        } else if (address == 0x4015) { // apu status

        } else if (address == 0x4016) { // controller
            joypad_write(data);
        } else if (address == 0x4017) { // apu frame counter

        } else {
            
        }    
    } else if (address < 0x6000) { // expansion rom
        if (panicOnUnimplemented) exc_panic_invalidIO(address);
    } else if (address < 0x8000) { // save ram
        if (panicOnUnimplemented) exc_panic_invalidIO(address);
    } else if (address < 0xFFFF) { // prg rom
        exc_panic_invalidIO(address);
    }
}

uint16_t bus_readCPUAddr(uint16_t address) {
    if (address < 0x2000) { // cpu ram
        address = address % 0x800; // mirrored
        return ((uint16_t) cpuRAM[address] << 8) | ((uint16_t) cpuRAM[address]);
    } else if (address < 0x4020) { // io registers
        if (panicOnUnimplemented) exc_panic_invalidIO(address);
    } else if (address < 0x6000) { // expansion rom
        if (panicOnUnimplemented) exc_panic_invalidIO(address);
    } else if (address < 0x8000) { // save ram
        if (panicOnUnimplemented) exc_panic_invalidIO(address);
    } else if (address < 0xFFFF) { // prg rom
        address -= 0x8000;
        return (((uint16_t) prgROM[address + 1]) << 8) | ((uint16_t) prgROM[address]);
    }

    return 0x0000;
}

void bus_writeCPUAddr(uint16_t address, uint16_t data) {
    if (address < 0x2000) { // cpu ram
        cpuRAM[address] = (data << 8) >> 8;
        cpuRAM[address + 1] = data >> 8;
    } else {
        if (panicOnUnimplemented) exc_panic_invalidIO(address);
    }
}

uint8_t bus_readPPU(uint16_t address) {
    address = address % 0x4000;
    if (address < 0x2000) { // chr rom
        return chrROM[address];
    } else if (address < 0x3F00) { // vram
        address = (address % 0x2000) + 0x2000;
        if (verticalMirroring) {
            if (address < 0x2400) {
                address = address - 0x2000;
            } else if (address < 0x2800) {
                address = address - 0x2400;
            } else if (address < 0x2C00) {
                address = address - 0x2400;
            } else if (address < 0x2FFF) {
                address = address - 0x2800;
            }
        } else {
            if (address < 0x2400) {
                address = address - 0x2000;
            } else if (address < 0x2800) {
                address = address - 0x2000;
            } else if (address < 0x2C00) {
                address = address - 0x2800;
            } else if (address < 0x3000) {
                address = address - 0x2800;
            }
        }
        return vidRAM[address];
    } else if (address < 0x4000) {
        address &= 0b0011111;
        if (address == 0x0010 || address == 0x0014 || address == 0x0018 || address == 0x001C) {
            address -= 0x0010;
        }
        return paletteRAM[address];
    }

    return 0x00;
}

void bus_writePPU(uint16_t address, uint8_t data) {
    address = address % 0x4000;
    if (address < 0x2000) { // chr rom
        if (panicOnUnimplemented) exc_panic_invalidPPUIO(address);
    } else if (address < 0x3F00) { // vram
        address = (address % 0x2000) + 0x2000;
        if (verticalMirroring) {
            if (address < 0x2400) {
                address = address - 0x2000;
            } else if (address < 0x2800) {
                address = address - 0x2400;
            } else if (address < 0x2C00) {
                address = address - 0x2400;
            } else if (address < 0x2FFF) {
                address = address - 0x2800;
            }
        } else {
            if (address < 0x2400) {
                address = address - 0x2000;
            } else if (address < 0x2800) {
                address = address - 0x2000;
            } else if (address < 0x2C00) {
                address = address - 0x2800;
            } else if (address < 0x3000) {
                address = address - 0x2800;
            }
        }
        vidRAM[address] = data;
    } else if (address < 0x4000) {
        address &= 0b00011111;
        if (address == 0x0010 || address == 0x0014 || address == 0x0018 || address == 0x001C) {
            address -= 0x0010;
        }
        paletteRAM[address] = data;
    }
    
}

void bus_loadPRGROM(uint8_t* romData_PTR, uint16_t romSize) {
    prgROM = romData_PTR;
}

void bus_loadCHRROM(uint8_t* romData_PTR, uint16_t romSize) {
    chrROM = romData_PTR;
}

void bus_startTimeMonitor() {
    clock_gettime(CLOCK_REALTIME, &t1);
    gettimeofday(&total1, NULL);
}

uint64_t bus_endTimeMonitor() {

    gettimeofday(&total2, NULL);
    totalTime = (total2.tv_sec - total1.tv_sec) * 1000.0;      // sec to ms
    totalTime += (total2.tv_usec - total1.tv_usec) / 1000.0;   // us to ms
    totalTime *= 1000;

    double frequency = ((double) cpu_getCycles()) / ((double) totalTime);
    double framerate = ((double) ppu_getFrames()) / (((double) totalTime) / 1000000);

    printf("Finished %llu cycles and %llu frames in %llu microseconds\n", cpu_getCycles(), ppu_getFrames(), totalTime);
    printf("CPU Frequency: %.5f MHz\n", frequency);
    printf("    Framerate: %.5f FPS\n", framerate);
    return (uint64_t) elapsedTime;
}

uint64_t bus_pollTimeMonitor() {
    clock_gettime(CLOCK_REALTIME, &t2);
    gettimeofday(&total2, NULL);
    uint64_t elapsedTime;
    elapsedTime = (t2.tv_nsec - t1.tv_nsec);
    totalTime = (total2.tv_sec - total1.tv_sec) * 1000.0;      // sec to ms
    totalTime += (total2.tv_usec - total1.tv_usec) / 1000.0;   // us to ms
    totalTime *= 1000;
    clock_gettime(CLOCK_REALTIME, &t1);
    return elapsedTime;
}

void bus_initCPU() {
    cpu_init();
}

void bus_initPPU() {
    ppu_init(vidRAM, chrROM);
}

void bus_initDisplay() {
    uint16_t x = 256;
    uint16_t y = 240;
    uint16_t scale = 2;
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, x * scale, y * scale, SDL_WINDOW_SHOWN);
    surface = SDL_GetWindowSurface(window);
    keyMap.up = SDLK_w;
    keyMap.down = SDLK_s;
    keyMap.left = SDLK_a;
    keyMap.right = SDLK_d;
    keyMap.a = SDLK_SPACE;
    keyMap.b = SDLK_RSHIFT;
    keyMap.start = SDLK_RETURN;
    keyMap.select = SDLK_p;
}

void bus_cpuReport(uint8_t cycleCount) {

    ppu_runCycles(cycleCount * 3);

    if (ppu_getControlFlag(PPUCTRL_GENVBNMI) && ppu_getStatusFlag(PPUSTAT_VBLKSTART)) {
        bus_triggerNMI();
        ppu_setStatusFlag(PPUSTAT_VBLKSTART, false);
    }
    
    // It's recommended to keep this enabled since a higher framerate will
    // not only mess up timing/audio, but will also use 100% of CPU
    if (limitClockSpeed) {
        clocksPerMsec += cycleCount;
        msecCounter += bus_pollTimeMonitor();
        if (msecCounter >= 1000000) {
        
            int32_t remainingClocks = clocksPerMsec - 1789;
            if (remainingClocks > 0) {
                uint16_t catchup = ((double) remainingClocks) * 0.559f;
                uint16_t framerate = ((double) ppu_getFrames()) / (((double) totalTime) / 1000000);
                if (framerate > 60) {
                    usleep(catchup);
                }
            }

            msecCounter = 0;
            clocksPerMsec = 0;
        }
    }
}

void bus_ppuReport(uint32_t* bitmap) {
    
    // Sort of a v-sync
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == keyMap.up) {
                joypad_setButton(JP_UP);
            } else if (event.key.keysym.sym == keyMap.down) {
                joypad_setButton(JP_DOWN);
            } else if (event.key.keysym.sym == keyMap.left) {
                joypad_setButton(JP_LEFT);
            } else if (event.key.keysym.sym == keyMap.right) {
                joypad_setButton(JP_RIGHT);
            } else if (event.key.keysym.sym == keyMap.a) {
                joypad_setButton(JP_BTN_A);
            } else if (event.key.keysym.sym == keyMap.b) {
                joypad_setButton(JP_BTN_B);
            } else if (event.key.keysym.sym == keyMap.select) {
                joypad_setButton(JP_SELECT);
            } else if (event.key.keysym.sym == keyMap.start) {
                joypad_setButton(JP_START);
            }
        }

        if (event.type == SDL_KEYUP) {
            if (event.key.keysym.sym == keyMap.up) {
                joypad_unsetButton(JP_UP);
            } else if (event.key.keysym.sym == keyMap.down) {
                joypad_unsetButton(JP_DOWN);
            } else if (event.key.keysym.sym == keyMap.left) {
                joypad_unsetButton(JP_LEFT);
            } else if (event.key.keysym.sym == keyMap.right) {
                joypad_unsetButton(JP_RIGHT);
            } else if (event.key.keysym.sym == keyMap.a) {
                joypad_unsetButton(JP_BTN_A);
            } else if (event.key.keysym.sym == keyMap.b) {
                joypad_unsetButton(JP_BTN_B);
            } else if (event.key.keysym.sym == keyMap.select) {
                joypad_unsetButton(JP_SELECT);
            } else if (event.key.keysym.sym == keyMap.start) {
                joypad_unsetButton(JP_START);
            }
        }

        if (event.type == SDL_QUIT) {
            cpu_panic();
        }
    }

    SDL_FreeSurface(surface);
    uint32_t* pixels = (uint32_t*)surface->pixels;
    SDL_LockSurface(surface);
    
    uint8_t scale = 2;
    for (int i = 0; i < 256 * 240; i++) {
        
        int x = i % 256;
        int y = i / 256;
        pixels[(x * scale) + (y * scale * scale * 256)] = bitmap[i];
        pixels[(x * scale) + (y * scale * scale * 256) + 1] = bitmap[i];
        pixels[(x * scale) + (y * scale * scale * 256) + (scale * 256)] = bitmap[i];
        pixels[(x * scale) + (y * scale * scale * 256) + (scale * 256) + 1] = bitmap[i];
        
    }
    SDL_UnlockSurface(surface);
    SDL_UpdateWindowSurface(window);
}

void bus_triggerNMI() {
    cpu_vblankNMI();
}

void bus_triggerCPUPanic() {
    cpu_panic();
}