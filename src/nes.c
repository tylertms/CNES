#include "nes.h"
#include <string.h>

void nes_init(_nes *nes) {
    memset(nes, 0, sizeof(_nes));

    // attach references for cpu-side access
    nes->cpu.p_ppu = &nes->ppu;
    nes->cpu.p_cart = &nes->cart;
}

void nes_reset(_nes *nes) {

}

void nes_clock(_nes *nes) {

}
