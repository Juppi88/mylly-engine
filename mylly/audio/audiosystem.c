#include "audiosystem.h"
#include "io/log.h"
#include <AL/al.h>
#include <AL/alc.h>

// -------------------------------------------------------------------------------------------------

static ALCdevice *device; // Audio device
static ALCcontext *context; // Audio context

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

	// Play the sound.
	alSourcePlay(source);
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
	if (error == AL_INVALID_VALUE ||
		error == AL_OUT_OF_MEMORY) {
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
