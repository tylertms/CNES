#include "nes.h"
#include "cpu.h"
#include <string.h>

void nes_init(_nes* nes, _gui* gui) {
    memset(nes, 0, sizeof(_nes));

    nes->cpu.p_ppu = &nes->ppu;
    nes->cpu.p_cart = &nes->cart;
    nes->cpu.p_input = &nes->input;
    nes->cpu.p_dma = &nes->dma;
    nes->ppu.p_cart = &nes->cart;
    nes->ppu.p_gui = gui;
    nes->ppu.p_cpu = &nes->cpu;
}

void nes_reset(_nes* nes) {
    cpu_reset(&nes->cpu);
}
#include <stdio.h>
void nes_clock(_nes* nes) {
    uint8_t frame_complete = 0;
    while (!frame_complete) {
        frame_complete =
            ppu_clock(&nes->ppu) |
            ppu_clock(&nes->ppu) |
            ppu_clock(&nes->ppu) ;

        if (!nes->dma.is_tranfer) {
            cpu_clock(&nes->cpu);
        } else {
            if (nes->dma.dummy_cycle) {
                if (nes->master_clock & 1)
                    nes->dma.dummy_cycle = 0;
            } else {
                if (nes->master_clock & 1) {
                    ((uint8_t*)nes->ppu.oam)[nes->dma.addr++] = nes->dma.data;

                    if (!nes->dma.addr) {
                        nes->dma.is_tranfer = 0;
                        nes->dma.dummy_cycle = 1;
                    }
                } else {
                    nes->dma.data = cpu_read(&nes->cpu, (nes->dma.page << 8) | nes->dma.addr);
                }
            }
        }

        nes->master_clock++;
    }
}
