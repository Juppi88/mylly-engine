#pragma once
#ifndef __ANIMATOR_H
#define __ANIMATOR_H

#include "core/defines.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef struct animator_t {

	object_t *parent; // The object this animator is attached to

	float frame_time; // Animation frame time. Default is 1/60

	uint32_t current_frame; // Current keyframe index
	float frame_elapsed; // Time in seconds how long this frame has played
	float frame_duration; // Duration of the current frame in seconds

	sprite_anim_t *animation; // Currently played animation
	bool is_looping; // Set to true when the current animation is looping

	sprite_anim_t *next_animation; // Animation which will be played next
	bool is_next_animation_looping; // Set to true when the next animation will loop

} animator_t;

// -------------------------------------------------------------------------------------------------

animator_t *animator_create(object_t *parent);
void animator_destroy(animator_t *animator);

void animator_process(animator_t *animator);

void animator_set_frame_time(animator_t *animator, float time);

void animator_play(animator_t *animator, const char *animation, bool loop);
void animator_set_next_animation(animator_t *animator, const char *animation, bool loop);

// -------------------------------------------------------------------------------------------------

END_DECLARATIONS;

#endif
