#include "audio.h"
#include "config.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/sync.h"
#include "hardware/clocks.h"

static volatile const uint8_t *current_wav = NULL;
static volatile uint32_t current_length = 0;
static volatile int32_t wav_position = -1;
static semaphore_t audio_sem;

void pwm_interrupt_handler(void) {
    pwm_clear_irq(pwm_gpio_to_slice_num(AUDIO_PIN));

    if (wav_position < 0 || current_wav == NULL) return;

    if (wav_position < (int32_t)(current_length << 3) - 1) {
        int sample = (int)current_wav[wav_position >> 3];
        int scaled = 128 + (sample - 128) / VOLUME_DIV;
        pwm_set_gpio_level(AUDIO_PIN, (uint16_t)scaled);
        wav_position++;
    } else {
        pwm_set_gpio_level(AUDIO_PIN, 0);
        wav_position = -1;
        sem_release(&audio_sem);
    }
}

void audio_init(void) {
    sem_init(&audio_sem, 1, 1);

    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
    int audio_slice = pwm_gpio_to_slice_num(AUDIO_PIN);

    pwm_clear_irq(audio_slice);
    pwm_set_irq_enabled(audio_slice, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_interrupt_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 8.0f);
    pwm_config_set_wrap(&config, 250);
    pwm_init(audio_slice, &config, true);
    pwm_set_gpio_level(AUDIO_PIN, 0);
}

void audio_core_entry(void) {
    audio_init();
    multicore_fifo_push_blocking(1);

    while (true) __wfi();
}

void play_audio(const uint8_t *wav, uint32_t length) {
    sem_acquire_blocking(&audio_sem);
    current_wav = wav;
    current_length = length;
    __dmb();
    wav_position = 0;
}

void wait_audio(void) {
    sem_acquire_blocking(&audio_sem);
    sem_release(&audio_sem);
}

void prepare_tone(int freq, int duration_ms) {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    uint32_t sys_hz = clock_get_hz(clk_sys);
    uint32_t div = sys_hz / ((uint32_t)freq * 65535u) + 1u;
    uint32_t wrap = sys_hz / (div * (uint32_t)freq);

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_int_frac(&cfg, (uint8_t)div, 0);
    pwm_config_set_wrap(&cfg, (uint16_t)(wrap - 1u));
    pwm_init(slice, &cfg, true);
    pwm_set_gpio_level(BUZZER_PIN, (uint16_t)(wrap / 2u));

    sleep_ms((uint32_t)duration_ms);

    pwm_set_gpio_level(BUZZER_PIN, 0);
    pwm_set_enabled(slice, false);
}
