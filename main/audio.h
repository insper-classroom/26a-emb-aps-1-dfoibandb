#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>

void audio_core_entry(void);
void play_audio(const uint8_t *wav, uint32_t length);
void wait_audio(void);
void prepare_tone(int freq, int duration_ms);
void pwm_interrupt_handler(void);

#endif
