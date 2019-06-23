#pragma once
#ifndef __AUDIOSYSTEM_H
#define __AUDIOSYSTEM_H

#include "core/defines.h"

// -------------------------------------------------------------------------------------------------

typedef uint32_t audio_buffer_t; // Audio buffer index used by OpenAL
typedef uint32_t audio_source_t; // Ditto for audio sources

// -------------------------------------------------------------------------------------------------

#define MAX_AUDIO_GROUPS 4 // Number of different audio (mixer) groups

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

void audio_initialize(void);
void audio_shutdown(void);
void audio_update(void);

void audio_play_sound(sound_t *sound, uint8_t group);
void audio_play_sound_from_source(sound_t *sound, audiosrc_t *source);
void audio_stop_source(audiosrc_t *source);

void audio_set_master_gain(float gain);
void audio_set_group_gain(uint8_t group_index, float gain);

object_t *audio_get_listener(void);
void audio_set_listener(object_t *object);

// For internal use.
audio_buffer_t audio_create_buffer(uint32_t channels, uint32_t bits_per_sample,
                                   const void *data, size_t data_size, size_t frequency);

void audio_destroy_buffer(audio_buffer_t buffer);

END_DECLARATIONS;

#endif
