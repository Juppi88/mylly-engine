#pragma once
#ifndef __RENDERSYSTEM_H
#define __RENDERSYSTEM_H

#include "core/defines.h"

BEGIN_DECLARATIONS;

typedef struct scene_t scene_t;

// --------------------------------------------------------------------------------

void rsys_initialize(void);
void rsys_shutdown(void);

void rsys_begin_frame(void);
void rsys_end_frame(void);

// Creates all render views for a single scene and adds them to the list of items to be rendered
// during this frame.
void rsys_render_scene(scene_t *scene);

// Report a UI mesh to be rendered during this frame.
void rsys_render_ui_mesh(mesh_t *mesh);

END_DECLARATIONS;

#endif
