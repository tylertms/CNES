#pragma once
#include <SDL3/SDL_audio.h>

#define SAMPLE_RATE 48000.f

typedef struct _apu {
    SDL_AudioStream* audio_stream;
    uint32_t sine_sample_index;
} _apu;

void apu_init(_apu* apu);
void apu_deinit(_apu* apu);
void apu_callback(void *userdata, SDL_AudioStream *astream, int additional_amount, int total_amount);
