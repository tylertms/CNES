#include "cart.h"
#include "nes.h"
#include <stdio.h>

int main(int argc, char** argv) {
    if (argc <= 1) {
        printf("Usage: %s <file.nes>\n", argv[0]);
        return 1;
    }

    _nes nes;
    nes_init(&nes);

    uint8_t res = cart_load(&nes.cart, argv[1]);
    if (res) return res;

    nes_reset(&nes);

    while (!nes.cpu.halt) {
        nes_clock(&nes);
    }

    return 0;
}
