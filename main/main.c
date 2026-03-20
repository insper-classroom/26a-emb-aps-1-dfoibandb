#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "wahwah.h"
#include "success.h"

#define NUM_PAIRS   4
#define BUZZER_PIN  9
#define AUDIO_PIN   2
#define BEEP_MS     100
#define DEBOUNCE_US 200000  // 200 ms

const unsigned int LEDS[]    = {10, 11, 12, 13};
const unsigned int BUTTONS[] = {18, 19, 20, 21};
const unsigned int FREQS[]   = {440, 880, 1320, 1760};

// --- Lógica do buzzer (original, sem alterações) ---
volatile int pressed_index = -1;

void button_callback(unsigned int gpio, uint32_t events) {
    pressed_index = gpio - BUTTONS[0];
}

void buzzer(int freq, int time) {
    int periodo = 1000000 / freq;
    int ciclos  = (freq * time) / 1000;

    for (int i = 0; i < ciclos; i++) {
        gpio_put(BUZZER_PIN, 1);
        sleep_us((unsigned int)(periodo / 2));
        gpio_put(BUZZER_PIN, 0);
        sleep_us((unsigned int)(periodo / 2));
    }
}

// --- Lógica de áudio PWM no GPIO 2 ---
volatile const uint8_t *current_wav    = NULL;
volatile uint32_t       current_length = 0;
volatile int32_t        wav_position   = -1; // -1 = parado

void pwm_interrupt_handler() {
    pwm_clear_irq(pwm_gpio_to_slice_num(AUDIO_PIN));

    if (wav_position < 0 || current_wav == NULL) return;

    if (wav_position < (int32_t)(current_length << 3) - 1) {
        pwm_set_gpio_level(AUDIO_PIN, current_wav[wav_position >> 3]);
        wav_position++;
    } else {
        pwm_set_gpio_level(AUDIO_PIN, 0);
        wav_position = -1;
    }
}

void play_audio(const uint8_t *wav, uint32_t length) {
    while (wav_position >= 0) tight_loop_contents(); // aguarda terminar
    current_wav    = wav;
    current_length = length;
    wav_position   = 0;
}

int main(void) {
    stdio_init_all();

    // Clock a 176 MHz para taxa de amostragem precisa de 11 KHz
    set_sys_clock_khz(176000, true);

    // --- Setup buzzer (original) ---
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    // --- Setup áudio PWM no GPIO 2 ---
    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);
    int audio_slice = pwm_gpio_to_slice_num(AUDIO_PIN);

    pwm_clear_irq(audio_slice);
    pwm_set_irq_enabled(audio_slice, true);
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_interrupt_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    // 176 MHz / (clkdiv=8 * wrap=250) / 8 repetições = 11 000 Hz ✓
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 8.0f);
    pwm_config_set_wrap(&config, 250);
    pwm_init(audio_slice, &config, true);
    pwm_set_gpio_level(AUDIO_PIN, 0);

    // --- Setup LEDs e botões (original) ---
    gpio_set_irq_enabled_with_callback(BUTTONS[0], GPIO_IRQ_EDGE_FALL, true, button_callback);

    for (int i = 0; i < NUM_PAIRS; i++) {
        gpio_init(LEDS[i]);
        gpio_set_dir(LEDS[i], GPIO_OUT);

        gpio_init(BUTTONS[i]);
        gpio_set_dir(BUTTONS[i], GPIO_IN);
        gpio_pull_up(BUTTONS[i]);

        gpio_set_irq_enabled(BUTTONS[i], GPIO_IRQ_EDGE_FALL, true);
    }

    static uint32_t last_time = 0;

    while (true) {
        int idx = pressed_index;

        if (idx >= 0 && idx < NUM_PAIRS) {
            uint32_t now = to_us_since_boot(get_absolute_time());

            if (now - last_time >= DEBOUNCE_US) {
                last_time = now;
                pressed_index = -1;
                gpio_put(LEDS[idx], 1);

                if (idx == 0) {
                    play_audio(WAV_SUCCESS, WAV_SUCCESS_LENGTH);
                    while (wav_position >= 0) tight_loop_contents();
                } else if (idx == 1) {
                    play_audio(WAV_WAHWAH, WAV_WAHWAH_LENGTH);
                    while (wav_position >= 0) tight_loop_contents();
                } else {
                    buzzer((int)FREQS[idx], BEEP_MS);
                }

                gpio_put(LEDS[idx], 0);
            } else {
                // Bounce detectado — descarta
                pressed_index = -1;
            }
        }
    }
}
