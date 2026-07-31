#pragma once
#include "Arduino.h"
#include "board_config.h"

// Simple I2S Audio for the PCM5101/PCM5100A DAC.
// I2S_BCLK / I2S_LRC / I2S_DOUT are board-specific and defined in
// board_config.h. I2S_SWITCH_PIN (Knob 1.8 only) selects the CH445P audio
// mux input, see simple_audio_init().

// Audio settings
#define AUDIO_VOLUME_MAX 21
// Note: AUDIO_VOLUME_DEFAULT is defined in config.h

// Sound types
typedef enum {
    SOUND_TIMER_FINISH = 0,
    SOUND_TIMER_FINISH_ALT1,
    SOUND_TIMER_FINISH_ALT2,
    SOUND_TIMER_FINISH_ALT3,
    SOUND_BUTTON_CLICK,
    SOUND_SUCCESS,
    SOUND_ERROR,
    SOUND_STARTUP,
    SOUND_COUNT
} sound_type_t;

// Simple audio functions
void simple_audio_init();
void simple_audio_beep(int frequency, int duration_ms);
void simple_audio_play_sound(sound_type_t sound);
void simple_audio_set_volume(int volume);
int simple_audio_get_volume();
void simple_audio_set_enabled(bool enabled);
bool simple_audio_is_enabled();
void simple_audio_set_timer_sound(sound_type_t sound);
sound_type_t simple_audio_get_timer_sound();
void simple_audio_cleanup();

// Wrapper functions for settings compatibility
bool getAudioEnabled();
void toggleAudioEnabled();
int getVolume();
void setVolume(int volume);
bool getSoundEffectsEnabled();
void toggleSoundEffectsEnabled();
