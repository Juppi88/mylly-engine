#include "debug.h"
#include "mesh.h"
#include "rendersystem.h"
#include "resources/resources.h"
#include "io/log.h"
#include "core/memory.h"
#include "scene/object.h"
#include "scene/camera.h"
#include "math/math.h"
#include "math/quaternion.h"

// -------------------------------------------------------------------------------------------------

// Temporary structure for storing debug primitive data before transforming them with the camera.
typedef struct {

	enum { LINE, RECTANGLE, CIRCLE } type;
	colour_t colour;

	union {
		struct { vec3_t start; vec3_t end; } line;
		struct { vec3_t centre; vec2_t extents; } rectangle;
		struct { vec3_t origin; float radius; } circle;
	};
	
} debug_primitive_t;

// -------------------------------------------------------------------------------------------------

static const int MAX_PRIMITIVES = 1000; // Number of maximum visible primitives per layer
static const int MAX_LINES = 10000; // Number of maximum lines visible at once, per layer
static const int NUM_CIRCLE_SEGMENTS = 32; // A circle consists of 32 line segments

// Number of primitives added to each layer during the current frame
static int num_primitives_scene = 0;
static int num_primitives_overlay = 0;

// Debug primitives before being transformed to the camera
static debug_primitive_t primitives_scene[MAX_PRIMITIVES];
static debug_primitive_t primitives_overlay[MAX_PRIMITIVES];

// Number of lines added to each layer during the current frame
static int num_lines_scene = 0;
static int num_lines_overlay = 0;

// Meshes containing the debug primitives to be rendered
static mesh_t *debug_mesh_scene;
static mesh_t *debug_mesh_overlay;

// -------------------------------------------------------------------------------------------------

static bool debug_ensure_buffer_has_space(bool overlay);
static void debug_add_line_mesh(camera_t *camera, debug_primitive_t *primitive, bool overlay);
static void debug_add_rect_mesh(camera_t *camera, debug_primitive_t *primitive, bool overlay);
static void debug_add_circle_mesh(camera_t *camera, debug_primitive_t *primitive, bool overlay);

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
	// Destroy debug meshes.
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
	// Clear buffers (i.e. reset counters).
	num_primitives_scene = 0;
	num_primitives_overlay = 0;
	num_lines_scene = 0;
	num_lines_overlay = 0;
}

void debug_end_frame(camera_t *camera)
{
	if (camera == NULL) {
		return;
	}

	// Add 3D scene and overlay debug primitives into their own meshes.
	for (int layer = 0; layer <= 1; layer++) {

		bool is_overlay = (layer & 1);
		int num_primitives = (is_overlay ? num_primitives_overlay : num_primitives_scene);
		int num_lines;
		mesh_t *mesh;

		debug_primitive_t *primitives = (is_overlay ? primitives_overlay : primitives_scene);

		for (int i = 0; i < num_primitives; i++) {

			// A circle uses the most vertices, so make sure the buffer can fit at least a circle.
			if (num_lines_scene >= MAX_LINES - NUM_CIRCLE_SEGMENTS) {
				break;
			}

			switch (primitives[i].type) {

				case LINE:
					debug_add_line_mesh(camera, &primitives[i], is_overlay);
					break;

				case RECTANGLE:
					debug_add_rect_mesh(camera, &primitives[i], is_overlay);
					break;

				case CIRCLE:
					debug_add_circle_mesh(camera, &primitives[i], is_overlay);
					break;
			}
		}
		
		if (is_overlay) {
			mesh = debug_mesh_overlay;
			num_lines = num_lines_overlay;
		}
		else {
			mesh = debug_mesh_scene;
			num_lines = num_lines_scene;
		}

		// Re-upload changed vertex data to the GPU.
		mesh_refresh_vertices(mesh);

		// Set the number of indices to render to each mesh in order to minimize rendered indices.
		mesh->num_indices_to_render = 2 * num_lines;

		// Tell the renderer to draw the debug meshes.
		rsys_render_mesh(mesh, is_overlay);
	}
}

void debug_draw_line(vec3_t start, vec3_t end, colour_t colour, bool overlay)
{
	if (!debug_ensure_buffer_has_space(overlay)) {
		return;
	}

	debug_primitive_t *primitives;
	size_t num_primitives;

	if (overlay) {
		primitives = primitives_overlay;
		num_primitives = num_primitives_overlay++;
	}
	else {
		primitives = primitives_scene;
		num_primitives = num_primitives_scene++;
	}

	primitives[num_primitives].type = LINE;
	primitives[num_primitives].colour = colour;
	primitives[num_primitives].line.start = start;
	primitives[num_primitives].line.end = end;
}

void debug_draw_rect(vec3_t centre, vec2_t extents, colour_t colour, bool overlay)
{
	if (!debug_ensure_buffer_has_space(overlay)) {
		return;
	}

	debug_primitive_t *primitives;
	size_t num_primitives;

	if (overlay) {
		primitives = primitives_overlay;
		num_primitives = num_primitives_overlay++;
	}
	else {
		primitives = primitives_scene;
		num_primitives = num_primitives_scene++;
	}

	primitives[num_primitives].type = RECTANGLE;
	primitives[num_primitives].colour = colour;
	primitives[num_primitives].rectangle.centre = centre;
	primitives[num_primitives].rectangle.extents = extents;
}

void debug_draw_circle(vec3_t origin, float radius, colour_t colour, bool overlay)
{
	if (!debug_ensure_buffer_has_space(overlay)) {
		return;
	}

	debug_primitive_t *primitives;
	size_t num_primitives;

	if (overlay) {
		primitives = primitives_overlay;
		num_primitives = num_primitives_overlay++;
	}
	else {
		primitives = primitives_scene;
		num_primitives = num_primitives_scene++;
	}

	primitives[num_primitives].type = CIRCLE;
	primitives[num_primitives].colour = colour;
	primitives[num_primitives].circle.origin = origin;
	primitives[num_primitives].circle.radius = radius;
}

static bool debug_ensure_buffer_has_space(bool overlay)
{
	if ((overlay && num_primitives_overlay >= MAX_PRIMITIVES) ||
	    (!overlay && num_primitives_scene >= MAX_PRIMITIVES)) {

		log_message("Renderer", "Exceeded maximum debug primitives.");
		return false;
	}

	return true;
}

static void debug_add_line_mesh(camera_t *camera, debug_primitive_t *primitive, bool overlay)
{
	UNUSED(camera);

	vertex_debug_t *vertices;
	size_t base;

	// Select the appropriate mesh vertex array (either overlay or scene).
	if (overlay) {
		vertices = debug_mesh_overlay->debug_vertices;
		base = num_lines_overlay;

		num_lines_overlay += 1;
	}
	else {
		vertices = debug_mesh_scene->debug_vertices;
		base = num_lines_scene;

		num_lines_scene += 1;
	}

	// Line vertices do not need translating or rotating as the end points of the line segment
	// already are the vertices.
	vertices[2 * base + 0] = vertex_debug(primitive->line.start, primitive->colour);
	vertices[2 * base + 1] = vertex_debug(primitive->line.end, primitive->colour);
}

static void debug_add_rect_mesh(camera_t *camera, debug_primitive_t *primitive, bool overlay)
{
	vertex_debug_t *vertices;
	size_t base;

	// Select the appropriate mesh vertex array (either overlay or scene).
	if (overlay) {
		vertices = debug_mesh_overlay->debug_vertices;
		base = num_lines_overlay;

		num_lines_overlay += 4;
	}
	else {
		vertices = debug_mesh_scene->debug_vertices;
		base = num_lines_scene;

		num_lines_scene += 4;
	}

	// Calculate edge vertices for the rectangle as if it were at the origin.
	vec2_t extents = primitive->rectangle.extents;
	vec3_t min = vec3(-extents.x, -extents.y, 0);
	vec3_t max = vec3(extents.x, extents.y, 0);

	vec3_t edge_vertices[8] = {
		vec3(min.x, min.y, 0),
		vec3(min.x, max.y, 0),
		vec3(max.x, min.y, 0),
		vec3(max.x, max.y, 0),
		vec3(min.x, min.y, 0),
		vec3(max.x, min.y, 0),
		vec3(min.x, max.y, 0),
		vec3(max.x, max.y, 0)
	};

	quat_t rotation = obj_get_rotation(camera->parent);
	vec3_t centre = primitive->rectangle.centre;

	for (int i = 0; i < 8; i++) {

		vec3_t vertex = edge_vertices[i];

		// In case of 3D scene primitives, also rotate the vertices towards the camera.
		if (!overlay) {
			vertex = quat_rotate_vec3(rotation, vertex);
		}

		// Translate each vertex before assigning to the mesh.
		vertex = vec3_add(vertex, centre);

		vertices[2 * base + i] = vertex_debug(vertex, primitive->colour);
	}
}

static void debug_add_circle_mesh(camera_t *camera, debug_primitive_t *primitive, bool overlay)
{
	vertex_debug_t *vertices;
	size_t base;

	// Select the appropriate mesh vertex array (either overlay or scene).
	if (overlay) {
		vertices = debug_mesh_overlay->debug_vertices;
		base = num_lines_overlay;

		num_lines_overlay += NUM_CIRCLE_SEGMENTS;
	}
	else {
		vertices = debug_mesh_scene->debug_vertices;
		base = num_lines_scene;

		num_lines_scene += NUM_CIRCLE_SEGMENTS;
	}

	// Create an approximation of a circle with line segments.
	float radius = primitive->circle.radius;
	vec3_t origin = primitive->circle.origin;
	vec3_t vertex1 = vec3(radius, 0, 0);
	vec3_t vertex2;
	quat_t rotation = obj_get_rotation(camera->parent);

	for (int i = 0; i < NUM_CIRCLE_SEGMENTS; i++) {

		float segment = ((i + 1.0f) / NUM_CIRCLE_SEGMENTS) * 2 * PI;
		vertex2 = vec3(radius * cosf(segment), radius * sinf(segment), 0);

		vec3_t v1 = vertex1;
		vec3_t v2 = vertex2;

		if (!overlay) {

			// Rotate 3D scene primitives towards the camera before translating.
			v1 = quat_rotate_vec3(rotation, v1);
			v2 = quat_rotate_vec3(rotation, v2);
		}

		// Translate vertices before assigning them to the mesh.
		v1 = vec3_add(v1, origin);
		v2 = vec3_add(v2, origin);

		vertices[2 * base + 0] = vertex_debug(v1, primitive->colour);
		vertices[2 * base + 1] = vertex_debug(v2, primitive->colour);	
		
		base++;
		vertex1 = vertex2;
	}
}
