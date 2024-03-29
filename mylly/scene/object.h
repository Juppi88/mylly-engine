#pragma once
#ifndef __OBJECT_H
#define __OBJECT_H

// -------------------------------------------------------------------------------------------------

#include "collections/array.h"
#include "math/matrix.h"
#include "math/quaternion.h"
#include "core/defines.h"

// -------------------------------------------------------------------------------------------------

typedef struct object_t {

	struct object_t *parent; // The parent of this object

	scene_t *scene; // The scene this object is a part of
	uint32_t scene_index; // Index in the scene

	bool is_active; // Set to true when the object is processed and rendered normally
	bool destroy_immediately; // When set to true, the object is destroyed at the end of the frame

	arr_t(struct object_t*) children; // A list of children attached to this object

	vec3_t local_position; // Local position in relation to the parent
	vec3_t local_scale; // Local scale in relation to the parent
	quat_t local_rotation; // Local rotation in relation to the parent

	vec3_t position; // Object position in world space
	vec3_t scale; // Object scale in world space
	quat_t rotation; // Object rotation in world space

	vec3_t forward; // Forward vector
	vec3_t up; // Up vector
	vec3_t right; // Right vector

	mat_t transform; // Transform matrix
	mat_t local_transform; // Local transform matrix (relative to parent)

	bool is_transform_dirty; // True when the transform matrix hasn't been updated
	bool is_local_transform_dirty; // True when the local transform matrix hasn't been updated
	bool is_rotation_dirty; // True when the world rotation has not been updated

	model_t *model; // 3D render model (collection of meshes)
	sprite_t *sprite; // A single mesh for a 2D sprite
	animator_t *animator; // Animator attached to this object
	emitter_t *emitter; // Particle emitter attached to this object
	camera_t *camera; // Camera attached to the object
	light_t *light; // Light attached to the object
	ai_t *ai; // An AI attached to this object, executing a behaviour tree
	audiosrc_t *audio_source; // Audio source. Required for positional sound effects
	
} object_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

// Create a new object.
// NOTE: Please do not use this method directly, use scene_create_object() instead.
object_t *obj_create(scene_t *scene, object_t *parent);
void obj_destroy(object_t *obj);

void obj_set_parent(object_t *obj, object_t *parent);
void obj_set_active(object_t *obj, bool active);

void obj_process(object_t *obj);

camera_t *obj_add_camera(object_t *object);
light_t *obj_add_light(object_t *object);
animator_t *obj_add_animator(object_t *object);
emitter_t *obj_add_emitter(object_t *object, const emitter_t *emitter_template);
ai_t *obj_add_ai(object_t *object);
audiosrc_t *obj_add_audio_source(object_t *object);
void obj_set_model(object_t *obj, model_t *model);
void obj_set_sprite(object_t *obj, sprite_t *sprite);

static INLINE vec3_t obj_get_local_position(object_t *obj);
static INLINE vec3_t obj_get_local_scale(object_t *obj);
static INLINE quat_t obj_get_local_rotation(object_t *obj);

static INLINE vec3_t obj_get_position(object_t *obj);
static INLINE vec3_t obj_get_scale(object_t *obj);
static INLINE quat_t obj_get_rotation(object_t *obj);

static INLINE void obj_set_position(object_t *obj, const vec3_t position);

static INLINE void obj_set_local_position(object_t *obj, const vec3_t position);
static INLINE void obj_set_local_rotation(object_t *obj, const quat_t rotation);
static INLINE void obj_set_local_scale(object_t *obj, const vec3_t scale);

static INLINE vec3_t obj_get_forward_vector(object_t *obj);
static INLINE vec3_t obj_get_up_vector(object_t *obj);
static INLINE vec3_t obj_get_right_vector(object_t *obj);

void obj_look_at(object_t *obj, const vec3_t target, const vec3_t upward);

void obj_set_dirty(object_t *obj);
void obj_update_transform(object_t *obj);
void obj_update_local_transform(object_t *obj);
void obj_update_rotation(object_t *obj);

// -------------------------------------------------------------------------------------------------

static INLINE const mat_t *obj_get_transform(object_t *obj)
{
	if (obj->is_transform_dirty) {
		obj_update_transform(obj);
	}

	return &obj->transform;
}

static INLINE const mat_t *obj_get_local_transform(object_t *obj)
{
	if (obj->is_local_transform_dirty) {
		obj_update_local_transform(obj);
	}

	return &obj->local_transform;
}

static INLINE vec3_t obj_get_local_position(object_t *obj)
{
	return obj->local_position;
}

static INLINE vec3_t obj_get_local_scale(object_t *obj)
{
	return obj->local_scale;
}

static INLINE quat_t obj_get_local_rotation(object_t *obj)
{
	return obj->local_rotation;
}

static INLINE vec3_t obj_get_position(object_t *obj)
{
	if (obj->is_transform_dirty) {
		obj_update_transform(obj);
	}

	return obj->position;
}

static INLINE vec3_t obj_get_scale(object_t *obj)
{
	if (obj->is_transform_dirty) {
		obj_update_transform(obj);
	}

	return obj->scale;
}

static INLINE quat_t obj_get_rotation(object_t *obj)
{
	if (obj->is_rotation_dirty) {
		obj_update_rotation(obj);
	}

	return obj->rotation;
}

static INLINE void obj_set_position(object_t *obj, const vec3_t position)
{
	vec3_t local = position;

	if (obj->parent != NULL) {

		// Offset the position by the parent's position.
		vec3_t parent_position = obj_get_position(obj->parent);
		local = vec3_subtract(position, parent_position);
	}

	// Set the world position by updating the local position.
	obj_set_local_position(obj, local);
}

static INLINE void obj_set_local_position(object_t *obj, const vec3_t position)
{
	obj->local_position = position;
	obj_set_dirty(obj);
}

static INLINE void obj_set_local_rotation(object_t *obj, const quat_t rotation)
{
	obj->local_rotation = rotation;
	obj_set_dirty(obj);
}

static INLINE void obj_set_local_scale(object_t *obj, const vec3_t scale)
{
	obj->local_scale = scale;
	obj_set_dirty(obj);
}

static INLINE vec3_t obj_get_forward_vector(object_t *obj)
{
	if (obj->is_transform_dirty) {
		obj_update_transform(obj);
	}

	return obj->forward;
}

static INLINE vec3_t obj_get_up_vector(object_t *obj)
{
	if (obj->is_transform_dirty) {
		obj_update_transform(obj);
	}

	return obj->up;
}

static INLINE vec3_t obj_get_right_vector(object_t *obj)
{
	if (obj->is_transform_dirty) {
		obj_update_transform(obj);
	}

	return obj->right;
}

// -------------------------------------------------------------------------------------------------

END_DECLARATIONS;

#endif
