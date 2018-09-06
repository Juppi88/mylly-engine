#pragma once
#ifndef __MYLLY_H
#define __MYLLY_H

#include "core/defines.h"

typedef void (*on_loop_t)(void);

bool mylly_initialize(int argc, char **argv);
void mylly_main_loop(on_loop_t callback);

#endif