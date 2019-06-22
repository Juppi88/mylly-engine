#pragma once
#ifndef __AUDIOSYSTEM_H
#define __AUDIOSYSTEM_H

#include "core/defines.h"

// -------------------------------------------------------------------------------------------------

typedef uint32_t audio_buffer_t; // Audio buffer index used by OpenAL
typedef uint32_t audio_source_t; // Ditto for audio sources

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

void audio_initialize(void);
void audio_shutdown(void);
void audio_update(void);

void audio_play_sound(sound_t *sound);

object_t *audio_get_listener(void);
void audio_set_listener(object_t *object);

// For internal use.
audio_buffer_t audio_create_buffer(uint32_t channels, uint32_t bits_per_sample,
                                   const void *data, size_t data_size, size_t frequency);

void audio_destroy_buffer(audio_buffer_t buffer);

END_DECLARATIONS;

#endif
