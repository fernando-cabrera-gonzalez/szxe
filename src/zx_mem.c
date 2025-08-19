/*
 * zx_mem.c - Memory Management
 * 
 * This file implements memory allocation, read, write, and cleanup functions
 * for the ZX emulator. It provides a simple memory model for the emulated system.
 * This module also handles memory contention between the Z80 CPU and the ULA.
 * 
 */

#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "zx_ula.h"

uint8_t *memory;
uint8_t *contendedCyclesByTstate;
uint8_t contendedCycles;

void mem_init(size_t size) {
    memory = (uint8_t*) calloc(size, sizeof(uint8_t));

    contendedCyclesByTstate = (uint8_t *) calloc(65535, sizeof(uint8_t));
    for(int line = 0; line < 192; line++) {
        for(int i = 0; i < 16; i++)  {
            int start = 14335 + (line * 224) + i * 8;
            contendedCyclesByTstate[start] = 6;
            contendedCyclesByTstate[start + 1] = 5;
            contendedCyclesByTstate[start + 2] = 4;
            contendedCyclesByTstate[start + 3] = 3;
            contendedCyclesByTstate[start + 4] = 2;
            contendedCyclesByTstate[start + 5] = 1;
        }
    }
}

void checkContention(uint16_t address) {
    if (address >= 0x4000 && address < 0x8000 && tStates >= 14335 && tStates <= 57244) {
        contendedCycles += contendedCyclesByTstate[tStates];
    }
}

uint8_t mem_read(uint16_t address) {
    checkContention(address);
    return memory[address];   
}

void mem_write(uint16_t address, uint8_t value) {
    checkContention(address);
    memory[address] = value;
}

uint8_t mem_getContendedCycles() {
    uint8_t result = contendedCycles;
    contendedCycles = 0;
    return result;
}

void mem_resetContendedCycles() {
    contendedCycles = 0;
}

void mem_dispose() {
    free(memory);
    free(contendedCyclesByTstate);
}