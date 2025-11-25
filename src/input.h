#pragma once
#include <stdint.h>

typedef struct _input {
    uint8_t controller[2];
    uint8_t shift[2];
    uint8_t strobe;
} _input;

void input_cpu_write(_input* input, uint16_t addr, uint8_t data);
uint8_t input_cpu_read(_input* input, uint16_t addr);
