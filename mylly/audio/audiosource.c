#include "audiosource.h"
#include "audiosystem.h"
#include "sound.h"
#include "scene/object.h"
#include "core/memory.h"
#include "math/math.h"
#include <AL/al.h>
#include <AL/alc.h>

// -------------------------------------------------------------------------------------------------

audiosrc_t *audiosrc_create(object_t *parent)
{
	NEW(audiosrc_t, source);

	source->parent = parent;
	source->group_index = 0;
	source->gain = 1;
	source->pitch = 1;

	return source;
}

void audiosrc_destroy(audiosrc_t *source)
{
	if (source == NULL) {
		return;
	}

	// Stop the audio source to release all source objects for other use.
	audio_stop_source(source);

	DESTROY(source);
}

void audiosrc_set_group(audiosrc_t *source, uint8_t group_index)
{
	if (source == NULL || group_index >= MAX_AUDIO_GROUPS) {
		return;
	}

	source->group_index = group_index;
}

void audiosrc_set_gain(audiosrc_t *source, float gain)
{
	if (source == NULL) {
		return;
	}

	source->gain = CLAMP01(gain);
	source->is_source_dirty = true;
}

void audiosrc_set_pitch(audiosrc_t *source, float pitch)
{
	if (source == NULL) {
		return;
	}

	if (pitch <= 0) {
		pitch = 0.01f;
	}

	source->pitch = pitch;
	source->is_source_dirty = true;
}
