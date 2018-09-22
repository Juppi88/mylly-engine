#pragma once
#ifndef __DEFINES_H
#define __DEFINES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#ifdef _WIN32
#define INLINE __inline
#else
#define INLINE inline
#endif

#define UNUSED_RETURN(x) if (x);

#define LENGTH(x) (sizeof(x) / sizeof(x[0]))

#endif
