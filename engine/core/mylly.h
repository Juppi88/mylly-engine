#pragma once
#ifndef __MYLLY_H
#define __MYLLY_H

#include "core/defines.h"

// --------------------------------------------------------------------------------

typedef struct scene_t scene_t;

// --------------------------------------------------------------------------------

typedef void (*on_loop_t)(void);

BEGIN_DECLARATIONS;

bool mylly_initialize(int argc, char **argv);
void mylly_main_loop(on_loop_t callback);

void mylly_set_scene(scene_t *scene);

void mylly_exit(void);

void mylly_get_resolution(uint16_t *width, uint16_t *height);

END_DECLARATIONS;

#endif
