#ifndef BUTTONS_H
#define BUTTONS_H

#include <stdint.h>

extern volatile uint8_t botoes_apertados;

void buttons_init(void);
void button_callback(unsigned int gpio, uint32_t events);
int primeiro_botao(uint8_t leitura);

#endif
