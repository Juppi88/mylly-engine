#include "animator.h"
#include "spriteanimation.h"
#include "object.h"
#include "core/memory.h"
#include "core/time.h"
#include "resources/resources.h"

// -------------------------------------------------------------------------------------------------

animator_t *animator_create(object_t *parent)
{
	NEW(animator_t, animator);

	animator->parent = parent;
	animator->frame_time = 1.0f / 60.0f;

	return animator;
}

void animator_destroy(animator_t *animator)
{
	if (animator == NULL) {
		return;
	}

	DELETE(animator);
}

void animator_process(animator_t *animator)
{
	if (animator == NULL) {
		return;
	}

	// If there is no animation playing right now, there's nothing to process.
	if (animator->animation == NULL) {
		return;
	}

	// Advance the animation.
	animator->frame_elapsed += get_time().delta_time;

	if (animator->frame_elapsed < animator->frame_duration) {
		return;
	}

	// Reset current animation time.
	animator->frame_elapsed = 0;

	// Animation frame is changing.
	sprite_anim_t *animation = animator->animation;

	if (++animator->current_frame >= animator->animation->num_keyframes) {

		// Go back to the first frame regardless of whether this animation loops or not.
		animator->current_frame = 0;

		if (!animator->is_looping) {

			// If the animation does not loop, start the next animation (assuming it is already set)
			if (animator->next_animation == NULL) {

				animator->animation = NULL;
				animator->is_looping = false;

				return;
			}
			
			animator->animation = animator->next_animation;
			animator->is_looping = animator->is_next_animation_looping;
		}
	}

	// Calculate the duration of the new keyframe in seconds.
	int frame = animator->current_frame;

	if (animator->animation->keyframes[frame].duration != 0) {
		animator->frame_duration = animator->frame_time * animation->keyframes[frame].duration;
	}
	else {
		animator->frame_duration = (animator->frame_time * animation->duration) /
                                    animation->num_keyframes;	
	}

	// Update the current sprite.
	obj_set_sprite(animator->parent, animator->animation->keyframes[frame].sprite);
}

void animator_set_frame_time(animator_t *animator, float time)
{
	if (animator == NULL) {
		return;
	}

	animator->frame_time = time;
}

void animator_play(animator_t *animator, const char *animation_name, bool loop)
{
	if (animator == NULL || animation_name == NULL) {
		return;
	}

	// Find the sprite animation that's going to be played.
	sprite_anim_t *animation = res_get_sprite_anim(animation_name);

	if (animation == NULL) {
		return;
	}

	// Set the current animation and start it.
	if (animation != NULL) {
		
		animator->animation = animation;
		animator->is_looping = loop;
		animator->current_frame = 0;
		animator->frame_elapsed = 0;

		// Calculate the duration of the keyframe in seconds.
		if (animation->keyframes[0].duration != 0) {
			animator->frame_duration = animator->frame_time * animation->keyframes[0].duration;
		}
		else {
			animator->frame_duration = (animator->frame_time * animation->duration) /
                                        animation->num_keyframes;	
		}

		// Change the current sprite.
		obj_set_sprite(animator->parent, animation->keyframes[0].sprite);
	}

	// Reset next animation.
	animator->next_animation = NULL;
}

void animator_set_next_animation(animator_t *animator, const char *animation_name, bool loop)
{
	if (animator == NULL || animation_name == NULL) {
		return;
	}

	// Start the animation directly if there is no animation playing right now.
	if (animator->animation == NULL) {

		animator_play(animator, animation_name, loop);
		return;
	}

	// Find the sprite animation that's going to be played.
	sprite_anim_t *animation = res_get_sprite_anim(animation_name);

	if (animation != NULL) {
		
		animator->next_animation = animation;
		animator->is_next_animation_looping = loop;
	}
}
