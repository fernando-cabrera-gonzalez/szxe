#include <SDL.h>

#include "z80_public.h"
#include "zx_ula.h"

void key_update(SDL_Event event)
{
    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        if (event.type == SDL_KEYDOWN && SDLK_ESCAPE == event.key.keysym.sym) {
            z80_debug();
        }

        for (int i = 0; i < 40; i++) {                    
            if (keyMappings[i].keyCode == event.key.keysym.sym) {
                ula_writePort_key(keyMappings[i].port, event.type == SDL_KEYDOWN 
                    ? ula_readPort(keyMappings[i].port) & ~keyMappings[i].mask
                    : ula_readPort(keyMappings[i].port) | keyMappings[i].mask);
                break;
            }
        }
    }
}