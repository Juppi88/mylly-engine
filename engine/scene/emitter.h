#pragma once
#ifndef __EMITTER_H
#define __EMITTER_H

#include "core/defines.h"
#include "renderer/vertex.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef struct particle_t {

	vec3_t position;
	vec3_t velocity;
	vec3_t acceleration;
	colour_t colour;
	bool is_active;
	vertex_t *vertices[4];

} particle_t;

// -------------------------------------------------------------------------------------------------

typedef struct emitter_t {

	object_t *parent; // The object this animator is attached to

	bool is_active; // Set to true when the emitter has particles that are still alive
	bool is_emitting; // Set to true when the emitter is emitting particles

	sprite_t *sprite; // The particle sprite
	mesh_t *mesh; // Mesh for every particle emitted by this emitter

	particle_t *particles;
	uint16_t max_particles; // Maximum number of particles alive at a time

} emitter_t;

// -------------------------------------------------------------------------------------------------

emitter_t *emitter_create(object_t *parent);
void emitter_destroy(emitter_t *emitter);

void emitter_process(emitter_t *emitter);

void emitter_initialize_particles(emitter_t *emitter);
void emitter_create_mesh(emitter_t *emitter);

void emitter_update_particle(emitter_t *emitter, particle_t *particle);

// -------------------------------------------------------------------------------------------------

END_DECLARATIONS;

#endif
