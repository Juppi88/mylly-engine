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
	float camera_distance; // Distance to camera, used for depth sorting
	bool is_active;
	vec3_t emit_position;
	vec3_t position;
	vec3_t velocity;
	vec3_t acceleration;
	colour_t start_colour, end_colour;
	float start_size, end_size;
	vertex_particle_t *vertices[4];
	int base_index; // The first index of this quad

} particle_t;

// -------------------------------------------------------------------------------------------------

// Type of shape to emit particles from
typedef enum {

	SHAPE_POINT,
	SHAPE_CIRCLE,
	SHAPE_BOX

} emit_shape_type_t;

// Emitter shape data
typedef union emit_shape_t {

	struct { vec3_t centre; } point;
	struct { vec3_t centre; float radius; } circle;
	struct { vec3_t min, max; } box;

} emit_shape_t;

// Macros to initialize the shape struct above
#define shape_point(c) (emit_shape_t){ .point = { .centre = c } }
#define shape_circle(c, r) (emit_shape_t){ .circle = { .centre = c, .radius = r } }
#define shape_box(low, high) (emit_shape_t){ .box = { .min = low, .max = high } }

// -------------------------------------------------------------------------------------------------

typedef struct emitter_t {

	object_t *parent; // The object this animator is attached to

	bool is_active; // Set to true when the emitter has particles that are still alive
	bool is_emitting; // Set to true when the emitter is emitting particles
	bool is_world_space; // Set to true when particles are emitted in world space (as opposed to local)

	sprite_t *sprite; // The particle sprite
	mesh_t *mesh; // Mesh for every particle emitted by this emitter

	particle_t *particles; // Individual particle data
	particle_t **particle_references; // An array containing a reference to each particle
	uint16_t max_particles; // Maximum number of particles alive at a time

	float emit_duration; // Number of seconds to emit particles for
	float emit_rate; // Number of particles to be emitted per second after the initial burst
	float time_emitting; // Number of seconds the emitter has been emitting particles
	float time_since_emit; // Time elapsed since a particle was emitted the last time
	uint16_t initial_burst; // The initial burst of particles

	vec3_t world_position; // Cached world position of the emitter object
	vec3_t camera_position; // Cached position of the camera rendering the particles

	// The shape of the particle emitter.
	emit_shape_type_t shape_type;
	emit_shape_t shape;

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

void emitter_start(emitter_t *emitter,
                   uint16_t max_particles, uint16_t burst,
                   float emit_rate, float emit_duration);

void emitter_set_emit_shape(emitter_t *emitter, emit_shape_type_t type, const emit_shape_t shape);

void emitter_set_particle_sprite(emitter_t *emitter, sprite_t *sprite);
void emitter_set_particle_life_time(emitter_t *emitter, float min, float max);
void emitter_set_particle_velocity(emitter_t *emitter, vec3_t min, vec3_t max);
void emitter_set_particle_acceleration(emitter_t *emitter, vec3_t min, vec3_t max);
void emitter_set_particle_start_colour(emitter_t *emitter, colour_t min, colour_t max);
void emitter_set_particle_end_colour(emitter_t *emitter, colour_t min, colour_t max);
void emitter_set_particle_start_size(emitter_t *emitter, float min, float max);
void emitter_set_particle_end_size(emitter_t *emitter, float min, float max);
void emitter_set_world_space(emitter_t *emitter, bool is_world_space);

// -------------------------------------------------------------------------------------------------

END_DECLARATIONS;

#endif
