#include "sound.h"
#include "audiosystem.h"
#include "core/memory.h"
#include "core/string.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_libs/dr_wav.h"

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

    // Allocate a buffer into which to store the sample data.
    size_t sample_data_size = (size_t)wav->totalPCMFrameCount * wav->channels * sizeof(int32_t);
    int32_t *sample_data = mem_alloc_fast(sample_data_size);

    uint64_t read = drwav_read_s32(wav, wav->totalPCMFrameCount, sample_data);

    // Generate an audio buffer object and upload the data to it.
    sound->buffer = audio_create_buffer(wav->channels, wav->bitsPerSample,
                                        sample_data, sample_data_size, wav->sampleRate);

    // Destroy temporary sample data.
    mem_free(sample_data);

    drwav_close(wav);

    return (read != 0 && sound->buffer != 0);
}
