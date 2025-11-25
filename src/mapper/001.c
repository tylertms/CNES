#include "../mapper.h"

typedef struct _mmc1 {
    uint8_t shift;
    uint8_t write_count;
    uint8_t control;
    uint16_t chr_bank0;
    uint16_t chr_bank1;
    uint16_t prg_bank;
} _mmc1;

void map_init_001(_cart* cart) {
    _mmc1* mmc1 = malloc(sizeof(_mmc1));
    cart->mapper.data = mmc1;

    *mmc1 = (_mmc1){
        .shift = 0x10,
        .write_count = 0,
        .control = 0x0C,
        .chr_bank0 = 0,
        .chr_bank1 = 0,
        .prg_bank = 0,
    };
}

void map_deinit_001(_cart* cart) {
    free(cart->mapper.data);
}

uint8_t map_cpu_read_001(_cart* cart, uint16_t addr) {
    uint8_t data = 0x00;

    if (cart->prg_ram.size && 0x6000 <= addr && addr <= 0x7FFF) {
        uint16_t offset = (addr - 0x6000) & (cart->prg_ram.size - 1);
        data = cart->prg_ram.data[offset];
    } else if (0x8000 <= addr && addr <= 0xFFFF) {
        _mmc1* mmc1 = cart->mapper.data;
        uint8_t prg_mode = (mmc1->control >> 2) & 0x03;
        uint8_t prg_bank = mmc1->prg_bank & 0x0F;
        uint32_t bank_index = 0;

        if (prg_mode == 0 || prg_mode == 1) {
            uint16_t offset = addr - 0x8000;
            uint32_t base = (prg_bank & ~1) * 0x4000;
            uint32_t addr = (base + offset) & (cart->prg_rom.size - 1);
            data = cart->prg_rom.data[addr];
        } else {
            if (addr <= 0xBFFF) {
                if (prg_mode == 2) {
                    bank_index = 0;
                } else {
                    bank_index = prg_bank % cart->prg_rom_banks;
                }
            } else {
                if (prg_mode == 2) {
                    bank_index = prg_bank % cart->prg_rom_banks;
                } else {
                    bank_index = cart->prg_rom_banks - 1;
                }
            }

            uint16_t offset = addr & 0x3FFF;
            uint32_t base = bank_index * 0x4000;
            uint32_t addr = (base + offset) & (cart->prg_rom.size - 1);
            data = cart->prg_rom.data[addr];
        }
    }

    return data;
}

void map_cpu_write_001(_cart* cart, uint16_t addr, uint8_t data) {
    if (cart->prg_ram.size && 0x6000 <= addr && addr <= 0x7FFF) {
        uint16_t offset = (addr - 0x6000) & (cart->prg_ram.size - 1);
        cart->prg_ram.data[offset] = data;
    } else if (0x8000 <= addr && addr <= 0xFFFF) {

    }
}

uint8_t map_ppu_read_001(_cart* cart, uint16_t addr) {
    uint8_t data = 0x00;

    if (cart->chr_ram.size) {
        data = cart->chr_ram.data[addr & (cart->chr_ram.size - 1)];
    } else {
        data = cart->chr_rom.data[addr & (cart->chr_rom.size - 1)];
    }

    return data;
}

void map_ppu_write_001(_cart* cart, uint16_t addr, uint8_t data) {
    if (cart->chr_ram.size) {
        cart->chr_ram.data[addr & (cart->chr_ram.size - 1)] = data;
    }
}
