#ifndef BPR_H
#define BPR_H

#include <SDL.h>

typedef struct {
    Uint8 value;
    uint32_t length;
} bpr_pulse_t;

extern bpr_pulse_t *bpr_pulses;
extern bpr_pulse_t *bpr_write_ptr;
extern bpr_pulse_t *bpr_read_ptr;
extern SDL_mutex *bpr_mutex;

extern SDL_AudioDeviceID bpr_device;
extern uint32_t bpr_pulse_tstates;
extern Uint8 bpr_last_value;
extern uint8_t bpr_new_value;
extern int bpr_prebuffered_samples;

void bpr_init();
void bpr_update(uint32_t tstates);
void bpr_generate_pulse();
void bpr_inc_pointer(bpr_pulse_t **bpr_ptr);
uint32_t bpr_bpr_convert_tstates_to_samples(uint32_t tstates);
void bpr_dispose();

#endif // BPR_H
