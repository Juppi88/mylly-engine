#pragma once
#ifndef __MYLLY_H
#define __MYLLY_H

#include "core/defines.h"

// -------------------------------------------------------------------------------------------------

// Engine initialization parameters

typedef struct mylly_params_t {

	struct {

		void (*on_loop)(void); // Method which is called every frame
		void (*on_exit)(void); // Called when the game is about to exit

	} callbacks;

	struct {

		char logo_path[260]; // Path to splash screen logo .png file
		uint8_t r, g, b; // Splash screen background colour

	} splash;

} mylly_params_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

bool mylly_initialize(int argc, char **argv, const mylly_params_t *params);
void mylly_main_loop(void);

void mylly_set_scene(scene_t *scene);

void mylly_exit(void);

void mylly_get_resolution(uint16_t *width, uint16_t *height);

END_DECLARATIONS;

#endif
