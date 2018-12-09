#include "emitter.h"
#include "object.h"
#include "sprite.h"
#include "core/memory.h"
#include "core/time.h"
#include "renderer/mesh.h"
#include "resources/resources.h"

// -------------------------------------------------------------------------------------------------

emitter_t *emitter_create(object_t *parent)
{
	NEW(emitter_t, emitter);

	emitter->parent = parent;

	emitter->is_active = true;
	emitter->sprite = res_get_sprite("bullets/blue/big");

	emitter_create_mesh(emitter, 10);

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

		emitter->vertices[0 + 4 * i].normal =
			vec3_add(emitter->vertices[0 + 4 * i].normal, vector3(0, 0.1f, 0));
		
		emitter->vertices[1 + 4 * i].normal =
			vec3_add(emitter->vertices[1 + 4 * i].normal, vector3(0, 0.1f, 0));

		emitter->vertices[2 + 4 * i].normal =
			vec3_add(emitter->vertices[2 + 4 * i].normal, vector3(0, 0.1f, 0));

		emitter->vertices[3 + 4 * i].normal =
			vec3_add(emitter->vertices[3 + 4 * i].normal, vector3(0, 0.1f, 0));
	}

	mesh_refresh_vertices(emitter->mesh);
}

void emitter_create_mesh(emitter_t *emitter, uint16_t max_particles)
{
	if (emitter == NULL) {
		return;
	}

	if (emitter->mesh != NULL) {
		mesh_destroy(emitter->mesh);
	}

	// Create the vertices and indices for the mesh.
	emitter->max_particles = max_particles;
	emitter->vertices = mem_alloc_fast(max_particles * 4 * sizeof(vertex_t));
	emitter->indices = mem_alloc_fast(max_particles * 6 * sizeof(vindex_t));

	float width = 0.5f;
	float height = 0.5f;

	for (int i = 0; i < max_particles; i++) {

		emitter->vertices[0 + 4 * i] = vertex(
			vec4(-width, -height, 0, 1),
			vec3(0, 0, 0),
			vec2(emitter->sprite->uv1.x, emitter->sprite->uv1.y),
			COL_WHITE
		);

		emitter->vertices[1 + 4 * i] = vertex(
			vec4(width, -height, 0, 1),
			vec3(0, 0, 0),
			vec2(emitter->sprite->uv2.x, emitter->sprite->uv1.y),
			COL_WHITE
		);

		emitter->vertices[2 + 4 * i] = vertex(
			vec4(-width, height, 0, 1),
			vec3(0, 0, 0),
			vec2(emitter->sprite->uv1.x, emitter->sprite->uv2.y),
			COL_WHITE
		);

		emitter->vertices[3 + 4 * i] = vertex(
			vec4(width, height, 0, 1),
			vec3(0, 0, 0),
			vec2(emitter->sprite->uv2.x, emitter->sprite->uv2.y),
			COL_WHITE
		);

		emitter->indices[0 + 6 * i] = 0 + 6 * i;
		emitter->indices[1 + 6 * i] = 1 + 6 * i;
		emitter->indices[2 + 6 * i] = 2 + 6 * i;
		emitter->indices[3 + 6 * i] = 2 + 6 * i;
		emitter->indices[4 + 6 * i] = 1 + 6 * i;
		emitter->indices[5 + 6 * i] = 3 + 6 * i;
	}

	// Create a mesh with the generated vertex data.
	emitter->mesh = mesh_create(0);

	mesh_set_vertices(emitter->mesh, emitter->vertices, 4 * max_particles);
	mesh_set_indices(emitter->mesh, emitter->indices, 6 * max_particles);

	mesh_set_material(emitter->mesh, res_get_shader("default-particle"), emitter->sprite->texture);
}
