#include "audiosystem.h"
#include "io/log.h"
#include "scene/object.h"
#include <AL/al.h>
#include <AL/alc.h>

// -------------------------------------------------------------------------------------------------

static ALCdevice *device; // Audio device
static ALCcontext *context; // Audio context
static object_t *listener_object; // The object which is the OpenAL listener

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

	log_message("AudioSystem", "Audio system initialized.");
}

void audio_shutdown(void)
{
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
}

void audio_play_sound(sound_t *sound)
{
	if (context == NULL ||
		sound == NULL) {
		return;
	}

	// TODO: For testing purposes we're creating a temporary audio source here.
	ALuint source;
	alGenSources(1, &source);

	if (alGetError() != 0) {

		log_warning("AudioSystem", "Could not generate an audio source.");
		return;
	}

	// Bind audio buffer to the source.
	alSourcei(source, AL_BUFFER, sound->buffer);

	if (alGetError() != 0) {

		log_warning("AudioSystem", "Could not bind audio buffer to a source.");
		return;
	}

	vec3_t position = obj_get_position(listener_object);

	alSourcefv(source, AL_POSITION, position.vec);
	alSource3f(source, AL_VELOCITY, 0, 0, 0);
	alSource3f(source, AL_DIRECTION, 0, 0, 0);

	alSourcePlay(source);
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
