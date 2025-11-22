#pragma once
#include <stdint.h>

typedef struct _input {
    uint8_t controller[2];
    uint8_t shift[2];
    uint8_t strobe;
} _input;
