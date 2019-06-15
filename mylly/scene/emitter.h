#pragma once
#ifndef __EMITTER_H
#define __EMITTER_H

#include "core/defines.h"
#include "collections/array.h"
#include "renderer/vertex.h"
#include "resources/resource.h"
#include "math/math.h"

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
	float rotation;
	float rotation_speed;
	vertex_particle_t *vertices[4];
	int base_index; // The first index of this quad

} particle_t;

// -------------------------------------------------------------------------------------------------

// Type of shape to emit particles from
typedef enum {

	SHAPE_CIRCLE,
	SHAPE_SPHERE,
	SHAPE_BOX,
	SHAPE_CONE,

} emit_shape_type_t;

// Emitter shape data
typedef struct emit_shape_t {

	emit_shape_type_t type; // Type of shape
	vec3_t position; // Offset from the emitter's base position

	union {
		struct { float radius; } circle;
		struct { float radius; } sphere;
		struct { vec3_t extents; } box;
		struct {
			float radius; // Maximum distance from the centre particles emit from
			float angle; // Angle of the cone at the centre
			float emit_volume; // Percentage of the volume emitting particles, 0 = edge, 1 = all
		} cone;
	};

} emit_shape_t;

// -------------------------------------------------------------------------------------------------

static inline emit_shape_t shape_circle(vec3_t pos, float radius)
{
	emit_shape_t shape;

	shape.position = pos;
	shape.type = SHAPE_CIRCLE;
	shape.circle.radius = radius;

	return shape;
}

static inline emit_shape_t shape_sphere(vec3_t pos, float radius)
{
	emit_shape_t shape;

	shape.position = pos;
	shape.type = SHAPE_SPHERE;
	shape.sphere.radius = radius;

	return shape;
}

static inline emit_shape_t shape_box(vec3_t pos, vec3_t extents)
{
	emit_shape_t shape;

	shape.position = pos;
	shape.type = SHAPE_BOX;
	shape.box.extents = extents;

	return shape;
}

static inline emit_shape_t shape_cone(vec3_t pos, float angle, float radius, float volume)
{
	emit_shape_t shape;

	shape.position = pos;
	shape.type = SHAPE_CONE;
	shape.cone.angle = angle;
	shape.cone.radius = radius;
	shape.cone.emit_volume = CLAMP01(volume);

	return shape;
}

// -------------------------------------------------------------------------------------------------

typedef enum subemitter_type_t {

	SUBEMITTER_CREATE,
	SUBEMITTER_PARTICLE_SPAWN,
	SUBEMITTER_PARTICLE_DEATH,

} subemitter_type_t;

typedef struct subemitter_t {

	subemitter_type_t type;

	union {
		const char *emitter_name;
		emitter_t *emitter;
	};

} subemitter_t;

// -------------------------------------------------------------------------------------------------

static inline subemitter_t create_subemitter(subemitter_type_t type, emitter_t *effect)
{
	subemitter_t subemitter;

	subemitter.type = type;
	subemitter.emitter = effect;

	return subemitter;
}

static inline subemitter_t create_subemitter_name(subemitter_type_t type, const char *effect_name)
{
	subemitter_t subemitter;

	subemitter.type = type;
	subemitter.emitter_name = effect_name;

	return subemitter;
}

// -------------------------------------------------------------------------------------------------

typedef struct emitter_t {

	resource_t resource; // Resource info

	object_t *parent; // The object this animator is attached to

	bool is_active; // Set to true when the emitter has particles that are still alive
	bool is_emitting; // Set to true when the emitter is emitting particles
	bool emit_on_request; // This is a subemitter which only emits particles on request

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

	// A list of subemitters
	arr_t(subemitter_t) subemitters;

	vec3_t world_position; // Cached world position of the emitter object
	vec3_t camera_position; // Cached position of the camera rendering the particles

	// The shape of the particle emitter.
	emit_shape_t shape;

	// Particle system data
	struct { float min, max; } life; // Particle life time in seconds
	struct { float min, max; } speed; // Particle start speed in units per second
	struct { vec3_t min, max; } acceleration; // Limits for particle acceleration
	struct { colour_t min, max; } start_colour; // Particle start colour
	struct { colour_t min, max; } end_colour; // Particle end colour
	struct { float min, max; } start_size; // Particle initial size in game units
	struct { float min, max; } end_size; // Particle final size in game units
	struct { float min, max; } rotation_speed; // Particle rotation speed in degrees/second

} emitter_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

emitter_t *emitter_create(object_t *parent, const emitter_t *emitter_template, bool is_subemitter);
emitter_t *emitter_create_from_resource(const char *name, const char *path); // For internal use

void emitter_destroy(emitter_t *emitter);
void emitter_process(emitter_t *emitter);

void emitter_start(emitter_t *emitter);
void emitter_stop(emitter_t *emitter);

void emitter_set_emit_shape(emitter_t *emitter, const emit_shape_t shape);
void emitter_set_world_space(emitter_t *emitter, bool is_world_space);
void emitter_set_max_particles(emitter_t *emitter, int num_particles);
void emitter_set_initial_burst(emitter_t *emitter, int num_particles);
void emitter_set_emit_duration(emitter_t *emitter, float duration);
void emitter_set_emit_rate(emitter_t *emitter, float particles_per_sec);

void emitter_set_particle_sprite(emitter_t *emitter, sprite_t *sprite);
void emitter_set_particle_life_time(emitter_t *emitter, float min, float max);
void emitter_set_particle_speed(emitter_t *emitter, float min, float max);
void emitter_set_particle_acceleration(emitter_t *emitter, vec3_t min, vec3_t max);
void emitter_set_particle_start_colour(emitter_t *emitter, colour_t min, colour_t max);
void emitter_set_particle_end_colour(emitter_t *emitter, colour_t min, colour_t max);
void emitter_set_particle_start_size(emitter_t *emitter, float min, float max);
void emitter_set_particle_end_size(emitter_t *emitter, float min, float max);
void emitter_set_particle_rotation_speed(emitter_t *emitter, float min, float max);

END_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

#endif
