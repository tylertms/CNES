#pragma once
#include "apu.h"
#include "cart.h"
#include "cpu.h"
#include "input.h"
#include "ppu.h"
#include <stdint.h>
#include <stddef.h>

typedef struct _dma {
    uint8_t is_tranfer;
    uint8_t dummy_cycle;
    uint8_t page;
    uint8_t addr;
    uint8_t data;
} _dma;

typedef struct _nes {
    _cpu cpu;
    _ppu ppu;
    _apu apu;
    _cart cart;
    _input input;
    _dma dma;

    size_t master_clock;
} _nes;

void nes_init(_nes* nes, _gui* gui);
void nes_reset(_nes* nes);
void nes_clock(_nes* nes);
