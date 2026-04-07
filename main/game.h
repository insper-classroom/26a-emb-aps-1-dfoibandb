#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

typedef enum { MODE_NORMAL, MODE_HARD } GameMode;
typedef enum { STATE_IDLE, STATE_SHOW, STATE_INPUT } GameState;

typedef struct {
    int show_ms;
    int gap_ms;
    int level_inc;
} ModeConfig;

extern const ModeConfig CONFIGS[];
extern volatile bool timeout_ocorreu;

void show_sequence(const int *seq, const int *seq2, int nivel, const ModeConfig *cfg);
void feedback_erro(int rounds_passed);
void feedback_acerto(void);
void feedback_botao(int btn);
void feedback_par(int a, int b);
void iniciar_prazo(void);
void cancelar_prazo(void);
bool prazo_ativo(void);

#endif
