#include "buttons.h"
#include "config.h"
#include "hardware/gpio.h"

volatile uint8_t botoes_apertados = 0;

void button_callback(unsigned int gpio, uint32_t events) {
    (void)events;
    botoes_apertados |= (uint8_t)(1u << (gpio - BUTTONS[0]));
}

int primeiro_botao(uint8_t leitura) {
    for (int btn = 0; btn < NUM_PAIRS; btn++) {
        if (leitura & (1u << btn))
            return btn;
    }
    return -1;
}

void buttons_init(void) {
    gpio_set_irq_enabled_with_callback(BUTTONS[0], GPIO_IRQ_EDGE_FALL, true, button_callback);

    for (int i = 0; i < NUM_PAIRS; i++) {
        gpio_init(BUTTONS[i]);
        gpio_set_dir(BUTTONS[i], GPIO_IN);
        gpio_pull_up(BUTTONS[i]);
        gpio_set_irq_enabled(BUTTONS[i], GPIO_IRQ_EDGE_FALL, true);
    }
}
