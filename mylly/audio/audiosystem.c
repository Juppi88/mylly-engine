#include "audiosystem.h"
#include "audiosource.h"
#include "sound.h"
#include "io/log.h"
#include "scene/object.h"
#include "collections/array.h"
#include "math/math.h"
#include <AL/al.h>
#include <AL/alc.h>

// -------------------------------------------------------------------------------------------------

// Temporary data storage grouping an OpenAL audio source to Mylly audiosrc_t component.
// Used for listing active audio sources.
typedef struct source_t {

	audiosrc_t *source;
	audio_source_t source_object;
	sound_t *sound;
	sound_instance_t instance;
	float gain;
	float pitch;

} source_t;

// -------------------------------------------------------------------------------------------------

static ALCdevice *device; // Audio device
static ALCcontext *context; // Audio context
static object_t *listener_object; // The object which is the OpenAL listener

static arr_t(audio_source_t) free_source_objects = arr_initializer; // Unused audio source objects
static arr_t(source_t) active_sources = arr_initializer; // Active audio sources

static audiosrc_t *nonpositional_sources[MAX_AUDIO_GROUPS]; // One nonpositional source per group
static float group_gains[MAX_AUDIO_GROUPS] = { 1, 1, 1, 1 }; // Per group gains

static sound_instance_t next_sound_instance = 1; // A running counter for played sounds

// -------------------------------------------------------------------------------------------------

static uint32_t audio_get_active_source_index(sound_instance_t sound);

// -------------------------------------------------------------------------------------------------

void audio_initialize(void)
{
	// Reset OpenAL error stack.
	alGetError();

	// Try to open the default audio device.
	if (device == NULL) {
		device = alcOpenDevice(NULL);
	}
	
	if (device == NULL) {

		log_error("AudioSystem", "Could not find an audio device.");
		return;
	}

	// Create an audio context.
	if (context == NULL) {
		context = alcCreateContext(device, NULL);
	}

	if (context == NULL ||
		!alcMakeContextCurrent(context)) {

		log_error("AudioSystem", "Could not setup an audio context.");
	}

	// Pre-create nonpositional audio sources.
	for (uint8_t i = 0; i < MAX_AUDIO_GROUPS; i++) {

		audiosrc_t *source = audiosrc_create(NULL);

		audiosrc_set_group(source, i);
		nonpositional_sources[i] = source;
	}

	log_message("AudioSystem", "Audio system initialized.");
}

void audio_shutdown(void)
{
	// Destroy nonpositional audio sources.
	for (uint8_t i = 0; i < MAX_AUDIO_GROUPS; i++) {

		audiosrc_destroy(nonpositional_sources[i]);
		nonpositional_sources[i] = NULL;
	}

	// Dereference audio source containers. If the system is used correctly, there should be
	// no sources in the list. If there are, though, print a warning.
	if (active_sources.count != 0) {

		log_warning("AudioSystem", "Destroying %u active audio sources.", active_sources.count);

		for (uint32_t i = 0; i < active_sources.count; i++) {
			arr_push(free_source_objects, active_sources.items[i].source_object);
		}
	}

	arr_clear(active_sources);

	// Destroy OpenAL audio source objects.
	audio_source_t source_object;

	arr_foreach(free_source_objects, source_object) {
		alDeleteSources(1, &source_object);
	}

	arr_clear(free_source_objects);

	// Destroy OpenAL context and close the audio device.
	if (context != NULL) {

		alcMakeContextCurrent(NULL);
		alcDestroyContext(context);
		context = NULL;
	}

	if (device != NULL) {

		alcCloseDevice(device);
		device = NULL;
	}
}

void audio_update(void)
{
	if (context == NULL ||
		listener_object == NULL) {
		return;
	}

	// Update the listener's position and orientation.
	vec3_t position = obj_get_position(listener_object);
	vec3_t up = obj_get_up_vector(listener_object);
	vec3_t forward = obj_get_forward_vector(listener_object);

	ALfloat orientation[] = { forward.x, forward.y, forward.z, up.x, up.y, up.z };

	alListenerfv(AL_POSITION, position.vec);
	alListener3f(AL_VELOCITY, 0, 0, 0);
	alListenerfv(AL_ORIENTATION, orientation);

	// Process all active sources.
	uint32_t i;
	arr_foreach_reverse_iter(active_sources, i) {

		// Get the state of the audio source object.
		audio_source_t source_object = active_sources.items[i].source_object;
		audiosrc_t *source = active_sources.items[i].source;

		ALint state;
		alGetSourcei(source_object, AL_SOURCE_STATE, &state);

		// If the source is no longer playing anything, move it to free source objects list.
		if (state != AL_PLAYING) {
			
			alSourceStop(source_object);
			arr_push(free_source_objects, source_object);

			arr_remove_at(active_sources, i);
		}
		else {

			if (source->parent != NULL) {

				// Update audio source position.
				vec3_t position = obj_get_position(source->parent);
				alSourcefv(source_object, AL_POSITION, position.vec);
			}

			if (source->is_source_dirty) {

				// Update audio source properties.
				alSourcef(source_object, AL_GAIN, source->gain * group_gains[source->group_index]);
				alSourcef(source_object, AL_PITCH, source->pitch);

				source->is_source_dirty = false;
			}
		}
	}
}

sound_instance_t audio_play_sound(sound_t *sound, uint8_t group)
{
	if (context == NULL ||
		sound == NULL ||
		group >= MAX_AUDIO_GROUPS) {
		return 0;
	}

	return audio_play_sound_from_source(sound, nonpositional_sources[group]);
}

sound_instance_t audio_play_sound_from_source(sound_t *sound, audiosrc_t *source)
{
	audio_source_t source_object = 0;

	// Find a free audio source object from which to play the sound.
	if (free_source_objects.count != 0) {

		source_object = arr_first(free_source_objects);
		arr_remove_at(free_source_objects, 0);
	}
	else {

		// Generate a new source if no existing ones are available.
		alGenSources(1, &source_object);
	}

	// Set audio source properties.
	alSourcef(source_object, AL_GAIN, source->gain * group_gains[source->group_index]);
	alSourcef(source_object, AL_PITCH, source->pitch);

	source->is_source_dirty = false;

	if (source->parent == NULL) {

		// Audio source has no parent scene object (i.e. it is attached to the listener).
		alSourcei(source_object, AL_SOURCE_RELATIVE, true);
		alSource3f(source_object, AL_POSITION, 0, 0, 0);
		alSource3f(source_object, AL_VELOCITY, 0, 0, 0);
		alSource3f(source_object, AL_DIRECTION, 0, 0, 0);
	}
	else {

		alSourcei(source_object, AL_SOURCE_RELATIVE, false);
		alSource3f(source_object, AL_VELOCITY, 0, 0, 0);
		alSource3f(source_object, AL_DIRECTION, 0, 0, 0);

		vec3_t position = obj_get_position(source->parent);
		alSourcefv(source_object, AL_POSITION, position.vec);
	}

	// Bind audio buffer to the source and play the sound.
	// TODO: Check whether the sound should be streamed!
	alSourcei(source_object, AL_BUFFER, sound->buffer);
	alSourcePlay(source_object);

	// Remember that this audio source object is now playing and requires processing.
	source_t active = (source_t){ source, source_object, sound, next_sound_instance++, 1, 1 };
	arr_push(active_sources, active);

	return active.instance;
}

void audio_stop_source(audiosrc_t *source)
{
	if (context == NULL ||
		source == NULL) {
		return;
	}

	uint32_t i;
	arr_foreach_reverse_iter(active_sources, i) {

		if (active_sources.items[i].source != source) {
			continue;
		}

		// Move the audio source object to free sources list and remove this entry.
		audio_source_t source_object = active_sources.items[i].source_object;

		alSourceStop(source_object);
		arr_push(free_source_objects, source_object);

		arr_remove_at(active_sources, i);
	}
}

void audio_stop_sound(sound_instance_t sound)
{
	uint32_t index = audio_get_active_source_index(sound);

	// If the sound instance can be found in the active sources list, stop that particular
	// source and move it to free audio sources list.
	if (index != INVALID_INDEX) {

		audio_source_t source_object = active_sources.items[index].source_object;

		alSourceStop(source_object);
		arr_push(free_source_objects, source_object);

		arr_remove_at(active_sources, index);
	}
}

void audio_set_master_gain(float gain)
{
	if (context == NULL) {
		return;
	}

	gain = CLAMP01(gain);
	alListenerf(AL_GAIN, gain);
}

void audio_set_group_gain(uint8_t group_index, float gain)
{
	if (context == NULL || group_index >= MAX_AUDIO_GROUPS) {
		return;
	}

	gain = CLAMP01(gain);
	group_gains[group_index] = gain;
	
	// Update each active audio source belonging to this group.
	uint32_t i;
	arr_foreach_iter(active_sources, i) {

		audiosrc_t *source = active_sources.items[i].source;

		if (source->group_index == group_index) {

			float source_gain = active_sources.items[i].gain * source->gain * gain;

			alSourcef(active_sources.items[i].source_object, AL_GAIN, source_gain);
		}
	}
}

void audio_set_sound_gain(sound_instance_t sound, float gain)
{
	uint32_t index = audio_get_active_source_index(sound);

	// If the sound instance can be found in the active sources list, update the total gain of that
	// particular source.
	if (index != INVALID_INDEX) {

		audiosrc_t *source = active_sources.items[index].source;
		audio_source_t source_object = active_sources.items[index].source_object;

		active_sources.items[index].gain = gain;

		alSourcef(source_object, AL_GAIN, gain * source->gain * group_gains[source->group_index]);
	}
}

void audio_set_sound_pitch(sound_instance_t sound, float pitch)
{
	uint32_t index = audio_get_active_source_index(sound);

	// If the sound instance can be found in the active sources list, update the total pitch of that
	// particular source.
	if (index != INVALID_INDEX) {

		audiosrc_t *source = active_sources.items[index].source;
		audio_source_t source_object = active_sources.items[index].source_object;

		active_sources.items[index].pitch = pitch;

		alSourcef(source_object, AL_PITCH, pitch * source->pitch);
	}
}

object_t *audio_get_listener(void)
{
	return listener_object;
}

void audio_set_listener(object_t *object)
{
	listener_object = object;
}

audio_buffer_t audio_create_buffer(uint32_t channels, uint32_t bits_per_sample,
                                   const void *data, size_t data_size, size_t frequency)
{
	if (context == NULL) {
		return 0;
	}

	// Generate the buffer object.
	ALuint buffer;
	alGenBuffers(1, &buffer);

	ALenum error = alGetError();

	// Check that the buffer was generated successfully.
	if (error != AL_NO_ERROR) {
		return 0;
	}

	bool stereo = (channels > 1);
	ALenum format;

	switch (bits_per_sample) {

		case 16:
			format = (stereo ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16);
			break;

		case 8:
			format = (stereo ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8);
			break;

		default:
			return 0;
	}

	// Upload sound data to buffer.
	alBufferData(buffer, format, data, data_size, frequency);

	return buffer;
}

void audio_destroy_buffer(audio_buffer_t buffer)
{
	if (context == NULL) {
		return;
	}

	alDeleteBuffers(1, &buffer);
}

static uint32_t audio_get_active_source_index(sound_instance_t sound)
{
	uint32_t i;

	arr_foreach_iter(active_sources, i) {

		if (active_sources.items[i].instance == sound) {
			return i;
		}
	}

	return INVALID_INDEX;
}
