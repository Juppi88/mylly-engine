#pragma once
#ifndef __AUDIOSOURCE_H
#define __AUDIOSOURCE_H

#include "core/defines.h"
#include "collections/array.h"

// -------------------------------------------------------------------------------------------------

typedef struct audiosrc_t {

	object_t *parent; // The scene object this audio source is attached to, or NULL if at listener

	uint8_t group_index; // Audio group index
	float gain; // Current gain for this audio source
	float pitch; // Current pitch for this audio source

	bool is_source_dirty; // Set to true when the source object requires an update

} audiosrc_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

// NOTE: Not to be called directly.
audiosrc_t *audiosrc_create(object_t *parent);
void audiosrc_destroy(audiosrc_t *source);

void audiosrc_set_group(audiosrc_t *source, uint8_t group_index);
void audiosrc_set_gain(audiosrc_t *source, float gain);
void audiosrc_set_pitch(audiosrc_t *source, float pitch);

END_DECLARATIONS;

#endif
