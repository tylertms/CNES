#pragma once
#include <stdint.h>

typedef struct _cart {
    uint8_t* prg;
    uint8_t* chr;

    uint16_t prg_rom_banks;
    uint16_t chr_rom_banks;
    uint16_t mapper_id;
    uint8_t ntbl_layout;
    uint8_t has_prg_ram;
    uint8_t trainer;
    uint8_t alt_ntbl_layout;
    uint8_t prg_ram_banks;
    uint8_t tv_system;

    uint8_t prg_ram_shift;
    uint8_t prg_nvram_shift;
    uint8_t chr_ram_shift;
    uint8_t chr_nvram_shift;
    uint8_t cpu_ppu_timing;
    uint8_t misc_roms;
    uint8_t expansion_device;
} _cart;

uint8_t cart_load(_cart* cart, char* file);
uint8_t parse_ines(_cart* cart, uint8_t header[16]);
uint8_t parse_nes2(_cart* cart, uint8_t header[16]);
uint8_t cart_read(_cart* cart, uint16_t addr);
void cart_write(_cart* cart, uint16_t addr, uint8_t data);
