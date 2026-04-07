#ifndef CONFIG_H
#define CONFIG_H

#define NUM_PAIRS     4
#define AUDIO_PIN     2
#define BUZZER_PIN    9
#define MAX_SEQ       20
#define VOLUME_DIV    8
#define TIMEOUT_MS    2000

#define BTN_GREEN  0
#define BTN_RED    1
#define BTN_BLUE   2
#define BTN_YELLOW 3

extern const unsigned int LEDS[];
extern const unsigned int BUTTONS[];
extern const unsigned int FREQS[];

#endif
