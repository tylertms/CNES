#include "../mapper.h"

typedef struct _uxrom {
    uint8_t prg_bank_low;
    uint8_t prg_bank_high;
} _uxrom;

void map_init_002(_cart* cart) {
    _uxrom* uxrom = malloc(sizeof(_uxrom));
    cart->mapper.data = uxrom;

    *uxrom = (_uxrom){
        .prg_bank_low = 0,
        .prg_bank_high = cart->prg_rom_banks - 1,
    };
}

void map_deinit_002(_cart* cart) {
    free(cart->mapper.data);
}

uint8_t map_cpu_read_002(_cart* cart, uint16_t addr) {
    uint8_t data = 0x00;
    _uxrom* uxrom = cart->mapper.data;

    if (0x8000 <= addr && addr <= 0xBFFF) {
        uint32_t offset = (uxrom->prg_bank_low * 0x4000) + (addr & 0x3FFF);
        data = cart->prg_rom.data[offset];
    } else if (0xC000 <= addr && addr <= 0xFFFF) {
        uint32_t offset = (uxrom->prg_bank_high * 0x4000) + (addr & 0x3FFF);
        data = cart->prg_rom.data[offset];
    }

    return data;
}

void map_cpu_write_002(_cart* cart, uint16_t addr, uint8_t data) {
    if (addr >= 0x8000) {
        _uxrom* uxrom = cart->mapper.data;
        uint8_t mask = cart->prg_rom_banks - 1;
        uxrom->prg_bank_low = data & mask;
    }
}


uint8_t map_ppu_read_002(_cart* cart, uint16_t addr) {
    uint8_t data = 0x00;

    if (0x0000 <= addr && addr <= 0x1FFF) {
        if (cart->chr_rom.size) {
            data = cart->chr_rom.data[addr & (cart->chr_rom.size - 1)];
        } else {
            data = cart->chr_ram.data[addr & (cart->chr_ram.size - 1)];
        }
    }

    return data;
}

void map_ppu_write_002(_cart* cart, uint16_t addr, uint8_t data) {
    if (0x0000 <= addr && addr <= 0x1FFF) {
        if (cart->chr_ram.size) {
            cart->chr_ram.data[addr & (cart->chr_ram.size - 1)] = data;
        }
    }
}
