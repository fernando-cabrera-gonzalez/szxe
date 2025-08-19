/*
 * zx_bpr.c - Beeper Implementation
 *
 * Implements the beeper (audio) functionality for the ZX Spectrum emulator.
 * Handles audio buffer, pulse generation, and SDL audio device management.
 */

#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "zx_bpr.h"
#include "zx_ula.h"
#include "zx_utl.h"

#define BPR_SAMPLE_RATE         22050
#define BPR_BUFFER_SIZE         8192
#define BPR_AUDIO_FORMAT        AUDIO_U8
#define BPR_VOLUME              128
#define BPR_CPU_FREQ            3494400.0

SDL_AudioDeviceID bpr_device = 0;
bpr_pulse_t *bpr_pulses;
bpr_pulse_t *bpr_write_ptr;
bpr_pulse_t *bpr_read_ptr;
SDL_mutex *bpr_mutex;

uint32_t bpr_pulse_tstates = 0;
bool bpr_on = false;
Uint8 bpr_last_value = -1;
uint8_t bpr_new_value = 0;
int bpr_prebuffered_samples = 0;

const double tstates_per_sample = BPR_CPU_FREQ / BPR_SAMPLE_RATE;
Uint32 last_callback = 0;

void bpr_audio_callback(void *userdata, Uint8 *stream, int len) {
    Uint32 now = SDL_GetTicks();
    last_callback = now;

    SDL_LockMutex(bpr_mutex);

    int count = len / (BPR_AUDIO_FORMAT / 8);
    uint32_t num_samples = 0;
    int i = 0;
    int total_samples = 0;

    while (bpr_read_ptr != bpr_write_ptr && count > 0) {
        num_samples = MIN(bpr_read_ptr->length, count);
        for (i = len - count; i < (len - count) + num_samples; i++) {
            stream[i] = bpr_read_ptr->value;
        }
        count -= num_samples;
        bpr_read_ptr->length -= num_samples;
        total_samples += num_samples;

        if (bpr_read_ptr->length == 0) {
            bpr_read_ptr->value = 0;
            bpr_inc_pointer(&bpr_read_ptr);
        }
    }

    SDL_UnlockMutex(bpr_mutex);

    last_callback = now; 
    bpr_prebuffered_samples = 0;
}

void bpr_init() {
    if (0 != SDL_Init(SDL_INIT_EVERYTHING)) {
        SDL_Log ("SDL_Init failed: %s\n", SDL_GetError ());
        return;
    }

    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = BPR_SAMPLE_RATE;
    want.format = AUDIO_U8;
    want.channels = 1;
    want.samples = 1024;
    want.callback = bpr_audio_callback;

    bpr_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (bpr_device == 0) {
        SDL_Log("Failed to open audio device: %s", SDL_GetError());
        return;
    }
    SDL_Log("Audio device opened successfully with frequency: %d, format: %d, channels: %d, samples: %d",
            have.freq, have.format, have.channels, have.samples);
    SDL_PauseAudioDevice (bpr_device, 1);

    bpr_pulses = (bpr_pulse_t*) calloc(BPR_BUFFER_SIZE, sizeof(bpr_pulse_t));
    for (int i = 0; i < BPR_BUFFER_SIZE; i++) {
        bpr_pulses[i].value = 0;
        bpr_pulses[i].length = 0;
    }
    bpr_write_ptr = bpr_read_ptr = bpr_pulses;

    bpr_mutex = SDL_CreateMutex();
    if (bpr_mutex == NULL) {
        printf("Error creando mutex: %s\n", SDL_GetError());
        return;
    }
}

void bpr_inc_pointer(bpr_pulse_t **bpr_ptr) {
    (*bpr_ptr)++;
    if (*bpr_ptr - bpr_pulses >= BPR_BUFFER_SIZE) {
        *bpr_ptr = bpr_pulses;
    }
}

uint32_t bpr_convert_tstates_to_samples(uint32_t tstates) {
    uint32_t result = tstates / tstates_per_sample;
    if (result > 0) {
        bpr_pulse_tstates = tstates % (uint32_t)tstates_per_sample;
    }
    return result; 
}

void bpr_generate_pulse() {
    uint32_t num_samples = bpr_convert_tstates_to_samples(bpr_pulse_tstates);
    if (num_samples > 0) {
        SDL_LockMutex(bpr_mutex);
        bpr_write_ptr->length = num_samples;
        bpr_write_ptr->value = bpr_last_value ? BPR_VOLUME : 0;
        bpr_inc_pointer(&bpr_write_ptr);
        SDL_UnlockMutex(bpr_mutex);

        bpr_prebuffered_samples += num_samples;
        if (!bpr_on && bpr_prebuffered_samples > 4000) {
            SDL_PauseAudioDevice(bpr_device, 0);
            bpr_on = true;    
        }
    }
}

void bpr_update(uint32_t tstates) {
    bpr_pulse_tstates += tstates;

    if (bpr_pulse_tstates >= tstates_per_sample * 80) {
        bpr_generate_pulse();   
    }
}

void bpr_dispose() {
    if (bpr_device) {
        SDL_CloseAudioDevice(bpr_device);
        bpr_device = 0;
    }
}