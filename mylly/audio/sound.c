#include "sound.h"
#include "audiosystem.h"
#include "core/memory.h"
#include "core/string.h"
#include "io/log.h"
#include "math/math.h"
#include <limits.h>

#define DR_WAV_IMPLEMENTATION
#include "dr_libs/dr_wav.h"

#define DR_MP3_IMPLEMENTATION
#include "dr_libs/dr_mp3.h"

// -------------------------------------------------------------------------------------------------

sound_t *sound_create(const char *name, const char *path)
{
	NEW(sound_t, sound);

	sound->resource.res_name = string_duplicate(name);
	sound->resource.name = sound->resource.res_name;

	if (path != NULL) {
		sound->resource.path = string_duplicate(path);
	}

	return sound;
}

void sound_destroy(sound_t *sound)
{
	if (sound == NULL) {
		return;
	}

	// Release the audio buffer object.
	if (sound->buffer != 0) {
		audio_destroy_buffer(sound->buffer);
	}

	DESTROY(sound->resource.res_name);
	DESTROY(sound->resource.path);
	DESTROY(sound);
}

bool sound_load_wav(sound_t *sound, void *data, size_t data_length)
{
	if (sound == NULL) {
		return false;
	}

	// Parse the wav data.
	drwav *wav = drwav_open_memory(data, data_length);

    if (wav == NULL) {
        return false;
    }

    // Ensure the sample is 16bit.
    if (wav->bitsPerSample != 16) {

    	log_error("AudioSystem", "%u bits per sample is not yet supported (%s).",
    		       wav->bitsPerSample, sound->resource.path);

    	drwav_close(wav);
    	return false;
    }

    if (wav->channels > 2) {

    	log_error("AudioSystem", "Only mono and stereo sounds are supported (%s).",
    		       wav->bitsPerSample, sound->resource.path);

    	drwav_close(wav);
    	return false;
    }

    // Allocate a buffer into which to store the sample data.
    size_t num_samples = wav->channels * wav->totalPCMFrameCount;
    size_t samples_size = num_samples * sizeof(int16_t);
    int16_t *samples = mem_alloc_fast(samples_size);

    // Read the samples from the wav.
    uint64_t read = drwav_read_s16(wav, num_samples, samples);

    // Convert stereo sounds to mono so they can be used for positional audio.
    if (wav->channels == 2) {

    	// Add the two channels together.
    	for (size_t i = 0; i < wav->totalPCMFrameCount; i++) {

    		int sample = (samples[2 * i] + samples[2 * i + 1]);
    		samples[i] = (int16_t)CLAMP(sample, SHRT_MIN, SHRT_MAX);
    	}

    	samples_size /= 2;
    }

    // Generate an audio buffer object and upload the data to it.
    sound->buffer = audio_create_buffer(1, wav->bitsPerSample,
                                        samples, samples_size, wav->sampleRate);

    // Destroy temporary sample and wav data.
    mem_free(samples);
    drwav_close(wav);

    return (read != 0 && sound->buffer != 0);
}

bool sound_load_mp3(sound_t *sound, void *data, size_t data_length)
{
	if (sound == NULL) {
		return false;
	}

	// Parse the wav data.
	drmp3 mp3;

    if (!drmp3_init_memory(&mp3, data, data_length, NULL)) {
        return false;
    }

    drmp3_uint64 num_pcm_samples = drmp3_get_pcm_frame_count(&mp3);

    if (num_pcm_samples == 0) {

    	drmp3_uninit(&mp3);
    	return false;
    }

    // Allocate a buffer into which to store the sample data.
    size_t num_samples = mp3.channels * num_pcm_samples;
    size_t samples_size = num_samples * sizeof(int16_t);
    int16_t *samples = mem_alloc_fast(samples_size);

    // Read the samples from the wav.
    uint64_t read = drmp3_read_pcm_frames_s16(&mp3, num_samples, samples);

    // Generate an audio buffer object and upload the data to it.
    sound->buffer = audio_create_buffer(mp3.channels, 16,
                                        samples, samples_size, mp3.sampleRate);

    // Destroy temporary sample and wav data.
    mem_free(samples);
    drmp3_uninit(&mp3);

    return (read != 0 && sound->buffer != 0);
}
