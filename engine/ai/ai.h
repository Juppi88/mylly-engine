#pragma once
#ifndef __AI_H
#define __AI_H

#include "core/defines.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef struct ai_t {

	object_t *parent; // Parent scene object for the AI
	ai_behaviour_t *behaviour; // Behaviour tree run by the AI

} ai_t;

// -------------------------------------------------------------------------------------------------

ai_t *ai_create(object_t *parent);
void ai_destroy(ai_t *ai);
void ai_process(ai_t *ai);

END_DECLARATIONS;

#endif
