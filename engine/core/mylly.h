#pragma once
#ifndef __MYLLY_H
#define __MYLLY_H

#include "core/defines.h"

// --------------------------------------------------------------------------------

typedef struct scene_t scene_t;

// --------------------------------------------------------------------------------

typedef void (*on_loop_t)(void);

bool mylly_initialize(int argc, char **argv);
void mylly_main_loop(on_loop_t callback);

void mylly_set_scene(scene_t *scene);

void mylly_exit(void);

#endif
