#include "object.h"
#include "scene.h"
#include "camera.h"
#include "light.h"
#include "animator.h"
#include "emitter.h"
#include "ai/ai.h"
#include "io/log.h"
#include "math/math.h"
#include "audio/audiosystem.h"
#include "audio/audiosource.h"

// -------------------------------------------------------------------------------------------------

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
	obj->is_active = true;

	// Move the object to the world origin.
	obj->local_position = vec3_zero();
	obj->local_scale = vec3_one();
	obj->local_rotation = quat_identity();

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
	if (obj->light != NULL) {
		light_destroy(obj->light);
	}
	if (obj->animator != NULL) {
		animator_destroy(obj->animator);
	}
	if (obj->emitter != NULL) {
		emitter_destroy(obj->emitter);
	}
	if (obj->ai != NULL) {
		ai_destroy(obj->ai);
	}
	if (obj->audio_source != NULL) {
		audiosrc_destroy(obj->audio_source);
	}

	arr_clear(obj->children);

	// If the object is the current audio listener, set listener to nothing.
	if (audio_get_listener() == obj) {
		audio_set_listener(NULL);
	}

	DESTROY(obj);
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

void obj_set_active(object_t *obj, bool active)
{
	if (obj == NULL) {
		return;
	}

	obj->is_active = active;
}

void obj_process(object_t *obj)
{
	if (obj == NULL || !obj->is_active) {
		return;
	}

	// Process animator if the object has one.
	if (obj->animator != NULL) {
		animator_process(obj->animator);
	}

	// Process particle emitter.
	if (obj->emitter != NULL) {
		emitter_process(obj->emitter);
	}

	// Process AI and behaviour tree.
	if (obj->ai != NULL) {
		ai_process(obj->ai);
	}

	// The object should be destroyed as soon as possible.
	if (obj->destroy_immediately) {
		obj_destroy(obj);
	}
}

camera_t *obj_add_camera(object_t *obj)
{
	if (obj == NULL) {
		return NULL;
	}

	// Allow only one camera per object.
	if (obj->camera != NULL) {

		log_warning("Scene", "Object already has a camera attached to it.");
		return obj->camera;
	}

	// Create a new camera component.
	obj->camera = camera_create(obj);

	// All active cameras must be registered to the object's parent scene in order to render
	// per-camera views.
	if (obj->scene != NULL) {
		scene_register_camera(obj->scene, obj);
	}

	return obj->camera;
}

light_t *obj_add_light(object_t *obj)
{
	if (obj == NULL) {
		return NULL;
	}

	// Allow only one light per object.
	if (obj->light != NULL) {

		log_warning("Scene", "Object already has a light attached to it.");
		return obj->light;
	}

	// Create a new light component.
	obj->light = light_create(obj);

	// All active lights must be registered to the object's parent scene in order to have them
	// affect the visible meshes in the scene.
	if (obj->scene != NULL) {
		scene_register_light(obj->scene, obj);
	}

	return obj->light;
}

animator_t *obj_add_animator(object_t *obj)
{
	if (obj == NULL) {
		return NULL;
	}

	// Allow only one animator per object.
	if (obj->animator != NULL) {

		log_warning("Scene", "Object already has an animator attached to it.");
		return obj->animator;
	}

	obj->animator = animator_create(obj);
	return obj->animator;
}

emitter_t *obj_add_emitter(object_t *obj, const emitter_t *emitter_template)
{
	if (obj == NULL) {
		return NULL;
	}

	// Allow only one particle emitter per object for now.
	if (obj->emitter != NULL) {

		log_warning("Scene", "Object already has an emitter attached to it.");
		return obj->emitter;
	}

	obj->emitter = emitter_create(obj, emitter_template, false);
	return obj->emitter;
}

ai_t *obj_add_ai(object_t *obj)
{
	if (obj == NULL) {
		return NULL;
	}

	// Allow only one AI per object.
	if (obj->ai != NULL) {

		log_warning("Scene", "Object already has an AI attached to it.");
		return obj->ai;
	}

	obj->ai = ai_create(obj);
	return obj->ai;
}

audiosrc_t *obj_add_audio_source(object_t *obj)
{
	if (obj == NULL) {
		return NULL;
	}

	// Allow only one audio source per object.
	if (obj->audio_source != NULL) {

		log_warning("Scene", "Object already has an audio source attached to it.");
		return obj->audio_source;
	}

	obj->audio_source = audiosrc_create(obj);
	return obj->audio_source;
}

void obj_set_model(object_t *obj, model_t *model)
{
	if (obj == NULL) {
		return;
	}

	// Set the model of the object. Models are shared between different scene objects.
	// TODO: Add reference counting to shared resources!
	obj->model = model;
}

void obj_set_sprite(object_t *obj, sprite_t *sprite)
{
	if (obj == NULL) {
		return;
	}

	// Set the sprite of the object. Sprites are shared between different scene objects.
	// TODO: Add reference counting to shared resources!
	obj->sprite = sprite;
}

void obj_look_at(object_t *obj, const vec3_t target, const vec3_t upward)
{
	if (obj == NULL) {
		return;
	}

	// Calculate forward vector (direction from the object to target)
	vec3_t position = obj_get_position(obj);
	vec3_t forward = vec3_subtract(target, position);
	
	// Ensure forward and up vectors are orthogonal, normalize them.
	vec3_t up = upward;
	vec3_orthonormalize(&forward, &up);

	// Calculate right vector.
	vec3_t right = vec3_cross(forward, up);

	// Create a rotation matrix from the directions.
	mat_t rotation;

	// Negate right vector for a left hand system where Y is up.
	// Don't know why this works, but it does so let's leave it here.
	mat_set(&rotation,
		-right.x, -right.y, -right.z, 0,
		up.x, up.y, up.z, 0,
		forward.x, forward.y, forward.z, 0,
		0, 0, 0, 1);

	// Convert the rotation matrix into a quaternion.
	quat_t quat = mat_to_quat(rotation);

	// Update the object's rotation and flag its transformation as dirty.
	obj->local_rotation = quat;
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
		
		obj->camera->state |= (
			CAMSTATE_VIEW_DIRTY |
			CAMSTATE_VIEWPROJ_DIRTY |
			CAMSTATE_VIEWPROJ_INV_DIRTY
		);
	}

	if (obj->light != NULL) {
		obj->light->is_dirty = true;
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
			*obj_get_transform(obj->parent),
			*obj_get_local_transform(obj),
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

	// Negate right vector for a left hand system where Y is up.
	obj->right = vector3(
			-obj->transform.col[0][0],
			-obj->transform.col[0][1],
			-obj->transform.col[0][2]
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

	mat_set(&obj->local_transform,

		obj->local_scale.x * (1.0f - (yy + zz)),
		obj->local_scale.x * (xy + wz),
		obj->local_scale.x * (xz - wy),
		0,

		obj->local_scale.y * (xy - wz),
		obj->local_scale.y * (1.0f - (xx + zz)),
		obj->local_scale.y * (yz + wx),
		0,

		obj->local_scale.z * (xz + wy),
		obj->local_scale.z * (yz - wx),
		obj->local_scale.z * (1.0f - (xx + yy)),
		0,

		obj->local_position.x,
		obj->local_position.y,
		obj->local_position.z,
		1
	);

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
