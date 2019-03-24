#include "sprite.h"
#include "renderer/texture.h"
#include "renderer/mesh.h"
#include "core/memory.h"
#include "core/string.h"
#include "math/math.h"
#include "resources/resources.h"

// -------------------------------------------------------------------------------------------------

static void sprite_create_mesh(sprite_t *sprite);

// -------------------------------------------------------------------------------------------------

sprite_t *sprite_create(texture_t *parent, const char *name)
{
	if (parent == NULL) {
		return NULL;
	}

	char sprite_name[200];

	if (string_is_null_or_empty(name)) {
		snprintf(sprite_name, sizeof(sprite_name), "%s", parent->resource.res_name);
	}
	else {
		snprintf(sprite_name, sizeof(sprite_name), "%s/%s", parent->resource.res_name, name);	
	}

	// Create a new sprite structure.
	NEW(sprite_t, sprite);

	sprite->resource.res_name = string_duplicate(sprite_name);
	sprite->resource.name = string_duplicate(name);
	sprite->resource.path = NULL; // The individual sprite does not have a path (the sheet does)

	return sprite;
}

void sprite_destroy(sprite_t *sprite)
{
	if (sprite == NULL) {
		return;
	}

	mesh_destroy(sprite->mesh);

	DESTROY(sprite->resource.res_name);
	DESTROY(sprite->resource.path);

	DESTROY(sprite);
}

void sprite_set(sprite_t *sprite, texture_t *texture,
                vec2_t position, vec2_t size, vec2_t pivot, float pixels_per_unit)
{
	if (sprite == NULL || texture == NULL) {
		return;
	}

	sprite->texture = texture;
	sprite->pixels_per_unit = pixels_per_unit;
	sprite->pivot = pivot;

	// Sanitize sprite coordinates.
	sprite->position.x = CLAMP(position.x, 0, texture->width);
	sprite->position.y = CLAMP(position.y, 0, texture->height);

	sprite->size.x = MIN(size.x, texture->width);
	sprite->size.y = MIN(size.y, texture->height);

	if (sprite->position.x + sprite->size.x > texture->width) {
		sprite->position.x = texture->width - sprite->size.x;
	}
	if (sprite->position.y + sprite->size.y > texture->height) {
		sprite->position.y = texture->height - sprite->size.y;
	}

	// Calculate texture coordinates.
	sprite->uv1 = vector2(
		sprite->position.x / texture->width,
		sprite->position.y / texture->height
	);

	sprite->uv2 = vector2(
		(sprite->position.x + sprite->size.x) / texture->width,
		(sprite->position.y + sprite->size.y) / texture->height
	);

	// Initialize slice position and coordinates so regular sprites can be used as 9-slice sprites.
	sprite->slice_position = sprite->position;
	sprite->slice_size = sprite->size;
	sprite->slice_uv1 = sprite->uv1;
	sprite->slice_uv2 = sprite->uv2;

	// Create a mesh for the sprite.
	if (sprite->mesh != NULL) {
		mesh_destroy(sprite->mesh);
	}

	sprite_create_mesh(sprite);

	mesh_set_material(sprite->mesh, res_get_shader("default-sprite"), texture);
}

void sprite_set_nine_slice(sprite_t *sprite, vec2_t slice_position, vec2_t slice_size)
{
	if (sprite == NULL || sprite->texture == NULL) {
		return;
	}

	texture_t *texture = sprite->texture;

	// Sanitize slice coordinates.
	sprite->slice_position.x = CLAMP(slice_position.x, 0, texture->width);
	sprite->slice_position.y = CLAMP(slice_position.y, 0, texture->height);

	sprite->slice_size.x = MIN(slice_size.x, texture->width);
	sprite->slice_size.y = MIN(slice_size.y, texture->height);

	if (sprite->slice_position.x + sprite->slice_size.x > texture->width) {
		sprite->slice_position.x = texture->width - sprite->slice_size.x;
	}
	if (sprite->slice_position.y + sprite->slice_size.y > texture->height) {
		sprite->slice_position.y = texture->height - sprite->slice_size.y;
	}

	// Calculate texture coordinates.
	sprite->slice_uv1 = vector2(
		sprite->slice_position.x / texture->width,
		sprite->slice_position.y / texture->height
	);

	sprite->slice_uv2 = vector2(
		(sprite->slice_position.x + sprite->slice_size.x) / texture->width,
		(sprite->slice_position.y + sprite->slice_size.y) / texture->height
	);
}

static void sprite_create_mesh(sprite_t *sprite)
{
	if (sprite == NULL) {
		return;
	}

	// Calculate half the width/height of the sprite in world units and the offset caused by pivot.
	float width = 0.5f * sprite->size.x / sprite->pixels_per_unit;
	float height = 0.5f * sprite->size.y / sprite->pixels_per_unit;
	float pivot_x = sprite->pivot.x / sprite->pixels_per_unit;
	float pivot_y = sprite->pivot.y / sprite->pixels_per_unit;

	// Create the vertices and indices for the sprite mesh (currenty a quad, could be optimized)
	vertex_t vertices[] = {
		{
			.pos = vec3(-width - pivot_x, -height - pivot_y, 0.0f),
			.normal = vec3(0, 0, 1),
			.uv = vec2(sprite->uv1.x, sprite->uv1.y)
		},
		{
			.pos = vec3(width - pivot_x, -height - pivot_y, 0.0f),
			.normal = vec3(0, 0, 1),
			.uv = vec2(sprite->uv2.x, sprite->uv1.y)
		},
		{
			.pos = vec3(-width - pivot_x, height - pivot_y, 0.0f),
			.normal = vec3(0, 0, 1),
			.uv = vec2(sprite->uv1.x, sprite->uv2.y)
		},
		{
			.pos = vec3(width - pivot_x, height - pivot_y, 0.0f),
			.normal = vec3(0, 0, 1),
			.uv = vec2(sprite->uv2.x, sprite->uv2.y)
		}
	};

	vindex_t indices[] = {
		0, 1, 2,
		2, 1, 3
	};

	// Create a mesh with the quad vertex data.
	sprite->mesh = mesh_create();

	mesh_set_vertices(sprite->mesh, vertices, LENGTH(vertices));
	mesh_set_indices(sprite->mesh, indices, LENGTH(indices));
}
