#pragma once
#ifndef __RENDERSYSTEM_H
#define __RENDERSYSTEM_H

#include "core/defines.h"

// -------------------------------------------------------------------------------------------------

typedef enum rendermode_t {

	RENDMODE_FORWARD,
	RENDMODE_DEFERRED,

} rendermode_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

void rsys_initialize(void);
void rsys_shutdown(void);

void rsys_begin_frame(void);
void rsys_end_frame(scene_t *scene);

// Creates all render views for a single scene and adds them to the list of items to be rendered
// during this frame.
void rsys_render_scene(scene_t *scene);

// Report a mesh to be rendered.
void rsys_render_mesh(mesh_t *mesh, bool is_ui_mesh);

void rsys_set_render_mode(rendermode_t mode);

END_DECLARATIONS;

#endif
