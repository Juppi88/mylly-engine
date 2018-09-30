#include "shader.h"
#include "renderer.h"
#include "core/string.h"
#include "io/log.h"

// --------------------------------------------------------------------------------

static void shader_destroy_program(shader_t *shader);

// --------------------------------------------------------------------------------

// Shader global variable names, used for getting the position of the variable in a shader.
static const char *shader_global_names[NUM_SHADER_GLOBALS] = {
	"MatrixModel",
	"MatrixMVP",
	"Texture",
	"Time"
};

// Shader vertex attribute names.
static const char *shader_attribute_names[NUM_SHADER_ATTRIBUTES] = {
	"Vertex",
	"Normal",
	"TexCoord",
	"Colour"
};

// --------------------------------------------------------------------------------

shader_t * shader_create(const char *name, const char *path)
{
	NEW(shader_t, shader);

	shader->resource.name = string_duplicate(name);

	if (path != NULL) {
		shader->resource.path = string_duplicate(path);
	}

	shader->vertex = 0;
	shader->fragment = 0;
	shader->program = 0;

	for (uint32_t i = 0; i < NUM_SHADER_GLOBALS; ++i) {
		shader->globals[i] = -1;
	}

	return shader;
}

void shader_destroy(shader_t *shader)
{
	// Destroy the GPU objects.
	shader_destroy_program(shader);

	DELETE(shader->resource.name);
	DELETE(shader->resource.path);
	DELETE(shader);
}

bool shader_load_from_source(shader_t *shader, const char *source)
{
	if (shader == NULL) {
		return false;
	}

	// Destroy previously created program first.
	shader_destroy_program(shader);

	const char *log;

	// Compile the vertex shader.
	shader->vertex = rend_create_shader(SHADER_VERTEX, source, &log);

	if (shader->vertex == 0) {

		log_note("Renderer", "Failed to compile vertex shader '%s':\n%s", shader->name, log);
		return false;
	}

	// Compile the fragment shader.
	shader->fragment = rend_create_shader(SHADER_FRAGMENT, source, &log);

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

		int global = rend_get_program_uniform_location(
			shader->program,
			shader_global_names[i]
		);

		shader->globals[i] = global;
	}

	// Get and cache vertex attribute indices.
	for (uint32_t i = 0; i < NUM_SHADER_ATTRIBUTES; ++i) {

		int attribute = rend_get_program_program_attribute_location(
			shader->program,
			shader_attribute_names[i]
		);

		shader->attributes[i] = attribute;
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
