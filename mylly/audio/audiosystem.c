#include "audiosystem.h"
#include "audiosource.h"
#include "sound.h"
#include "audiobuffer.h"
#include "core/parallel.h"
#include "core/ref.h"
#include "io/log.h"
#include "scene/object.h"
#include "collections/array.h"
#include "math/math.h"
#include <AL/al.h>
#include <AL/alc.h>

// -------------------------------------------------------------------------------------------------

// Temporary data storage for active sounds, grouping an OpenAL audio source to Mylly audiosrc_t
// component.
typedef struct active_sound_t {

	sound_instance_t instance; // A handle to this active sound instance
	sound_t *sound; // Mylly sound resource
	audiosrc_t *source; // Mylly audio source component
	audio_source_t source_object; // OpenAL audio source object
	audiobuffer_t *buffers[2]; // Rotating audio buffers for streaming sounds
	size_t current_sample; // Index of the sample up to which the sound has been streamed
	float gain; // Per sound instance gain
	float pitch; // Per sound instance pitch

} active_sound_t;

// -------------------------------------------------------------------------------------------------

// A temporary structure for filling audio buffers in a separate worker thread.
typedef struct buffer_load_job_t {

	sound_instance_t instance; // Active audio instance handle
	audiobuffer_t *buffer; // Reference to buffer being filled
	sound_t *sound; // Stream source
	size_t start_sample; // Sample index to start buffering from

} buffer_load_job_t;

// -------------------------------------------------------------------------------------------------

static ALCdevice *device; // Audio device
static ALCcontext *context; // Audio context
static object_t *listener_object; // The object which is the OpenAL listener

static arr_t(active_sound_t) active_sounds = arr_initializer; // Active sound instances

static audiosrc_t *nonpositional_sources[MAX_AUDIO_GROUPS]; // One nonpositional source per group
static float group_gains[MAX_AUDIO_GROUPS] = { 1, 1, 1, 1 }; // Per group gains

static sound_instance_t next_sound_instance = 1; // A running counter for played sounds

// -------------------------------------------------------------------------------------------------

static uint32_t audio_get_active_source_index(sound_instance_t sound);
static void audio_stop_instance_at(uint32_t index);

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
	if (active_sounds.count != 0) {

		log_warning("AudioSystem", "Destroying %u active audio sources.", active_sounds.count);

		for (uint32_t i = 0; i < active_sounds.count; i++) {
			audio_stop_sound(active_sounds.items[i].source_object);
		}
	}

	arr_clear(active_sounds);

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

static void audio_load_buffer_in_parallel(void *context)
{
	buffer_load_job_t *job = (buffer_load_job_t *)context;

	// This is the heavy task being done in a separate worker thread.
	sound_stream(job->sound, job->buffer->buffer, job->start_sample);
}

static void audio_parallel_load_completed(void *context)
{
	buffer_load_job_t *job = (buffer_load_job_t *)context;

	// Find sound source instance.
	uint32_t index = audio_get_active_source_index(job->instance);

	// If the active audio source could not be found for some reason, it may have been removed
	// due to eg. scene object removal. If not, queue the loaded buffer to the source.
	if (index != INVALID_INDEX) {
		alSourceQueueBuffers(active_sounds.items[index].source_object, 1, &job->buffer->buffer);
	}

	// Dereference the buffer so it can be destroyed.
	ref_dec(job->buffer);

	ALint state, num_queued_buffers;
	alGetSourcei(active_sounds.items[index].source_object, AL_SOURCE_STATE, &state);
	alGetSourcei(active_sounds.items[index].source_object, AL_BUFFERS_QUEUED, &num_queued_buffers);

	// Restart the sound in case it has ended prematurely (i.e. not buffering fast enough).
	// However, restart only when both buffers have been re-queued and when the end of the sound
	// has not been reached.
	if (state == AL_STOPPED &&
		num_queued_buffers > 1 &&
		active_sounds.items[index].current_sample != 0) {

		alSourcePlay(active_sounds.items[index].source_object);
	}

	// Remember to remove the temporary data of the job.
	mem_free(job);
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
	arr_foreach_reverse_iter(active_sounds, i) {

		// Get the state of the audio source object.
		active_sound_t *active = &active_sounds.items[i];
		audio_source_t source_object = active->source_object;
		audiosrc_t *source = active->source;
		
		ALint state;
		alGetSourcei(source_object, AL_SOURCE_STATE, &state);

		// If the source is no longer playing anything, move it to free source objects list.
		if (state != AL_PLAYING && active->current_sample == 0) {
			audio_stop_instance_at(i);
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

			sound_t *sound = active->sound;

			// Process audio streaming.
			if (sound->is_streaming) {

				// Get number of audio buffers that have been processed and are ready to be removed
				// from the queue and refilled.
				ALint num_processed_buffers = 0;
				ALint num_queued_buffers = 0;

				alGetSourcei(source_object, AL_BUFFERS_PROCESSED, &num_processed_buffers);
				alGetSourcei(source_object, AL_BUFFERS_QUEUED, &num_queued_buffers);

				// Ensure the source has at least one buffer in the queue so we don't end up in a
				// situation where the source stops while both of the buffers are being filled.
				if (num_processed_buffers != 0 &&//) {// &&
					num_queued_buffers != 0) {

					// Unqueue one processed buffer.
					audiobuffer_id_t unqueued = 0;
					alSourceUnqueueBuffers(source_object, 1, &unqueued);

					ALint err = alGetError();

					if (err != AL_NO_ERROR) {
						continue;
					}

					// There is more of the sound to play. Load new audio segments into the unqueued
					// buffer in a separate worker thread and queue it back to the audio source after
					// the loading is completed.
					if (unqueued != 0 &&
						active->current_sample != 0) {

						audiobuffer_t *buf = (
							active->buffers[0]->buffer == unqueued ?
							active->buffers[0] :
							active->buffers[1]
						);

						buffer_load_job_t *parallel_ctx = mem_alloc_fast(sizeof(*parallel_ctx));
						
						parallel_ctx->instance = active->instance;
						parallel_ctx->buffer = ref_inc(buf);
						parallel_ctx->sound = sound;
						parallel_ctx->start_sample = active->current_sample;

						parallel_submit_job(
							audio_load_buffer_in_parallel,
							audio_parallel_load_completed,
							parallel_ctx
						);

						// Move stream position forward for the next stream block. If the stream
						// end is reached, the method will return 0.
						active->current_sample =
							sound_get_next_stream_pos(sound, active->current_sample);
					}
				}
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

	// Generate a new audio source object.
	alGenSources(1, &source_object);

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

	// Remember that this audio source object is now playing and requires processing.
	active_sound_t active = (active_sound_t){
		next_sound_instance++,
		sound,
		source,
		source_object,
		{ NULL, NULL },
		0,
		1,
		1
	};

	// Bind audio buffer to the source.
	if (!sound->is_streaming) {

		// If the sound doesn't require streaming, load its entire buffer at once.
		alSourcei(source_object, AL_BUFFER, sound->buffer->buffer);
	}
	else {

		// Create two buffers into which to stream parts of the sound. These buffers are queued
		// to the source and rotated whenever their contents have been played.
		active.buffers[0] = audiobuffer_create();
		active.buffers[1] = audiobuffer_create();

		// Stream audio into the buffer objects.
		sound_stream(sound, active.buffers[0]->buffer, active.current_sample);
		active.current_sample = sound_get_next_stream_pos(sound, active.current_sample);

		sound_stream(sound, active.buffers[1]->buffer, active.current_sample);
		active.current_sample = sound_get_next_stream_pos(sound, active.current_sample);

		// Queue the audio buffers to the audio source.
		alSourceQueueBuffers(source_object, 1, &active.buffers[0]->buffer);
		alSourceQueueBuffers(source_object, 1, &active.buffers[1]->buffer);
	}

	arr_push(active_sounds, active);

	// Play the sound.
	alSourcePlay(source_object);

	return active.instance;
}

void audio_stop_source(audiosrc_t *source)
{
	if (context == NULL ||
		source == NULL) {
		return;
	}

	uint32_t i;
	arr_foreach_reverse_iter(active_sounds, i) {

		if (active_sounds.items[i].source == source) {
			audio_stop_instance_at(i);
		}
	}
}

void audio_stop_sound(sound_instance_t sound)
{
	uint32_t index = audio_get_active_source_index(sound);

	// If the sound instance can be found in the active sources list, stop that particular
	// source and move it to free audio sources list.
	if (index != INVALID_INDEX) {
		audio_stop_instance_at(index);
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
	arr_foreach_iter(active_sounds, i) {

		active_sound_t *active_sound = &active_sounds.items[i];
		audiosrc_t *source = active_sound->source;

		if (source->group_index == group_index) {

			float source_gain = active_sound->gain * source->gain * gain;

			alSourcef(active_sound->source_object, AL_GAIN, source_gain);
		}
	}
}

void audio_set_sound_gain(sound_instance_t sound, float gain)
{
	uint32_t index = audio_get_active_source_index(sound);

	// If the sound instance can be found in the active sources list, update the total gain of that
	// particular source.
	if (index != INVALID_INDEX) {

		active_sound_t *active_sound = &active_sounds.items[index];
		audiosrc_t *source = active_sound->source;
		audio_source_t source_object = active_sound->source_object;

		active_sound->gain = gain;

		alSourcef(source_object, AL_GAIN, gain * source->gain * group_gains[source->group_index]);
	}
}

void audio_set_sound_pitch(sound_instance_t sound, float pitch)
{
	uint32_t index = audio_get_active_source_index(sound);

	// If the sound instance can be found in the active sources list, update the total pitch of that
	// particular source.
	if (index != INVALID_INDEX) {

		active_sound_t *active_sound = &active_sounds.items[index];
		audiosrc_t *source = active_sound->source;
		audio_source_t source_object = active_sound->source_object;

		active_sound->pitch = pitch;

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

audiobuffer_id_t audio_create_buffer(void)
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

	return buffer;
}

void audio_load_buffer(audiobuffer_id_t buffer, uint32_t channels, uint32_t bits_per_sample,
                       const void *data, size_t data_size, size_t frequency)
{
	if (context == NULL || buffer == 0) {
		return;
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
			log_warning("AudioSystem", "Unsupported bits per sample (%u)", bits_per_sample);
			return;
	}

	// Upload sound data to buffer.
	alBufferData(buffer, format, data, data_size, frequency);
}

void audio_destroy_buffer(audiobuffer_id_t buffer)
{
	if (context == NULL || buffer == 0) {
		return;
	}

	alDeleteBuffers(1, &buffer);
}

static uint32_t audio_get_active_source_index(sound_instance_t sound)
{
	uint32_t i;

	arr_foreach_iter(active_sounds, i) {

		if (active_sounds.items[i].instance == sound) {
			return i;
		}
	}

	return INVALID_INDEX;
}

static void audio_stop_instance_at(uint32_t index)
{
	active_sound_t *source = &active_sounds.items[index];

	// Stop the sound and detach its buffer(s).
	alSourceStop(source->source_object);
	alSourcei(source->source_object, AL_BUFFER, AL_NONE);

	ref_dec_safe(source->buffers[0]);
	ref_dec_safe(source->buffers[1]);

	// Release audio source.
	alDeleteSources(1, &source->source_object);

	arr_remove_at(active_sounds, index);
}
