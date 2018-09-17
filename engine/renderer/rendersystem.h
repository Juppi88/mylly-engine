#pragma once
#ifndef __RENDERSYSTEM_H
#define __RENDERSYSTEM_H

#include "scene/object.h"

void rsys_initialize(void);
void rsys_shutdown(void);

void rsys_begin_frame(void);
void rsys_end_frame(void);

// Creates all render views for a single scene and adds them to the list of items to be rendered
// during this frame.
void rsys_render_scene(object_t *object);

#endif
