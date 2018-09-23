#include "renderer/renderer.h"
#include "extensions.h"
#include "renderer/vertex.h"
#include "io/log.h"
#include "platform/window.h"
#include <stdio.h>

#ifdef _WIN32
#error "Missing implementation"
#else
static GLXContext context;
#endif

static GLuint active_shader;

// --------------------------------------------------------------------------------

// The source code for a default GLSL shader which renders everything in purple.
// Used when no valid shaders are available.
static const char *default_shader_source =
"uniform mat4 MatrixModel;\n"
"uniform mat4 MatrixMVP;\n"
"\n"
"#if defined(VERTEX_SHADER)\n"
"void main()\n"
"{\n"
"	gl_Position = MatrixMVP * gl_Vertex;\n"
"}\n"
"#elif defined(FRAGMENT_SHADER)\n"
"void main()\n"
"{\n"
"	gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);\n"
"}\n"
"#endif\n";

// --------------------------------------------------------------------------------

static void rend_begin_draw(void);
static void rend_end_draw(void);

// --------------------------------------------------------------------------------

bool rend_initialize(void)
{
	// Create an OpenGL rendering context.	
#ifdef _WIN32
#error "Missing implementation"
#else
	context = glXCreateContext(window_get_display(), window_get_visual_info(), NULL, GL_TRUE);
 
	if (context == NULL) {
		log_error("Renderer", "Could not create an OpenGL rendering context");
		return false;
	}

	glXMakeCurrent(window_get_display(), window_get_handle(), context);
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
#error "Missing implementation"
#else
	glXMakeCurrent(window_get_display(), None, NULL);
	glXDestroyContext(window_get_display(), context);

	context = NULL;
#endif
}

void rend_draw_views(LIST(rview_t) views)
{
	rend_begin_draw();

	glEnableClientState(GL_VERTEX_ARRAY);

	LIST_FOREACH(rview_t, view, views) {

		LIST_FOREACH(rmesh_t, mesh, view->meshes) {

			// Select the active shader.
			GLuint shader = mesh->shader->program;

			if (shader != active_shader) {

				glUseProgram(shader);
				active_shader = shader;
			}

			/// Bind the meshes vertex buffer and set vertex data pointers.
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, mesh->vertices->vbo);

			// Set vertex data offsets in the buffer.
			glVertexPointer(3, GL_FLOAT, sizeof(vertex_t), (void *)offsetof(vertex_t, pos));
			glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(vertex_t), (void *)offsetof(vertex_t, colour));
			glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), (void *)offsetof(vertex_t, uv));
			glNormalPointer(GL_FLOAT, sizeof(vertex_t), (void *)offsetof(vertex_t, normal));

			// Set per-draw shader globals.
			if (shader_uses_global(mesh->shader, GLOBAL_MODEL_MATRIX)) {

				glUniformMatrix4fv(
					shader_get_global_position(mesh->shader, GLOBAL_MODEL_MATRIX),
					1, GL_FALSE, mat_as_ptr(mesh->parent->matrix)
				);
			}

			if (shader_uses_global(mesh->shader, GLOBAL_MODEL_MVP)) {

				glUniformMatrix4fv(
					shader_get_global_position(mesh->shader, GLOBAL_MODEL_MVP),
					1, GL_FALSE, mat_as_ptr(mesh->parent->mvp)
				);
			}

			// Draw the triangles of the mesh.
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, mesh->indices->vbo);
			glDrawElements(GL_TRIANGLES, mesh->indices->count, GL_UNSIGNED_SHORT, 0);
		}
	}

	glDisableClientState(GL_VERTEX_ARRAY);

	rend_end_draw();
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

void rend_upload_buffer_data(vbindex_t vbo, void *data, size_t size, bool is_index_data)
{
	if (is_index_data) {
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, vbo);
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, size, data, GL_STATIC_DRAW_ARB);
	}
	else {
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, size, data, GL_STATIC_DRAW_ARB);
	}
}

shader_object_t rend_create_shader(SHADER_TYPE type, const char *source, const char **compiler_log)
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

	const char *src[2];
	src[0] = defines;
	src[1] = source;

	// Compile the source code with the defines set above.
	glShaderSource(shader, 2, src, NULL);
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

const char *rend_get_default_shader_source(void)
{
	return default_shader_source;
}

static void rend_begin_draw(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glAlphaFunc(GL_GREATER, 1.0f);
	glEnable(GL_BLEND);

	// Setup the camera.
	// TODO: Get this from frame data and move it to drawframe!
//	glViewport(0, 0, 640, 480);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	/*float aspect = 480.0f / 640.0f;
	float width = 3.0f;

	glOrtho(-width * aspect, width * aspect, -width, width, 1, -1);
*/
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	//glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

}

static void rend_end_draw(void)
{
#ifdef _WIN32
#error "Missing implementation"
#else
	glXSwapBuffers(window_get_display(), window_get_handle());
#endif
}
