#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "wahwah.h"
#include "success.h"

#define NUM_PAIRS  4
#define BUZZER_PIN 9
#define BEEP_MS    100


const unsigned int LEDS[]    = {10, 11, 12, 13};
const unsigned int BUTTONS[] = {18, 19, 20, 21};
const unsigned int FREQS[]   = {440, 880, 1320, 1760};

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

int main(void) {
    stdio_init_all();
    
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    gpio_set_irq_enabled_with_callback(BUTTONS[0], GPIO_IRQ_EDGE_FALL, true, button_callback);

    for (int i = 0; i < NUM_PAIRS; i++) {
        gpio_init(LEDS[i]);
        gpio_set_dir(LEDS[i], GPIO_OUT);

        gpio_init(BUTTONS[i]);
        gpio_set_dir(BUTTONS[i], GPIO_IN);
        gpio_pull_up(BUTTONS[i]);

        gpio_set_irq_enabled(BUTTONS[i], GPIO_IRQ_EDGE_FALL, true);
    }

    while (true) {
        int idx = pressed_index;

        if (idx >= 0 && idx < NUM_PAIRS) {
            pressed_index = -1;
            gpio_put(LEDS[idx], 1);
            buzzer((int)FREQS[idx], BEEP_MS);
            gpio_put(LEDS[idx], 0);
        }
    }
}
