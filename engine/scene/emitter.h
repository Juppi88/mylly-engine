#pragma once
#ifndef __EMITTER_H
#define __EMITTER_H

#include "core/defines.h"
#include "renderer/vertex.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef struct particle_t {

	float life;
	float time_alive;
	bool is_active;
	vec3_t position;
	vec3_t velocity;
	vec3_t acceleration;
	colour_t start_colour, end_colour;
	float start_size, end_size;
	vertex_particle_t *vertices[4];

} particle_t;

// -------------------------------------------------------------------------------------------------

typedef struct emitter_t {

	object_t *parent; // The object this animator is attached to

	bool is_active; // Set to true when the emitter has particles that are still alive
	bool is_emitting; // Set to true when the emitter is emitting particles

	sprite_t *sprite; // The particle sprite
	mesh_t *mesh; // Mesh for every particle emitted by this emitter

	particle_t *particles; // Individual particle data
	uint16_t max_particles; // Maximum number of particles alive at a time

	float emit_duration; // Number of seconds to emit particles for
	float emit_rate; // Number of particles to be emitted per second after the initial burst
	float time_emitting; // Number of seconds the emitter has been emitting particles
	float time_since_emit; // Time elapsed since a particle was emitted the last time

	// Particle system data
	struct { float min, max; } life; // Particle life time in seconds
	struct { vec3_t min, max; } velocity; // Limits for particle start velocity
	struct { vec3_t min, max; } acceleration; // Limits for particle acceleration
	struct { colour_t min, max; } start_colour; // Particle start colour
	struct { colour_t min, max; } end_colour; // Particle end colour
	struct { float min, max; } start_size; // Particle initial size in game units
	struct { float min, max; } end_size; // Particle final size in game units

} emitter_t;

// -------------------------------------------------------------------------------------------------

emitter_t *emitter_create(object_t *parent);
void emitter_destroy(emitter_t *emitter);
void emitter_process(emitter_t *emitter);

void emitter_start(emitter_t *emitter, uint16_t max_particles, uint16_t burst, float emit_rate, float emit_duration);

void emitter_set_particle_sprite(emitter_t *emitter, sprite_t *sprite);
void emitter_set_particle_life_time(emitter_t *emitter, float min, float max);
void emitter_set_particle_velocity(emitter_t *emitter, vec3_t min, vec3_t max);
void emitter_set_particle_acceleration(emitter_t *emitter, vec3_t min, vec3_t max);
void emitter_set_particle_start_colour(emitter_t *emitter, colour_t min, colour_t max);
void emitter_set_particle_end_colour(emitter_t *emitter, colour_t min, colour_t max);
void emitter_set_particle_start_size(emitter_t *emitter, float min, float max);
void emitter_set_particle_end_size(emitter_t *emitter, float min, float max);

// -------------------------------------------------------------------------------------------------

END_DECLARATIONS;

#endif
