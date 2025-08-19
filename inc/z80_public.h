/*
 * z80_public.h - Public Interface
 *
 * Declares public types, constants, and function pointers for memory, I/O, and rendering integration.
 */

#ifndef Z80_PUBLIC_H
#define Z80_PUBLIC_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define DEBUG_LEVEL_NONE    0
#define DEBUG_LEVEL_USER    1
#define DEBUG_LEVEL_FULL    2

#define DEBUG_LEVEL DEBUG_LEVEL_USER

    /* ================================================================ */
    /*                          MEM INTERFACE                           */
    /* ================================================================ */

typedef uint8_t (*Z80MemReadFunc)(uint16_t address);
typedef void (*Z80MemWriteFunc)(uint16_t address, uint8_t value);

extern Z80MemReadFunc memReadFunc;
extern Z80MemWriteFunc memWriteFunc;

    /* ================================================================ */
    /*                          I/O INTERFACE                           */
    /* ================================================================ */

typedef uint8_t (*Z80IOReadFunc)(uint16_t address);
typedef void (*Z80IOWriteFunc)(uint16_t address, uint8_t value);

extern Z80IOReadFunc ioReadFunc;
extern Z80IOWriteFunc ioWriteFunc;

    /* ================================================================ */
    /*                          RENDERER INTERFACE                      */
    /* ================================================================ */

typedef void (*Z80RenderFunc)();
extern Z80RenderFunc renderFunc;

    /* ================================================================ */
    /*                          Z80 INTERFACE                           */
    /* ================================================================ */

typedef struct {
    char* ramFileName;
    uint8_t* memoryPtr;
    Z80MemReadFunc memReadFunc;
    Z80MemWriteFunc memWriteFunc;
    Z80IOReadFunc ioReadFunc;
    Z80IOWriteFunc ioWriteFunc;
    Z80RenderFunc renderFunc;
    uint16_t startAddress;
} Z80InitData;

void z80_init(Z80InitData* initData);
uint8_t z80_update();
#if DEBUG_LEVEL == DEBUG_LEVEL_USER
void z80_debug();
#endif
void z80_setInt();
bool z80_isHalted();

#endif