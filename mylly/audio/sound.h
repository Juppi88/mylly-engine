#pragma once
#ifndef __SOUND_H
#define __SOUND_H

#include "core/defines.h"
#include "resources/resource.h"
#include "audio/audiosystem.h"

// -------------------------------------------------------------------------------------------------

#define SOUND_STREAM_BLOCK_SIZE 1 * 44100 / 2 // Stream 0.5 second of 2 channel audio at 44.1kHz

// -------------------------------------------------------------------------------------------------

typedef struct sound_t {

	resource_t resource; // Resource info
	audio_buffer_t buffer; // Audio buffer object

	bool is_streaming; // Set to true when the sound is streamed (e.g. a music track)
	size_t num_samples; // Number of PCM samples in the sound
	void *decoder; // Decoder instance (drmp3) if the sound is to be streamed
	void *encoded_data; // Raw encoded audio data, used by the decoder

} sound_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

// NOTE: Should not be used directly. Load resources via the res_* API.
sound_t *sound_create(const char *name, const char *path);
void sound_destroy(sound_t *sound);

bool sound_load_wav(sound_t *sound, void *data, size_t data_length);
bool sound_load_mp3(sound_t *sound, void *data, size_t data_length);

bool sound_stream(sound_t *sound, audio_buffer_t buffer, size_t *start_sample);

END_DECLARATIONS;

#endif
