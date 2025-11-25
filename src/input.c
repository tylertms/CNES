#include "input.h"

void input_cpu_write(_input* input, uint16_t addr, uint8_t data) {
    uint8_t new_strobe = data & 0x01;
    if (input->strobe && !new_strobe) {
        input->shift[0] = input->controller[0];
        input->shift[1] = input->controller[1];
    }
    input->strobe = new_strobe;
}

uint8_t input_cpu_read(_input* input, uint16_t addr) {
    uint8_t data = 0x00;

    uint8_t pad = addr & 0x0001;
    if (input->strobe) {
        data = (input->controller[pad] & 0x80) ? 1 : 0;
    } else {
        data = (input->shift[pad] & 0x80) ? 1 : 0;
        input->shift[pad] <<= 1;
    }

    return data;
}
