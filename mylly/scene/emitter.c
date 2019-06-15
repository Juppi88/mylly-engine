#include "emitter.h"
#include "object.h"
#include "sprite.h"
#include "scene.h"
#include "camera.h"
#include "core/memory.h"
#include "core/string.h"
#include "core/time.h"
#include "renderer/mesh.h"
#include "resources/resources.h"
#include "math/math.h"
#include "math/random.h"
#include "io/log.h"
#include <stdlib.h>

// -------------------------------------------------------------------------------------------------

static void emitter_initialize_particles(emitter_t *emitter);
static void emitter_create_mesh(emitter_t *emitter);
static void emitter_emit(emitter_t *emitter, uint16_t count);
static void emitter_randomize_position_direction(emitter_t *emitter, vec3_t *pos, vec3_t *vel);
static inline void emitter_update_particle(emitter_t *emitter, particle_t *particle);
static int emitter_sort_particles(const void *p1, const void *p2);

// -------------------------------------------------------------------------------------------------

emitter_t *emitter_create(object_t *parent, const emitter_t *emitter_template, bool is_subemitter)
{
	NEW(emitter_t, emitter);

	emitter->parent = parent;

	arr_init(emitter->subemitters);

	if (emitter_template == NULL) {

		// Empty emitter.
		emitter->shape = shape_circle(vec3_zero(), 0);
		emitter->life.min = 1;
		emitter->life.max = 1;
		emitter->start_speed.min = 0;
		emitter->start_speed.max = 0;
		emitter->end_speed.min = 0;
		emitter->end_speed.max = 0;
		emitter->start_colour.min = COL_WHITE;
		emitter->start_colour.max = COL_WHITE;
		emitter->end_colour.min = COL_WHITE;
		emitter->end_colour.max = COL_WHITE;
		emitter->start_size.min = 1;
		emitter->start_size.max = 1;
		emitter->end_size.min = 1;
		emitter->end_size.max = 1;
	}
	else {

		// Copy emitter values from a template.
		emitter->is_world_space = emitter_template->is_world_space;
		emitter->sprite = emitter_template->sprite;

		emitter->max_particles = emitter_template->max_particles;
		emitter->emit_duration = emitter_template->emit_duration;
		emitter->emit_rate = emitter_template->emit_rate;
		emitter->initial_burst = emitter_template->initial_burst;

		emitter->shape = emitter_template->shape;

		emitter->life = emitter_template->life;
		emitter->start_speed = emitter_template->start_speed;
		emitter->end_speed = emitter_template->end_speed;
		emitter->acceleration = emitter_template->acceleration;
		emitter->start_colour = emitter_template->start_colour;
		emitter->end_colour = emitter_template->end_colour;
		emitter->start_size = emitter_template->start_size;
		emitter->end_size = emitter_template->end_size;
		emitter->rotation_speed = emitter_template->rotation_speed;

		emitter->resource.res_name = string_duplicate(emitter_template->resource.name);
		emitter->resource.name = emitter->resource.res_name;

		if (emitter_template->resource.path != NULL) {
			emitter->resource.path = string_duplicate(emitter_template->resource.path);
		}

		// Copy subemitters.
		if (!is_subemitter) {

			subemitter_t subemitter;

			arr_foreach(emitter_template->subemitters, subemitter) {

				emitter_t *subemitter_effect = emitter_create(parent, subemitter.emitter, true);
				subemitter_effect->emit_on_request = (subemitter.type != SUBEMITTER_CREATE);

				arr_push(
					emitter->subemitters, create_subemitter(subemitter.type, subemitter_effect);
				);
			}
		}
	}

	return emitter;
}

emitter_t *emitter_create_from_resource(const char *name, const char *path)
{
	emitter_t *emitter = emitter_create(NULL, NULL, false);

	if (emitter != NULL) {

		emitter->resource.res_name = string_duplicate(name);
		emitter->resource.name = emitter->resource.res_name;

		if (path != NULL) {
			emitter->resource.path = string_duplicate(path);
		}
	}

	return emitter;
}

void emitter_destroy(emitter_t *emitter)
{
	if (emitter == NULL) {
		return;
	}

	// Destroy subemitters.
	subemitter_t subemitter;

	arr_foreach_reverse(emitter->subemitters, subemitter) {
		emitter_destroy(subemitter.emitter);
	}

	arr_clear(emitter->subemitters);

	// Destroy particle mesh.
	if (emitter->mesh != NULL) {
		mesh_destroy(emitter->mesh);
	}

	// Destroy everything else.
	DESTROY(emitter->resource.res_name);
	DESTROY(emitter->resource.path);
	DESTROY(emitter->particles);
	DESTROY(emitter);
}

void emitter_process(emitter_t *emitter)
{
	if (emitter == NULL) {
		return;
	}

	// Process subemitters which are active.
	for (uint32_t i = 0; i < emitter->subemitters.count; i++) {

		emitter_t *subemitter = emitter->subemitters.items[i].emitter;

		if (subemitter->is_active) {
			emitter_process(subemitter);
		}
	}

	if (!emitter->is_active) {
		return;
	}

	// Calculate camera position in relation to the emitter. This is used for depth sorting.
	// TODO: We're using the main camera to sort particles by depth. If there are multiple cameras
	// being used, this should be done per camera.
	vec3_t camera_pos = vec3_zero();
	scene_t *scene = emitter->parent->scene;

	if (scene != NULL) {
		camera_t *camera = scene_get_main_camera(scene);

		if (camera != NULL) {

			camera_pos = obj_get_position(camera->parent);
			camera_pos = vec3_subtract(camera_pos, obj_get_position(emitter->parent));

			// Take into account the rotation of the particle emitter by rotating the camera
			// direction vector with the inverse of the object's rotation.
			quat_t rotation = obj_get_rotation(emitter->parent);
			rotation = quat_inverse(rotation);

			camera_pos = quat_rotate_vec3(rotation, camera_pos);
		}
	}

	emitter->camera_position = camera_pos;
	emitter->world_position = obj_get_position(emitter->parent);

	// If the emitter is actively emitting particles, create new ones by activating old particles.
	if (emitter->is_emitting) {

		float delta = get_time().delta_time;

		emitter->time_emitting += delta;
		emitter->time_since_emit += delta;

		// Calculate how many particles should be emitted to satisfy the emit rate.
		uint16_t particles_to_emit = 0;

		while (emitter->time_since_emit >= (1 / emitter->emit_rate)) {

			emitter->time_since_emit -= (1 / emitter->emit_rate);
			particles_to_emit++;
		}

		// Emit the particles.
		if (particles_to_emit != 0) {
			emitter_emit(emitter, particles_to_emit);
		}

		// Stop the emitting if a total emit duration has been set.
		if (emitter->emit_duration != 0 &&
			emitter->time_emitting >= emitter->emit_duration) {

			emitter->is_emitting = false;
		}
	}

	// Update all active particles.
	bool has_active_particles = false;

	for (int i = 0; i < emitter->max_particles; i++) {

		if (emitter->particles[i].is_active) {

			emitter_update_particle(emitter, &emitter->particles[i]);
			has_active_particles = true;
		}
	}

	if (!has_active_particles) {

		// Stop the particle system.
		emitter->is_active = false;
	}

	// Sort the particles by distance to camera. Sort the reference array for faster sorting.
	qsort(emitter->particle_references, emitter->max_particles, sizeof(particle_t*),
	      emitter_sort_particles);

	// Update the index buffer with the sorted particle indices.
	size_t num_active_particles = 0;

	for (int i = 0; i < emitter->max_particles; i++) {

		particle_t *particle = emitter->particle_references[i];

		// Skip inactive particles.
		if (!particle->is_active) {
			continue;
		}

		int base = particle->base_index;

		emitter->mesh->indices[0 + 6 * i] = 0 + 4 * base;
		emitter->mesh->indices[1 + 6 * i] = 1 + 4 * base;
		emitter->mesh->indices[2 + 6 * i] = 2 + 4 * base;
		emitter->mesh->indices[3 + 6 * i] = 2 + 4 * base;
		emitter->mesh->indices[4 + 6 * i] = 1 + 4 * base;
		emitter->mesh->indices[5 + 6 * i] = 3 + 4 * base;

		num_active_particles++;
	}

	mesh_refresh_indices(emitter->mesh);
	emitter->mesh->num_indices_to_render = 6 * num_active_particles; // TODO: Create a setter

	// TODO: We could possibly only refresh the range of particles which were updated?
	mesh_refresh_vertices(emitter->mesh);
}

void emitter_start(emitter_t *emitter)
{
	if (emitter == NULL) {
		return;
	}

	if (emitter->sprite == NULL) {

		log_message("Emitter", "Particle system has no sprite selected.");
		return;		
	}

	if (emitter->max_particles == 0) {

		log_message("Emitter", "Particle system has a maximum particle count of 0.");
		return;
	}

	emitter->is_active = true;
	emitter->is_emitting = (emitter->emit_rate > 0 && !emitter->emit_on_request);
	emitter->time_emitting = 0;
	emitter->time_since_emit = 0;
	emitter->world_position = obj_get_position(emitter->parent);

	emitter->apply_acceleration =
		(!vec3_is_zero(emitter->acceleration.min) && !vec3_is_zero(emitter->acceleration.max));

	// Initialize particle data.
	emitter_initialize_particles(emitter);

	// Create a mesh for drawing the particles.
	emitter_create_mesh(emitter);

	// Emit the initial burst of particles.
	if (!emitter->emit_on_request) {
		emitter_emit(emitter, emitter->initial_burst);
	}

	// Start subemitters.
	for (uint32_t i = 0; i < emitter->subemitters.count; i++) {
		emitter_start(emitter->subemitters.items[i].emitter);
	}
}

void emitter_stop(emitter_t *emitter)
{
	if (emitter == NULL) {
		return;
	}

	emitter->is_emitting = false;
}

void emitter_set_emit_shape(emitter_t *emitter, const emit_shape_t shape)
{
	if (emitter != NULL) {
		emitter->shape = shape;
	}
}

void emitter_set_world_space(emitter_t *emitter, bool is_world_space)
{
	if (emitter != NULL) {
		emitter->is_world_space = is_world_space;
	}
}

void emitter_set_max_particles(emitter_t *emitter, int num_particles)
{
	if (emitter == NULL) {
		return;
	}

	if (emitter->mesh != NULL) {

		// Don't allow changing the maximum number of particles for a particle system which already
		// has a particle mesh. Use emitter_start() to restart the system instead.
		log_warning("Emitter", "Cannot set max particles when the particle system is active.");
		return;
	}

	emitter->max_particles = (uint16_t)CLAMP(num_particles, 0, 65535);
}

void emitter_set_initial_burst(emitter_t *emitter, int num_particles)
{
	if (emitter != NULL) {
		emitter->initial_burst = (uint16_t)CLAMP(num_particles, 0, 65535);
	}
}

void emitter_set_emit_duration(emitter_t *emitter, float duration)
{
	if (emitter != NULL) {
		emitter->emit_duration = MAX(0, duration);
	}
}

void emitter_set_emit_rate(emitter_t *emitter, float particles_per_sec)
{
	if (emitter != NULL) {
		emitter->emit_rate = MAX(0, particles_per_sec);
	}
}

void emitter_set_particle_sprite(emitter_t *emitter, sprite_t *sprite)
{
	if (emitter != NULL && sprite != NULL) {
		emitter->sprite = sprite;
	}
}

void emitter_set_particle_life_time(emitter_t *emitter, float min, float max)
{
	if (emitter != NULL) {
		emitter->life.min = min;
		emitter->life.max = max;
	}
}

void emitter_set_particle_start_speed(emitter_t *emitter, float min, float max)
{
	if (emitter != NULL) {
		emitter->start_speed.min = min;
		emitter->start_speed.max = max;
	}
}

void emitter_set_particle_end_speed(emitter_t *emitter, float min, float max)
{
	if (emitter != NULL) {
		emitter->end_speed.min = min;
		emitter->end_speed.max = max;
	}
}

void emitter_set_particle_acceleration(emitter_t *emitter, vec3_t min, vec3_t max)
{
	if (emitter != NULL) {
		emitter->acceleration.min = min;
		emitter->acceleration.max = max;
	}
}

void emitter_set_particle_start_colour(emitter_t *emitter, colour_t min, colour_t max)
{
	if (emitter != NULL) {
		emitter->start_colour.min = min;
		emitter->start_colour.max = max;
	}
}
void emitter_set_particle_end_colour(emitter_t *emitter, colour_t min, colour_t max)
{
	if (emitter != NULL) {
		emitter->end_colour.min = min;
		emitter->end_colour.max = max;
	}
}

void emitter_set_particle_start_size(emitter_t *emitter, float min, float max)
{
	if (emitter != NULL) {
		emitter->start_size.min = min;
		emitter->start_size.max = max;
	}
}

void emitter_set_particle_end_size(emitter_t *emitter, float min, float max)
{
	if (emitter != NULL) {
		emitter->end_size.min = min;
		emitter->end_size.max = max;
	}
}

void emitter_set_particle_rotation_speed(emitter_t *emitter, float min, float max)
{
	if (emitter != NULL) {
		emitter->rotation_speed.min = min;
		emitter->rotation_speed.max = max;
	}
}

static void emitter_initialize_particles(emitter_t *emitter)
{
	if (emitter == NULL) {
		return;
	}

	if (emitter->particles != NULL) {
		DESTROY(emitter->particles);
	}

	// Create a storage for all the particles of the system.
	emitter->particles = mem_alloc_fast(emitter->max_particles * sizeof(particle_t));
	emitter->particle_references = mem_alloc_fast(emitter->max_particles * sizeof(particle_t*));

	// Setup the particles as non-active.
	for (int i = 0; i < emitter->max_particles; i++) {

		emitter->particles[i].is_active = false;
		emitter->particles[i].position = vec3_zero();
		emitter->particles[i].start_colour = COL_TRANSPARENT;
		emitter->particles[i].end_colour = COL_TRANSPARENT;

		emitter->particle_references[i] = &emitter->particles[i];
	}
}

static void emitter_create_mesh(emitter_t *emitter)
{
	if (emitter == NULL || emitter->particles == NULL) {
		return;
	}

	if (emitter->mesh != NULL) {

		mesh_destroy(emitter->mesh);
		//emitter->mesh = NULL;
	}

	// Create the vertices and indices for the mesh.
	vertex_particle_t *vertices = mem_alloc_fast(emitter->max_particles * 4 * sizeof(vertex_particle_t));
	vindex_t *indices = mem_alloc_fast(emitter->max_particles * 6 * sizeof(vindex_t));

	for (int i = 0; i < emitter->max_particles; i++) {

		// Setup vertices with initial data (including completely transparent colour).
		vertices[0 + 4 * i] = vertex_particle(

			vec3(-0.5f, -0.5f, 0),
			vec3_zero(),
			vec4_zero(),
			vec2(emitter->sprite->uv1.x, emitter->sprite->uv1.y),
			COL_TRANSPARENT,
			0,
			0
		);

		vertices[1 + 4 * i] = vertex_particle(

			vec3(0.5f, -0.5f, 0),
			vec3_zero(),
			vec4_zero(),
			vec2(emitter->sprite->uv2.x, emitter->sprite->uv1.y),
			COL_TRANSPARENT,
			0,
			0
		);

		vertices[2 + 4 * i] = vertex_particle(

			vec3(-0.5f, 0.5f, 0),
			vec3_zero(),
			vec4_zero(),
			vec2(emitter->sprite->uv1.x, emitter->sprite->uv2.y),
			COL_TRANSPARENT,
			0,
			0
		);

		vertices[3 + 4 * i] = vertex_particle(

			vec3(0.5f, 0.5f, 0),
			vec3_zero(),
			vec4_zero(),
			vec2(emitter->sprite->uv2.x, emitter->sprite->uv2.y),
			COL_TRANSPARENT,
			0,
			0
		);

		indices[0 + 6 * i] = 0 + 4 * i;
		indices[1 + 6 * i] = 1 + 4 * i;
		indices[2 + 6 * i] = 2 + 4 * i;
		indices[3 + 6 * i] = 2 + 4 * i;
		indices[4 + 6 * i] = 1 + 4 * i;
		indices[5 + 6 * i] = 3 + 4 * i;
	}

	// Create a mesh with the generated vertex data.
	emitter->mesh = mesh_create();

	mesh_set_particle_vertices(emitter->mesh, vertices, 4 * emitter->max_particles);
	mesh_set_indices(emitter->mesh, indices, 6 * emitter->max_particles);

	mesh_set_shader(emitter->mesh, res_get_shader("default-particle"));
	mesh_set_texture(emitter->mesh, emitter->sprite->texture);

	// Copy vertex references to each particle.
	// This way updating the vertices directly will be faster and easier.
	for (int i = 0; i < emitter->max_particles; i++) {

		emitter->particles[i].vertices[0] = &emitter->mesh->part_vertices[0 + 4 * i];
		emitter->particles[i].vertices[1] = &emitter->mesh->part_vertices[1 + 4 * i];
		emitter->particles[i].vertices[2] = &emitter->mesh->part_vertices[2 + 4 * i];
		emitter->particles[i].vertices[3] = &emitter->mesh->part_vertices[3 + 4 * i];

		emitter->particles[i].base_index = i;
	}

	// Delete temporary data.
	DESTROY(vertices);
	DESTROY(indices);
}

static void emitter_emit(emitter_t *emitter, uint16_t count)
{
	if (emitter == NULL || emitter->particles == NULL || count == 0) {
		return;
	}

	uint16_t emitted = 0;

	// Loop through the particle list finding all non-active particles. When a non-active particle
	// is found, activate and emit it.
	// TODO: This could be optimized somehow by keeping track of last emitted particle index,
	// in order to avoid having to start looping from the start every time.
	for (int i = 0; i < emitter->max_particles && emitted < count; i++) {

		if (!emitter->particles[i].is_active) {

			// Mark the particle as active (i.e. emit it)
			emitter->particles[i].is_active = true;
			emitter->particles[i].time_alive = 0;
			emitter->particles[i].emit_position = emitter->world_position;

			// Randomize particle details.
			emitter_randomize_position_direction(
				emitter,
				&emitter->particles[i].position,
				&emitter->particles[i].direction
			);

			emitter->particles[i].life =
				randomf(emitter->life.min, emitter->life.max);

			emitter->particles[i].acceleration =
				randomv(emitter->acceleration.min, emitter->acceleration.max);

			emitter->particles[i].start_colour =
				randomc(emitter->start_colour.min, emitter->start_colour.max);

			emitter->particles[i].end_colour =
				randomc(emitter->end_colour.min, emitter->end_colour.max);

			emitter->particles[i].start_size =
				randomf(emitter->start_size.min, emitter->start_size.max);
			
			emitter->particles[i].end_size =
				randomf(emitter->end_size.min, emitter->end_size.max);

			emitter->particles[i].start_speed =
				randomf(emitter->start_speed.min, emitter->start_speed.max);

			emitter->particles[i].end_speed =
				randomf(emitter->end_speed.min, emitter->end_speed.max);

			emitter->particles[i].velocity =
				vec3_multiply(emitter->particles[i].direction, emitter->particles[i].start_speed);

			emitter->particles[i].rotation =
				randomf(0, 2 * PI);

			emitter->particles[i].rotation_speed =
				randomf(emitter->rotation_speed.min, emitter->rotation_speed.max);

			emitted++;
		}
	}
}

static void emitter_randomize_position_direction(emitter_t *emitter, vec3_t *pos, vec3_t *dir)
{
	vec3_t position;
	float radius;

	switch (emitter->shape.type) {

		case SHAPE_CIRCLE:
			position = random_point_on_circle(); // Randomize a point on a unit circle
			radius = randomf(0, emitter->shape.circle.radius); // Randomize a radius

			*pos = vector3(
				emitter->shape.position.x + radius * position.x,
				emitter->shape.position.y + radius * position.y,
				emitter->shape.position.z
			);

			*dir = position;
			break;

		case SHAPE_SPHERE:
			position = random_point_on_shpere(); // Randomize a point on a unit sphere
			radius = randomf(0, emitter->shape.sphere.radius); // Randomize a radius

			*pos = vector3(
				emitter->shape.position.x + radius * position.x,
				emitter->shape.position.y + radius * position.y,
				emitter->shape.position.z + radius * position.z
			);

			*dir = position;
			break;

		case SHAPE_BOX:
			*pos = vector3(
				emitter->shape.position.x + emitter->shape.box.extents.x * randomf(-1.0f, 1.0f),
				emitter->shape.position.y + emitter->shape.box.extents.y * randomf(-1.0f, 1.0f),
				emitter->shape.position.z + emitter->shape.box.extents.z * randomf(-1.0f, 1.0f)
			);

			// Box shaped emitter always emits forward.
			*dir = obj_get_forward_vector(emitter->parent);
			break;

		case SHAPE_CONE:
			position = random_point_on_cone(DEG_TO_RAD(emitter->shape.cone.angle));
			radius = emitter->shape.cone.radius;
			radius *= randomf(1 - CLAMP01(emitter->shape.cone.emit_volume), 1);

			*pos = vector3(
				emitter->shape.position.x + radius * position.x,
				emitter->shape.position.y + radius * position.y,
				emitter->shape.position.z + radius * position.z
			);

			*dir = vec3_normalized(position);
			break;
	}
}

static inline void emitter_update_particle(emitter_t *emitter, particle_t *particle)
{
	UNUSED(emitter);
	
	float delta = get_time().delta_time;

	particle->time_alive += delta;

	// The particle has run its life. Mark the particle as inactive and hide it.
	if (particle->time_alive >= particle->life) {

		particle->is_active = false;
		particle->camera_distance = -1.0f;

		for (int i = 0; i < 4; i++) {
			particle->vertices[i]->colour = COL_TRANSPARENT;
		}

		// TODO: Spawn the initial burst particles of 'death' subemitters
		
		return;
	}

	float t = (particle->time_alive / particle->life);

	// Update particle velocity and position.
	if (emitter->apply_acceleration) {

		particle->velocity.x += delta * particle->acceleration.x;
		particle->velocity.y += delta * particle->acceleration.y;
		particle->velocity.z += delta * particle->acceleration.z;
	}
	else {

		float speed = lerpf(particle->start_speed, particle->end_speed, t);
		particle->velocity = vec3_multiply(particle->direction, speed);
	}
	
	particle->position.x += delta * particle->velocity.x;
	particle->position.y += delta * particle->velocity.y;
	particle->position.z += delta * particle->velocity.z;
	particle->rotation += delta * DEG_TO_RAD(particle->rotation_speed);

	// Update size and colour.
	float size = lerpf(particle->start_size, particle->end_size, t);
	colour_t colour = lerpc(particle->start_colour, particle->end_colour, t);

	vec4_t emit_position = vec4_zero();
	vec3_t position = particle->position;

	// Update distance to main camera for depth sorting.
	if (emitter->is_world_space) {

		emit_position = vec3_to_vec4(particle->emit_position);

		vec3_t offset = vec3_subtract(emitter->world_position, particle->emit_position);
		position = vec3_subtract(position, offset);
	}

	particle->camera_distance = vec3_distance_sq(position, emitter->camera_position);

	// Copy updated data to vertices.
	for (int i = 0; i < 4; i++) {

		particle->vertices[i]->centre = particle->position;
		particle->vertices[i]->emit_position = emit_position;
		particle->vertices[i]->colour = colour;
		particle->vertices[i]->size = size;
		particle->vertices[i]->rotation = particle->rotation;
	}
}

static int emitter_sort_particles(const void *p1, const void *p2)
{
	const particle_t *particle1 = *(const particle_t **)p1;
	const particle_t *particle2 = *(const particle_t **)p2;

	if (particle1->camera_distance > particle2->camera_distance) {
		return -1;
	}
	else if (particle1->camera_distance < particle2->camera_distance) {
		return 1;
	}

	return 0;
}
