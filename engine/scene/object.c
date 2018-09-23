#include "object.h"
#include "math/math.h"
#include "camera.h"

object_t *obj_create(object_t *parent)
{
	NEW(object_t, obj);

	obj->parent = NULL;

	obj->local_position = vec3_zero;
	obj->local_scale = vec3_one;
	obj->local_rotation = quat_identity;

	obj->is_transform_dirty = true;
	obj->is_local_transform_dirty = true;
	obj->is_rotation_dirty = true;

	// Set the object's parent.
	if (parent != NULL) {
		obj_set_parent(obj, parent);
	}

	return obj;
}

void obj_destroy(object_t *obj)
{
	if (obj == NULL) {
		return;
	}

	// Detach from parent.
	if (obj->parent != NULL) {
		obj_set_parent(obj, NULL);
	}

	// TODO: Destroy all child objects.

	DELETE(obj);
}

void obj_set_parent(object_t *obj, object_t *parent)
{
	if (obj == NULL) {
		return;
	}

	// Remove this object from the old parent.
	if (obj->parent != NULL) {
		arr_remove(obj->parent->children, obj);
	}

	// Add this object to the new parent.
	if (parent != NULL) {
		arr_push(parent->children, obj);
	}

	obj->parent = parent;

	// Flag the model matrix dirty.
	obj_set_dirty(obj);
}

void obj_set_dirty(object_t *obj)
{
	if (obj == NULL) {
		return;
	}

	// Flag the object itself as dirty.
	obj->is_transform_dirty = true;
	obj->is_local_transform_dirty = true;
	obj->is_rotation_dirty = true;

	// Flag components dirty.
	if (obj->camera != NULL) {
		obj->camera->is_view_matrix_dirty = true;
	}

	// Do the same to each child object.
	object_t *child;

	arr_foreach(obj->children, child) {
		obj_set_dirty(child);
	}
}

void obj_update_transform(object_t *obj)
{
	if (obj == NULL || !obj->is_transform_dirty) {
		return;
	}

	if (obj->parent != NULL) {

		// Calculate object transform by multiplying the parent's transform and the local transform.
		mat_multiply(
			obj_get_transform(obj->parent),
			obj_get_local_transform(obj),
			&obj->transform
		);
	}
	else {

		// If the object has no parent, its transform is just the local transform.
		mat_cpy(&obj->transform, obj_get_local_transform(obj));
	}

	// Cache world space position, scale and direction vectors.
	obj->position = vector3(obj->transform[0][3], obj->transform[1][3], obj->transform[2][3]);

	obj->right = vector3(obj->transform[0][0], obj->transform[1][0], obj->transform[2][0]);
	obj->up = vector3(obj->transform[0][1], obj->transform[1][1], obj->transform[2][1]);
	obj->forward = vector3(obj->transform[0][2], obj->transform[1][2], obj->transform[2][2]);

	obj->scale = vector3(
		vec3_normalize(&obj->right),
		vec3_normalize(&obj->up),
		vec3_normalize(&obj->forward)
	);

	obj->is_transform_dirty = false;
}

void obj_update_local_transform(object_t *obj)
{
	if (obj == NULL || !obj->is_local_transform_dirty) {
		return;
	}

	float rx = obj->local_rotation.x;
	float ry = obj->local_rotation.y;
	float rz = obj->local_rotation.z;
	float rw = obj->local_rotation.w;
	float wx, wy, wz;
	float xx, yy, yz;
	float xy, xz, zz;
	float x2, y2, z2;

	x2 = rx + rx;
	y2 = ry + ry;
	z2 = rz + rz;

	xx = rx * x2;
	xy = rx * y2;
	xz = rx * z2;

	yy = ry * y2;
	yz = ry * z2;
	zz = rz * z2;

	wx = rw * x2;
	wy = rw * y2;
	wz = rw * z2;

	obj->local_transform[0][0] = obj->local_scale.x * (1.0f - (yy + zz));
	obj->local_transform[1][0] = obj->local_scale.x * (xy + wz);
	obj->local_transform[2][0] = obj->local_scale.x * (xz - wy);
	obj->local_transform[0][3] = obj->local_position.x;
	obj->local_transform[0][1] = obj->local_scale.y * (xy - wz);
	obj->local_transform[1][1] = obj->local_scale.y * (1.0f - (xx + zz));
	obj->local_transform[2][1] = obj->local_scale.y * (yz + wx);
	obj->local_transform[1][3] = obj->local_position.y;
	obj->local_transform[0][2] = obj->local_scale.z * (xz + wy);
	obj->local_transform[1][2] = obj->local_scale.z * (yz - wx);
	obj->local_transform[2][2] = obj->local_scale.z * (1.0f - (xx + yy));
	obj->local_transform[2][3] = obj->local_position.z;
	obj->local_transform[3][0] = 0.0f;
	obj->local_transform[3][1] = 0.0f;
	obj->local_transform[3][2] = 0.0f;
	obj->local_transform[3][3] = 1.0f;

	obj->is_local_transform_dirty = false;
}

void obj_update_rotation(object_t *obj)
{
	if (obj == NULL || !obj->is_rotation_dirty) {
		return;
	}

	if (obj->is_transform_dirty) {
		obj_update_transform(obj);
	}

	if (obj->parent != NULL) {
		obj->rotation.w = sqrtf(MAX(0, 1 + obj->right.x + obj->up.y + obj->forward.z)) / 2;
		obj->rotation.x = sqrtf(MAX(0, 1 + obj->right.x - obj->up.y - obj->forward.z)) / 2;
		obj->rotation.y = sqrtf(MAX(0, 1 - obj->right.x + obj->up.y - obj->forward.z)) / 2;
		obj->rotation.z = sqrtf(MAX(0, 1 - obj->right.x - obj->up.y + obj->forward.z)) / 2;
	}
	else {
		obj->rotation = obj->local_rotation;
	}

	obj->is_rotation_dirty = false;
}
