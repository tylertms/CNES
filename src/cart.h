#pragma once
#include <stdint.h>

typedef struct _cart {

} _cart;

uint8_t cart_read(_cart* cart, uint16_t addr);
void cart_write(_cart* cart, uint16_t addr, uint8_t data);
