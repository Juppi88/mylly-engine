#include "shader.h"
#include "renderer.h"
#include "core/string.h"
#include "io/log.h"

// --------------------------------------------------------------------------------

static bool shader_create_program(shader_t *shader, const char *src);
static void shader_destroy_program(shader_t *shader);

// --------------------------------------------------------------------------------

// Shader global variable names, used for getting the position of the variable in a shader.
static const char *shader_global_names[NUM_SHADER_GLOBALS] = {
	"MatrixModel",
	"MatrixMVP"
};

// --------------------------------------------------------------------------------

shader_t * shader_create(const char *name, const char *source)
{
	NEW(shader_t, shader);

	shader->name = string_duplicate(name);

	shader->vertex = 0;
	shader->fragment = 0;
	shader->program = 0;

	for (uint32_t i = 0; i < NUM_SHADER_GLOBALS; ++i) {
		shader->globals[i] = -1;
	}

	// Compile the shader.
	if (shader_create_program(shader, source)) {
		log_message("Renderer", "Loaded shader '%s'.", name);
	}

	return shader;
}

void shader_destroy(shader_t *shader)
{
	// Destroy the GPU objects.
	shader_destroy_program(shader);

	DELETE(shader->name);
	DELETE(shader);
}

static bool shader_create_program(shader_t *shader, const char *src)
{
	if (shader == NULL) {
		return false;
	}

	// Destroy previously created program first.
	shader_destroy_program(shader);

	const char *log;

	// Compile the vertex shader.
	shader->vertex = rend_create_shader(SHADER_VERTEX, src, &log);

	if (shader->vertex == 0) {

		log_note("Renderer", "Failed to compile vertex shader '%s':\n%s", shader->name, log);
		return false;
	}

	// Compile the fragment shader.
	shader->fragment = rend_create_shader(SHADER_FRAGMENT, src, &log);

	if (shader->fragment == 0) {

		log_note("Renderer", "Failed to compile fragment shader '%s':\n%s", shader->name, log);
		return false;
	}

	// Link the shader objects into a shader program.
	shader_object_t shaders[2] = { shader->vertex, shader->fragment };

	shader->program = rend_create_shader_program(shaders, 2);

	if (shader->program == 0) {

		log_note("Renderer", "Failed to link shader program '%s'", shader->name);
		return false;
	}

	// Get and cache shader uniform locations.
	for (uint32_t i = 0; i < NUM_SHADER_GLOBALS; ++i) {
		shader->globals[i] = rend_get_program_uniform_location(shader->program, shader_global_names[i]);
	}

	return true;
}

static void shader_destroy_program(shader_t *shader)
{
	if (shader == NULL) {
		return;
	}

	if (shader->vertex != 0) {

		rend_destroy_shader(shader->vertex);
		shader->vertex = 0;
	}

	if (shader->fragment != 0) {

		rend_destroy_shader(shader->fragment);
		shader->fragment = 0;
	}

	if (shader->program != 0) {

		rend_destroy_shader(shader->program);
		shader->program = 0;
	}
}
