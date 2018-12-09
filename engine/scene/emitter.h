#pragma once
#ifndef __EMITTER_H
#define __EMITTER_H

#include "core/defines.h"
#include "renderer/vertex.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef struct emitter_t {

	object_t *parent; // The object this animator is attached to

	bool is_active; // Set to true when the emitter has particles that are still alive
	bool is_emitting; // Set to true when the emitter is emitting particles

	sprite_t *sprite; // The particle sprite
	mesh_t *mesh; // Mesh for every particle emitted by this emitter

	uint16_t max_particles; // Maximum number of particles alive at a time
	vertex_t *vertices;
	vindex_t *indices;

} emitter_t;

// -------------------------------------------------------------------------------------------------

emitter_t *emitter_create(object_t *parent);
void emitter_destroy(emitter_t *emitter);

void emitter_process(emitter_t *emitter);

void emitter_create_mesh(emitter_t *emitter, uint16_t max_particles);

// -------------------------------------------------------------------------------------------------

END_DECLARATIONS;

#endif
