#pragma once
#ifndef __AUDIO_BUFFER_H
#define __AUDIO_BUFFER_H

#include "core/defines.h"
#include "core/ref.h"

// -------------------------------------------------------------------------------------------------

typedef uint32_t audiobuffer_id_t; // Audio buffer index used by OpenAL

// -------------------------------------------------------------------------------------------------

typedef struct audiobuffer_t {

	ref_counted();
	audiobuffer_id_t buffer;

} audiobuffer_t;

// -------------------------------------------------------------------------------------------------

BEGIN_DECLARATIONS;

audiobuffer_t *audiobuffer_create(void);

END_DECLARATIONS;

#endif
