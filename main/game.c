#include "game.h"
#include "config.h"
#include "audio.h"
#include "leds.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "wahwah.h"
#include "success.h"

const ModeConfig CONFIGS[] = {
    { 500, 150, 1 },
    { 500, 150, 1 },
};

volatile bool timeout_ocorreu = false;
static alarm_id_t alarm_id = -1;

static int64_t ao_esgotar_prazo(alarm_id_t id, void *dados) {
    (void)id;
    (void)dados;
    timeout_ocorreu = true;
    return 0;
}

void iniciar_prazo(void) {
    if (alarm_id > 0) cancel_alarm(alarm_id);
    timeout_ocorreu = false;
    alarm_id = add_alarm_in_ms(TIMEOUT_MS, ao_esgotar_prazo, NULL, false);
}

void cancelar_prazo(void) {
    if (alarm_id > 0) cancel_alarm(alarm_id);
    alarm_id = -1;
    timeout_ocorreu = false;
}

bool prazo_ativo(void) {
    return alarm_id > 0;
}

void show_sequence(const int *seq, const int *seq2, int nivel, const ModeConfig *cfg) {
    for (int i = 0; i < nivel; i++) {
        int elem = seq[i];
        gpio_put(LEDS[elem], 1);
        if (seq2 != NULL) gpio_put(LEDS[seq2[i]], 1);
        if (seq2 != NULL) {
            int avg = ((int)FREQS[elem] + (int)FREQS[seq2[i]]) / 2;
            prepare_tone(avg, cfg->show_ms);
        } else {
            prepare_tone((int)FREQS[elem], cfg->show_ms);
        }
        wait_audio();
        gpio_put(LEDS[elem], 0);
        if (seq2 != NULL) gpio_put(LEDS[seq2[i]], 0);
        sleep_ms(cfg->gap_ms);
    }
}

void feedback_botao(int btn) {
    gpio_put(LEDS[btn], 1);
    prepare_tone((int)FREQS[btn], 150);
    wait_audio();
    gpio_put(LEDS[btn], 0);
}

void feedback_par(int a, int b) {
    gpio_put(LEDS[a], 1);
    gpio_put(LEDS[b], 1);
    prepare_tone(((int)FREQS[a] + (int)FREQS[b]) / 2, 150);
    wait_audio();
    gpio_put(LEDS[a], 0);
    gpio_put(LEDS[b], 0);
}

void feedback_erro(int rounds_passed) {
    play_audio(WAV_WAHWAH, WAV_WAHWAH_LENGTH);
    leds_all(1);
    sleep_ms(300);
    leds_all(0);
    sleep_ms(200);
    for (int i = 0; i < rounds_passed; i++) {
        gpio_put(LEDS[i % NUM_PAIRS], 1);
        sleep_ms(400);
        gpio_put(LEDS[i % NUM_PAIRS], 0);
        sleep_ms(150);
    }
    wait_audio();
}

void feedback_acerto(void) {
    play_audio(WAV_SUCCESS, WAV_SUCCESS_LENGTH);
    for (int i = 0; i < NUM_PAIRS; i++) {
        gpio_put(LEDS[i], 1);
        sleep_ms(80);
        gpio_put(LEDS[i], 0);
    }
    wait_audio();
}
