#include "object.h"
#include "camera.h"
#include "scene.h"
#include "io/log.h"
#include "math/math.h"
#include "renderer/model.h"

// --------------------------------------------------------------------------------

object_t *obj_create(scene_t *scene, object_t *parent)
{
	// Make sure the object is created to a scene to avoid leaking objects.
	if (scene == NULL) {
		log_error("Scene", "Can't create an object without a scene.");
		return NULL;
	}

	// Create the object.
	NEW(object_t, obj);

	obj->parent = NULL;
	obj->scene = scene;
	obj->scene_index = INVALID_INDEX;

	// Move the object to the world origin.
	obj->local_position = vec3_zero;
	obj->local_scale = vec3_one;
	obj->local_rotation = quat_identity;

	obj->is_transform_dirty = true;
	obj->is_local_transform_dirty = true;
	obj->is_rotation_dirty = true;

	// Attach the object to a parent.
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

	// Additional cleanup when the object is still in a scene. If the object is no longer in a
	// scene, the parent scene is being deleted and no additional clean up is necessary.
	if (obj->scene != NULL) {

		// Remove object references in the scene.
		scene_remove_references_to_object(obj->scene, obj);

		// Destroy all child objects.
		object_t *child;

		arr_foreach_reverse(obj->children, child) {
			obj_destroy(child);
		}
	}

	// Destroy components.
	if (obj->camera != NULL) {
		camera_destroy(obj->camera);
	}

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

		if (parent->scene == obj->scene) {

			arr_push(parent->children, obj);
			obj->parent = parent;
		}
		else {
			log_error("Scene", "Parent object is in a different scene.");
		}
	}
	else {
		obj->parent = NULL;
	}

	// Flag the model matrix dirty.
	obj_set_dirty(obj);
}

camera_t *obj_add_camera(object_t *obj)
{
	// One camera per object.
	if (obj == NULL || obj->camera != NULL) {
		return NULL;
	}

	obj->camera = camera_create(obj);

	// Register the new camera to the object's parent scene.
	if (obj->scene != NULL) {
		scene_register_camera(obj->scene, obj);
	}

	return obj->camera;
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
			obj_get_local_transform(obj),
			obj_get_transform(obj->parent),
			&obj->transform
		);
	}
	else {

		// If the object has no parent, its transform is just the local transform.
		mat_cpy(&obj->transform, obj_get_local_transform(obj));
	}

	// Cache world space position, scale and direction vectors.
	obj->position = vector3(
			obj->transform.col[3][0],
			obj->transform.col[3][1],
			obj->transform.col[3][2]
		);

	obj->right = vector3(
			obj->transform.col[0][0],
			obj->transform.col[0][1],
			obj->transform.col[0][2]
		);

	obj->up = vector3(
			obj->transform.col[1][0],
			obj->transform.col[1][1],
			obj->transform.col[1][2]
		);

	obj->forward = vector3(
			obj->transform.col[2][0],
			obj->transform.col[2][1],
			obj->transform.col[2][2]
		);

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

	obj->local_transform.col[0][0] = obj->local_scale.x * (1.0f - (yy + zz));
	obj->local_transform.col[0][1] = obj->local_scale.x * (xy + wz);
	obj->local_transform.col[0][2] = obj->local_scale.x * (xz - wy);
	
	obj->local_transform.col[1][0] = obj->local_scale.y * (xy - wz);
	obj->local_transform.col[1][1] = obj->local_scale.y * (1.0f - (xx + zz));
	obj->local_transform.col[1][2] = obj->local_scale.y * (yz + wx);

	obj->local_transform.col[2][0] = obj->local_scale.z * (xz + wy);
	obj->local_transform.col[2][1] = obj->local_scale.z * (yz - wx);
	obj->local_transform.col[2][2] = obj->local_scale.z * (1.0f - (xx + yy));

	obj->local_transform.col[3][0] = obj->local_position.x;
	obj->local_transform.col[3][1] = obj->local_position.y;
	obj->local_transform.col[3][2] = obj->local_position.z;

	obj->local_transform.col[0][3] = 0.0f;
	obj->local_transform.col[1][3] = 0.0f;
	obj->local_transform.col[2][3] = 0.0f;
	obj->local_transform.col[3][3] = 1.0f;

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
