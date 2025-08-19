/*
 * zx_rnd.h - Renderer Interface
 *
 * Contains declarations for rendering and display output in the ZX Spectrum emulator.
 * Defines screen dimensions, palette, and function prototypes for SDL-based rendering.
 */

#ifndef RND_H
#define RND_H

#include <SDL.h>

#define SCALE                   4
#define H_BORDER_WIDTH          48
#define V_RETRACE_HEIGHT        16
#define TOP_BORDER_HEIGHT       48
#define BOTTOM_BORDER_HEIGHT    56
#define SCREEN_WIDTH            256
#define SCREEN_HEIGHT           192
#define LOGICAL_WIDTH           (H_BORDER_WIDTH + SCREEN_WIDTH + H_BORDER_WIDTH)
#define LOGICAL_HEIGHT          (TOP_BORDER_HEIGHT + SCREEN_HEIGHT + BOTTOM_BORDER_HEIGHT)

int rnd_init();
void rnd_update();
void rnd_lockTexture();
void rnd_renderByte(const SDL_Colour* palette, uint8_t pixelData, uint8_t colourAttribute, uint8_t flash, uint16_t x, uint16_t y);
void rnd_unlockTexture();
void rnd_dispose();

#endif // RND_H