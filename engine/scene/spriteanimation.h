#pragma once
#ifndef __SPRITEANIMATION_H
#define __SPRITEANIMATION_H

#include "core/types.h"
#include "core/defines.h"
#include "collections/array.h"
#include "resources/resource.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef struct keyframe_t {
	sprite_t *sprite; // The sprite to be displayed during this frame
	uint32_t duration; // Keyframe duration in frames. Can be left to 0 if animation has a time
} keyframe_t;

// -------------------------------------------------------------------------------------------------

typedef struct sprite_anim_t {

	resource_t resource; // Resource info

	keyframe_t *keyframes; // An array of keyframes
	uint32_t num_keyframes; // Number of keyframes in this animation
	uint32_t duration; // Animation duration in frames
	bool use_even_frame_times; // Use even frame times (none of the keyframes define durations)

} sprite_anim_t;

// -------------------------------------------------------------------------------------------------

sprite_anim_t *sprite_anim_create(const char *name);
void sprite_anim_destroy(sprite_anim_t *animation);

bool sprite_anim_set_frames(sprite_anim_t *animation,
	                        keyframe_t *keyframes, uint32_t num_keyframes, uint32_t duration);

END_DECLARATIONS;

#endif
