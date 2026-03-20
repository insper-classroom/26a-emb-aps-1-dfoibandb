#include <stdint.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "config.h"
#include "audio.h"
#include "buttons.h"
#include "leds.h"
#include "game.h"

int main(void) {
    stdio_init_all();
    set_sys_clock_khz(176000, true);

    multicore_launch_core1(audio_core_entry);
    multicore_fifo_pop_blocking();

    leds_init();
    buttons_init();

    int sequencia[MAX_SEQ];
    int sequencia2[MAX_SEQ];
    GameState estado = STATE_IDLE;
    GameMode modo = MODE_NORMAL;
    int nivel = 1;
    int pos_jogador = 0;

    while (true) {
        switch (estado) {

            case STATE_IDLE: {
                botoes_apertados = 0;
                idle_animation();

                uint8_t leitura = botoes_apertados;
                if (leitura == 0) break;

                srand((unsigned int)to_us_since_boot(get_absolute_time()));

                int btn = primeiro_botao(leitura);
                modo = (btn == BTN_RED || btn == BTN_GREEN) ? MODE_HARD : MODE_NORMAL;

                for (int i = 0; i < MAX_SEQ; i++) {
                    sequencia[i] = rand() % NUM_PAIRS;
                    if (modo == MODE_HARD) {
                        do {
                            sequencia2[i] = rand() % NUM_PAIRS;
                        } while (sequencia2[i] == sequencia[i]);
                    }
                }

                botoes_apertados = 0;
                nivel = 1;
                estado = STATE_SHOW;
                break;
            }

            case STATE_SHOW: {
                sleep_ms(800);
                const ModeConfig *cfg = &CONFIGS[modo];
                show_sequence(sequencia, modo == MODE_HARD ? sequencia2 : NULL, nivel, cfg);
                pos_jogador = 0;
                cancelar_prazo();
                botoes_apertados = 0;
                estado = STATE_INPUT;
                break;
            }

            case STATE_INPUT: {
                if (timeout_ocorreu) {
                    cancelar_prazo();
                    feedback_erro(nivel - 1);
                    nivel = 1;
                    estado = STATE_IDLE;
                    break;
                }

                const ModeConfig *cfg = &CONFIGS[modo];

                if (modo == MODE_NORMAL) {
                    uint8_t leitura = botoes_apertados;
                    if (leitura == 0) break;
                    botoes_apertados = 0;

                    int btn = primeiro_botao(leitura);
                    feedback_botao(btn);
                    botoes_apertados = 0;

                    if (btn != sequencia[pos_jogador]) {
                        cancelar_prazo();
                        feedback_erro(nivel - 1);
                        nivel = 1;
                        estado = STATE_IDLE;
                        break;
                    }

                    pos_jogador++;
                    iniciar_prazo();

                    if (pos_jogador == nivel) {
                        cancelar_prazo();
                        feedback_acerto();
                        nivel += cfg->level_inc;
                        if (nivel > MAX_SEQ) nivel = MAX_SEQ;
                        estado = STATE_SHOW;
                    }

                } else { //modo dificil
                    uint8_t bit_a = (1u << sequencia[pos_jogador]);
                    uint8_t bit_b = (1u << sequencia2[pos_jogador]);
                    uint8_t esperado = bit_a | bit_b;
                    uint8_t leitura = botoes_apertados;

                    if (leitura == 0) break;

                    bool botao_errado = leitura & ~esperado;
                    bool par_completo = (leitura & esperado) == esperado;
                    bool botao_parcial = !botao_errado && !par_completo;

                    if (botao_errado) {
                        botoes_apertados = 0;
                        cancelar_prazo();
                        feedback_erro(nivel - 1);
                        nivel = 1;
                        estado = STATE_IDLE;
                        break;
                    }

                    if (botao_parcial) {
                        if (!prazo_ativo()) iniciar_prazo();
                        break;
                    }

                    botoes_apertados = 0;
                    feedback_par(sequencia[pos_jogador], sequencia2[pos_jogador]);
                    botoes_apertados = 0;

                    pos_jogador++;
                    iniciar_prazo();

                    if (pos_jogador == nivel) {
                        cancelar_prazo();
                        feedback_acerto();
                        nivel += cfg->level_inc;
                        if (nivel > MAX_SEQ) nivel = MAX_SEQ;
                        estado = STATE_SHOW;
                    }
                }
                break;
            }
        }
    }
}
