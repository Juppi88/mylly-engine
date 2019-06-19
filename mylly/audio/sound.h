#pragma once
#ifndef __SOUND_H
#define __SOUND_H

#include "core/defines.h"
#include "resources/resource.h"

// -------------------------------------------------------------------------------------------------

typedef uint32_t audio_buffer_t; // Audio buffer index used by OpenAL

// -------------------------------------------------------------------------------------------------

typedef struct sound_t {

	resource_t resource; // Resource info
	audio_buffer_t buffer; // Audio buffer object

} sound_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

// NOTE: Should not be used directly. Load resources via the res_* API.
sound_t *sound_create(const char *name, const char *path);
void sound_destroy(sound_t *sound);

bool sound_load_wav(sound_t *sound, void *data, size_t data_length);

END_DECLARATIONS;

#endif
