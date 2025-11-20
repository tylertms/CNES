#pragma once
#include <SDL3/SDL.h>

typedef struct _gui {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    uint32_t* pixels;
} _gui;

int init_gui(_gui* gui);
void set_pixel(_gui* gui, uint16_t x, uint16_t y, uint32_t color);
void draw_gui(_gui* gui);
void deinit_gui(_gui *gui);
