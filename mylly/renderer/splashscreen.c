#include "splashscreen.h"
#include "texture.h"
#include "shader.h"
#include "renderer.h"
#include "core/string.h"
#include "io/file.h"
#include "platform/thread.h"
#include "platform/timer.h"

// -------------------------------------------------------------------------------------------------

static texture_t *logo = NULL; // Logo texture
static shader_t *shader = NULL; // Shader which is used to render the splash screen logo
static colour_t colour = { .r = 0, .g = 0, .b = 0, .a = 255 }; // Splash screen background colour
static uint64_t display_time = 0; // Time when the splash screen is displayed.

static const int SPLASH_MIN_DURATION = 500;
static const int SPLASH_FADE_TIME = 500;

// -------------------------------------------------------------------------------------------------

void splash_display(const char *logo_path, colour_t background_colour)
{
	colour = background_colour;
	colour.a = 255;

	// Load the logo texture from a .png file.
	if (!string_is_null_or_empty(logo_path)) {

		void *buffer;
		size_t length;

		if (file_read_all_data(logo_path, &buffer, &length)) {
			
			logo = texture_create("splash_logo", logo_path);

			if (!texture_load_png(logo, buffer, length, TEX_FILTER_BILINEAR)) {

				// Destroy the texture in case the file could not be loaded.
				texture_destroy(logo);
				logo = NULL;
			}
		}
	}

	// Instantiate a shader program for rendering the splash screen.
	const char *shader_source[] = {
		NULL, // Leave an empty line for shader defines.
		rend_get_default_shader_source(DEFAULT_SHADER_SPLASHSCREEN)
	};

	const char *uniforms[] = { "Texture", "Colour" };
	UNIFORM_TYPE uniform_types[] = { UNIFORM_TYPE_INT, UNIFORM_TYPE_VECTOR4 };

	shader = shader_create("splash", NULL);
	shader_load_from_source(shader, 2, shader_source, 2, uniforms, uniform_types);

	// Draw the splash screen once at full alpha before loading the rest of the engine
	// behind the scenes.
	rend_draw_splash_screen(logo, shader, background_colour);

	// Store the current timestamp so we can enforce the splash screen to be displayed a minimum
	// amount of time.
	display_time = timer_get_ticks();
}

void splash_fade_out(void)
{
	// Display the splash logo for a minimum period of time.
	float duration = timer_get_ticks() - display_time;

	if (duration < SPLASH_MIN_DURATION) {
		thread_sleep(SPLASH_MIN_DURATION - duration);
	}

	// Fade out the logo smoothly over a small period of time.
	timeout_t timer = 0;

	do {
		float t = 1.0f - timer_get_factor(&timer, SPLASH_FADE_TIME);
		colour.a = (uint8_t)(255 * t);

		rend_draw_splash_screen(logo, shader, colour);
		thread_sleep(1);

	} while (!timer_has_expired(&timer, SPLASH_FADE_TIME));

	// Destroy splash screen resources as they're no longer needed.
	if (logo != NULL) {

		texture_destroy(logo);
		logo = NULL;
	}

	if (shader != NULL) {

		shader_destroy(shader);
		shader = NULL;
	}

	// A brief delay before resuming normal operation.
	thread_sleep(200);
}
