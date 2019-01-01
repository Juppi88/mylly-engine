#include "spriteanimation.h"
#include "core/memory.h"
#include "core/string.h"
#include "io/log.h"

// -------------------------------------------------------------------------------------------------

sprite_anim_t *sprite_anim_create(const char *name)
{
	// Create a new container for a sprite animation.
	NEW(sprite_anim_t, animation);

	animation->resource.name = string_duplicate(name);
	animation->resource.path = NULL; // The individual animation does not have a path

	return animation;
}

void sprite_anim_destroy(sprite_anim_t *animation)
{
	if (animation == NULL) {
		return;
	}

	DESTROY(animation->resource.name);
	DESTROY(animation->resource.path);
	DESTROY(animation->keyframes);
	DESTROY(animation);
}

bool sprite_anim_set_frames(sprite_anim_t *animation,
	                        keyframe_t *keyframes, uint32_t num_keyframes, uint32_t duration)
{
	// Animation must define at least one keyframe.
	if (animation == NULL || keyframes == NULL || num_keyframes == 0) {
		return false;
	}

	// Delete previous keyframes.
	if (animation->keyframes != NULL) {
		DESTROY(animation->keyframes);
	}

	// Create a new array for the keyframes.
	animation->keyframes = mem_alloc_fast(sizeof(keyframe_t) * num_keyframes);
	animation->num_keyframes = num_keyframes;
	animation->use_even_frame_times = true;

	uint32_t total_duration = 0;
	bool has_zero_length_keyframes = false;
	bool has_missing_sprites = false;

	for (uint32_t i = 0; i < num_keyframes; i++) {

		if (keyframes[i].duration != 0) {

			// If even one of the keyframes defines a duration, consider the entire animation to use
			// keyframe timings instead of even timings based on animation duration.
			animation->use_even_frame_times = false;
			total_duration += keyframes[i].duration;
		}
		else if (duration == 0) {

			// If the keyframe does not define a duration nor does the animation it belongs to,
			// the keyframe will have no effect because it is zero-length.
			has_zero_length_keyframes = true;
		}

		if (keyframes[i].sprite == NULL) {

			// Warn about missing sprites.
			has_missing_sprites = true;
		}

		animation->keyframes[i].sprite = keyframes[i].sprite;
		animation->keyframes[i].duration = keyframes[i].duration;
	}

	// Warn about invalid keyframes in animation.
	if (has_zero_length_keyframes) {
		
		log_message("Animation", "Sprite animation '%s' has zero-length keyframes.",
                    animation->resource.name);
	}
	if (has_missing_sprites) {
		
		log_message("Animation", "Sprite animation '%s' has missing sprites.",
                    animation->resource.name);
	}

	// Override animation duration with total keyframe duration.
	animation->duration = (total_duration != 0 ? total_duration : duration);

	return (!has_zero_length_keyframes && !has_missing_sprites);
}
