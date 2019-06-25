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

	// Release the decoder instance.
	if (sound->decoder != NULL) {

		drmp3_uninit((drmp3 *)sound->decoder);
		mem_free(sound->decoder);
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
    uint16_t channels = wav->channels;

    // Read the samples from the wav.
    uint64_t read = drwav_read_s16(wav, num_samples, samples);

    // Convert stereo sounds to mono so they can be used for positional audio.
    if (channels == 2) {

    	// Add the two channels together.
    	for (size_t i = 0; i < wav->totalPCMFrameCount; i++) {

    		int sample = (samples[2 * i] + samples[2 * i + 1]);
    		samples[i] = (int16_t)CLAMP(sample, SHRT_MIN, SHRT_MAX);
    	}

    	samples_size /= 2;
    	channels = 1;
    }

    // Generate an audio buffer object and upload the data to it.
    sound->buffer = audio_create_buffer();

    audio_load_buffer(sound->buffer, channels, wav->bitsPerSample,
                      samples, samples_size, wav->sampleRate);

    sound->is_streaming = false;
    sound->encoded_data = NULL;
    sound->num_samples = wav->totalPCMFrameCount;

    // Destroy temporary sample and wav data.
    mem_free(samples);
    drwav_close(wav);

    mem_free(data);

    return (read != 0 && sound->buffer != 0);
}

bool sound_load_mp3(sound_t *sound, void *data, size_t data_length)
{
	if (sound == NULL) {
		return false;
	}

	// Initialize a parser instance for the MP3 file.
	drmp3 *mp3 = mem_alloc(sizeof(*mp3));

    if (!drmp3_init_memory(mp3, data, data_length, NULL)) {

    	mem_free(mp3);
        return false;
    }

    sound->decoder = mp3;
    sound->encoded_data = data;
    sound->num_samples = drmp3_get_pcm_frame_count(mp3);
    sound->is_streaming = true;

    return (sound->num_samples != 0);
}

bool sound_stream(sound_t *sound, audio_buffer_t buffer, size_t *start_sample)
{
	if (sound == NULL || start_sample == NULL) {
		return false;
	}

	if (!sound->is_streaming || sound->decoder == NULL) {

		log_warning("AudioSystem", "Sound %s can not be streamed.", sound->resource.name);
		return false;
	}

	drmp3 *mp3 = (drmp3 *)sound->decoder;

	size_t start = *start_sample;
	size_t num_samples = SOUND_STREAM_BLOCK_SIZE;

	// Ensure we don't exceed the end of the stream.
	if (start + num_samples > sound->num_samples) {
		num_samples = sound->num_samples - start;
	}

	// Number of PCM samples to be read into the block.
	size_t num_block_samples = mp3->channels * num_samples;

	// Static stream block storage for stereo audio.
	// TODO: Figure out why the double size, and consider using allocated static buffer
	static int16_t block[2 * 2 * SOUND_STREAM_BLOCK_SIZE];

	// Seek to the requested position and read samples into the array.
	drmp3_seek_to_pcm_frame(mp3, start);
	drmp3_read_pcm_frames_s16(mp3, num_block_samples, block);

	// Upload samples to an audio buffer.
	audio_load_buffer(buffer, mp3->channels, 16, block, sizeof(int16_t) * num_block_samples, mp3->sampleRate);

	// Advance read position for next stream read. If the file end has been reached, move back to
	// the beginning.
	if (start + num_samples == sound->num_samples) {
		*start_sample = 0;
	}
	else {
		*start_sample = start + num_samples;
	}

	return true;
}
