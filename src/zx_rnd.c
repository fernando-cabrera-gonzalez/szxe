/*
 * zx_rnd.c - Renderer Management
*
 * Handles rendering and display output for the ZX Spectrum emulator.
 * Manages SDL window, renderer, texture, and pixel drawing routines.
 */

#include <stdio.h>

#include "zx_rnd.h"

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture*texture;
SDL_PixelFormat *rgba32;
void *pixels;
int pitch;

void renderPixel(Uint32 *pixels32, int pitch, uint16_t x, uint16_t y, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    pixels32[(y * (pitch / 4)) + x] = SDL_MapRGBA(rgba32, r, g, b, 255);
}

SDL_Colour getPixelColour(uint8_t isSet, uint8_t colourAttribute, const SDL_Colour *palette) {
    uint8_t b = (colourAttribute & 0x40) >> 6;
    return isSet 
        ? palette[(colourAttribute & 0x07) + (b * 8)]
        : palette[((colourAttribute & 0x38) >> 3) + (b * 8)];
}

int rnd_init() {
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        printf("Error inicializando SDL: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Simple ZX Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LOGICAL_WIDTH * SCALE, LOGICAL_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Error creando la ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Error creando el renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_RenderSetLogicalSize(renderer, LOGICAL_WIDTH, LOGICAL_HEIGHT);

    texture  = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, LOGICAL_WIDTH, LOGICAL_HEIGHT);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    rgba32 = SDL_AllocFormat(SDL_PIXELFORMAT_BGRA32);

    return 0;
}

void rnd_lockTexture() {
    SDL_LockTexture(texture, NULL, &pixels, &pitch);
}

void rnd_renderByte(const SDL_Colour *palette, uint8_t pixelData, uint8_t colourAttribute, uint8_t flash, uint16_t x, uint16_t y) {
    SDL_Colour rgb; 

    Uint32 *pixels32 = (Uint32 *)pixels;
    for (uint8_t pixelIndex = 0;  pixelIndex < 8; pixelIndex++) {
        if (flash) {
            rgb = getPixelColour(pixelData & (0x01 << (7 - pixelIndex)), colourAttribute, palette);
        } else {
            rgb = getPixelColour(!(pixelData & (0x01 << (7 - pixelIndex))), colourAttribute, palette);
        }
        renderPixel(pixels32, pitch, x + pixelIndex, y, rgb.r, rgb.g, rgb.b, 255);
    }    
}

void rnd_unlockTexture() {
    SDL_UnlockTexture(texture);
}

void rnd_update() {
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void rnd_dispose() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}