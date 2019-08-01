#include "renderer/renderer.h"
#include "extensions.h"
#include "framebuffer.h"
#include "renderer/vertex.h"
#include "renderer/texture.h"
#include "renderer/buffercache.h"
#include "renderer/mesh.h"
#include "io/log.h"
#include "platform/window.h"
#include "core/time.h"
#include "core/mylly.h"
#include "resources/resources.h"
#include <stdio.h>

// Include source code for default shaders.
#include "defaultshaders.c"

// -------------------------------------------------------------------------------------------------

// Device and OpenGL context
#ifdef _WIN32
static HDC context;
static HGLRC gl_context;
#else
static GLXContext gl_context;
#endif

// Currently used shader and texture object.
static GLuint active_shader = -1;
static GLuint active_texture = -1;
static GLuint active_normal_map = -1;

// Uniform arrays.
static mat_t matrix_array[NUM_MAT_UNIFORMS];
static vec4_t vector_array[NUM_VEC_UNIFORMS];
static int sampler_array[NUM_SAMPLER_UNIFORMS];

static int num_mesh_lights;
static mat_t light_array[MAX_LIGHTS_PER_MESH];

// Vertices for covering the entire screen.
static GLuint screen_vertices;
static GLuint screen_indices;

// Loading/splash screen FBOs.
static GLuint splash_screen_vertices;
static GLuint splash_screen_indices;

// Debug variables. Used to override normal rendering pipeline.
static gbuffer_component_t override_gbuffer_component = GBUFFER_NONE;

// -------------------------------------------------------------------------------------------------


static void rend_draw_mesh(rview_t *view, rmesh_t *mesh);

static void rend_set_active_material(shader_t *shader, texture_t *texture, texture_t *normal_map,
                                     rview_t *view, rmesh_t *mesh, vertex_type_t vertex_type);

static bool rend_bind_shader_attribute(shader_t *shader, int attr_type, GLint size, GLenum type,
                                       GLboolean normalized, GLsizei stride, const GLvoid *pointer);

static void rend_update_material_uniforms(shader_t *shader);
static void rend_update_uniforms(robject_t *parent_obj, rview_t *view, bool is_effect);
static void rend_commit_uniforms(shader_t *shader);
static void rend_clear_uniforms(void);

static void rend_draw_post_processing_effects(rview_t *view);
static void rend_draw_framebuffer_with_shader(int index, shader_t *shader,
                                              bool is_first_effect, bool override_buffer);

static void rend_set_blend_mode(int queue, bool post_processing);

// -------------------------------------------------------------------------------------------------

bool rend_initialize(void)
{
	if (gl_context != NULL) {
		return true;
	}

	// Create an OpenGL rendering context.	
#ifdef _WIN32
	
	// Get a handle for the main window and a device context for it.
	HWND window = (HWND)window_get_handle();
	context = GetDC(window);

	// Attempt to select and set an appropriate pixel format.
	PIXELFORMATDESCRIPTOR descriptor;

	descriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	descriptor.nVersion = 1;
	descriptor.dwFlags = (PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW);
	descriptor.iPixelType = PFD_TYPE_RGBA;
	descriptor.cColorBits = 24;
	descriptor.cDepthBits = 32;

	int format;
	format = ChoosePixelFormat(context, &descriptor);

	if (format == 0) {
		log_error("Renderer", "Could not create an OpenGL rendering context");
		return false;
	}

	// Set pixel format for the device context.
	SetPixelFormat(context, format, &descriptor);

	// Create an OpenGL rendering context.
	gl_context = wglCreateContext(context);

	if (gl_context == NULL) {
		log_error("Renderer", "Could not create an OpenGL rendering context");
		return false;
	}

	// Make the new rendering context the current one for the application.
	wglMakeCurrent(context, gl_context);

#else
	gl_context = glXCreateContext(window_get_display(), window_get_visual_info(), NULL, GL_TRUE);
 
	if (gl_context == NULL) {
		log_error("Renderer", "Could not create an OpenGL rendering context");
		return false;
	}

	glXMakeCurrent(window_get_display(), window_get_handle(), gl_context);
#endif

	if (!glext_initialize()) {

		log_error("Renderer", "Could not load all essential OpenGL extensions");
		return false;	
	}

	// Create two rotating framebuffers for deferred rendering and post-processing, as well as one
	// buffer for the geometry pass.
	// TODO: Resize the buffers every time rendering resolution changes!
	if (!rend_fb_initialize()) {

		log_error("Renderer", "Could not create framebuffers.");
		return false;
	}

	// Create a list of vertices covering the entire screen. This is used to render post-processing
	// effects.
	vertex_ui_t vertices[] = {
		vertex_ui(vec2(-1, -1), vec2(0, 0), COL_WHITE),
		vertex_ui(vec2(1, -1),  vec2(1, 0), COL_WHITE),
		vertex_ui(vec2(-1, 1),  vec2(0, 1), COL_WHITE),
		vertex_ui(vec2(1, 1),   vec2(1, 1), COL_WHITE)
	};

	screen_vertices = rend_generate_buffer();
	rend_upload_buffer_data(screen_vertices, vertices, sizeof(vertices), false, true);

	vindex_t indices[] = { 0, 1, 2, 3 };

	screen_indices = rend_generate_buffer();
	rend_upload_buffer_data(screen_indices, indices, sizeof(indices), true, true);

	// Generate buffer objects for the splash screen.
	glGenBuffersARB(1, &splash_screen_vertices);
	glGenBuffersARB(1, &splash_screen_indices);

	log_message("Renderer", "Renderer initialization complete.");
	log_message("Renderer", "GPU: %s", glGetString(GL_RENDERER));
	log_message("Renderer", "OpenGL version: %s", glGetString(GL_VERSION));
	log_message("Renderer", "GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glDisable(GL_DEPTH_TEST);

	return true;
}

void rend_shutdown(void)
{
	// Destroy framebuffers.
	rend_fb_shutdown();

	glDeleteBuffersARB(1, &screen_vertices);
	glDeleteBuffersARB(1, &screen_indices);

	glDeleteBuffersARB(1, &splash_screen_vertices);
	glDeleteBuffersARB(1, &splash_screen_indices);
	
	// Destroy the rendering context.
#ifdef _WIN32
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(gl_context);

	context = NULL;
	gl_context = NULL;
#else
	glXMakeCurrent(window_get_display(), None, NULL);
	glXDestroyContext(window_get_display(), gl_context);

	gl_context = NULL;
#endif
}

void rend_begin_draw(void)
{
	// Clear the framebuffers.
	rend_clear_fbs();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//glDisable(GL_CULL_FACE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glEnableClientState(GL_VERTEX_ARRAY);

	rend_clear_uniforms();
}

void rend_end_draw(void)
{
#ifdef _WIN32
	SwapBuffers(context);
#else
	glXSwapBuffers(window_get_display(), window_get_handle());
#endif

	glDisableClientState(GL_VERTEX_ARRAY);
}

void rend_draw_views(rview_t *first_view)
{
	// Load a dummy shader for drawing contents of a framebuffer onto a screen unaltered.
	shader_t *dummy = res_get_shader("default-draw-framebuffer");

	// Create a temporary list from which to render the views.
	list_t(rview_t) views;
	list_init(views);
	
	views.first = first_view;

	// Ensure the geometry buffer is bound before drawing any actual geometry.
	rend_bind_fb(FB_GEOMETRY);

	rview_t *view;
	rmesh_t *mesh;

	// TODO: Group meshes by a) shader/texture and b) the vertex buffer used, in order to minimize
	// draw calls. This could be done in render system before passing the renderer data to render.

	// TODO: See rend_draw_ui_view below - don't issue any draw call as long as the objects are
	// using the same buffer object!

	int meshes_drawn = 0;

	// Process render queues.
	for (int queue = 0; queue < NUM_QUEUES; queue++) {

		// Clear depth buffer after background and before foreground queues.
		if (meshes_drawn != 0) {
		
			switch (queue) {

			case QUEUE_BACKGROUND:
				break;

			case QUEUE_BACKGROUND + 1:
			case QUEUE_OVERLAY:

				glClear(GL_DEPTH_BUFFER_BIT);
				meshes_drawn = 0;
				break;

			default:
				break;

			}
		}

		// Draw meshes from each view.
		list_foreach(views, view) {

			// Apply appropriate blending mode for the queue.
			rend_set_blend_mode(queue, false);

			// Draw the meshes of this queue.
			list_foreach(view->meshes[queue], mesh) {

				rend_draw_mesh(view, mesh);
				meshes_drawn++;
			}
		}

		// Handle deferred lighting and post processing effects.
		if (queue == QUEUE_GEOMETRY) {

			// TODO: Draw deferred lighting after geometry queue! Draw it as a post process
			// effect, then copy the result color buffer back to the geometry buffer for the
			// transparent queue.
		}
		else if (queue == QUEUE_TRANSPARENT) {

			// Disable blending for render textures.
			rend_set_blend_mode(queue, true);

			// TODO: Draw post processing effects per-view! This will require a separate framebuffer
			// for each view/camera.
			view = first_view;

			bool post_process = (view->post_processing_effects.count != 0);

			if (post_process) {

				// Draw post process effects. The last effect will switch to the screen's
				// framebuffer, and anything after that (foreground queue/UI) is drawn there.
				rend_draw_post_processing_effects(view);
			}
			else {

				// No post process effects to render. Render the contents of the geometry buffer
				// onto the screen with a dummy shader.
				rend_update_uniforms(NULL, view, true);

				rend_bind_fb(FB_SCREEN);
				rend_draw_framebuffer_with_shader(FB_GEOMETRY, dummy, true, false);
			}
		}
	}

	// Debug override. Draw the contents of the raw G-buffer onto the screen.
	if (override_gbuffer_component != GBUFFER_NONE) {

		rend_bind_fb(FB_SCREEN);
		rend_draw_framebuffer_with_shader(FB_GEOMETRY, dummy, true, true);
	}
}
/*
void rend_draw_ui_view(rview_ui_t *view)
{
	// Disable depth testing for UI.
	glDisable(GL_DEPTH_TEST);

	rmesh_ui_t *mesh;

	// Bind UI buffers.
	bufcache_t *ui_buffer = bufcache_get(BUFIDX_UI);

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, ui_buffer->vertex_buffer.object);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, ui_buffer->index_buffer.object);

	shader_t *shader = NULL;
	texture_t *texture = NULL;
	robject_t *parent = NULL;
	uint32_t offset = 0;
	uint32_t length = 0;

	list_foreach(view->meshes, mesh) {
		
		// Check whether we need a new draw call.
		if (shader != mesh->shader ||
			texture != mesh->texture ||
			parent != mesh->parent) {

			// Flush indices.
			if (length != 0) {
				glDrawRangeElementsARB(GL_TRIANGLES, offset, offset + length, length,
                                       GL_UNSIGNED_SHORT,
				                       (const GLvoid *)(sizeof(vindex_t) * offset));

				offset += length;
				length = 0;
			}

			shader = mesh->shader;
			texture = mesh->texture;
			parent = mesh->parent;

			rend_set_active_material(shader, texture, parent);
		}

		// Continue until we need to use a different shader or a texture.
		length += BUFFER_GET_SIZE(mesh->handle_indices) / sizeof(vindex_t);
	}

	// Draw remaining elements.
	if (length != 0) {
		glDrawRangeElementsARB(GL_TRIANGLES, offset, offset + length, length,
		                       GL_UNSIGNED_SHORT, (const GLvoid *)(sizeof(vindex_t) * offset));
	}
}
*/
static void rend_draw_mesh(rview_t *view, rmesh_t *mesh)
{
	// Bind the meshes vertex buffer and set vertex data pointers.
	if (mesh->handle_vertices != 0 && mesh->handle_indices != 0) {

		// Mesh uses a buffer handle (i.e. only a part of the buffer is drawn).
		bufcache_t *buffer = bufcache_get(BUFFER_GET_INDEX(mesh->handle_vertices));

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, buffer->vertex_buffer.object);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, buffer->index_buffer.object);
	}
	else if (mesh->vertices != NULL && mesh->indices != NULL) {

		// Mesh uses a buffer cache object (i.e. the entire buffer is drawn).
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->vertices->vbo);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->indices->vbo);
	}
	else {

		// Mesh does not contain vertex data, unable to render.
		return;
	}

	// Check whether the shader or texture needs to be changed. Bind vertex attributes.
	rend_set_active_material(mesh->shader, mesh->texture, mesh->normal_map,
	                         view, mesh, mesh->vertex_type);

	// Draw the triangles of the mesh.
	if (mesh->vertices != NULL && mesh->indices != NULL) {
		glDrawElements(GL_TRIANGLES, mesh->indices->count, GL_UNSIGNED_SHORT, 0);
	}
	else {

		uint32_t offset = BUFFER_GET_OFFSET(mesh->handle_indices) / sizeof(vindex_t);
		uint32_t length = BUFFER_GET_SIZE(mesh->handle_indices) / sizeof(vindex_t);

		GLenum mode = (
			BUFFER_GET_INDEX(mesh->handle_vertices) == BUFIDX_DEBUG_LINE ?
			GL_LINES :
			GL_TRIANGLES
		);

		glDrawRangeElementsARB(mode, offset, offset + length, length,
		                       GL_UNSIGNED_SHORT, (const GLvoid *)(sizeof(vindex_t) * offset));
	}
}

static void rend_set_active_material(shader_t *shader, texture_t *texture, texture_t *normal_map,
                                     rview_t *view, rmesh_t *mesh, vertex_type_t vertex_type)
{
	if (shader == NULL || mesh == NULL) {
		return;
	}
	
	// Select the active shader.
	GLuint shader_id = shader->program;

	if (shader_id != active_shader) {

		glUseProgram(shader_id);
		active_shader = shader_id;
	}

	// Update custom uniforms.
	if (shader->has_updated_uniforms) {
		rend_update_material_uniforms(shader);
	}

	// Select the active texture.
	GLuint texture_id = (texture != NULL ? texture->gpu_texture : 0);

	if (texture_id != active_texture) {

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_id);

		active_texture = texture_id;
	}

	// Select normal map.
	GLuint normal_map_id = (normal_map != NULL ? normal_map->gpu_texture : 0);

	if (normal_map_id != active_normal_map) {

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normal_map_id);
		active_normal_map = normal_map_id;
	}

	// Disable vertex attributes to avoid using them when there is no such data available.
	for (int i = 0; i < NUM_SHADER_ATTRIBUTES; i++) {
		glDisableVertexAttribArray(i);
	}

	// Disable depth write for particles.
	glDepthMask(vertex_type != VERTEX_PARTICLE);

	// Set pointers to vertex attributes.
	switch (vertex_type) {

	// Normal vertex attributes.
	case VERTEX_NORMAL:

		rend_bind_shader_attribute(shader, ATTR_VERTEX, 3, GL_FLOAT, GL_FALSE,
                                   sizeof(vertex_t), (void *)offsetof(vertex_t, pos));

		rend_bind_shader_attribute(shader, ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE,
                                   sizeof(vertex_t), (void *)offsetof(vertex_t, normal));

		rend_bind_shader_attribute(shader, ATTR_TANGENT, 3, GL_FLOAT, GL_FALSE,
                                   sizeof(vertex_t), (void *)offsetof(vertex_t, tangent));

		rend_bind_shader_attribute(shader, ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE,
                                   sizeof(vertex_t), (void *)offsetof(vertex_t, uv));

		break;

	// Debug vertex attributes.
	case VERTEX_DEBUG:

		rend_bind_shader_attribute(shader, ATTR_VERTEX, 3, GL_FLOAT, GL_FALSE,
		                           sizeof(vertex_debug_t), (void *)offsetof(vertex_debug_t, pos));

		rend_bind_shader_attribute(shader, ATTR_COLOUR, 4, GL_UNSIGNED_BYTE, GL_TRUE,
		                           sizeof(vertex_debug_t), (void *)offsetof(vertex_debug_t, colour));

		break;

	// Bind vertex attributes.
	case VERTEX_UI:

		rend_bind_shader_attribute(shader, ATTR_VERTEX, 2, GL_FLOAT, GL_FALSE,
	                               sizeof(vertex_ui_t), (void *)offsetof(vertex_ui_t, pos));

		rend_bind_shader_attribute(shader, ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE,
	                               sizeof(vertex_ui_t), (void *)offsetof(vertex_ui_t, uv));

		rend_bind_shader_attribute(shader, ATTR_COLOUR, 4, GL_UNSIGNED_BYTE, GL_TRUE,
	                               sizeof(vertex_ui_t), (void *)offsetof(vertex_ui_t, colour));

		break;
	
	// Particle vertex attributes.
	case VERTEX_PARTICLE:
		
		rend_bind_shader_attribute(shader, ATTR_VERTEX, 3, GL_FLOAT, GL_FALSE,
                        sizeof(vertex_particle_t), (void *)offsetof(vertex_particle_t, pos));

		rend_bind_shader_attribute(shader, ATTR_CENTRE, 3, GL_FLOAT, GL_FALSE,
                        sizeof(vertex_particle_t), (void *)offsetof(vertex_particle_t, centre));

		rend_bind_shader_attribute(shader, ATTR_EMIT_POSITION, 4, GL_FLOAT, GL_FALSE,
                        sizeof(vertex_particle_t), (void *)offsetof(vertex_particle_t, emit_position));

		rend_bind_shader_attribute(shader, ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE,
                        sizeof(vertex_particle_t), (void *)offsetof(vertex_particle_t, uv));

		rend_bind_shader_attribute(shader, ATTR_COLOUR, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                        sizeof(vertex_particle_t), (void *)offsetof(vertex_particle_t, colour));

		rend_bind_shader_attribute(shader, ATTR_ROTATION, 1, GL_FLOAT, GL_FALSE,
                        sizeof(vertex_particle_t), (void *)offsetof(vertex_particle_t, rotation));

		rend_bind_shader_attribute(shader, ATTR_SIZE, 1, GL_FLOAT, GL_FALSE,
                        sizeof(vertex_particle_t), (void *)offsetof(vertex_particle_t, size));

		break;
	}

	// Set per-draw shader uniforms.
	// TODO: See if this can be optimized by setting per-view matrices separately.
	rend_update_uniforms(mesh->parent, view, false);

	// Copy lighting info.
	if (shader_is_affected_by_light(shader)) {

		num_mesh_lights = mesh->num_lights;

		for (uint32_t i = 0; i < mesh->num_lights; i++) {
			mat_cpy(&light_array[i], &mesh->lights[i]->shader_params);
		}
	}

	// Commit uniforms to shader program.
	rend_commit_uniforms(shader);
}

static bool rend_bind_shader_attribute(shader_t *shader, int attr_type, GLint size, GLenum type,
                                       GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
	if (!shader_uses_attribute(shader, attr_type)) {
		return false;
	}

	int attribute = shader_get_attribute(shader, attr_type);

	glEnableVertexAttribArray(attribute);
	glVertexAttribPointer(attribute, size, type, normalized, stride, pointer);

	return true;
}

vbindex_t rend_generate_buffer(void)
{
	GLuint vbo;

	glGenBuffersARB(1, &vbo);
	return vbo;
}

void rend_destroy_buffer(vbindex_t vbo)
{
	if (vbo != 0) {
		glDeleteBuffersARB(1, &vbo);
	}
}

void rend_upload_buffer_data(vbindex_t vbo, void *data, size_t size, bool is_index, bool is_static)
{
	GLenum target = (is_index ? GL_ELEMENT_ARRAY_BUFFER_ARB : GL_ARRAY_BUFFER_ARB);
	GLenum usage = (is_static ? GL_STATIC_DRAW_ARB : GL_DYNAMIC_DRAW_ARB);

	glBindBufferARB(target, vbo);
	glBufferDataARB(target, size, data, usage);
}

void rend_update_buffer_subdata(vbindex_t vbo, const void *data, size_t offset, size_t size,
	                            bool is_index)
{
	GLenum target = (is_index ? GL_ELEMENT_ARRAY_BUFFER_ARB : GL_ARRAY_BUFFER_ARB);

	glBindBufferARB(target, vbo);
	glBufferSubDataARB(target, offset, size, data);
}

shader_object_t rend_create_shader(SHADER_TYPE type, const char **lines, size_t num_lines,
								   const char **compiler_log)
{
	GLenum shader_type;

	// Set the type of the shader and prepend the shader version to the source.
	const char *defines_format = "#version %s\n#define %s\n";
	char defines[100];

	switch (type) {

	case SHADER_FRAGMENT:
		snprintf(defines, sizeof(defines), defines_format, "130", "FRAGMENT_SHADER");
		shader_type = GL_FRAGMENT_SHADER;
		break;

	default:
		snprintf(defines, sizeof(defines), defines_format, "130", "VERTEX_SHADER");
		shader_type = GL_VERTEX_SHADER;
		break;
	}

	// Create a shader object.
	GLuint shader = glCreateShader(shader_type);

	// Prepend the defines to the list of source code lines.
	// The resource loader should have left the first line empty for this.
	lines[0] = defines;

	// Compile the source code with the defines set above.
	glShaderSource(shader, num_lines, lines, NULL);
	glCompileShader(shader);

	// Get the compiler log.
	if (compiler_log != NULL) {

		static char log[1024];

		glGetShaderInfoLog(shader, sizeof(log), NULL, log);
		*compiler_log = log;
	}

	// Check whether the shader was compiled successfully.
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE) {

		// Something went wrong, invalidate the shader object.
		glDeleteShader(shader);
		shader = 0;
	}

	// Remove the defines string from the list since it is not on the heap.
	lines[0] = NULL;

	return shader;
}

shader_program_t rend_create_shader_program(shader_object_t *shaders, size_t num_shaders)
{
	if (shaders == NULL || num_shaders == 0) {
		return 0;
	}

	// Create a program object.
	GLuint program = glCreateProgram();

	// Link the shaders into a shader program.
	for (uint32_t i = 0; i < num_shaders; ++i) {
		glAttachShader(program, shaders[i]);
	}

	glLinkProgram(program);

	return program;
}

void rend_destroy_shader(shader_object_t shader)
{
	if (shader != 0) {
		glDeleteShader(shader);
	}
}

void rend_destroy_shader_program(shader_program_t program)
{
	if (program != 0) {
		glDeleteProgram(program);
	}
}

int rend_get_program_uniform_location(shader_program_t program, const char *name)
{
	if (program != 0) {
		return glGetUniformLocation(program, name);
	}

	return -1;
}

int rend_get_program_program_attribute_location(shader_program_t program, const char *name)
{
	if (program != 0) {
		return glGetAttribLocation(program, name);
	}

	return -1;
}

const char *rend_get_default_shader_source(default_shader_t shader)
{
	switch (shader) {

		case DEFAULT_SHADER_MAIN:
			return default_shader_source;

		case DEFAULT_SHADER_SPLASHSCREEN:
			return splash_shader_source;

		case DEFAULT_SHADER_DRAW_FRAMEBUFFER:
			return draw_fb_shader_source;
	}
}

const char *rend_get_splash_shader_source(void)
{
	return splash_shader_source;
}

texture_name_t rend_generate_texture(void *image, size_t width, size_t height,
                                     TEX_FORMAT fmt, TEX_FILTER filter)
{
	// Generate a texture name.
	GLuint texture;
	glGenTextures(1, &texture);

	// Upload the texture to the GPU.
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);

	// Select a filter for interpolating the texture.
	int filter_param = (filter == TEX_FILTER_POINT ? GL_NEAREST : GL_LINEAR);

	int internal_format;
	int image_format;

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_param);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_param);

	switch (fmt) {

		case TEX_FORMAT_GRAYSCALE:

			// Convert grayscale textures into 4-channel texture where the only colour is red.
			internal_format = GL_RED;
			image_format = GL_RED;
			break;

		case TEX_FORMAT_RGB:

			// Assume 24bit RGB texture by default.
			internal_format = GL_RGBA;
			image_format = GL_RGB;
			break;

		default:

			// Assume 32bit RGBA texture by default.
			internal_format = GL_RGBA;
			image_format = GL_RGBA;
			break;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0,
	             image_format, GL_UNSIGNED_BYTE, image);

	return texture;
}

void rend_delete_texture(texture_name_t texture)
{
	glDeleteTextures(1, &texture);
}

void rend_draw_splash_screen(texture_t *texture, shader_t *shader, colour_t background)
{
	vec4_t colour = col_to_vec4(background);

	// Clear the entire screen to the background colour.
	glClearColor(colour.x, colour.y, colour.z, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	if (texture != NULL && shader != NULL) {

		// Setup splash screen logo vertices.
		struct splash_vertex_t {
			vec2_t vertex;
			vec2_t uv;
		};

		// The layout is based on a virtual 1080p screen.
		uint16_t scr_w, scr_h;
		mylly_get_resolution(&scr_w, &scr_h);

		float aspect = 1.0f * scr_w / scr_h;

		float w = 0.5f * texture->width / (aspect * 1080);
		float h = 0.5f * texture->height / 1080;

		struct splash_vertex_t vertices[4] = {
			{ vec2(-1 * w, -1 * h), vec2(0, 0) },
			{ vec2(1 * w, -1 * h),  vec2(1, 0) },
			{ vec2(-1 * w, 1 * h),  vec2(0, 1) },
			{ vec2(1 * w, 1 * h),   vec2(1, 1) }
		};

		vindex_t indices[] = { 0, 1, 2, 3 };

		glEnableClientState(GL_VERTEX_ARRAY);

		// Upload logo vertex data to the GPU.
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, splash_screen_vertices);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertices), vertices, GL_STATIC_DRAW_ARB);

		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, splash_screen_indices);
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, sizeof(indices), indices, GL_STATIC_DRAW_ARB);

		// Use logo shader program and texture.
		glUseProgram(shader != NULL ? shader->program : 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture != NULL ? texture->gpu_texture : 0);

		GLuint attr_vertex = 0, attr_texcoord = 0;

		if (shader != NULL) {

			// Setup shader program uniforms and vertex attributes.
			glUniform1i(glGetUniformLocation(shader->program, "Texture"), 0);
			glUniform4fv(glGetUniformLocation(shader->program, "Colour"), 1, &colour.x);

			attr_vertex   = shader_get_attribute(shader, ATTR_VERTEX);
			attr_texcoord = shader_get_attribute(shader, ATTR_TEXCOORD);

			glEnableVertexAttribArray(attr_vertex);
			glEnableVertexAttribArray(attr_texcoord);

			glVertexAttribPointer(
				attr_vertex, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]),
				(void *)offsetof(struct splash_vertex_t, vertex));

			glVertexAttribPointer(
				attr_texcoord, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]),
				(void *)offsetof(struct splash_vertex_t, uv));
		}

		// Draw the framebuffer's contents into a screen sized quad.
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);

		glDisableVertexAttribArray(attr_vertex);
		glDisableVertexAttribArray(attr_texcoord);
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	// Swap buffers to draw immediately.
#ifdef _WIN32
	SwapBuffers(context);
#else
	glXSwapBuffers(window_get_display(), window_get_handle());
#endif
}

static void rend_update_material_uniforms(shader_t *shader)
{
	if (!shader->has_updated_uniforms) {
		return;
	}

	for (uint32_t i = 0; i < shader->material_uniforms.count; i++) {

		shader_uniform_t *uniform = &shader->material_uniforms.items[i];

		switch (uniform->type) {

			case UNIFORM_TYPE_INT:
				glUniform1i(uniform->position, uniform->value.i);
				break;

			case UNIFORM_TYPE_FLOAT:
				glUniform1f(uniform->position, uniform->value.f);
				break;

			case UNIFORM_TYPE_VECTOR4:
			case UNIFORM_TYPE_COLOUR:
				glUniform4fv(uniform->position, 1, &uniform->value.vec.x);
				break;

			default:
				break;
		}
	}

	shader->has_updated_uniforms = false;
}

static void rend_update_uniforms(robject_t *parent_obj, rview_t *view, bool is_effect)
{
	if (parent_obj != NULL) {
		matrix_array[UNIFORM_MAT_MVP] = parent_obj->mvp;
		matrix_array[UNIFORM_MAT_MODEL] = parent_obj->matrix;
	}

	matrix_array[UNIFORM_MAT_VIEW] = view->view;
	matrix_array[UNIFORM_MAT_VIEW_INV] = view->view_inv;
	matrix_array[UNIFORM_MAT_PROJECTION] = view->projection;
	matrix_array[UNIFORM_MAT_PROJECTION_INV] = view->projection_inv;
	matrix_array[UNIFORM_MAT_VIEWPROJ] = view->view_projection;
	matrix_array[UNIFORM_MAT_VIEWPROJ_INV] = view->view_projection_inv;

	vector_array[UNIFORM_VEC_VIEW_POSITION] = view->view_position;
	vector_array[UNIFORM_VEC_TIME] = get_shader_time();
	vector_array[UNIFORM_VEC_COLOUR] = view->ambient_light;

	uint16_t width, height;
	mylly_get_resolution(&width, &height);
	vector_array[UNIFORM_VEC_SCREEN] = vec4(width, height, view->near, view->far);

	sampler_array[UNIFORM_SAMPLER_MAIN] = 0;
	sampler_array[UNIFORM_SAMPLER_NORMAL] = 1;
	sampler_array[UNIFORM_SAMPLER_DEPTH] = (is_effect ? 2 : -1);
}

static void rend_commit_uniforms(shader_t *shader)
{
	if (shader->matrix_array >= 0) {
		glUniformMatrix4fv(shader->matrix_array, NUM_MAT_UNIFORMS, false, &matrix_array[0].col[0][0]);
	}
	if (shader->vector_array >= 0) {
		glUniform4fv(shader->vector_array, NUM_VEC_UNIFORMS, &vector_array[0].vec[0]);
	}
	if (shader->sampler_array >= 0) {
		glUniform1iv(shader->sampler_array, NUM_SAMPLER_UNIFORMS, &sampler_array[0]);
	}
	
	if (shader_is_affected_by_light(shader)) {
	
		if (shader->light_array >= 0 && num_mesh_lights > 0) {
			glUniformMatrix4fv(shader->light_array, num_mesh_lights, false, &light_array[0].col[0][0]);
		}
		if (shader->num_lights_position >= 0) {
			glUniform1i(shader->num_lights_position, num_mesh_lights);
		}
	}
}

static void rend_clear_uniforms(void)
{
	for (int i = 0; i < NUM_MAT_UNIFORMS; i++) {
		matrix_array[i] = mat_identity();
	}

	for (int i = 0; i < NUM_VEC_UNIFORMS; i++) {
		vector_array[i] = vec4_zero();
	}

	for (int i = 0; i < NUM_SAMPLER_UNIFORMS; i++) {
		sampler_array[i] = -1;
	}

	for (int i = 0; i < MAX_LIGHTS_PER_MESH; i++) {
		light_array[i] = mat_identity();
	}

	num_mesh_lights = 0;
}

static void rend_draw_post_processing_effects(rview_t *view)
{
	// Update relevant uniform arrays.
	rend_update_uniforms(NULL, view, true);

	// Apply each post processing effect in order.
	for (uint32_t i = 0, c = view->post_processing_effects.count; i < c; i++) {

		if (i < c - 1) {
			// Bind the framebuffer not used by the previous render pass.
			rend_bind_fb(1 + i & 1);
		}
		else {
			// Last effect, bind back to the screen's framebuffer.
			rend_bind_fb(FB_SCREEN);
		}
		
		// Render the contents of the previous framebuffer into the next one (or the
		// screen if this is the last effect).
		shader_t *effect = view->post_processing_effects.items[i];
		rend_draw_framebuffer_with_shader(i & 1, effect, i == 0, false);
	}
}

static void rend_draw_framebuffer_with_shader(int index, shader_t *shader,
                                              bool is_first_effect, bool override_buffer)
{
	// Switch to the post-processing shader.
	glUseProgram(shader->program);
	active_shader = shader->program;

	// Select the framebuffer textures. Normal and depth textures are fetched from the geometry pass
	// framebuffer.
	const gl_framebuffer_t *framebuffer = rend_get_fb(index);
	const gl_framebuffer_t *geometry_buffer = rend_get_fb(FB_GEOMETRY);

	// Colour texture. The colour texture can be overriden with other G-buffer components for
	// debugging purposes.
	glActiveTexture(GL_TEXTURE0);

	if (!override_buffer) {

		glBindTexture(GL_TEXTURE_2D, (is_first_effect ? geometry_buffer->colour : framebuffer->colour));
		active_texture = framebuffer->colour;
	}
	else {

		switch (override_gbuffer_component) {

			case GBUFFER_NORMAL:
				glBindTexture(GL_TEXTURE_2D, geometry_buffer->normal);
				active_texture = geometry_buffer->normal;
				break;

			case GBUFFER_DEPTH:
				glBindTexture(GL_TEXTURE_2D, geometry_buffer->depth);
				active_texture = geometry_buffer->depth;
				break;

			default:
				glBindTexture(GL_TEXTURE_2D, geometry_buffer->colour);
				active_texture = geometry_buffer->colour;
				break;
		}
	}

	// Normal texture
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, geometry_buffer->normal);

	// Depth texture
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, geometry_buffer->depth);

	// Update custom uniforms.
	if (shader->has_updated_uniforms) {
		rend_update_material_uniforms(shader);
	}

	// Disable all vertex attributes.
	for (int i = 0; i < NUM_SHADER_ATTRIBUTES; i++) {
		glDisableVertexAttribArray(i);
	}

	// Bind vertex buffers.
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, screen_vertices);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, screen_indices);
	
	// Bind vertex attributes. Post-processing shaders use the UI vertex format.
	rend_bind_shader_attribute(shader, ATTR_VERTEX, 2, GL_FLOAT, GL_FALSE,
	                           sizeof(vertex_ui_t), (void *)offsetof(vertex_ui_t, pos));

	rend_bind_shader_attribute(shader, ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE,
	                           sizeof(vertex_ui_t), (void *)offsetof(vertex_ui_t, uv));

	rend_bind_shader_attribute(shader, ATTR_COLOUR, 4, GL_UNSIGNED_BYTE, GL_TRUE,
	                           sizeof(vertex_ui_t), (void *)offsetof(vertex_ui_t, colour));

	// Apply relevant uniform arrays.
	if (shader->vector_array >= 0) {
		glUniform4fv(shader->vector_array, NUM_VEC_UNIFORMS, &vector_array[0].vec[0]);
	}
	if (shader->sampler_array >= 0) {
		glUniform1iv(shader->sampler_array, NUM_SAMPLER_UNIFORMS, &sampler_array[0]);
	}

	rend_commit_uniforms(shader);

	// Draw the framebuffer's contents into a screen sized quad.
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, 0);
}

static void rend_set_blend_mode(int queue, bool post_processing)
{
	if (post_processing ||
		queue == QUEUE_BACKGROUND ||
		queue == QUEUE_GEOMETRY) {

		// Disable blending for objects in the background and non-transparent geometry.
		glDisable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ZERO);
	}
	else {

		// Enable blending for everything that is or could be transparent.
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

void rend_override_draw_gbuffer(gbuffer_component_t buffer)
{
	override_gbuffer_component = buffer;
}
