#include "debug.h"
#include "mesh.h"
#include "rendersystem.h"
#include "resources/resources.h"
#include "io/log.h"
#include "core/memory.h"

// -------------------------------------------------------------------------------------------------

static const int MAX_LINES = 10000; // Number of maximum lines visible at once, ber layer

// Number of lines added to each layer during the current frame
static int num_lines_scene = 0;
static int num_lines_overlay = 0;

// Meshes containing the debug primitives to be rendered
static mesh_t *debug_mesh_scene;
static mesh_t *debug_mesh_overlay;

// -------------------------------------------------------------------------------------------------

void debug_initialize(void)
{
	// Create a mesh for storing the debug primitives into.
	debug_mesh_scene = mesh_create();
	debug_mesh_overlay = mesh_create();

	// Prealloc buffers on the GPU.
	mesh_prealloc_vertices(debug_mesh_scene, VERTEX_DEBUG, 2 * MAX_LINES);
	mesh_prealloc_vertices(debug_mesh_overlay, VERTEX_DEBUG, 2 * MAX_LINES);

	// Generate indices into a temporary buffer and upload them to the GPU.
	NEW_ARRAY(vindex_t, indices, 2 * MAX_LINES);

	for (int i = 0; i < 2 * MAX_LINES; i++) {
		indices[i] = (vindex_t)i;
	}

	mesh_set_indices(debug_mesh_scene, indices, 2 * MAX_LINES);
	mesh_set_indices(debug_mesh_overlay, indices, 2 * MAX_LINES);

	DESTROY(indices);

	debug_mesh_scene->handle_indices = bufcache_alloc_indices(
		BUFIDX_DEBUG_LINE, debug_mesh_scene->indices, debug_mesh_scene->num_indices);

	debug_mesh_overlay->handle_indices = bufcache_alloc_indices(
		BUFIDX_DEBUG_LINE, debug_mesh_overlay->indices, debug_mesh_overlay->num_indices);

	// Use a default shader for rendering UI primitives.
	mesh_set_shader(debug_mesh_scene, res_get_shader("default-debug"));
	mesh_set_shader(debug_mesh_overlay, res_get_shader("default-debug-overlay"));
}

void debug_shutdown(void)
{
	// Destroy debug 
	if (debug_mesh_scene != NULL) {

		mesh_destroy(debug_mesh_scene);
		debug_mesh_scene = NULL;
	}

	if (debug_mesh_overlay != NULL) {

		mesh_destroy(debug_mesh_overlay);
		debug_mesh_overlay = NULL;
	}
}

void debug_begin_frame(void)
{
	// Clear buffer (i.e. reset active line count).
	num_lines_scene = 0;
	num_lines_overlay = 0;
}

void debug_end_frame(void)
{
	// Re-upload changed vertex data to the GPU.
	mesh_refresh_vertices(debug_mesh_overlay);
	mesh_refresh_vertices(debug_mesh_scene);

	// Set the number of indices to render to each mesh in order to minimize rendered indices.
	debug_mesh_overlay->num_indices_to_render = 2 * num_lines_overlay;
	debug_mesh_scene->num_indices_to_render = 2 * num_lines_scene;

	// Tell the renderer to draw the debug meshes.
	rsys_render_mesh(debug_mesh_overlay, true);
	rsys_render_mesh(debug_mesh_scene, false);
}

void debug_draw_line(vec3_t start, vec3_t end, colour_t colour, bool overlay)
{
	// Ensure there is enough space in the buffer.
	if ((overlay && num_lines_overlay >= MAX_LINES) ||
	    (!overlay && num_lines_scene >= MAX_LINES)) {

		log_message("Renderer", "Exceeded maximum debug lines!");
		return;
	}

	vertex_debug_t *vertices;
	int base;

	// Add the vertices to the correct buffer (either 3D scene or overlay).
	if (overlay) {

		vertices = debug_mesh_overlay->debug_vertices;
		base = num_lines_overlay++;
	}
	else {

		vertices = debug_mesh_scene->debug_vertices;
		base = num_lines_scene++;
	}

	vertices[2 * base + 0] = vertex_debug(start, colour);
	vertices[2 * base + 1] = vertex_debug(end, colour);
}

void debug_draw_line_2d(vec2_t start, vec2_t end, colour_t colour, bool overlay)
{
	if ((overlay && num_lines_overlay >= MAX_LINES) ||
	    (!overlay && num_lines_scene >= MAX_LINES)) {

		log_message("Renderer", "Exceeded maximum debug lines!");
		return;
	}

	vertex_debug_t *vertices;
	int base;

	if (overlay) {

		vertices = debug_mesh_overlay->debug_vertices;
		base = num_lines_overlay++;
	}
	else {

		vertices = debug_mesh_scene->debug_vertices;
		base = num_lines_scene++;
	}

	vertices[2 * base + 0] = vertex_debug(vec2_to_vec3(start), colour);
	vertices[2 * base + 1] = vertex_debug(vec2_to_vec3(end), colour);
}

void debug_draw_rect(vec2_t min, vec2_t max, colour_t colour, bool overlay)
{
	if ((overlay && num_lines_overlay - 3 >= MAX_LINES) ||
	    (!overlay && num_lines_scene - 3 >= MAX_LINES)) {

		log_message("Renderer", "Exceeded maximum debug lines!");
		return;
	}

	vertex_debug_t *vertices;
	int base;

	if (overlay) {

		vertices = debug_mesh_overlay->debug_vertices;
		base = num_lines_overlay;

		num_lines_overlay += 4;
	}
	else {

		vertices = debug_mesh_scene->debug_vertices;
		base = num_lines_scene++;

		num_lines_scene += 4;
	}

	vertices[2 * base + 0] = vertex_debug(vec3(min.x, min.y, 0), colour);
	vertices[2 * base + 1] = vertex_debug(vec3(min.x, max.y, 0), colour);
	vertices[2 * base + 2] = vertex_debug(vec3(max.x, min.y, 0), colour);
	vertices[2 * base + 3] = vertex_debug(vec3(max.x, max.y, 0), colour);
	vertices[2 * base + 4] = vertex_debug(vec3(min.x, min.y, 0), colour);
	vertices[2 * base + 5] = vertex_debug(vec3(max.x, min.y, 0), colour);
	vertices[2 * base + 6] = vertex_debug(vec3(min.x, max.y, 0), colour);
	vertices[2 * base + 7] = vertex_debug(vec3(max.x, max.y, 0), colour);
}
