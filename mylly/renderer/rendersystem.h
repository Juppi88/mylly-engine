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
void rsys_end_frame(scene_t *scene);

// Creates all render views for a single scene and adds them to the list of items to be rendered
// during this frame.
void rsys_render_scene(scene_t *scene);

// Report a mesh to be rendered.
void rsys_render_mesh(mesh_t *mesh, bool is_ui_mesh);

END_DECLARATIONS;

#endif
