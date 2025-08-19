/*
 * main.c - Main Entry Point
 *
 * Entry point and main initialization routines for the ZX Spectrum emulator.
 * Sets up memory, random number generator, Z80 CPU, ULA, and beeper.
 */

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

#include "z80_public.h"
#include "zx_bpr.h"
#include "zx_key.h"
#include "zx_mem.h"
#include "zx_rnd.h"
#include "zx_ula.h"

void zx_init(char* file) {
    mem_init(MEM_SIZE);
    rnd_init();
    Z80InitData z80InitData = {file, memory, &mem_read, &mem_write, &ula_readPort, &ula_writePort, &rnd_update};
    z80_init(&z80InitData);
    ula_init();
    bpr_init();
}

void zx_update() {
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            key_update(event);
        }

        uint8_t totalCycles = z80_update();
        totalCycles += mem_getContendedCycles();
        ula_update(!z80_isHalted() ? totalCycles : 4);
        bpr_update(!z80_isHalted() ? totalCycles : 4);
    }
}

void zx_dispose() {
    bpr_dispose();
    ula_dispose();
    rnd_dispose();
    mem_dispose();
}

int main(int argc, char* argv[]) {
    zx_init(argv[1]);
    zx_update();
    zx_dispose();

    return 0;
}