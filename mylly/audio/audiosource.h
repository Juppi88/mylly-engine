#pragma once
#ifndef __AUDIOSOURCE_H
#define __AUDIOSOURCE_H

#include "core/defines.h"
#include "collections/array.h"

// -------------------------------------------------------------------------------------------------

typedef struct active_source_t {

	sound_t *sound; // Sound that is being played
	uint32_t source; // Backend source handle

} active_source_t;

// -------------------------------------------------------------------------------------------------

typedef struct audiosrc_t {

	object_t *parent; // The scene object this audio source is attached to, or NULL if at listener
	arr_t(active_source_t) sources; // A list of sources which are currently active

} audiosrc_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

// NOTE: Not to be called directly.
audiosrc_t *audiosrc_create(object_t *parent);
void audiosrc_destroy(audiosrc_t *source);

void audiosrc_process(audiosrc_t *source);

void audiosrc_play(audiosrc_t *source, sound_t *sound);
void audiosrc_stop(audiosrc_t *source);

bool audiosrc_has_sounds_playing(audiosrc_t *source);

END_DECLARATIONS;

#endif
