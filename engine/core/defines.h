#pragma once
#ifndef __DEFINES_H
#define __DEFINES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef _WIN32
#define INLINE __inline
#else
#define INLINE inline
#endif

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)

#define CLAMP(x, min, max) (x < min ? min : (x > max ? max : x))

#define UNUSED_RETURN(x) if (x);

#endif
