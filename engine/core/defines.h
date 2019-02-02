#pragma once
#ifndef __DEFINES_H
#define __DEFINES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include "core/types.h"

#ifdef _WIN32
#define INLINE __inline
#else
#define INLINE inline
#endif

#define UNUSED(x) (void)x;
#define UNUSED_RETURN(x) if (x);

#define LENGTH(x) (sizeof(x) / sizeof(x[0]))

#ifdef __cplusplus
#define BEGIN_DECLARATIONS extern "C" {
#define END_DECLARATIONS }
#else
#define BEGIN_DECLARATIONS
#define END_DECLARATIONS
#endif

#endif
