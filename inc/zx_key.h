/*
 * zx_key.h - Keyboard Interface
 
 * Contains declarations for keyboard input handling in the ZX Spectrum emulator.
 * Declares the key_update function for processing SDL keyboard events.
 */

#ifndef KEY_H
#define KEY_H

void key_update(SDL_Event event);

#endif // KEY_H