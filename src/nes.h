#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct _cpu {
    uint8_t a;              // accumulator
    uint8_t x;              // x register
    uint8_t y;              // y register
    uint8_t p;              // status flags
    uint8_t s;              // stack pointer
    uint8_t pc;             // program counter
    uint8_t ram[0x800];     // cpu ram
    uint8_t cycles;         // instr cycle counter
    uint8_t irq_pending;    // interrupt request
    uint8_t nmi_pending;    // non-maskable interrupt
} _cpu;

typedef struct _ppu {

} _ppu;

typedef struct _apu {

} _apu;

typedef struct _cart {

} _cart;

typedef struct _input {

} _input;

typedef struct _nes {
    _cpu cpu;
    _ppu ppu;
    _apu apu;
    _cart cart;
} _nes;
