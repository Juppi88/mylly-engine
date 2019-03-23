#pragma once
#ifndef __AI_BEHAVIOUR_H
#define __AI_BEHAVIOUR_H

#include "core/defines.h"

BEGIN_DECLARATIONS;

// -------------------------------------------------------------------------------------------------

typedef struct ai_behaviour_t {

	ai_t *ai; // AI this behaviour tree is running under, or NULL if this tree is a template
	ai_node_t *root; // Root node for the tree

} ai_behaviour_t;

// -------------------------------------------------------------------------------------------------

ai_behaviour_t *ai_behaviour_create(ai_t *ai);
void ai_behaviour_destroy(ai_behaviour_t *tree);
void ai_behaviour_process(ai_behaviour_t *tree);

END_DECLARATIONS;

#endif
