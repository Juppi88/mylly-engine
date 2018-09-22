#include "object.h"

object_t *obj_create(object_t *parent)
{
	NEW(object_t, obj);

	obj->parent = NULL;
	obj->position = vec3_zero;
	obj->scale = vec3_one;
	obj->rotation = quat_identity;
	obj->is_transform_dirty = true;
	obj->is_local_transform_dirty = true;

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

	obj->is_transform_dirty = false;
}

void obj_update_local_transform(object_t *obj)
{
	if (obj == NULL || !obj->is_local_transform_dirty) {
		return;
	}

	float rx = obj->rotation.x;
	float ry = obj->rotation.y;
	float rz = obj->rotation.z;
	float rw = obj->rotation.w;
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

	obj->local_transform[0][0] = obj->scale.x * (1.0f - (yy + zz));
	obj->local_transform[1][0] = obj->scale.x * (xy + wz);
	obj->local_transform[2][0] = obj->scale.x * (xz - wy);
	obj->local_transform[0][3] = obj->position.x;
	obj->local_transform[0][1] = obj->scale.y * (xy - wz);
	obj->local_transform[1][1] = obj->scale.y * (1.0f - (xx + zz));
	obj->local_transform[2][1] = obj->scale.y * (yz + wx);
	obj->local_transform[1][3] = obj->position.y;
	obj->local_transform[0][2] = obj->scale.z * (xz + wy);
	obj->local_transform[1][2] = obj->scale.z * (yz - wx);
	obj->local_transform[2][2] = obj->scale.z * (1.0f - (xx + yy));
	obj->local_transform[2][3] = obj->position.z;
	obj->local_transform[3][0] = 0.0f;
	obj->local_transform[3][1] = 0.0f;
	obj->local_transform[3][2] = 0.0f;
	obj->local_transform[3][3] = 1.0f;

	obj->is_local_transform_dirty = false;
}
