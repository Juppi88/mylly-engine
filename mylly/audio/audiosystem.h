#pragma once
#ifndef __AUDIOSYSTEM_H
#define __AUDIOSYSTEM_H

#include "core/defines.h"
#include "audio/audiobuffer.h"

// -------------------------------------------------------------------------------------------------

typedef uint32_t audio_source_t; // Ditto for audio sources
typedef uint32_t sound_instance_t; // A handle to playing sound instance

// -------------------------------------------------------------------------------------------------

#define MAX_AUDIO_GROUPS 4 // Number of different audio (mixer) groups

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

void audio_initialize(void);
void audio_shutdown(void);
void audio_update(void);

sound_instance_t audio_play_sound(sound_t *sound, uint8_t group);
sound_instance_t audio_play_sound_from_source(sound_t *sound, audiosrc_t *source);
void audio_stop_sound(sound_instance_t sound);
void audio_stop_source(audiosrc_t *source);

void audio_set_master_gain(float gain);
void audio_set_group_gain(uint8_t group_index, float gain);

void audio_set_sound_gain(sound_instance_t sound, float gain);
void audio_set_sound_pitch(sound_instance_t sound, float pitch);
void audio_set_sound_looping(sound_instance_t sound, bool loop);

object_t *audio_get_listener(void);
void audio_set_listener(object_t *object);

// For internal use.
audiobuffer_id_t audio_create_buffer(void);
void audio_destroy_buffer(audiobuffer_id_t buffer);

void audio_load_buffer(audiobuffer_id_t buffer, uint32_t channels, uint32_t bits_per_sample,
                       const void *data, size_t data_size, size_t frequency);

END_DECLARATIONS;

#endif
