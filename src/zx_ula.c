#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "z80_public.h"
#include "zx_bpr.h"
#include "zx_mem.h"
#include "zx_rnd.h"
#include "zx_ula.h"
#include "zx_utl.h"

void verticalRetrace(uint8_t numCycles);
void topVBorder(uint8_t numCycles);
void leftBorder(uint8_t numCycles);
void drawScreen(uint8_t numCycles);
void rightBorder(uint8_t numCycles);
void horizontalRetrace(uint8_t numCycles);
void bottomVBorder(uint8_t numCycles);

const SDL_Colour palette[16] = {
    {0x00, 0x00, 0x00, 0xFF},         // BLACK
    {0x00, 0x00, 0xD7, 0xFF},         // BLUE
    {0xD7, 0x00, 0x00, 0xFF},         // RED
    {0xD7, 0x00, 0xD7, 0xFF},         // MAGENTA
    {0x00, 0xD7, 0x00, 0xFF},         // GREEN
    {0x00, 0xD7, 0xD7, 0xFF},         // CYAN
    {0xD7, 0xD7, 0x00, 0xFF},         // YELLOW
    {0xD7, 0xD7, 0xD7, 0xFF},         // WHITE

    {0x00, 0x00, 0x00, 0xFF},         // BLACK
    {0x00, 0x00, 0xFF, 0xFF},         // BRIGHT BLUE
    {0xFF, 0x00, 0x00, 0xFF},         // BRIGHT RED
    {0xFF, 0x00, 0xFF, 0xFF},         // BRIGHT MAGENTA
    {0x00, 0xFF, 0x00, 0xFF},         // BRIGHT GREEN
    {0x00, 0xFF, 0xFF, 0xFF},         // BRIGHT CYAN
    {0xFF, 0xFF, 0x00, 0xFF},         // BRIGHT YELLOW
    {0xFF, 0xFF, 0xFF, 0xFF}          // BRIGHT WHITE
};

uint8_t ports[65536] = {[0 ... 65535] = 0xFF};

KeyMapping keyMappings[40] = {
    {SDLK_LSHIFT,   0xFEFE, SET_MASK(0)},
    {SDLK_z,        0xFEFE, SET_MASK(1)},
    {SDLK_x,        0xFEFE, SET_MASK(2)},
    {SDLK_c,        0xFEFE, SET_MASK(3)},
    {SDLK_v,        0xFEFE, SET_MASK(4)},

    {SDLK_a,        0xFDFE, SET_MASK(0)},
    {SDLK_s,        0xFDFE, SET_MASK(1)},
    {SDLK_d,        0xFDFE, SET_MASK(2)},
    {SDLK_f,        0xFDFE, SET_MASK(3)},
    {SDLK_g,        0xFDFE, SET_MASK(4)},

    {SDLK_q,        0xFBFE, SET_MASK(0)},
    {SDLK_w,        0xFBFE, SET_MASK(1)},
    {SDLK_e,        0xFBFE, SET_MASK(2)},
    {SDLK_r,        0xFBFE, SET_MASK(3)},
    {SDLK_t,        0xFBFE, SET_MASK(4)},

    {SDLK_1,        0xF7FE, SET_MASK(0)},
    {SDLK_2,        0xF7FE, SET_MASK(1)},
    {SDLK_3,        0xF7FE, SET_MASK(2)},
    {SDLK_4,        0xF7FE, SET_MASK(3)},
    {SDLK_5,        0xF7FE, SET_MASK(4)},

    {SDLK_0,        0xEFFE, SET_MASK(0)},
    {SDLK_9,        0xEFFE, SET_MASK(1)},
    {SDLK_8,        0xEFFE, SET_MASK(2)},
    {SDLK_7,        0xEFFE, SET_MASK(3)},
    {SDLK_6,        0xEFFE, SET_MASK(4)},

    {SDLK_p,        0xDFFE, SET_MASK(0)},
    {SDLK_o,        0xDFFE, SET_MASK(1)},
    {SDLK_i,        0xDFFE, SET_MASK(2)},
    {SDLK_u,        0xDFFE, SET_MASK(3)},
    {SDLK_y,        0xDFFE, SET_MASK(4)},

    {SDLK_RETURN,   0xBFFE, SET_MASK(0)},
    {SDLK_l,        0xBFFE, SET_MASK(1)},
    {SDLK_k,        0xBFFE, SET_MASK(2)},
    {SDLK_j,        0xBFFE, SET_MASK(3)},
    {SDLK_h,        0xBFFE, SET_MASK(4)},

    {SDLK_SPACE,    0x7FFE, SET_MASK(0)},
    {SDLK_RSHIFT,   0x7FFE, SET_MASK(1)},
    {SDLK_m,        0x7FFE, SET_MASK(2)},
    {SDLK_n,        0x7FFE, SET_MASK(3)},
    {SDLK_b,        0x7FFE, SET_MASK(4)}
};

statePtr ulaStatePtr;
uint16_t ulaCycles = 0, x = 0, y = 0;
uint32_t tStates = 0, lastTStates = 0;
uint32_t frame_start_time = 0, frame_Acc_time = 0;
uint8_t *flash_counters = NULL;

uint16_t calculatePixelDataAddress(uint8_t x, uint8_t y) {
    return 0x4000
        | (x & 0x1F)
        | (y & 0xC0) << 5
        | (y & 0x07) << 8
        | (y & 0x38) << 2;
}

uint16_t calculateColourAttributeAddress(uint8_t x, uint8_t y) {
    return 0x5800 + (y / 8) * 32 + x;
}

void verticalRetrace(uint8_t numCycles) {
    ulaCycles += numCycles;
    tStates += numCycles;

    if (ulaCycles >= V_RETRACE_T_STATES) {
        ulaCycles -= V_RETRACE_T_STATES;
        ulaStatePtr = &topVBorder;
        rnd_lockTexture();

        #ifdef ULA_DEBUG
        printf("V = %d | ", tStates - lastTStates);
        lastTStates = tStates;
        #endif
    }
}

void topVBorder(uint8_t numCycles) {
    ulaCycles += numCycles;
    tStates += numCycles;

    if (ulaCycles >= TOP_BORDER_T_STATES) {
        y = TOP_BORDER_HEIGHT;
        x = 0;
        ulaCycles -= TOP_BORDER_T_STATES;
        ulaStatePtr = &leftBorder;

        #ifdef ULA_DEBUG
        printf("T = %d | ", tStates - lastTStates);
        lastTStates = tStates;
        #endif
    }
}

void leftBorder(uint8_t numCycles) {
    ulaCycles += numCycles;
    tStates += numCycles;

    if (ulaCycles >= H_BORDER_T_STATES) {   
        x = H_BORDER_WIDTH;
        ulaStatePtr = &drawScreen;
        ulaCycles -= H_BORDER_T_STATES;

        #ifdef ULA_DEBUG
        printf("L = %d | ", tStates - lastTStates);
        lastTStates = tStates;
        #endif
    }
}

void drawScreen(uint8_t numCycles) {
    ulaCycles += numCycles;
    tStates += numCycles;
    
    uint8_t bytesToRender = MIN(ulaCycles / 4, ((H_BORDER_WIDTH + SCREEN_WIDTH) - x) / 8);
    ulaCycles -= bytesToRender * 4;
    
    if (bytesToRender > 0) {
        uint8_t screenY = y - TOP_BORDER_HEIGHT;
        for (int i = 0; i < bytesToRender; i++) {
            uint8_t screenX = x - H_BORDER_WIDTH;
            uint16_t pixelDataPtr = calculatePixelDataAddress(screenX/8,screenY);
            uint16_t colourAttributePtr = calculateColourAttributeAddress(screenX/8,screenY);
            rnd_renderByte(palette, memory[pixelDataPtr], memory[colourAttributePtr], (flash_counters[colourAttributePtr - 0x5800] / 16) % 2 == 0, x, y);
            x += 8;
        }
        if (x == H_BORDER_WIDTH + SCREEN_WIDTH) {
            ulaStatePtr = &rightBorder;

            #ifdef ULA_DEBUG
            printf("S = %d | ", tStates - lastTStates);
            lastTStates = tStates;
            #endif
        }
    }
}

void rightBorder(uint8_t numCycles) {
    ulaCycles += numCycles;
    tStates += numCycles;

    if (ulaCycles >= H_BORDER_T_STATES) {
        ulaCycles -= H_BORDER_T_STATES;
        ulaStatePtr = &horizontalRetrace;

        #ifdef ULA_DEBUG
        printf("R = %d | ", tStates - lastTStates);
        lastTStates = tStates;
        #endif
    }
}

void horizontalRetrace(uint8_t numCycles) {
    ulaCycles += numCycles;
    tStates += numCycles;

    if (ulaCycles >= H_RETRACE_T_STATES) {   
        if (y < TOP_BORDER_HEIGHT + SCREEN_HEIGHT - 1) {
            x = 0;
            ulaStatePtr = &leftBorder;
            #ifdef ULA_DEBUG
            printf("H = %d | ", tStates - lastTStates);
            lastTStates = tStates;
            #endif
        } else {
            ulaStatePtr = &bottomVBorder;
            rnd_unlockTexture();

            #ifdef ULA_DEBUG
            printf("H = %d | ", tStates - lastTStates);
            lastTStates = tStates;
            #endif
        }
        ulaCycles -= H_RETRACE_T_STATES;
        y++;
    }
}

void fix_frame_time() {
    uint32_t frame_time = SDL_GetTicks() - frame_start_time;
    if (frame_time < FRAME_TIME_MS) {
        frame_time = frame_time + frame_Acc_time;
        
        if (frame_time > FRAME_TIME_MS) {
            frame_Acc_time -= FRAME_TIME_MS - (frame_time - frame_Acc_time);
            frame_time = FRAME_TIME_MS;
        } else {
            frame_Acc_time = 0;
        }
        
        SDL_Delay(FRAME_TIME_MS - frame_time);
    } else if (frame_time > FRAME_TIME_MS) {
        frame_Acc_time += frame_time - FRAME_TIME_MS;
        if (frame_Acc_time >= FRAME_TIME_MS) {
            frame_Acc_time -= FRAME_TIME_MS;
        }
    }
    frame_start_time = SDL_GetTicks();
}

void update_flash_counters() {
    for (uint16_t i = 0; i < 768; i++) {
        uint8_t color_attribute = memory[0x5800 + i];
        uint8_t f_bit = (color_attribute & 0x80) >> 7;
        if (f_bit) {
            flash_counters[i]++;
        } else {
            flash_counters[i] = 0;
        }
    }
}

void bottomVBorder(uint8_t numCycles) {
    ulaCycles += numCycles;
    tStates += numCycles;

    if (ulaCycles >= BOTTOM_BORDER_T_STATES) {
        z80_setInt();
        ulaCycles -= BOTTOM_BORDER_T_STATES;
        ulaStatePtr = &verticalRetrace;

        #ifdef ULA_DEBUG
        printf("B = %d | TOTAL = %d\n", tStates - lastTStates, tStates);
        lastTStates = 0;
        #endif
        tStates = 0;

        update_flash_counters();
        fix_frame_time();
    }
}

void ula_init() {
    flash_counters = (uint8_t*) calloc(768, sizeof(uint8_t));
    ulaStatePtr = &verticalRetrace;

    z80_setInt();
    frame_start_time = SDL_GetTicks();
}

void ula_update(uint8_t numCycles) {
    ulaStatePtr(numCycles);
}

uint8_t ula_readPort(uint16_t address) {
    // Kempston Joystick - no joystick (Manic Miner fix)
    if ((address & 0xFF) == 0x1F) {
        return 0x00;
    }
    
    uint8_t result = 0x1f;
    uint8_t port = address >> 8;

    for (uint16_t i = 1; i <= 128; i = i * 2) {
        if ((port & i) == 0) {
            result &= ports[((255 - i) << 8 ) | 0x00FE];
        }
    }

    return result;
}

void ula_writePort(uint16_t address, uint8_t value) {
    ports[address] = value;
    
    if ((address & 0xFF) == 0xFE) {
        bpr_new_value = (value & 0x10) >> 4;
        if (bpr_last_value == -1 || bpr_last_value != bpr_new_value) {
            bpr_generate_pulse();
            bpr_last_value = bpr_new_value;
        }
    }
}

void ula_writePort_key(uint16_t address, uint8_t value) {
    ports[address] = value;
}

void ula_dispose() {
    free(flash_counters);
}