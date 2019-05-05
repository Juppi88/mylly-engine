#include "renderer/renderer.h"
#include "extensions.h"
#include "renderer/vertex.h"
#include "renderer/texture.h"
#include "renderer/buffercache.h"
#include "io/log.h"
#include "platform/window.h"
#include "core/time.h"
#include "core/mylly.h"
#include <stdio.h>

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

// Framebuffers for post processing.
static GLuint framebuffer_vertices;
static GLuint framebuffers[2];
static GLuint framebuffer_textures[2];
static GLuint depth_buffers[2];

// -------------------------------------------------------------------------------------------------

// The source code for a default GLSL shader which renders everything in purple.
// Used when no valid shaders are available.
static const char *default_shader_source =

"uniform mat4 MatrixArr[1];\n"
"\n"
"#if defined(VERTEX_SHADER)\n"
"\n"
"attribute vec3 Vertex;\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = MatrixArr[0] * vec4(Vertex, 1.0);\n"
"}\n"
"\n"
"#elif defined(FRAGMENT_SHADER)\n"
"\n"
"void main()\n"
"{\n"
"	gl_FragColor = vec4(1, 0, 1, 1);\n"
"}\n"
"\n"
"#endif\n";

// -------------------------------------------------------------------------------------------------

static void rend_draw_mesh(rview_t *view, rmesh_t *mesh);

static void rend_set_active_material(shader_t *shader, texture_t *texture, texture_t *normal_map,
                                     rview_t *view, rmesh_t *mesh, vertex_type_t vertex_type);

static bool rend_bind_shader_attribute(shader_t *shader, int attr_type, GLint size, GLenum type,
                                       GLboolean normalized, GLsizei stride, const GLvoid *pointer);

static void rend_update_material_uniforms(shader_t *shader);
static void rend_commit_uniforms(shader_t *shader);
static void rend_clear_uniforms(void);

static bool rend_create_framebuffers(void);
static void rend_destroy_framebuffers(void);
static void rend_draw_post_processing_effects(rview_t *view);
static void rend_draw_framebuffer_with_shader(shader_t *shader, int fb_index);

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

	// Create two framebuffers for rendering post processing effects.
	// TODO: Resize the buffers every time rendering resolution changes!
	if (!rend_create_framebuffers()) {

		log_error("Renderer", "Could not create framebuffers.");
		return false;
	}

	log_message("Renderer", "Renderer initialization complete.");
	log_message("Renderer", "GPU: %s", glGetString(GL_RENDERER));
	log_message("Renderer", "OpenGL version: %s", glGetString(GL_VERSION));
	log_message("Renderer", "GLSL version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glEnable(GL_DEPTH_TEST);

	return true;
}

void rend_shutdown(void)
{
	// Destroy framebuffers.
	rend_destroy_framebuffers();
	
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
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	
	// Clear the framebuffers.
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[1]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Clear hardware buffer.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
	glEnable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//glDisable(GL_CULL_FACE);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glEnableClientState(GL_VERTEX_ARRAY);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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
	// Create a temporary list from which to render the views.
	list_t(rview_t) views;
	list_init(views);
	
	views.first = first_view;

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

			bool post_process = (view->post_processing_effects.count != 0);
			
			// Bind the post processing framebuffer if the view defines any post processing effects.
			glBindFramebuffer(GL_FRAMEBUFFER, (post_process ? framebuffers[0] : 0));

			list_foreach(view->meshes[queue], mesh) {

				rend_draw_mesh(view, mesh);
				meshes_drawn++;
			}

			// Render the view again with post processing.
			if (post_process) {
				rend_draw_post_processing_effects(view);
			}
		}
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

		rend_bind_shader_attribute(shader, ATTR_TEXCOORD, 2, GL_FLOAT, GL_FALSE,
                        sizeof(vertex_particle_t), (void *)offsetof(vertex_particle_t, uv));

		rend_bind_shader_attribute(shader, ATTR_COLOUR, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                        sizeof(vertex_particle_t), (void *)offsetof(vertex_particle_t, colour));

		rend_bind_shader_attribute(shader, ATTR_SIZE, 1, GL_FLOAT, GL_FALSE,
                        sizeof(vertex_particle_t), (void *)offsetof(vertex_particle_t, size));

		break;
	}

	// Set per-draw shader uniforms.
	// TODO: See if this can be optimized by setting per-view matrices separately.
	matrix_array[UNIFORM_MAT_MVP] = mesh->parent->mvp;
	matrix_array[UNIFORM_MAT_MODEL] = mesh->parent->matrix;
	matrix_array[UNIFORM_MAT_VIEW] = view->view;
	matrix_array[UNIFORM_MAT_PROJECTION] = view->projection;

	vector_array[UNIFORM_VEC_VIEW_POSITION] = view->view_position;
	vector_array[UNIFORM_VEC_TIME] = get_shader_time();
	vector_array[UNIFORM_VEC_COLOUR] = view->ambient_light;

	uint16_t width, height;
	mylly_get_resolution(&width, &height);
	vector_array[UNIFORM_VEC_SCREEN] = vec4(width, height, 0, 0);

	sampler_array[UNIFORM_SAMPLER_MAIN] = 0;
	sampler_array[UNIFORM_SAMPLER_NORMAL] = 1;

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

const char *rend_get_default_shader_source(void)
{
	return default_shader_source;
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

static bool rend_create_framebuffers(void)
{
	// Generate a screen sized texture for the frame buffer.
	uint16_t screen_width, screen_height;
	mylly_get_resolution(&screen_width, &screen_height);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(2, framebuffer_textures);

	for (int i = 0; i < 2; i++) {

		glBindTexture(GL_TEXTURE_2D, framebuffer_textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	// Create a depth buffer.
	glGenRenderbuffers(2, depth_buffers);

	for (int i = 0; i < 2; i++) {	

		glBindRenderbuffer(GL_RENDERBUFFER, depth_buffers[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, screen_width, screen_height);
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	// Create a framebuffer and attach the texture and depth buffer to it.
	glGenFramebuffers(2, framebuffers);

	for (int i = 0; i < 2; i++) {

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_textures[i], 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffers[i]);

		// Check that the framebuffer was created successfully.
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (status != GL_FRAMEBUFFER_COMPLETE) {
			return false;
		}	
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Create a list of vertices covering the entire screen.
	vertex_ui_t vertices[] = {
		vertex_ui(vec2(-1, -1), vec2(0, 0), COL_WHITE),
		vertex_ui(vec2(1, -1),  vec2(1, 0), COL_WHITE),
		vertex_ui(vec2(-1, 1),  vec2(0, 1), COL_WHITE),
		vertex_ui(vec2(1, 1),   vec2(1, 1), COL_WHITE)
	};

	glGenBuffersARB(1, &framebuffer_vertices);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, framebuffer_vertices);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);

	return true;
}

static void rend_destroy_framebuffers(void)
{
	for (int i = 0; i < 2; i++) {

		glDeleteFramebuffers(1, &framebuffers[i]);
		glDeleteTextures(1, &framebuffer_textures[i]);
		glDeleteRenderbuffers(1, &depth_buffers[i]);
	}

	glDeleteBuffersARB(1, &framebuffer_vertices);
}

static void rend_draw_post_processing_effects(rview_t *view)
{
	// Update relevant uniform arrays.
	sampler_array[UNIFORM_SAMPLER_MAIN] = 0;
	sampler_array[UNIFORM_SAMPLER_NORMAL] = -1;

	vector_array[UNIFORM_VEC_VIEW_POSITION] = view->view_position;
	vector_array[UNIFORM_VEC_TIME] = get_shader_time();
	vector_array[UNIFORM_VEC_COLOUR] = view->ambient_light;

	uint16_t width, height;
	mylly_get_resolution(&width, &height);
	vector_array[UNIFORM_VEC_SCREEN] = vec4(width, height, 0, 0);
	

	// Apply each post processing effect in order.
	for (uint32_t i = 0, c = view->post_processing_effects.count; i < c; i++) {

		if (i < c - 1) {

			// Bind the framebuffer not used by the previous render pass.
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffers[1 + i & 1]);
		}
		else {
			// Last effect, bind back to the screen's framebuffer.
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		
		// Render the contents of the previous framebuffer into the next one (or the
		// screen if this is the last effect).
		shader_t *effect = view->post_processing_effects.items[i];
		rend_draw_framebuffer_with_shader(effect, i & 1);
	}
}

static void rend_draw_framebuffer_with_shader(shader_t *shader, int fb_index)
{
	// Switch to the post-processing shader.
	glUseProgram(shader->program);
	active_shader = shader->program;

	// Select the framebuffer texture.
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, framebuffer_textures[fb_index]);
	active_texture = framebuffer_textures[fb_index];

	// Update custom uniforms.
	if (shader->has_updated_uniforms) {
		rend_update_material_uniforms(shader);
	}

	// Disable all vertex attributes.
	for (int i = 0; i < NUM_SHADER_ATTRIBUTES; i++) {
		glDisableVertexAttribArray(i);
	}

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
	
	// Draw the framebuffer's contents into a screen sized quad.
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, framebuffer_vertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
