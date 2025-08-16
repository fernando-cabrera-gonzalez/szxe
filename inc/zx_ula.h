#ifndef ULA_H
#define ULA_H

#include "SDL.h"

#define SCREEN_T_STATES         128
#define H_BORDER_T_STATES       24
#define H_RETRACE_T_STATES      48 
#define LINE_T_STATES           (H_BORDER_T_STATES + SCREEN_T_STATES + H_BORDER_T_STATES + H_RETRACE_T_STATES)
#define V_RETRACE_T_STATES      16 * LINE_T_STATES
#define TOP_BORDER_T_STATES     48 * LINE_T_STATES
#define BOTTOM_BORDER_T_STATES  56 * LINE_T_STATES

#define FRAME_TIME_MS           20
#define SET_MASK(bit) 0x01<<bit

// #define ULA_DEBUG

typedef struct {
    SDL_KeyCode keyCode;
    uint16_t port;
    uint8_t mask;
} KeyMapping;

typedef void (*statePtr)(uint8_t);

extern KeyMapping keyMappings[40];
extern statePtr ulaStatePtr;
extern uint32_t tStates;

void ula_init();
void ula_update(uint8_t numCycles);
uint8_t ula_readPort(uint16_t address);
void ula_writePort(uint16_t address, uint8_t value);
void ula_writePort_key(uint16_t address, uint8_t value);
void ula_dispose();

#endif // ULA_H