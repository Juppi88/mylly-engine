#include "audiosource.h"
#include "audiosystem.h"
#include "sound.h"
#include "scene/object.h"
#include "core/memory.h"
#include <AL/al.h>
#include <AL/alc.h>

// -------------------------------------------------------------------------------------------------

static void audiosrc_remove_active_source(audiosrc_t *source, uint32_t active_source_idx);

// -------------------------------------------------------------------------------------------------

audiosrc_t *audiosrc_create(object_t *parent)
{
	NEW(audiosrc_t, source);

	source->parent = parent;
	arr_init(source->sources);

	return source;
}

void audiosrc_destroy(audiosrc_t *source)
{
	if (source == NULL) {
		return;
	}

	audiosrc_stop(source);
	arr_clear(source->sources);

	DESTROY(source);
}

void audiosrc_process(audiosrc_t *source)
{
	if (source == NULL) {
		return;
	}

	for (uint32_t i = source->sources.count; i > 0; i--) {

		// Get the current state of the source.
		ALint state;
		audio_source_t source_index = source->sources.items[i - 1].source;

		alGetSourcei(source_index, AL_SOURCE_STATE, &state);

		// If the source is no longer playing anything, remove it.
		if (state != AL_PLAYING) {
			audiosrc_remove_active_source(source, i - 1);
		}

		// TODO: Process streaming!
		// TODO: Move object attached to a scene object!
	}
}

void audiosrc_play(audiosrc_t *source, sound_t *sound)
{
	if (source == NULL || sound == NULL) {
		return;
	}

	// Create a new audio source and add it to the active source array.
	audio_source_t source_index;
	alGenSources(1, &source_index);

	if (source->parent == NULL) {

		// Audio source has no parent scene object i.e. it is attached to the listener.
		alSourcei(source_index, AL_SOURCE_RELATIVE, true);
		alSource3f(source_index, AL_POSITION, 0, 0, 0);
		alSource3f(source_index, AL_VELOCITY, 0, 0, 0);
		alSource3f(source_index, AL_DIRECTION, 0, 0, 0);
	}
	else {
		// TODO: Attach to scene object!
	}

	active_source_t active_source = (active_source_t){ sound, source_index };
	arr_push(source->sources, active_source);

	// Bind audio buffer to the source and play the sound.
	// TODO: Check whether the sound should be streamed!
	alSourcei(source_index, AL_BUFFER, sound->buffer);
	alSourcePlay(source_index);
}

void audiosrc_stop(audiosrc_t *source)
{
	if (source == NULL) {
		return;
	}

	// Stop and free all active sources.
	for (uint32_t i = source->sources.count; i > 0; i--) {
		audiosrc_remove_active_source(source, i - 1);
	}
}

bool audiosrc_has_sounds_playing(audiosrc_t *source)
{
	return (source != NULL && source->sources.count != 0);
}

static void audiosrc_remove_active_source(audiosrc_t *source, uint32_t active_source_idx)
{
	alDeleteSources(1, &source->sources.items[active_source_idx].source);
	arr_remove_at(source->sources, active_source_idx);
}
