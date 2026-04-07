#include "leds.h"
#include "config.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

void leds_init(void) {
    for (int i = 0; i < NUM_PAIRS; i++) {
        gpio_init(LEDS[i]);
        gpio_set_dir(LEDS[i], GPIO_OUT);
    }
}

void leds_all(int state) {
    for (int i = 0; i < NUM_PAIRS; i++) {
        gpio_put(LEDS[i], state);
    }
}

void idle_animation(void) {
    gpio_put(LEDS[BTN_YELLOW], 1);
    gpio_put(LEDS[BTN_BLUE],   1);
    gpio_put(LEDS[BTN_RED],    0);
    gpio_put(LEDS[BTN_GREEN],  0);
    sleep_ms(300);
    gpio_put(LEDS[BTN_YELLOW], 0);
    gpio_put(LEDS[BTN_BLUE],   0);
    gpio_put(LEDS[BTN_RED],    1);
    gpio_put(LEDS[BTN_GREEN],  1);
    sleep_ms(300);
    leds_all(0);
}
