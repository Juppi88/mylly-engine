#include "debug.h"
#include "mesh.h"
#include "rendersystem.h"
#include "resources/resources.h"
#include "io/log.h"
#include "core/memory.h"

// -------------------------------------------------------------------------------------------------

static const int MAX_LINES = 10000; // Number of maximum lines visible at once
static int num_lines = 0; // Number of lines added to the buffer during the current frame
static mesh_t *debug_mesh; // Mesh containing all debug primitives to be rendered

// -------------------------------------------------------------------------------------------------

void debug_initialize(void)
{
	// Create a mesh for storing the debug primitives into.
	debug_mesh = mesh_create();

	// Prealloc buffers on the GPU..
	mesh_prealloc_vertices(debug_mesh, VERTEX_DEBUG, 2 * MAX_LINES);
	
	// Generate indices into a temporary buffer and upload them to the GPU.
	NEW_ARRAY(vindex_t, indices, 2 * MAX_LINES);

	for (int i = 0; i < 2 * MAX_LINES; i++) {
		indices[i] = (vindex_t)i;
	}

	mesh_set_indices(debug_mesh, indices, 2 * MAX_LINES);
	DESTROY(indices);

	debug_mesh->handle_indices =
		bufcache_alloc_indices(BUFIDX_DEBUG_LINE, debug_mesh->indices, debug_mesh->num_indices);

	// Use a default shader for rendering UI primitives.
	mesh_set_shader(debug_mesh, res_get_shader("default-debug"));
}

void debug_shutdown(void)
{
	if (debug_mesh != NULL) {

		mesh_destroy(debug_mesh);
		debug_mesh = NULL;
	}
}

void debug_begin_frame(void)
{
	// Clear buffer (i.e. reset active line count).
	num_lines = 0;
}

void debug_end_frame(void)
{
	if (num_lines == 0) {
		return;
	}

	// Re-upload vertex data.
	mesh_refresh_vertices(debug_mesh);

	// Set the number of indices to render.
	debug_mesh->num_indices_to_render = 2 * num_lines;

	// Tell the renderer to draw the debug mesh.
	rsys_render_mesh(debug_mesh);
}

void debug_draw_line(vec3_t start, vec3_t end, colour_t colour)
{
	if (num_lines >= MAX_LINES) {

		log_message("Renderer", "Exceeded maximum debug lines!");
		return;
	}

	debug_mesh->debug_vertices[2 * num_lines + 0] = vertex_debug(start, colour);
	debug_mesh->debug_vertices[2 * num_lines + 1] = vertex_debug(end, colour);

	num_lines++;
}

void debug_draw_2d_line(vec2_t start, vec2_t end, colour_t colour)
{
	if (num_lines >= MAX_LINES) {

		log_message("Renderer", "Exceeded maximum debug lines!");
		return;
	}

	debug_mesh->debug_vertices[2 * num_lines + 0] = vertex_debug(vec3(start.x, start.y, 0), colour);
	debug_mesh->debug_vertices[2 * num_lines + 1] = vertex_debug(vec3(end.x, end.y, 0), colour);

	num_lines++;
}

void debug_draw_2d_rect(vec2_t min, vec2_t max, colour_t colour)
{
	if (num_lines > MAX_LINES - 4) {

		log_message("Renderer", "Exceeded maximum debug lines!");
		return;
	}

	debug_mesh->debug_vertices[2 * num_lines + 0] = vertex_debug(vec3(min.x, min.y, 0), colour);
	debug_mesh->debug_vertices[2 * num_lines + 1] = vertex_debug(vec3(min.x, max.y, 0), colour);
	debug_mesh->debug_vertices[2 * num_lines + 2] = vertex_debug(vec3(max.x, min.y, 0), colour);
	debug_mesh->debug_vertices[2 * num_lines + 3] = vertex_debug(vec3(max.x, max.y, 0), colour);
	debug_mesh->debug_vertices[2 * num_lines + 4] = vertex_debug(vec3(min.x, min.y, 0), colour);
	debug_mesh->debug_vertices[2 * num_lines + 5] = vertex_debug(vec3(max.x, min.y, 0), colour);
	debug_mesh->debug_vertices[2 * num_lines + 6] = vertex_debug(vec3(min.x, max.y, 0), colour);
	debug_mesh->debug_vertices[2 * num_lines + 7] = vertex_debug(vec3(max.x, max.y, 0), colour);

	num_lines += 4;
}
