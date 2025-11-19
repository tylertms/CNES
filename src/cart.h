#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct _mem {
    uint8_t* data;
    size_t size;
    uint8_t writeable;
} _mem;

typedef struct _cart {
    _mem prg_rom;
    _mem prg_ram;
    _mem prg_nvram;

    _mem chr_rom;
    _mem chr_ram;
    _mem chr_nvram;

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
