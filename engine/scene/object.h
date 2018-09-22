#pragma once
#ifndef __OBJECT_H
#define __OBJECT_H

#include "core/array.h"
#include "math/matrix.h"
#include "math/quaternion.h"
#include "renderer/model.h"

// --------------------------------------------------------------------------------

typedef struct object_t {

	struct object_t *parent; // The parent of this object
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

	model_t *model; // Render model
	
} object_t;

// --------------------------------------------------------------------------------

object_t *obj_create(object_t *parent);
void obj_destroy(object_t *obj);

void obj_set_parent(object_t *obj, object_t *parent);

static INLINE vec3_t obj_get_local_position(object_t *obj);
static INLINE vec3_t obj_get_local_scale(object_t *obj);
static INLINE quat_t obj_get_local_rotation(object_t *obj);

static INLINE vec3_t obj_get_position(object_t *obj);
static INLINE vec3_t obj_get_scale(object_t *obj);
static INLINE quat_t obj_get_rotation(object_t *obj);

static INLINE void obj_set_local_position(object_t *obj, const vec3_t position);
static INLINE void obj_set_local_rotation(object_t *obj, const quat_t rotation);
static INLINE void obj_set_local_scale(object_t *obj, const vec3_t scale);

static INLINE vec3_t obj_get_forward_vector(object_t *obj);
static INLINE vec3_t obj_get_up_vector(object_t *obj);
static INLINE vec3_t obj_get_right_vector(object_t *obj);

void obj_set_dirty(object_t *obj);
void obj_update_transform(object_t *obj);
void obj_update_local_transform(object_t *obj);
void obj_update_rotation(object_t *obj);

// --------------------------------------------------------------------------------

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

#endif
