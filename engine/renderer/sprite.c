#include "sprite.h"
#include "mesh.h"
#include "texture.h"
#include "core/memory.h"
#include "math/math.h"

// -------------------------------------------------------------------------------------------------

static void sprite_create_mesh(sprite_t *sprite);

// -------------------------------------------------------------------------------------------------

sprite_t *sprite_create(texture_t *texture, vec2_t position, vec2_t size,
						vec2_t pivot, float pixels_per_unit)
{
	if (texture == NULL) {
		return NULL;
	}

	// Create a new sprite structure.
	NEW(sprite_t, sprite);

	sprite->texture = texture;
	sprite->pixels_per_unit = pixels_per_unit;
	sprite->pivot = pivot;

	// Sanitize sprite coordinates.
	sprite->position.x = CLAMP(position.x, 0, texture->width);
	sprite->position.y = CLAMP(position.y, 0, texture->height);
	sprite->size.x = MIN(size.x, texture->width - position.x);
	sprite->size.y = MIN(size.y, texture->height - position.y);

	// Calculate texture coordinates.
	sprite->uv1 = vector2(
		sprite->position.x / texture->width,
		sprite->position.y / texture->height
	);

	sprite->uv2 = vector2(
		(sprite->position.x + sprite->size.x) / texture->width,
		(sprite->position.y + sprite->size.y) / texture->height
	);

	// Create a mesh for the sprite.
	sprite_create_mesh(sprite);

	mesh_set_material(sprite->mesh, res_get_shader("default-sprite"), texture);

	return sprite;
}

void sprite_destroy(sprite_t *sprite)
{
	if (sprite == NULL) {
		return;
	}

	mesh_destroy(sprite->mesh);
	DELETE(sprite);
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
			.pos = vec4(-width - pivot_x, -height - pivot_y, 0.0f, 1.0f),
			.normal = vec3(0, 0, 1),
			.uv = vec2(sprite->uv1.x, sprite->uv1.y),
			.colour = COL_WHITE
		},
		{
			.pos = vec4(width - pivot_x, -height - pivot_y, 0.0f, 1.0f),
			.normal = vec3(0, 0, 1),
			.uv = vec2(sprite->uv2.x, sprite->uv1.y),
			.colour = COL_WHITE
		},
		{
			.pos = vec4(-width - pivot_x, height - pivot_y, 0.0f, 1.0f),
			.normal = vec3(0, 0, 1),
			.uv = vec2(sprite->uv1.x, sprite->uv2.y),
			.colour = COL_WHITE
		},
		{
			.pos = vec4(width - pivot_x, height - pivot_y, 0.0f, 1.0f),
			.normal = vec3(0, 0, 1),
			.uv = vec2(sprite->uv2.x, sprite->uv2.y),
			.colour = COL_WHITE
		}
	};

	vindex_t indices[] = {
		0, 1, 2,
		2, 1, 3
	};

	// Create a mesh with the quad vertex data.
	sprite->mesh = mesh_create(0);

	mesh_set_vertices(sprite->mesh, vertices, LENGTH(vertices));
	mesh_set_indices(sprite->mesh, indices, LENGTH(indices));
}
