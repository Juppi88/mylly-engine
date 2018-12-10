#include "emitter.h"
#include "object.h"
#include "sprite.h"
#include "core/memory.h"
#include "core/time.h"
#include "renderer/mesh.h"
#include "resources/resources.h"
#include <stdlib.h>

// -------------------------------------------------------------------------------------------------

emitter_t *emitter_create(object_t *parent)
{
	NEW(emitter_t, emitter);

	emitter->parent = parent;

	emitter->is_active = true;
	emitter->sprite = res_get_sprite("bullets/blue/particles/small");

	emitter->max_particles = 100;
	emitter_initialize_particles(emitter);
	emitter_create_mesh(emitter);

	return emitter;
}


void emitter_destroy(emitter_t *emitter)
{
	if (emitter == NULL) {
		return;
	}

	DELETE(emitter);
}

void emitter_process(emitter_t *emitter)
{
	if (emitter == NULL || !emitter->is_active) {
		return;
	}

	for (int i = 0; i < emitter->max_particles; i++) {

		emitter_update_particle(emitter, &emitter->particles[i]);
	}

	mesh_refresh_vertices(emitter->mesh);
}

float randomf(float a, float b)
{
	return a + ((double)rand() / (RAND_MAX)) * (b - a);
}

void emitter_initialize_particles(emitter_t *emitter)
{
	if (emitter == NULL || emitter->particles != NULL) {
		return;
	}

	emitter->particles = mem_alloc_fast(emitter->max_particles * sizeof(particle_t));

	for (int i = 0; i < emitter->max_particles; i++) {

		emitter->particles[i].position = vector3(0, 0, 1);
		emitter->particles[i].velocity = vector3(randomf(0, 15), randomf(-3, 22), 0);
		emitter->particles[i].acceleration = vector3(0, -50, 0);
		emitter->particles[i].colour = COL_WHITE;
		emitter->particles[i].is_active = true;
	}
}

void emitter_create_mesh(emitter_t *emitter)
{
	if (emitter == NULL || emitter->particles == NULL) {
		return;
	}

	if (emitter->mesh != NULL) {
		mesh_destroy(emitter->mesh);
	}

	// Create the vertices and indices for the mesh.
	vertex_t *vertices = mem_alloc_fast(emitter->max_particles * 4 * sizeof(vertex_t));
	vindex_t *indices = mem_alloc_fast(emitter->max_particles * 6 * sizeof(vindex_t));

	float width = 0.5f;
	float height = 0.5f;

	for (int i = 0; i < emitter->max_particles; i++) {

		vertices[0 + 4 * i] = vertex(
			vec4(-width, -height, 0, 1),
			vec3_zero,
			vec2(emitter->sprite->uv1.x, emitter->sprite->uv1.y),
			COL_WHITE
		);

		vertices[1 + 4 * i] = vertex(
			vec4(width, -height, 0, 1),
			vec3_zero,
			vec2(emitter->sprite->uv2.x, emitter->sprite->uv1.y),
			COL_WHITE
		);

		vertices[2 + 4 * i] = vertex(
			vec4(-width, height, 0, 1),
			vec3_zero,
			vec2(emitter->sprite->uv1.x, emitter->sprite->uv2.y),
			COL_WHITE
		);

		vertices[3 + 4 * i] = vertex(
			vec4(width, height, 0, 1),
			vec3_zero,
			vec2(emitter->sprite->uv2.x, emitter->sprite->uv2.y),
			COL_WHITE
		);

		indices[0 + 6 * i] = 0 + 4 * i;
		indices[1 + 6 * i] = 1 + 4 * i;
		indices[2 + 6 * i] = 2 + 4 * i;
		indices[3 + 6 * i] = 2 + 4 * i;
		indices[4 + 6 * i] = 1 + 4 * i;
		indices[5 + 6 * i] = 3 + 4 * i;
	}

	// Create a mesh with the generated vertex data.
	emitter->mesh = mesh_create(0);

	mesh_set_vertices(emitter->mesh, vertices, 4 * emitter->max_particles);
	mesh_set_indices(emitter->mesh, indices, 6 * emitter->max_particles);

	mesh_set_material(emitter->mesh, res_get_shader("default-particle"), emitter->sprite->texture);

	// Copy vertex references to each particle.
	// This way updating the vertices directly will be faster and easier.
	for (int i = 0; i < emitter->max_particles; i++) {

		emitter->particles[i].vertices[0] = &emitter->mesh->vertices[0 + 4 * i];
		emitter->particles[i].vertices[1] = &emitter->mesh->vertices[1 + 4 * i];
		emitter->particles[i].vertices[2] = &emitter->mesh->vertices[2 + 4 * i];
		emitter->particles[i].vertices[3] = &emitter->mesh->vertices[3 + 4 * i];
	}

	// Delete temporary data.
	DELETE(vertices);
	DELETE(indices);
}

void emitter_update_particle(emitter_t *emitter, particle_t *particle)
{
	float delta = get_time().delta_time;

	// Update particle.
	particle->velocity.x += delta * particle->acceleration.x;
	particle->velocity.y += delta * particle->acceleration.y;
	particle->velocity.z += delta * particle->acceleration.z;
	particle->position.x += delta * particle->velocity.x;
	particle->position.y += delta * particle->velocity.y;
	particle->position.z += delta * particle->velocity.z;

	// Update vertex data.
	for (int i = 0; i < 4; i++) {
		particle->vertices[i]->normal = particle->position;
		particle->vertices[i]->colour = particle->colour;
	}
}
