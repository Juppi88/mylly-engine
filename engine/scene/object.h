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

	vec3_t position; // Local position in relation to the parent
	vec3_t scale; // Local scale in relation to the parent
	quat_t rotation; // Local rotation in relation to the parent

	mat_t model_matrix; // Model matrix of this object
	bool is_model_matrix_dirty; // True when the model matrix hasn't been updated
	
} object_t;

// --------------------------------------------------------------------------------

object_t *obj_create(object_t *parent);
void obj_destroy(object_t *obj);

void obj_set_parent(object_t *obj, object_t *parent);

void obj_set_dirty(object_t *obj);
void obj_update_model_matrix(object_t *obj);

// --------------------------------------------------------------------------------

static INLINE const mat_t *obj_get_model_matrix(object_t *obj)
{
	if (obj->is_model_matrix_dirty) {
		obj_update_model_matrix(obj);
	}

	return &obj->model_matrix;
}

static INLINE void obj_set_local_position(object_t *obj, const vec3_t *position)
{
	obj->position = *position;
	obj_set_dirty(obj);
}

static INLINE void obj_set_local_rotation(object_t *obj, const quat_t *rotation)
{
	obj->rotation = *rotation;
	obj_set_dirty(obj);
}

static INLINE void obj_set_local_scale(object_t *obj, const vec3_t *scale)
{
	obj->scale = *scale;
	obj_set_dirty(obj);
}

#endif
