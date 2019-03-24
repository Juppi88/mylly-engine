#include "emitter.h"
#include "object.h"
#include "sprite.h"
#include "core/memory.h"
#include "core/time.h"
#include "renderer/mesh.h"
#include "resources/resources.h"
#include "math/math.h"
#include "math/random.h"
#include <stdlib.h>

// -------------------------------------------------------------------------------------------------

static void emitter_initialize_particles(emitter_t *emitter);
static void emitter_create_mesh(emitter_t *emitter);
static void emitter_emit(emitter_t *emitter, uint16_t count);
static vec3_t emitter_randomize_position(emitter_t *emitter);
static void emitter_update_particle(emitter_t *emitter, particle_t *particle);

// -------------------------------------------------------------------------------------------------

emitter_t *emitter_create(object_t *parent)
{
	NEW(emitter_t, emitter);

	emitter->parent = parent;
	emitter->shape_type = SHAPE_POINT;
	emitter->shape = shape_point(vec3_zero());
	emitter->life.min = 1;
	emitter->life.max = 1;
	emitter->start_colour.min = COL_WHITE;
	emitter->start_colour.max = COL_WHITE;
	emitter->end_colour.min = COL_WHITE;
	emitter->end_colour.max = COL_WHITE;
	emitter->start_size.min = 1;
	emitter->start_size.max = 1;
	emitter->end_size.min = 1;
	emitter->end_size.max = 1;

	return emitter;
}

void emitter_destroy(emitter_t *emitter)
{
	if (emitter == NULL) {
		return;
	}

	if (emitter->mesh != NULL) {
		mesh_destroy(emitter->mesh);
	}

	DESTROY(emitter->particles);
	DESTROY(emitter);
}

void emitter_process(emitter_t *emitter)
{
	if (emitter == NULL || !emitter->is_active) {
		return;
	}

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
	for (int i = 0; i < emitter->max_particles; i++) {

		if (emitter->particles[i].is_active) {
			emitter_update_particle(emitter, &emitter->particles[i]);
		}
	}

	// TODO: We could possibly only refresh the range of particles which were updated?
	mesh_refresh_vertices(emitter->mesh);
}

void emitter_start(emitter_t *emitter,
                   uint16_t max_particles, uint16_t burst,
                   float emit_rate, float emit_duration)
{
	if (emitter == NULL || emitter->sprite == NULL || max_particles == 0) {
		return;
	}

	emitter->max_particles = max_particles;
	emitter->is_active = true;
	emitter->is_emitting = (emit_rate > 0);
	emitter->emit_duration = emit_duration;
	emitter->emit_rate = emit_rate;
	emitter->time_emitting = 0;
	emitter->time_since_emit = 0;

	// Initialize particle data.
	emitter_initialize_particles(emitter);

	// Create a mesh for drawing the particles.
	emitter_create_mesh(emitter);

	// Emit the initial burst of particles.
	emitter_emit(emitter, burst);
}

void emitter_set_emit_shape(emitter_t *emitter, emit_shape_type_t type, const emit_shape_t shape)
{
	if (emitter != NULL) {
		emitter->shape_type = type;
		emitter->shape = shape;
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

void emitter_set_particle_velocity(emitter_t *emitter, vec3_t min, vec3_t max)
{
	if (emitter != NULL) {
		emitter->velocity.min = min;
		emitter->velocity.max = max;
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

static void emitter_initialize_particles(emitter_t *emitter)
{
	if (emitter == NULL || emitter->particles != NULL) {
		return;
	}

	// Create a storage for all the particles of the system.
	emitter->particles = mem_alloc_fast(emitter->max_particles * sizeof(particle_t));

	// Setup the particles as non-active.
	for (int i = 0; i < emitter->max_particles; i++) {

		emitter->particles[i].is_active = false;
		emitter->particles[i].position = vec3_zero();
		emitter->particles[i].start_colour = COL_TRANSPARENT;
		emitter->particles[i].end_colour = COL_TRANSPARENT;
	}
}

static void emitter_create_mesh(emitter_t *emitter)
{
	if (emitter == NULL || emitter->particles == NULL) {
		return;
	}

	if (emitter->mesh != NULL) {
		mesh_destroy(emitter->mesh);
	}

	// Create the vertices and indices for the mesh.
	vertex_particle_t *vertices = mem_alloc_fast(emitter->max_particles * 4 * sizeof(vertex_particle_t));
	vindex_t *indices = mem_alloc_fast(emitter->max_particles * 6 * sizeof(vindex_t));

	for (int i = 0; i < emitter->max_particles; i++) {

		// Setup vertices with initial data (including completely transparent colour).
		vertices[0 + 4 * i] = vertex_particle(

			vec3(-0.5f, -0.5f, 0),
			vec3_zero(),
			vec2(emitter->sprite->uv1.x, emitter->sprite->uv1.y),
			COL_TRANSPARENT,
			0
		);

		vertices[1 + 4 * i] = vertex_particle(

			vec3(0.5f, -0.5f, 0),
			vec3_zero(),
			vec2(emitter->sprite->uv2.x, emitter->sprite->uv1.y),
			COL_TRANSPARENT,
			0
		);

		vertices[2 + 4 * i] = vertex_particle(

			vec3(-0.5f, 0.5f, 0),
			vec3_zero(),
			vec2(emitter->sprite->uv1.x, emitter->sprite->uv2.y),
			COL_TRANSPARENT,
			0
		);

		vertices[3 + 4 * i] = vertex_particle(

			vec3(0.5f, 0.5f, 0),
			vec3_zero(),
			vec2(emitter->sprite->uv2.x, emitter->sprite->uv2.y),
			COL_TRANSPARENT,
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

	mesh_set_material(emitter->mesh, res_get_shader("default-particle"), emitter->sprite->texture);

	// Copy vertex references to each particle.
	// This way updating the vertices directly will be faster and easier.
	for (int i = 0; i < emitter->max_particles; i++) {

		emitter->particles[i].vertices[0] = &emitter->mesh->part_vertices[0 + 4 * i];
		emitter->particles[i].vertices[1] = &emitter->mesh->part_vertices[1 + 4 * i];
		emitter->particles[i].vertices[2] = &emitter->mesh->part_vertices[2 + 4 * i];
		emitter->particles[i].vertices[3] = &emitter->mesh->part_vertices[3 + 4 * i];
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

			// Randomize particle details.
			emitter->particles[i].life = randomf(emitter->life.min, emitter->life.max);

			emitter->particles[i].position = emitter_randomize_position(emitter);

			emitter->particles[i].velocity = randomv(emitter->velocity.min,
	                                                 emitter->velocity.max);

			emitter->particles[i].acceleration = randomv(emitter->acceleration.min,
				                                         emitter->acceleration.max);

			emitter->particles[i].start_colour = randomc(emitter->start_colour.min,
	                                                     emitter->start_colour.max);
			emitter->particles[i].end_colour = randomc(emitter->end_colour.min,

	                                                   emitter->end_colour.max);

			emitter->particles[i].start_size = randomf(emitter->start_size.min,
	                                                   emitter->start_size.max);
			
			emitter->particles[i].end_size = randomf(emitter->end_size.min,
	                                                 emitter->end_size.max);

			emitted++;
		}
	}
}

static vec3_t emitter_randomize_position(emitter_t *emitter)
{
	float r, t;

	switch (emitter->shape_type) {

		case SHAPE_POINT:
			return emitter->shape.point.centre;

		case SHAPE_CIRCLE:
			r = randomf(0, emitter->shape.circle.radius); // Randomize a distance from the centre
			t = randomf(0, 2 * PI); // Randomize an angle

			return vector3(
				emitter->shape.circle.centre.x + r * cosf(t),
				emitter->shape.circle.centre.y + r * sinf(t),
				emitter->shape.circle.centre.z
			);

		case SHAPE_BOX:
			return vector3(
				randomf(emitter->shape.box.min.x, emitter->shape.box.max.x),
				randomf(emitter->shape.box.min.y, emitter->shape.box.max.y),
				randomf(emitter->shape.box.min.z, emitter->shape.box.max.z)
			);

		default:
			return vec3_zero();
	}
}

static void emitter_update_particle(emitter_t *emitter, particle_t *particle)
{
	UNUSED(emitter);
	
	float delta = get_time().delta_time;

	particle->time_alive += delta;

	// The particle has run its life. Mark the particle as inactive and hide it.
	if (particle->time_alive >= particle->life) {

		particle->is_active = false;

		for (int i = 0; i < 4; i++) {
			particle->vertices[i]->colour = COL_TRANSPARENT;
		}
		
		return;
	}

	// Update particle velocity and position.
	particle->velocity.x += delta * particle->acceleration.x;
	particle->velocity.y += delta * particle->acceleration.y;
	particle->velocity.z += delta * particle->acceleration.z;
	particle->position.x += delta * particle->velocity.x;
	particle->position.y += delta * particle->velocity.y;
	particle->position.z += delta * particle->velocity.z;

	// Update size and colour.
	float t = (particle->time_alive / particle->life);
	float size = lerpf(particle->start_size, particle->end_size, t);
	colour_t colour = lerpc(particle->start_colour, particle->end_colour, t);

	// Copy updated data to vertices.
	for (int i = 0; i < 4; i++) {

		particle->vertices[i]->centre = particle->position;
		particle->vertices[i]->colour = colour;
		particle->vertices[i]->size = size;
	}
}
