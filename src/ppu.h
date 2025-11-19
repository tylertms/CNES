#pragma once
#include <stdint.h>

typedef struct _ppu {
    uint8_t nametable[0x800];
    uint8_t palette_idx[0x20];

    uint8_t ppuctrl;
    uint8_t ppumask;
    uint8_t ppustatus;
    uint8_t oamaddr;
    uint8_t oamdata;
    uint16_t ppuscroll;
    uint16_t ppuaddr;
    uint8_t ppudata;
    uint8_t oamdma;
} _ppu;

typedef enum _ppuctrl_flag {
    NTBL_SEL_LOW    = (1 << 0),
    NTBL_SEL_HIGH   = (1 << 1),
    INC_MODE        = (1 << 2),
    SPRITE_SEL      = (1 << 3),
    BGRND_SEL       = (1 << 4),
    SPRITE_HEIGHT   = (1 << 5),
    PPU_MS          = (1 << 6),
    NMI_EN          = (1 << 7),
} _ppuctrl_flag;

typedef enum _ppumask_flag {
    GREYSCALE       = (1 << 0),
    BGRND_LC_EN     = (1 << 1),
    SPRITE_LC_EN    = (1 << 2),
    BGRND_EN        = (1 << 3),
    SPRITE_EN       = (1 << 4),
    CLR_EM_R        = (1 << 5),
    CLR_EM_G        = (1 << 6),
    CLR_EM_B        = (1 << 7),
} _ppumask_flag;

typedef enum _ppustatus_flag {
    SPRITE_OVERFLOW = (1 << 5),
    SPRITE_0_HIT    = (1 << 6),
    VBLANK          = (1 << 7),
} _ppustatus_flag;

void ppu_clock(_ppu* ppu);

uint8_t ppu_cpu_read(_ppu* ppu, uint16_t addr);
void ppu_cpu_write(_ppu* ppu, uint16_t addr, uint8_t data);

uint8_t ppu_read(_ppu* ppu, uint16_t addr);
void ppu_write(_ppu* ppu, uint16_t addr, uint8_t data);
