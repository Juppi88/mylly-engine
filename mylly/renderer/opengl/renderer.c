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

// --------------------------------------------------------------------------------

// The source code for a default GLSL shader which renders everything in purple.
// Used when no valid shaders are available.
static const char *default_shader_source =

"uniform mat4 MatrixMVP;\n"
"\n"
"#if defined(VERTEX_SHADER)\n"
"\n"
"attribute vec3 Vertex;\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = MatrixMVP * vec4(Vertex, 1.0);\n"
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

// --------------------------------------------------------------------------------

static void rend_draw_mesh(rview_t *view, rmesh_t *mesh);

static void rend_set_active_material(shader_t *shader, texture_t *texture, rview_t *view,
                                     robject_t *parent, vertex_type_t vertex_type);

static bool rend_bind_shader_attribute(shader_t *shader, int attr_type, GLint size, GLenum type,
                                       GLboolean normalized, GLsizei stride, const GLvoid *pointer);

// --------------------------------------------------------------------------------

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

	glEnable(GL_DEPTH_TEST);

	return true;
}

void rend_shutdown(void)
{
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

			list_foreach(view->meshes[queue], mesh) {

				rend_draw_mesh(view, mesh);
				meshes_drawn++;
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
	rend_set_active_material(mesh->shader, mesh->texture, view, mesh->parent, mesh->vertex_type);

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

static void rend_set_active_material(shader_t *shader, texture_t *texture, rview_t *view,
                                     robject_t *parent, vertex_type_t vertex_type)
{
	if (shader == NULL || parent == NULL) {
		return;
	}
	
	// Select the active shader.
	GLuint shader_id = shader->program;

	if (shader_id != active_shader) {

		glUseProgram(shader_id);
		active_shader = shader_id;
	}

	// Select the active texture.
	GLuint texture_id = (texture != NULL ? texture->gpu_texture : 0);

	if (texture_id != active_texture) {

		glBindTexture(GL_TEXTURE_2D, texture_id);
		active_texture = texture_id;
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
	vec4_t vector;
	uint16_t width, height;

	for (uint32_t i = 0, c = shader->uniforms.count; i < c; i++) {

		int position = shader->uniforms.items[i].position;
		const shader_uniform_t *uniform = shader->uniforms.items[i].uniform;

		switch (uniform->index) {

			case UNIFORM_MODEL_MATRIX:
				glUniformMatrix4fv(position, 1, GL_FALSE, mat_as_ptr(parent->matrix));
				break;

			case UNIFORM_VIEW_MATRIX:
				glUniformMatrix4fv(position, 1, GL_FALSE, mat_as_ptr(view->view));
				break;

			case UNIFORM_PROJECTION_MATRIX:
				glUniformMatrix4fv(position, 1, GL_FALSE, mat_as_ptr(view->projection));
				break;

			case UNIFORM_MVP_MATRIX:
				glUniformMatrix4fv(position, 1, GL_FALSE, mat_as_ptr(parent->mvp));
				break;

			case UNIFORM_VIEW_POSITION:
				glUniform4fv(position, 1, (const GLfloat *)&view->view_position);
				break;

			case UNIFORM_TEXTURE:
				glUniform1i(position, 0);
				break;

			case UNIFORM_TIME:
				vector = get_shader_time();
				glUniform4fv(position, 1, (const GLfloat *)&vector);
				break;

			case UNIFORM_SCREEN:
				mylly_get_resolution(&width, &height);
				vector = vec4(width, height, 0, 0);

				glUniform4fv(position, 1, (const GLfloat *)&vector);
				break;

			default:
				// TODO: Add custom material uniforms here
				break;
		}
	}
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

texture_name_t rend_generate_texture(void *image, size_t width, size_t height, texture_format_t fmt)
{
	// Generate a texture name.
	GLuint texture;
	glGenTextures(1, &texture);

	// Upload the texture to the GPU.
	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, texture);

	switch (fmt) {

		case TEX_FORMAT_GRAYSCALE:

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			// Convert grayscale textures into 4-channel texture where the only colour is red.
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0,
                         GL_RED, GL_UNSIGNED_BYTE, image);

			break;

		default:

			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			// Assume 32bit RGBA texture by default.
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, image);

			break;
	}

	return texture;
}

void rend_delete_texture(texture_name_t texture)
{
	glDeleteTextures(1, &texture);
}
